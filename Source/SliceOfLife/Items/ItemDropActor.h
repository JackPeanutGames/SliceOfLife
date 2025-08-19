#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "SliceOfLife/Items/ItemDropTypes.h"
#include "ItemDropActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class APlayerCharacter;

UCLASS(Blueprintable)
class SLICEOFLIFE_API AItemDropActor : public AActor
{
	GENERATED_BODY()

public:
	AItemDropActor();

protected:
	virtual void BeginPlay() override;

public:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UProjectileMovementComponent* ProjectileMovement;

	// Metadata
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	ERarity Rarity = ERarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EBodyPart BodyPart = EBodyPart::Torso;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	ECategory Category = ECategory::Organ;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EPreparedState PreparedState = EPreparedState::None;

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetPreparedState(EPreparedState NewState);
	
	// Overlap
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Blueprint event for pickup
	UFUNCTION(BlueprintImplementableEvent, Category = "Item")
	void OnPickedUp(APlayerCharacter* Player);

	// 2.5D constraint toggle — if true, constrains movement to Y=0 plane
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|PlaneConstraint")
	bool bConstrainToYPlane = true;

	// Launch settings for designers to tweak
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Launch")
	float MinLaunchSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Launch")
	float MaxLaunchSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Launch")
	float BounceVelocity = 0.6f;

	void ApplyPlaneConstraintSettings();

	// Launch the item in a given direction
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void LaunchItem(const FVector& Direction, float Speed = -1.0f);

	// Launch the item with a specific velocity vector
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void LaunchItemWithVelocity(const FVector& Velocity);
};


