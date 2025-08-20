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
#include "HAL/IConsoleManager.h"
#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/DataTable.h"
#include "SliceOfLife/Items/ItemDropTypes.h"
#include "SliceOfLife/Items/ItemActor.h"
#include "SliceOfLife/Items/ItemDropActor.h"
#include "SliceOfLife/Weapons/WeaponBase.h"

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
        
        // Configure collision rules for the enemy root capsule
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Capsule->SetCollisionObjectType(ECC_WorldDynamic);
        Capsule->SetGenerateOverlapEvents(true);
        Capsule->SetCollisionResponseToAllChannels(ECR_Ignore);
        Capsule->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);    // stand on ground
        Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);          // block player
        Capsule->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap); // overlap weapons & drops
    }

    // Body part hit colliders
    HeadCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadCollider"));
    TorsoCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("TorsoCollider"));
    LegCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("LegCollider"));

    if (GetMesh())
    {
        HeadCollider->SetupAttachment(GetMesh());
        TorsoCollider->SetupAttachment(GetMesh());
        LegCollider->SetupAttachment(GetMesh());
    }
    // Apply designer-configurable hitbox settings
    HeadCollider->SetBoxExtent(HeadBoxExtent);
    HeadCollider->SetRelativeLocation(HeadRelativeLocation);

    TorsoCollider->SetBoxExtent(TorsoBoxExtent);
    TorsoCollider->SetRelativeLocation(TorsoRelativeLocation);

    LegCollider->SetBoxExtent(LegBoxExtent);
    LegCollider->SetRelativeLocation(LegRelativeLocation);

    // Configure body part hitboxes (overlap with weapon hitboxes, no blocking)
    auto ConfigureHitbox = [](UPrimitiveComponent* Comp)
    {
        if (!Comp) return;
        Comp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Comp->SetCollisionObjectType(ECC_WorldDynamic);
        Comp->SetGenerateOverlapEvents(true);
        Comp->SetCollisionResponseToAllChannels(ECR_Overlap);  // no blocking, only overlap
    };
    ConfigureHitbox(HeadCollider);
    ConfigureHitbox(TorsoCollider);
    ConfigureHitbox(LegCollider);

    HeadCollider->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::OnBodyPartOverlap);
    TorsoCollider->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::OnBodyPartOverlap);
    LegCollider->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::OnBodyPartOverlap);

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
	
	// Initialize remaining drops (randomize between 2-4 drops)
	RemainingDrops = FMath::RandRange(2, 4);

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

    // Spawn and attach weapon actor if weapon mesh is provided
    if (GetMesh() && WeaponMesh)
    {
        // Spawn a weapon actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        // Get the world and spawn the weapon
        if (UWorld* World = GetWorld())
        {
            // For now, we'll spawn a generic WeaponBase
            // TODO: Allow designers to specify weapon class in editor
            AWeaponBase* SpawnedWeapon = World->SpawnActor<AWeaponBase>(AWeaponBase::StaticClass(), GetActorTransform(), SpawnParams);
            if (SpawnedWeapon)
            {
                // Attach the weapon to the enemy
                SpawnedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocketName);
                
                // Set weapon properties
                SpawnedWeapon->SetAttackParams(20.0f, 300.0f); // Default enemy weapon damage
                
                UE_LOG(LogTemp, Log, TEXT("Enemy %s spawned weapon"), *GetName());
            }
        }
        
        // Keep the old mesh component for backward compatibility
        WeaponMeshComponent = NewObject<UStaticMeshComponent>(this, TEXT("WeaponMeshComponent"));
        if (WeaponMeshComponent)
        {
            WeaponMeshComponent->SetStaticMesh(WeaponMesh);
            WeaponMeshComponent->SetupAttachment(GetMesh(), WeaponSocketName);
            WeaponMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            WeaponMeshComponent->RegisterComponent();
        }
    }
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateAI(DeltaTime);

    // Debug draw hurt box (capsule) and body colliders
    static const auto CVarShowHitboxes = IConsoleManager::Get().FindConsoleVariable(TEXT("SliceOfLife.ShowHitboxes"));
    const bool bShow = CVarShowHitboxes ? (CVarShowHitboxes->GetInt() != 0) : false;
    if (bShow)
    {
        DebugDrawHurtCapsule();
        DebugDrawBodyColliders();
    }
}

