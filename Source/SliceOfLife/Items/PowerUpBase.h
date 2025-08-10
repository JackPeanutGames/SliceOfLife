#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "PowerUpBase.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class APlayerCharacter;
class USoundBase;
class UParticleSystem;

UENUM(BlueprintType)
enum class EPowerUpType : uint8
{
	Health,
	Movement,
	Combat,
	Special
};

USTRUCT(BlueprintType)
struct FPowerUpData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	FString PowerUpName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	EPowerUpType PowerUpType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	float Value = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	bool bIsPermanent = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	USoundBase* PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	UParticleSystem* PickupEffect;
};

UCLASS(Blueprintable)
class SLICEOFLIFE_API APowerUpBase : public AActor
{
	GENERATED_BODY()

public:
	APowerUpBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Power Up Logic
	UFUNCTION(BlueprintCallable, Category = "Power Up")
	virtual void ApplyPowerUp(APlayerCharacter* Player);

	UFUNCTION(BlueprintCallable, Category = "Power Up")
	virtual void RemovePowerUp(APlayerCharacter* Player);

	// Interaction
	UFUNCTION(BlueprintCallable, Category = "Power Up")
	void OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// State Queries
	UFUNCTION(BlueprintPure, Category = "Power Up")
	bool IsCollected() const { return bIsCollected; }

	UFUNCTION(BlueprintPure, Category = "Power Up")
	EPowerUpType GetPowerUpType() const { return PowerUpData.PowerUpType; }

	// Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Power Up")
	void OnPowerUpCollected(APlayerCharacter* Player);

	UFUNCTION(BlueprintImplementableEvent, Category = "Power Up")
	void OnPowerUpApplied(APlayerCharacter* Player);

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComponent;

	// Power Up Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	FPowerUpData PowerUpData;

	// State
	UPROPERTY(BlueprintReadOnly, Category = "Power Up")
	bool bIsCollected;

	// Visual Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	float RotationSpeed = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	float BobbingSpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power Up")
	float BobbingHeight = 20.0f;

	// Cached initial location for bobbing effect
	FVector InitialLocation;

	// Internal Functions
	void UpdateVisualEffects(float DeltaTime);
	void PlayPickupEffects(APlayerCharacter* Player);
	bool CanApplyToPlayer(APlayerCharacter* Player) const;
};
