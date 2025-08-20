#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

class UStaticMeshComponent;
class UPrimitiveComponent;
class UBoxComponent;
class ACharacter;

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Skewer,
	Crusher,
	Slicer
};

UCLASS(Blueprintable)
class SLICEOFLIFE_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	AWeaponBase();

	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EnableHitbox();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void DisableHitbox();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetAttackParams(float InDamage, float InKnockback);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EWeaponType GetWeaponType() const { return WeaponType; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	UPrimitiveComponent* GetHitboxComponent() const { return HitboxComponent; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsHitboxActive() const { return bHitboxActive; }

	// Optional helper to adjust hitbox shape/offset at runtime (e.g., from notify)
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ConfigureHitbox(const FVector& LocalOffset, const FVector& BoxExtent);

protected:
	UFUNCTION()
	void OnHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	UPrimitiveComponent* HitboxComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponType WeaponType = EWeaponType::Skewer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Params")
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Params")
	float KnockbackForce = 500.0f;

	// Debug/State
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Debug")
	bool bHitboxActive = false;
};


