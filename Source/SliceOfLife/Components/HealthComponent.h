#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageReceived, float, Damage, FVector, KnockbackDirection, float, KnockbackForce);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitstunChanged, bool, bInHitstun);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthReachedZero);

USTRUCT(BlueprintType)
struct FWeightSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight")
	float Weight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight")
	float BaseKnockbackResistance = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight")
	float KnockbackScaling = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight")
	float HitstunMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct FDamageSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float BaseDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float BaseKnockback = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float KnockbackScaling = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float HitstunDuration = 0.5f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SLICEOFLIFE_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Health Management
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage(float Damage, FVector KnockbackDirection, float KnockbackForce, AActor* DamageInstigator = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float HealAmount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetMaxHealth(float NewMaxHealth);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHealth();

	// Knockback and Hitstun
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ApplyKnockback(FVector Direction, float Force);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHitstun(float Duration);

	UFUNCTION(BlueprintCallable, Category = "Health")
	void ClearHitstun();

	// State Queries
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsAlive() const { return CurrentHealth > 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsInHitstun() const { return bInHitstun; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentDamagePercent() const { return CurrentDamagePercent; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetRemainingHitstunTime() const { return HitstunTimer; }
	
	UFUNCTION(BlueprintPure, Category = "Health")
	FVector GetCurrentKnockbackVelocity() const { return CurrentKnockbackVelocity; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDamageReceived OnDamageReceived;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHitstunChanged OnHitstunChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthReachedZero OnHealthReachedZero;

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	FWeightSettings WeightSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	FDamageSettings DamageSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	bool bInvulnerable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	bool bCanBeKnockedOut = true;

protected:
	// Current State
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Health")
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Health")
	float CurrentDamagePercent;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	bool bInHitstun;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float HitstunTimer;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	FVector CurrentKnockbackVelocity;

	// Internal Functions
	void UpdateHitstun(float DeltaTime);
	void ApplyHitstunEffects();
	void ClearHitstunEffects();
	float CalculateKnockbackForce(float BaseForce, float DamagePercent) const;

private:
    void UpdatePlayerStateDamagePercent();
};
