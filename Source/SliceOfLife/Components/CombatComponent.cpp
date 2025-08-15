#include "CombatComponent.h"
#include "HealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimInstance.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "SliceOfLife/Characters/PlayerCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "HAL/IConsoleManager.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// Initialize default values
	CurrentAttackState = EAttackState::Idle;
	AttackTimer = 0.0f;
	ChargeTimer = 0.0f;
	CurrentChargeMultiplier = 1.0f;
	
	// Initialize default attack data
	CurrentAttack = FAttackData();
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("SliceOfLife.ShowOffensiveHitboxes")))
	{
		bShowOffensiveHitboxes = (CVar->GetInt() != 0);
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	UpdateAttack(DeltaTime);
	UpdateMoveStaling(DeltaTime);
}

void UCombatComponent::LightAttack()
{
	if (!CanAttack())
	{
		return;
	}
	
	FAttackData* AttackData = GetAttackData(EAttackType::Light);
	if (AttackData)
	{
		StartAttack(*AttackData);
	}
}

void UCombatComponent::TiltAttack(FVector2D Direction)
{
	if (!CanAttack())
	{
		return;
	}
	
	FAttackData* AttackData = GetAttackData(EAttackType::Tilt, Direction);
	if (AttackData)
	{
		StartAttack(*AttackData);
	}
}

void UCombatComponent::AerialAttack()
{
	if (!CanAttack())
	{
		return;
	}
	
	FAttackData* AttackData = GetAttackData(EAttackType::Aerial);
	if (AttackData)
	{
		StartAttack(*AttackData);
	}
}

void UCombatComponent::SmashAttackStart()
{
	if (!CanAttack())
	{
		return;
	}
	
	FAttackData* AttackData = GetAttackData(EAttackType::Smash);
	if (AttackData && AttackData->ChargeTime > 0.0f)
	{
		CurrentAttackState = EAttackState::Charging;
		CurrentAttack = *AttackData;
		ChargeTimer = 0.0f;
		CurrentChargeMultiplier = 1.0f;
		
		UE_LOG(LogTemp, Log, TEXT("Started charging smash attack: %s"), *AttackData->AttackName);
	}
}

void UCombatComponent::SmashAttackRelease()
{
	if (CurrentAttackState == EAttackState::Charging)
	{
		// Finalize charge
		CurrentChargeMultiplier = FMath::Clamp(1.0f + (ChargeTimer / CurrentAttack.ChargeTime) * (CurrentAttack.MaxChargeMultiplier - 1.0f), 1.0f, CurrentAttack.MaxChargeMultiplier);
		CurrentAttack.Damage *= CurrentChargeMultiplier;
		CurrentAttack.KnockbackForce *= CurrentChargeMultiplier;
		
		StartAttack(CurrentAttack);
	}
}

bool UCombatComponent::CanAttack() const
{
	return CurrentAttackState == EAttackState::Idle || CurrentAttackState == EAttackState::Charging;
}

bool UCombatComponent::IsAttacking() const
{
	return CurrentAttackState == EAttackState::Attacking || CurrentAttackState == EAttackState::Recovery;
}

bool UCombatComponent::IsCharging() const
{
	return CurrentAttackState == EAttackState::Charging;
}

float UCombatComponent::GetChargeProgress() const
{
	if (CurrentAttackState == EAttackState::Charging && CurrentAttack.ChargeTime > 0.0f)
	{
		return FMath::Clamp(ChargeTimer / CurrentAttack.ChargeTime, 0.0f, 1.0f);
	}
	return 0.0f;
}

void UCombatComponent::ResetMoveStaling()
{
}

float UCombatComponent::GetStaleMultiplier(const FString& MoveName) const
{
	for (const FMoveStaleData& Data : StaleData)
	{
		if (Data.MoveName == MoveName)
		{
			return Data.StaleMultiplier;
		}
	}
	return 1.0f;
}