void AEnemyBase::DebugDrawHurtCapsule()
{
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        const FVector Location = Capsule->GetComponentLocation();
        const float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();
        const float Radius = Capsule->GetScaledCapsuleRadius();
        DrawDebugCapsule(GetWorld(), Location, HalfHeight, Radius, FQuat::Identity, FColor::Green, false, 0.f, 0, 1.5f);
    }
}

void AEnemyBase::DebugDrawBodyColliders()
{
    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    const float FlashDuration = 0.2f;

    if (HeadCollider)
    {
        const bool bFlash = (Now - LastHeadHitTime) < FlashDuration;
        const FColor Color = bFlash ? FColor::Red : FColor::Green;
        const FVector BoxExtent = HeadCollider->GetScaledBoxExtent();
        const FTransform& CompToWorld = HeadCollider->GetComponentTransform();
        DrawDebugBox(GetWorld(), CompToWorld.GetLocation(), BoxExtent, CompToWorld.GetRotation(), Color, false, 0.f, 0, 2.f);
    }
    if (TorsoCollider)
    {
        const bool bFlash = (Now - LastTorsoHitTime) < FlashDuration;
        const FColor Color = bFlash ? FColor::Red : FColor::Green;
        const FVector BoxExtent = TorsoCollider->GetScaledBoxExtent();
        const FTransform& CompToWorld = TorsoCollider->GetComponentTransform();
        DrawDebugBox(GetWorld(), CompToWorld.GetLocation(), BoxExtent, CompToWorld.GetRotation(), Color, false, 0.f, 0, 2.f);
    }
    if (LegCollider)
    {
        const bool bFlash = (Now - LastLegHitTime) < FlashDuration;
        const FColor Color = bFlash ? FColor::Red : FColor::Green;
        const FVector BoxExtent = LegCollider->GetScaledBoxExtent();
        const FTransform& CompToWorld = LegCollider->GetComponentTransform();
        DrawDebugBox(GetWorld(), CompToWorld.GetLocation(), BoxExtent, CompToWorld.GetRotation(), Color, false, 0.f, 0, 2.f);
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

	// Check if enemy is dead from health
	if (HealthComponent && !HealthComponent->IsAlive())
	{
		SetEnemyState(EEnemyState::Dead);
		OnEnemyDeath();
		return;
	}
	
	// Check if enemy should die from no more drops
	if (RemainingDrops <= 0)
	{
		Die();
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

void AEnemyBase::ReactToHit()
{
	if (HealthComponent && HealthComponent->IsAlive())
	{
		if (CurrentState != EEnemyState::Stunned)
		{
			StunEnemy(EnemyStats.StunDuration);
		}
	}
}

void AEnemyBase::ResetSwingHits()
{
	const int32 PreviousCount = HitByWeaponsThisSwing.Num();
	HitByWeaponsThisSwing.Empty();
	UE_LOG(LogTemp, Log, TEXT("Enemy %s reset swing hits (cleared %d weapons)"), *GetName(), PreviousCount);
}

void AEnemyBase::Die()
{
	UE_LOG(LogTemp, Log, TEXT("Enemy %s is dying"), *GetName());
	
	// Call blueprint event for death effects
	OnEnemyDeath();
	
	// Stop all movement and AI
	StopMovement();
	SetEnemyState(EEnemyState::Dead);
	
	// Destroy the enemy after a short delay to allow death effects to play
	FTimerHandle DestroyTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, [this]()
	{
		Destroy();
	}, 1.0f, false);
}

void AEnemyBase::EnableWeaponHitbox()
{
	if (WeaponMeshComponent)
	{
		// Find the weapon actor attached to this component
		TArray<AActor*> AttachedActors;
		GetAttachedActors(AttachedActors);
		
		for (AActor* AttachedActor : AttachedActors)
		{
			if (AWeaponBase* Weapon = Cast<AWeaponBase>(AttachedActor))
			{
				Weapon->EnableHitbox();
				UE_LOG(LogTemp, Log, TEXT("Enemy %s enabled weapon hitbox"), *GetName());
				break;
			}
		}
	}
}

void AEnemyBase::DisableWeaponHitbox()
{
	if (WeaponMeshComponent)
	{
		// Find the weapon actor attached to this component
		TArray<AActor*> AttachedActors;
		GetAttachedActors(AttachedActors);
		
		for (AActor* AttachedActor : AttachedActors)
		{
			if (AWeaponBase* Weapon = Cast<AWeaponBase>(AttachedActor))
			{
				Weapon->DisableHitbox();
				UE_LOG(LogTemp, Log, TEXT("Enemy %s disabled weapon hitbox"), *GetName());
				break;
			}
		}
	}
}

void AEnemyBase::OnBodyPartOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // Record hit time for flash
    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    if (OverlappedComp == HeadCollider) LastHeadHitTime = Now;
    else if (OverlappedComp == TorsoCollider) LastTorsoHitTime = Now;
    else if (OverlappedComp == LegCollider) LastLegHitTime = Now;

    // Basic guard
    if (!OtherActor || OtherActor == this) return;

    // Find the weapon that hit us (either directly or through its owner)
    AWeaponBase* HittingWeapon = nullptr;
    if (AWeaponBase* AsWeapon = Cast<AWeaponBase>(OtherActor))
    {
        HittingWeapon = AsWeapon;
    }
    else if (AWeaponBase* OwnerWeapon = OtherActor ? Cast<AWeaponBase>(OtherActor->GetOwner()) : nullptr)
    {
        HittingWeapon = OwnerWeapon;
    }
    
    // If no weapon found, this might be a fist attack or other hitbox
    // For now, we'll use a dummy pointer to track these hits too
    if (!HittingWeapon)
    {
        // Create a unique identifier for non-weapon hits (e.g., fist attacks)
        // We'll use the OtherActor as a key since it's unique per attack
        if (HitByWeaponsThisSwing.Contains(TWeakObjectPtr<AWeaponBase>(nullptr)))
        {
            return; // Already hit by a non-weapon attack this swing
        }
        HitByWeaponsThisSwing.Add(TWeakObjectPtr<AWeaponBase>(nullptr));
    }
    else
    {
        // Check if this weapon has already hit us this swing
        if (HitByWeaponsThisSwing.Contains(TWeakObjectPtr<AWeaponBase>(HittingWeapon)))
        {
            return; // Already hit by this weapon this swing
        }
        HitByWeaponsThisSwing.Add(TWeakObjectPtr<AWeaponBase>(HittingWeapon));
    }

    // Determine body part
    EBodyPart BodyPart = EBodyPart::Torso;
    if (OverlappedComp == HeadCollider) BodyPart = EBodyPart::Head;
    else if (OverlappedComp == TorsoCollider) BodyPart = EBodyPart::Torso;
    else if (OverlappedComp == LegCollider) BodyPart = EBodyPart::Leg;

    // Drop an item for this hit if we have remaining drops
    if (RemainingDrops > 0)
    {
        --RemainingDrops;
        UE_LOG(LogTemp, Log, TEXT("Enemy %s hit on %s, remaining drops: %d"), *GetName(), 
               BodyPart == EBodyPart::Head ? TEXT("Head") : BodyPart == EBodyPart::Torso ? TEXT("Torso") : TEXT("Leg"), 
               RemainingDrops);
        
        // Attempt to drop now
        if (ItemDropTable)
        {
            // Build a filtered list by body part
            TArray<FItemDropRow*> Candidates;
            ItemDropTable->GetAllRows<FItemDropRow>(TEXT("Drops"), Candidates);
            TArray<FItemDropRow*> Matching;
            for (FItemDropRow* Row : Candidates)
            {
                if (Row && Row->BodyPart == BodyPart)
                {
                    Matching.Add(Row);
                }
            }
            if (Matching.Num() > 0)
            {
                // Weighted rarity selection: Common 70, Uncommon 25, Rare 5
                auto GetWeight = [](ERarity Rarity)
                {
                    switch (Rarity)
                    {
                    case ERarity::Common: return 70;
                    case ERarity::Uncommon: return 25;
                    case ERarity::Rare: return 5;
                    default: return 0;
                    }
                };

                int32 TotalWeight = 0;
                for (FItemDropRow* R : Matching) TotalWeight += GetWeight(R->Rarity);
                const int32 Roll = FMath::RandRange(1, FMath::Max(TotalWeight, 1));
                int32 Accum = 0;
                FItemDropRow* Chosen = Matching[0];
                for (FItemDropRow* R : Matching)
                {
                    Accum += GetWeight(R->Rarity);
                    if (Roll <= Accum) { Chosen = R; break; }
                }

                // Determine prepared state from weapon type (50% chance)
                EPreparedState PreparedState = EPreparedState::None;
                EWeaponType WeaponType = EWeaponType::Skewer;
                if (HittingWeapon)
                {
                    WeaponType = HittingWeapon->GetWeaponType();
                }

                if (FMath::RandBool())
                {
                    switch (WeaponType)
                    {
                    case EWeaponType::Skewer: PreparedState = EPreparedState::Skewered; break;
                    case EWeaponType::Crusher: PreparedState = EPreparedState::Crushed; break;
                    case EWeaponType::Slicer: PreparedState = EPreparedState::Sliced; break;
                    default: break;
                    }
                }

                // Spawn the item drop actor behind the enemy
                const FVector Facing = GetActorForwardVector();
                const FVector SpawnLoc = GetActorLocation() - Facing * 50.f + FVector(0, 0, 30.f);
                const FRotator SpawnRot = FRotator::ZeroRotator;
                FActorSpawnParameters Params; Params.Owner = this;
                if (Chosen->ItemClass.IsValid())
                {
                    UClass* ItemClassToSpawn = Chosen->ItemClass.LoadSynchronous();
                    if (ItemClassToSpawn)
                    {
                        if (AItemDropActor* Drop = GetWorld()->SpawnActor<AItemDropActor>(ItemClassToSpawn, SpawnLoc, SpawnRot, Params))
                        {
                            Drop->ItemName = Chosen->ItemName;
                            Drop->Rarity = Chosen->Rarity;
                            Drop->BodyPart = Chosen->BodyPart;
                            Drop->Category = Chosen->Category;
                            Drop->SetPreparedState(PreparedState);
                            
                            // Launch the item using the new projectile movement system
                            FVector LaunchDirection = -Facing; // Away from the enemy
                            Drop->LaunchItem(LaunchDirection);
                        }
                    }
                }
            }
        }
        
        // Check if we've dropped all our items
        if (RemainingDrops <= 0)
        {
            UE_LOG(LogTemp, Log, TEXT("Enemy %s has no more drops, dying"), *GetName());
            Die();
            return;
        }
    }
}

void AEnemyBase::RefreshHitboxConfigurations()
{
    if (HeadCollider)
    {
        HeadCollider->SetBoxExtent(HeadBoxExtent);
        HeadCollider->SetRelativeLocation(HeadRelativeLocation);
    }
    
    if (TorsoCollider)
    {
        TorsoCollider->SetBoxExtent(TorsoBoxExtent);
        TorsoCollider->SetRelativeLocation(TorsoRelativeLocation);
    }
    
    if (LegCollider)
    {
        LegCollider->SetBoxExtent(LegBoxExtent);
        LegCollider->SetRelativeLocation(LegRelativeLocation);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Enemy %s refreshed hitbox configurations"), *GetName());
}

void AEnemyBase::SetHeadHitbox(FVector NewExtent, FVector NewLocation)
{
    HeadBoxExtent = NewExtent;
    HeadRelativeLocation = NewLocation;
    
    if (HeadCollider)
    {
        HeadCollider->SetBoxExtent(HeadBoxExtent);
        HeadCollider->SetRelativeLocation(HeadRelativeLocation);
    }
}

void AEnemyBase::SetTorsoHitbox(FVector NewExtent, FVector NewLocation)
{
    TorsoBoxExtent = NewExtent;
    TorsoRelativeLocation = NewLocation;
    
    if (TorsoCollider)
    {
        TorsoCollider->SetBoxExtent(TorsoBoxExtent);
        TorsoCollider->SetRelativeLocation(TorsoRelativeLocation);
    }
}

void AEnemyBase::SetLegHitbox(FVector NewExtent, FVector NewLocation)
{
    LegBoxExtent = NewExtent;
    LegRelativeLocation = NewLocation;
    
    if (LegCollider)
    {
        LegCollider->SetBoxExtent(LegBoxExtent);
        LegCollider->SetRelativeLocation(LegRelativeLocation);
    }
}
