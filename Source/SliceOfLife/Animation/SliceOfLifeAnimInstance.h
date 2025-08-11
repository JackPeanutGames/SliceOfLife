#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SliceOfLifeAnimInstance.generated.h"

class APlayerCharacter;
class UPlayerMovementComponent;

UCLASS(Blueprintable)
class SLICEOFLIFE_API USliceOfLifeAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    USliceOfLifeAnimInstance();

    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
    // Movement
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    bool bIsDashing;

    // Combat
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsAttacking;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsCharging;

    UPROPERTY(BlueprintReadOnly, Category = "Animation|Combat")
    bool bIsInHitstun;

    // Optional: Direction for blend spaces
    UPROPERTY(BlueprintReadOnly, Category = "Animation|Movement")
    float MovementDirection;

private:
    // Cached owning character for quick access
    UPROPERTY(Transient)
    APlayerCharacter* CachedPlayerCharacter;

    UPROPERTY(Transient)
    UPlayerMovementComponent* CachedMovementComponent;
};