void UCombatComponent::StartAttack(const FAttackData& AttackData)
{
	CurrentAttack = AttackData;
	CurrentAttackState = EAttackState::Attacking;
	AttackTimer = AttackData.AttackDuration;
    ActorsHitThisSwing.Reset();
	
	// Apply move staling
	if (bEnableMoveStaling)
	{
		float StaleMultiplier = GetStaleMultiplier(AttackData.AttackName);
		CurrentAttack.Damage *= StaleMultiplier;
		CurrentAttack.KnockbackForce *= StaleMultiplier;
		
		UE_LOG(LogTemp, Log, TEXT("Started attack: %s with stale multiplier: %f"), 
			*AttackData.AttackName, StaleMultiplier);
	}
	
    // Notify-driven hitboxes handle overlap & damage; no timed fallback

    // Play montage if available
    if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
    {
        if (UAnimInstance* AnimInst = OwnerChar->GetMesh() ? OwnerChar->GetMesh()->GetAnimInstance() : nullptr)
        {
            UAnimMontage* MontageToPlay = nullptr;
            // Determine attack direction from last movement input (X,Z in 2.5D)
            const FVector LastInput = OwnerChar->GetLastMovementInputVector();
            const float X = LastInput.X;
            const float Z = LastInput.Z;
            if (FMath::Abs(Z) > 0.3f)
            {
                if (Z > 0.3f && FMath::Abs(X) > 0.3f) CurrentAttackDirection = EAttackDirection::UpDiagonal;
                else if (Z > 0.3f) CurrentAttackDirection = EAttackDirection::Up;
                else if (Z < -0.3f && FMath::Abs(X) > 0.3f) CurrentAttackDirection = EAttackDirection::DownDiagonal;
                else if (Z < -0.3f) CurrentAttackDirection = EAttackDirection::Down;
            }
            else
            {
                CurrentAttackDirection = EAttackDirection::Forward;
            }

            switch (AttackData.AttackType)
            {
            case EAttackType::Light:
                if (CurrentAttackDirection == EAttackDirection::Up && LightUpMontage) MontageToPlay = LightUpMontage;
                else if (CurrentAttackDirection == EAttackDirection::Down && LightDownMontage) MontageToPlay = LightDownMontage;
                else MontageToPlay = LightForwardMontage ? LightForwardMontage : LightMontage;
                break;
            case EAttackType::Tilt:
                MontageToPlay = TiltMontage; break;
            case EAttackType::Aerial:
                if (CurrentAttackDirection == EAttackDirection::Up && AerialUpMontage) MontageToPlay = AerialUpMontage;
                else if (CurrentAttackDirection == EAttackDirection::Down && AerialDownMontage) MontageToPlay = AerialDownMontage;
                else MontageToPlay = AerialForwardMontage ? AerialForwardMontage : AerialMontage;
                break;
            case EAttackType::Smash:
                MontageToPlay = SmashMontage; break;
            default: break;
            }
            if (MontageToPlay)
            {
                FOnMontageEnded MontageEnded;
                MontageEnded.BindLambda([this](UAnimMontage*, bool)
                {
                    if (CurrentAttackState == EAttackState::Attacking)
                    {
                        // Ensure we go to recovery/end when montage ends
                        EndAttack();
                    }
                });
                AnimInst->Montage_Play(MontageToPlay, MontagePlayRate);
                AnimInst->Montage_SetEndDelegate(MontageEnded, MontageToPlay);
            }
        }
    }
}

void UCombatComponent::UpdateAttack(float DeltaTime)
{
	switch (CurrentAttackState)
	{
		case EAttackState::Charging:
			ChargeTimer += DeltaTime;
			CurrentChargeMultiplier = 1.0f + (ChargeTimer / CurrentAttack.ChargeTime) * (CurrentAttack.MaxChargeMultiplier - 1.0f);
			break;
			
		case EAttackState::Attacking:
			AttackTimer -= DeltaTime;
			if (AttackTimer <= 0.0f)
			{
				CurrentAttackState = EAttackState::Recovery;
				AttackTimer = CurrentAttack.RecoveryDuration;
			}
			break;
			
		case EAttackState::Recovery:
			AttackTimer -= DeltaTime;
			if (AttackTimer <= 0.0f)
			{
				EndAttack();
			}
			break;
			
		default:
			break;
	}
}

void UCombatComponent::EndAttack()
{
	CurrentAttackState = EAttackState::Idle;
	AttackTimer = 0.0f;
	ChargeTimer = 0.0f;
	CurrentChargeMultiplier = 1.0f;
    ActorsHitThisSwing.Reset();
	
	// Update move staling
	if (bEnableMoveStaling)
	{
		UpdateMoveStaling(0.0f); // Force update
	}
	
	UE_LOG(LogTemp, Log, TEXT("Attack ended"));
}

// SpawnHitbox() deleted; notify-driven UBoxComponent overlaps are the sole damage path now

void UCombatComponent::DetectHits()
{
    // Montages + anim notifies should drive precise hit detection; not used by default
}

