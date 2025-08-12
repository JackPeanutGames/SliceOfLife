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
#include "DrawDebugHelpers.h"

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
	
	// Spawn hitbox after a short delay
	FTimerHandle HitboxTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		HitboxTimerHandle,
		this,
		&UCombatComponent::SpawnHitbox,
		AttackData.AttackDuration * 0.5f, // Spawn hitbox halfway through attack
		false
	);

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

void UCombatComponent::SpawnHitbox()
{
    // Designers will add UAnimNotify(States) to montages to call ApplyDamage at correct frames.
    // Fallback: simple point damage at owner forward hitbox for prototyping.
    if (AActor* OwnerActor = GetOwner())
    {
        const FVector Start = OwnerActor->GetActorLocation();
        const FVector End = Start + OwnerActor->GetActorForwardVector() * 150.f;
        FHitResult Hit;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, OwnerActor);
        if (OwnerActor->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
        {
            if (AActor* HitActor = Hit.GetActor())
            {
                UGameplayStatics::ApplyPointDamage(HitActor, CurrentAttack.Damage, OwnerActor->GetActorForwardVector(), Hit, nullptr, OwnerActor, nullptr);
            }
        }
    }
}

void UCombatComponent::DetectHits()
{
    // Montages + anim notifies should drive precise hit detection; not used by default
}

void UCombatComponent::SpawnHitboxParams(const FVector& LocalOffset, const FVector& BoxExtent, float Damage, float KnockbackForce)
{
    if (AActor* OwnerActor = GetOwner())
    {
        // Compute world center from local offset
        const FVector Center = OwnerActor->GetActorLocation() 
            + OwnerActor->GetActorForwardVector() * LocalOffset.X
            + OwnerActor->GetActorRightVector() * LocalOffset.Y
            + OwnerActor->GetActorUpVector() * LocalOffset.Z;

        // Store parameters for overlap callback (single active hitbox recommended)
        PendingHitboxLocalOffset = LocalOffset;
        PendingHitboxExtent = BoxExtent;
        PendingHitboxDamage = Damage;
        PendingHitboxKnockback = KnockbackForce;

        // Create a transient box component as the hitbox
        UBoxComponent* Hitbox = NewObject<UBoxComponent>(OwnerActor);
        if (!Hitbox)
        {
            return;
        }
        Hitbox->AttachToComponent(OwnerActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
        Hitbox->RegisterComponent();
        Hitbox->SetBoxExtent(BoxExtent, true);
        Hitbox->SetWorldLocation(Center);
        Hitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        Hitbox->SetCollisionObjectType(ECC_WorldDynamic);
        Hitbox->SetGenerateOverlapEvents(true);
        Hitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
        Hitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
        Hitbox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);

        // Bind overlap to apply damage once, then destroy
        Hitbox->OnComponentBeginOverlap.AddDynamic(this, &UCombatComponent::OnHitboxBeginOverlap);

        // Ignore owner collisions
        Hitbox->IgnoreActorWhenMoving(OwnerActor, true);

        // Debug draw
        if (bEnableHitboxDebug)
        {
            const FTransform HitboxTransform(Hitbox->GetComponentRotation(), Hitbox->GetComponentLocation());
            DrawDebugBox(OwnerActor->GetWorld(), HitboxTransform.GetLocation(), BoxExtent, HitboxTransform.GetRotation(), FColor::Red, false, DebugHitboxDuration, 0, 2.0f);
        }

        // Auto-destroy shortly after spawn to avoid lingering
        FTimerHandle DestroyHandle;
        OwnerActor->GetWorldTimerManager().SetTimer(DestroyHandle, [Hitbox]()
        {
            if (Hitbox)
            {
                Hitbox->DestroyComponent();
            }
        }, 0.2f, false);

        UE_LOG(LogTemp, Verbose, TEXT("Spawned hitbox at %s extent %s dmg %.1f kb %.1f"), *Center.ToString(), *BoxExtent.ToString(), Damage, KnockbackForce);
    }
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

    // Apply damage to the other actor
    AController* InstigatorController = nullptr;
    if (APawn* PawnOwner = Cast<APawn>(OwnerActor))
    {
        InstigatorController = PawnOwner->GetController();
    }
    UGameplayStatics::ApplyPointDamage(OtherActor, PendingHitboxDamage, OwnerActor->GetActorForwardVector(), SweepResult, InstigatorController, OwnerActor, nullptr);

    // Debug hit confirmation
    if (bEnableHitboxDebug)
    {
        DrawDebugPoint(OwnerActor->GetWorld(), SweepResult.ImpactPoint, 16.f, FColor::Yellow, false, 0.2f);
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
