#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DummyCharacter.generated.h"

class UHealthComponent;
class UWidgetComponent;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class SLICEOFLIFE_API ADummyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ADummyCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWidgetComponent* DamageDisplayWidget;

    // Placeholder visual: "Buddy" sphere to knock around (2m tall dummy)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
    UStaticMeshComponent* PlaceholderBuddy;

	// Dummy Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy")
	bool bInvulnerable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy")
	bool bCanBeKnockedOut = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy")
	bool bShowDamageDisplay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy")
	bool bLogAllHits = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dummy")
	float DamageDisplayHeight = 200.0f;

	// Internal Functions
	void UpdateDamageDisplay();
	void LogHit(float Damage, FVector KnockbackDirection, float KnockbackForce);

public:
	// Component Getters
	UFUNCTION(BlueprintPure, Category = "Components")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	// Dummy Functions
	UFUNCTION(BlueprintCallable, Category = "Dummy")
	void ResetDummy();

	UFUNCTION(BlueprintCallable, Category = "Dummy")
	void SetInvulnerable(bool bNewInvulnerable);

	UFUNCTION(BlueprintCallable, Category = "Dummy")
	void ToggleDamageDisplay();

	// Event Handlers
    // Must be UFUNCTION for AddDynamic binding (matches FOnDamageReceived signature)
    UFUNCTION()
    void OnDamageReceived(float Damage, FVector KnockbackDirection, float KnockbackForce);

	UFUNCTION()
	void OnHealthChanged(float NewHealth);

	UFUNCTION()
	void OnHitstunChanged(bool bInHitstun);
};