void UCombatComponent::SpawnHitboxParams(const FVector& LocalOffset, const FVector& BoxExtent, float Damage, float KnockbackForce)
{
    // Legacy path not used now that notify-spawned hitboxes call overlap directly.
    // Keep parameters for overlap callback to use damage values.
    PendingHitboxLocalOffset = LocalOffset;
    PendingHitboxExtent = BoxExtent;
    PendingHitboxDamage = Damage;
    PendingHitboxKnockback = KnockbackForce;
}

void UCombatComponent::OnHitboxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                           int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor || !OtherActor || OtherActor == OwnerActor)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Hitbox overlap: %s -> %s"), *OwnerActor->GetName(), *OtherActor->GetName());

    // Prevent multi-hits within the same swing
    if (ActorsHitThisSwing.Contains(OtherActor))
    {
        return;
    }
    ActorsHitThisSwing.Add(OtherActor);

    // Determine instigator controller for damage
    AController* InstigatorController = nullptr;
    if (APawn* PawnOwner = Cast<APawn>(OwnerActor))
    {
        InstigatorController = PawnOwner->GetController();
    }
    // Apply damage using PendingHitboxDamage
    FPointDamageEvent PointEvent;
    OtherActor->TakeDamage(PendingHitboxDamage, PointEvent, InstigatorController, OwnerActor);
    // Apply knockback using PendingHitboxKnockback
    if (UHealthComponent* OtherHealth = OtherActor->FindComponentByClass<UHealthComponent>())
    {
        FVector FacingDir = OwnerActor->GetActorForwardVector();
        OtherHealth->ApplyKnockback(FacingDir, PendingHitboxKnockback);
    }
    // Reset pending values
    PendingHitboxDamage = 0.f;
    PendingHitboxKnockback = 0.f;

    // Briefly tint enemy capsule red on hit for feedback
    if (UCapsuleComponent* Capsule = OtherActor->FindComponentByClass<UCapsuleComponent>())
    {
        FColor OriginalColor = Capsule->ShapeColor;
        Capsule->ShapeColor = FColor::Red;

        FTimerHandle RevertTimer;
        Capsule->GetWorld()->GetTimerManager().SetTimer(
            RevertTimer,
            [Capsule, OriginalColor]()
            {
                if (::IsValid(Capsule))
                {
                    Capsule->ShapeColor = OriginalColor;
                }
            },
            0.2f,
            false);
    }

    // Debug hit confirmation (global CVAR)
    static const auto CVarShowHitboxes = IConsoleManager::Get().FindConsoleVariable(TEXT("SliceOfLife.ShowHitboxes"));
    const bool bShow = CVarShowHitboxes ? (CVarShowHitboxes->GetInt() != 0) : false;
    if (bShow)
    {
        FVector Impact = OtherActor->GetActorLocation();
        if (SweepResult.bBlockingHit)
        {
            Impact = FVector(SweepResult.ImpactPoint);
        }
        DrawDebugPoint(OwnerActor->GetWorld(), Impact, 16.f, FColor::Yellow, false, 0.2f);
        if (HitImpactFX)
        {
            UGameplayStatics::SpawnEmitterAtLocation(OwnerActor->GetWorld(), HitImpactFX, Impact);
        }
    }

    // Destroy the hitbox component after hit
    if (UPrimitiveComponent* Comp = OverlappedComp)
    {
        Comp->DestroyComponent();
    }
}

void UCombatComponent::EndAttackNow()
{
    EndAttack();
}

void UCombatComponent::BeginAttackWindow()
{
    ActorsHitThisSwing.Reset();
}

void UCombatComponent::EndAttackWindow()
{
    ActorsHitThisSwing.Reset();
}

float UCombatComponent::CalculateStaleMultiplier(const FString& MoveName)
{
	// Placeholder: could implement decay over time
	return 1.0f;
}

void UCombatComponent::UpdateMoveStaling(float DeltaTime)
{
	// Placeholder for move staling update logic
}

FAttackData* UCombatComponent::GetAttackData(EAttackType AttackType, const FVector2D& Direction)
{
	if (!AttackDataTable)
	{
		return nullptr;
	}
	
	TArray<FName> RowNames = AttackDataTable->GetRowNames();
	for (FName RowName : RowNames)
	{
		if (FAttackData* Row = AttackDataTable->FindRow<FAttackData>(RowName, TEXT("")))
		{
			if (Row->AttackType == AttackType)
			{
				return Row;
			}
		}
	}
	
	return nullptr;
}
