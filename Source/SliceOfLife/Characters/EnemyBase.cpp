#include "EnemyBase.h"
#include "SliceOfLife/Components/HealthComponent.h"
#include "SliceOfLife/Components/CombatComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "PlayerCharacter.h"
#include "Navigation/PathFollowingComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));

    // Visual placeholders
    PlaceholderBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderBody"));
    PlaceholderHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderHead"));
    PlaceholderBody->SetupAttachment(RootComponent);
    PlaceholderHead->SetupAttachment(PlaceholderBody);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereFinder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (SphereFinder.Succeeded())
    {
        PlaceholderBody->SetStaticMesh(SphereFinder.Object);
        PlaceholderHead->SetStaticMesh(SphereFinder.Object);
        // 2m tall: main body sphere of 150cm diameter with a 50cm head above to total ~200cm visible
        PlaceholderBody->SetWorldScale3D(FVector(1.5f));   // 150cm diameter
        PlaceholderBody->SetRelativeLocation(FVector(0, 0, 75.0f));
        PlaceholderHead->SetWorldScale3D(FVector(0.5f));   // 50cm diameter
        // Place head so top reaches 200cm: body center 75 + head center offset 100 = 175; 175 + 25 radius = 200 top
        PlaceholderHead->SetRelativeLocation(FVector(0, 0, 100.0f));
        PlaceholderBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PlaceholderHead->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        PlaceholderBody->SetGenerateOverlapEvents(false);
        PlaceholderHead->SetGenerateOverlapEvents(false);
    }

    // Capsule to roughly match 2m height
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->InitCapsuleSize(40.0f, 100.0f); // radius, half-height
    }

	// Setup pawn sensing
	PawnSensingComponent->SightRadius = 800.0f;
	PawnSensingComponent->SetPeripheralVisionAngle(90.0f);
	PawnSensingComponent->bOnlySensePlayers = true;

	// Initialize state
	CurrentState = EEnemyState::Idle;
	CurrentTarget = nullptr;
	StateTimer = 0.0f;
	PatrolCenter = FVector::ZeroVector;
	CurrentPatrolTarget = FVector::ZeroVector;

	// Set default stats
	EnemyStats = FEnemyStats();
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	// Set patrol center to current location
	PatrolCenter = GetActorLocation();
	FindNewPatrolTarget();

	// Bind events
	if (HealthComponent)
	{
		HealthComponent->OnDamageReceived.AddDynamic(this, &AEnemyBase::OnDamageReceived);
	}

	if (PawnSensingComponent)
	{
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemyBase::OnPawnSeen);
	}

	// Start patrolling
	StartPatrolling();
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateAI(DeltaTime);
}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    const float Actual = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (HealthComponent && Actual > 0.f)
    {
        FVector KnockDir = (DamageCauser ? (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal() : GetActorForwardVector());
        const float KnockForce = 600.f;
        HealthComponent->TakeDamage(Actual, KnockDir, KnockForce, DamageCauser);
        OnDamageReceived(Actual, KnockDir, KnockForce);
    }
    return Actual;
}

void AEnemyBase::SetEnemyState(EEnemyState NewState)
{
	if (CurrentState != NewState)
	{
		EEnemyState OldState = CurrentState;
		CurrentState = NewState;
		StateTimer = 0.0f;

		// Call blueprint event
		OnEnemyStateChanged(OldState, NewState);

		UE_LOG(LogTemp, Log, TEXT("Enemy %s state changed from %d to %d"), *GetName(), (int32)OldState, (int32)NewState);
	}
}

void AEnemyBase::StartPatrolling()
{
	SetEnemyState(EEnemyState::Patrolling);
	FindNewPatrolTarget();
}

void AEnemyBase::StartChasing(AActor* Target)
{
	if (Target)
	{
		CurrentTarget = Target;
		SetEnemyState(EEnemyState::Chasing);
	}
}

void AEnemyBase::StartAttacking(AActor* Target)
{
	if (Target && CanAttack())
	{
		CurrentTarget = Target;
		SetEnemyState(EEnemyState::Attacking);
		PerformAttack();
	}
}

void AEnemyBase::StunEnemy(float Duration)
{
	SetEnemyState(EEnemyState::Stunned);
	StateTimer = Duration;
	StopMovement();
}

