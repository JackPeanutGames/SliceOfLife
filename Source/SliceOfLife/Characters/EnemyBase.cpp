#include "EnemyBase.h"
#include "SliceOfLife/Components/HealthComponent.h"
#include "SliceOfLife/Components/CombatComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "PlayerCharacter.h"
#include "Navigation/PathFollowingComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "SliceOfLife/AI/EnemyAIController.h"
#include "UObject/ConstructorHelpers.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	

    // Capsule to roughly match 2m height
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->InitCapsuleSize(40.0f, 100.0f); // radius, half-height
    }

    // Configure AI Perception (Sight)
    if (SightConfig)
    {
        SightConfig->SightRadius = 800.0f;
        SightConfig->LoseSightRadius = 900.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    }
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->ConfigureSense(*SightConfig);
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AEnemyBase::OnPerceptionUpdated);
    }

	// Initialize state
	CurrentState = EEnemyState::Idle;
	CurrentTarget = nullptr;
	StateTimer = 0.0f;
	PatrolCenter = FVector::ZeroVector;
	CurrentPatrolTarget = FVector::ZeroVector;

	// Set default stats
	EnemyStats = FEnemyStats();

    // Create combat component
    CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

    // Default to 2.5D constraint at construction time
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->bConstrainToPlane = true;
        Move->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f));
        Move->SetPlaneConstraintOrigin(FVector::ZeroVector);
        Move->SetPlaneConstraintEnabled(true);
    }

    // Ensure AI controller is used when placed/spawned
    AIControllerClass = AEnemyAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
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

    // Perception binding is handled in constructor

	// Start patrolling
	StartPatrolling();

    // Ensure plane constraint is applied according to toggle
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->bConstrainToPlane = bConstrainToYPlane;
        Move->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f));
        Move->SetPlaneConstraintOrigin(FVector::ZeroVector);
        Move->SetPlaneConstraintEnabled(bConstrainToYPlane);
    }

    // If AIController is already possessing, ensure BT starts (safety in case OnPossess ran before asset assignment)
    if (AEnemyAIController* AI = Cast<AEnemyAIController>(GetController()))
    {
        if (GetBlackboardData() && AI->GetBlackboardComponent())
        {
            AI->GetBlackboardComponent()->InitializeBlackboard(*const_cast<UBlackboardData*>(GetBlackboardData()));
        }
        if (GetBehaviorTree())
        {
            AI->RunBehaviorTree(const_cast<UBehaviorTree*>(GetBehaviorTree()));
        }
    }
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateAI(DeltaTime);

    // Debug draw hurt box (capsule)
    // Uses console variable: SliceOfLife.ShowHitboxes (0/1)
    static const auto CVarShowHitboxes = IConsoleManager::Get().FindConsoleVariable(TEXT("SliceOfLife.ShowHitboxes"));
    const bool bShow = CVarShowHitboxes ? (CVarShowHitboxes->GetInt() != 0) : false;
    if (bShow)
    {
        if (UCapsuleComponent* Capsule = GetCapsuleComponent())
        {
            const FVector Location = Capsule->GetComponentLocation();
            const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
            const float Radius = Capsule->GetScaledCapsuleRadius();
            DrawDebugCapsule(GetWorld(), Location, HalfHeight, Radius, FQuat::Identity, FColor::Green, false, 0.f, 0, 1.5f);
        }
    }
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
    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        FAIMoveRequest Req;
        Req.SetGoalLocation(Location);
        Req.SetAcceptanceRadius(AcceptanceRadius);
        AI->MoveTo(Req);
    }
}

void AEnemyBase::StopMovement()
{
    if (AAIController* AI = Cast<AAIController>(GetController()))
    {
        AI->StopMovement();
    }
}

void AEnemyBase::PerformAttack()
{
    UE_LOG(LogTemp, Log, TEXT("Enemy %s performing attack"), *GetName());

    // Play assigned attack montage if available
    if (AttackMontage && GetMesh())
    {
        if (UAnimInstance* AnimInst = GetMesh()->GetAnimInstance())
        {
            FOnMontageEnded MontageEnded;
            MontageEnded.BindLambda([this](UAnimMontage* Montage, bool bInterrupted)
            {
                // After attack finishes, resume chasing if target valid
                if (CurrentTarget && IsValid(CurrentTarget))
                {
                    SetEnemyState(EEnemyState::Chasing);
                }
                else
                {
                    SetEnemyState(EEnemyState::Patrolling);
                }
            });
            AnimInst->Montage_Play(AttackMontage, 1.0f);
            AnimInst->Montage_SetEndDelegate(MontageEnded, AttackMontage);
        }
    }
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

    // Update blackboard with the new patrol location if available
    if (AEnemyAIController* AI = Cast<AEnemyAIController>(GetController()))
    {
        if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
        {
            BB->SetValueAsVector(AEnemyAIController::PatrolLocationKey, CurrentPatrolTarget);
        }
    }
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

void AEnemyBase::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    // Find best perceived player target
    AActor* BestPlayer = nullptr;
    float BestDistSq = FLT_MAX;
    for (AActor* Actor : UpdatedActors)
    {
        if (!IsValid(Actor)) continue;
        if (APlayerCharacter* Player = Cast<APlayerCharacter>(Actor))
        {
            const float DistSq = FVector::DistSquared(Player->GetActorLocation(), GetActorLocation());
            if (DistSq < BestDistSq)
            {
                BestDistSq = DistSq;
                BestPlayer = Player;
            }
        }
    }
    if (BestPlayer)
    {
        StartChasing(BestPlayer);
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
