#include "HealthComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	// Initialize default values
	CurrentHealth = MaxHealth;
	CurrentDamagePercent = 0.0f;
	bInHitstun = false;
	HitstunTimer = 0.0f;
	CurrentKnockbackVelocity = FVector::ZeroVector;
	
	// Set default settings
	WeightSettings = FWeightSettings();
	DamageSettings = FDamageSettings();
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;
	CurrentDamagePercent = 0.0f;
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	UpdateHitstun(DeltaTime);
}

void UHealthComponent::TakeDamage(float Damage, FVector KnockbackDirection, float KnockbackForce, AActor* DamageInstigator)
{
	if (bInvulnerable || !IsAlive())
	{
		return;
	}
	
	// Apply damage
	float ActualDamage = Damage * DamageSettings.DamageMultiplier;
	CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);
	
	// Calculate damage percentage (for knockback scaling)
	CurrentDamagePercent = ((MaxHealth - CurrentHealth) / MaxHealth) * 100.0f;
	
	// Calculate knockback force
	float FinalKnockbackForce = CalculateKnockbackForce(KnockbackForce, CurrentDamagePercent);
	
	// Apply knockback
	ApplyKnockback(KnockbackDirection, FinalKnockbackForce);
	
	// Set hitstun
	float HitstunDuration = DamageSettings.HitstunDuration * WeightSettings.HitstunMultiplier;
	SetHitstun(HitstunDuration);
	
	// Broadcast events
	OnDamageReceived.Broadcast(ActualDamage, KnockbackDirection, FinalKnockbackForce);
	OnHealthChanged.Broadcast(CurrentHealth);
	
	// Log damage for debugging
	UE_LOG(LogTemp, Log, TEXT("Actor %s took %f damage. Health: %f/%f, Damage%%: %f"), 
		*GetOwner()->GetName(), ActualDamage, CurrentHealth, MaxHealth, CurrentDamagePercent);
	
	// Check if knocked out
	if (CurrentHealth <= 0.0f && bCanBeKnockedOut)
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor %s has been knocked out!"), *GetOwner()->GetName());
		// Additional knockout logic can be added here
	}
}

void UHealthComponent::Heal(float HealAmount)
{
	if (!IsAlive())
	{
		return;
	}
	
	float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + HealAmount);
	CurrentDamagePercent = ((MaxHealth - CurrentHealth) / MaxHealth) * 100.0f;
	
	if (CurrentHealth != OldHealth)
	{
		OnHealthChanged.Broadcast(CurrentHealth);
	}
}

void UHealthComponent::SetMaxHealth(float NewMaxHealth)
{
	if (NewMaxHealth > 0.0f)
	{
		float HealthRatio = CurrentHealth / MaxHealth;
		MaxHealth = NewMaxHealth;
		CurrentHealth = MaxHealth * HealthRatio;
		CurrentDamagePercent = ((MaxHealth - CurrentHealth) / MaxHealth) * 100.0f;
		
		OnHealthChanged.Broadcast(CurrentHealth);
	}
}

void UHealthComponent::ResetHealth()
{
	CurrentHealth = MaxHealth;
	CurrentDamagePercent = 0.0f;
	ClearHitstun();
	
	OnHealthChanged.Broadcast(CurrentHealth);
}

void UHealthComponent::ApplyKnockback(FVector Direction, float Force)
{
    if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
    {
        // Calculate launch impulse
        FVector LaunchVel = Direction.GetSafeNormal() * Force;
        LaunchVel /= FMath::Max(WeightSettings.BaseKnockbackResistance, 0.01f);
        CurrentKnockbackVelocity = LaunchVel;
        OwnerCharacter->LaunchCharacter(LaunchVel, true, true);
        UE_LOG(LogTemp, Log, TEXT("Applied knockback via LaunchCharacter: Dir=%s Force=%f Vel=%s"), *Direction.ToString(), Force, *LaunchVel.ToString());
    }
}

void UHealthComponent::SetHitstun(float Duration)
{
	if (Duration > 0.0f)
	{
		bInHitstun = true;
		HitstunTimer = Duration;
		ApplyHitstunEffects();
		OnHitstunChanged.Broadcast(true);
		
		UE_LOG(LogTemp, Log, TEXT("Hitstun applied for %f seconds"), Duration);
	}
}

void UHealthComponent::ClearHitstun()
{
	if (bInHitstun)
	{
		bInHitstun = false;
		HitstunTimer = 0.0f;
		CurrentKnockbackVelocity = FVector::ZeroVector;
		ClearHitstunEffects();
		OnHitstunChanged.Broadcast(false);
		
		UE_LOG(LogTemp, Log, TEXT("Hitstun cleared"));
	}
}

void UHealthComponent::UpdateHitstun(float DeltaTime)
{
	if (bInHitstun)
	{
		HitstunTimer -= DeltaTime;
		
		if (HitstunTimer <= 0.0f)
		{
			ClearHitstun();
		}
	}
}

void UHealthComponent::ApplyHitstunEffects()
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
		{
            // Ensure physics-based reaction; prevent braking fighting the launch
            MovementComp->DisableMovement();
			
			// Apply knockback velocity
			if (!CurrentKnockbackVelocity.IsNearlyZero())
			{
				MovementComp->Velocity = CurrentKnockbackVelocity;
			}
		}
	}
}

void UHealthComponent::ClearHitstunEffects()
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement())
		{
            MovementComp->SetMovementMode(MOVE_Walking);
		}
	}
}

float UHealthComponent::CalculateKnockbackForce(float BaseForce, float DamagePercent) const
{
	// Base knockback from damage settings
	float FinalForce = DamageSettings.BaseKnockback;
	
	// Scale by damage percentage
	FinalForce *= FMath::Pow(DamageSettings.KnockbackScaling, DamagePercent / 100.0f);
	
	// Apply weight scaling
	FinalForce *= WeightSettings.KnockbackScaling;
	
	// Apply input force multiplier
	FinalForce *= (BaseForce / 1000.0f); // Normalize base force
	
	return FinalForce;
}
