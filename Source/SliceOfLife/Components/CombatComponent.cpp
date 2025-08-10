#include "CombatComponent.h"
#include "HealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

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
	
	// Update move staling
	if (bEnableMoveStaling)
	{
		UpdateMoveStaling(0.0f); // Force update
	}
	
	UE_LOG(LogTemp, Log, TEXT("Attack ended"));
}

void UCombatComponent::SpawnHitbox()
{
	// Placeholder for spawning hitbox logic
}

void UCombatComponent::DetectHits()
{
	// Placeholder for hit detection logic
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
