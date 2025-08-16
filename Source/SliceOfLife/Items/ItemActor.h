#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class SLICEOFLIFE_API AItemActor : public AActor
{
	GENERATED_BODY()

public:
	AItemActor();

	UFUNCTION(BlueprintCallable)
	void SetItemMesh(UStaticMesh* Mesh);

	UFUNCTION(BlueprintCallable)
	void ImpulseBackward(const FVector& Direction, float Strength);

protected:
	UPROPERTY(VisibleAnywhere)
	USphereComponent* Collision;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;
};