void AEnemyBase::MoveToLocation(FVector Location, float AcceptanceRadius)
{
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		// Simple movement towards location
		FVector Direction = (Location - GetActorLocation()).GetSafeNormal();
		FVector NewVelocity = Direction * GetMovementSpeed();
		
		// Only apply horizontal movement
		NewVelocity.Z = MovementComp->Velocity.Z;
		MovementComp->Velocity = NewVelocity;
	}
}

void AEnemyBase::StopMovement()
{
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->Velocity = FVector::ZeroVector;
	}
}

void AEnemyBase::PerformAttack()
{
	// This can be overridden in derived classes or implemented with behavior trees
	UE_LOG(LogTemp, Log, TEXT("Enemy %s performing attack"), *GetName());
}

bool AEnemyBase::CanAttack() const
{
	// Basic attack cooldown check
	return StateTimer <= 0.0f && CurrentState != EEnemyState::Stunned;
}

void AEnemyBase::UpdateAI(float DeltaTime)
{
	StateTimer -= DeltaTime;

	switch (CurrentState)
	{
	case EEnemyState::Patrolling:
		UpdatePatrol(DeltaTime);
		break;
	case EEnemyState::Chasing:
		UpdateChase(DeltaTime);
		break;
	case EEnemyState::Attacking:
		UpdateAttack(DeltaTime);
		break;
	case EEnemyState::Stunned:
		UpdateStun(DeltaTime);
		break;
	default:
		break;
	}
}

void AEnemyBase::UpdatePatrol(float DeltaTime)
{
	// Check if we've reached the patrol target
	if (FVector::Dist(GetActorLocation(), CurrentPatrolTarget) < 100.0f)
	{
		FindNewPatrolTarget();
	}

	// Move towards patrol target
	MoveToLocation(CurrentPatrolTarget);
}

void AEnemyBase::UpdateChase(float DeltaTime)
{
	if (!CurrentTarget || !IsValid(CurrentTarget))
	{
		StartPatrolling();
		return;
	}

	// Check if target is in attack range
	if (IsTargetInRange(CurrentTarget, EnemyStats.AttackRange))
	{
		StartAttacking(CurrentTarget);
		return;
	}

	// Check if target is still in detection range
	if (!IsTargetInRange(CurrentTarget, EnemyStats.DetectionRange))
	{
		StartPatrolling();
		return;
	}

	// Chase the target
	MoveToLocation(CurrentTarget->GetActorLocation());
}

void AEnemyBase::UpdateAttack(float DeltaTime)
{
	// Attack state is handled by PerformAttack
	// Return to chasing after attack
	if (StateTimer <= 0.0f)
	{
		if (CurrentTarget && IsValid(CurrentTarget))
		{
			StartChasing(CurrentTarget);
		}
		else
		{
			StartPatrolling();
		}
	}
}

void AEnemyBase::UpdateStun(float DeltaTime)
{
	if (StateTimer <= 0.0f)
	{
		StartPatrolling();
	}
}

void AEnemyBase::FindNewPatrolTarget()
{
	// Generate a random point within patrol radius
	float Angle = FMath::RandRange(0.0f, 2.0f * PI);
	float Distance = FMath::RandRange(100.0f, EnemyStats.PatrolRadius);
	
	FVector Offset = FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f);
	CurrentPatrolTarget = PatrolCenter + Offset;
}

bool AEnemyBase::IsTargetInRange(AActor* Target, float Range) const
{
	if (!Target || !IsValid(Target))
	{
		return false;
	}

	float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
	return Distance <= Range;
}

void AEnemyBase::OnPawnSeen(APawn* SeenPawn)
{
	// Check if the seen pawn is a player
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(SeenPawn))
	{
		// Start chasing the player
		StartChasing(Player);
	}
}

void AEnemyBase::OnDamageReceived(float Damage, FVector KnockbackDirection, float KnockbackForce)
{
	// Call blueprint event
	OnEnemyDamaged(Damage, nullptr);

	// Check if enemy is dead
	if (HealthComponent && !HealthComponent->IsAlive())
	{
		SetEnemyState(EEnemyState::Dead);
		OnEnemyDeath();
		return;
	}

	// Apply stun if not already stunned
	if (CurrentState != EEnemyState::Stunned)
	{
		StunEnemy(EnemyStats.StunDuration);
	}
}

float AEnemyBase::GetMovementSpeed() const
{
	switch (CurrentState)
	{
	case EEnemyState::Patrolling:
		return EnemyStats.PatrolSpeed;
	case EEnemyState::Chasing:
		return EnemyStats.ChaseSpeed;
	default:
		return EnemyStats.PatrolSpeed;
	}
}
