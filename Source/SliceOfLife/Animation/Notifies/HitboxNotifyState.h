#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "HitboxNotifyState.generated.h"

class UCombatComponent;

UCLASS(DisplayName = "SOL_HitboxNotifyState")
class SLICEOFLIFE_API USOL_HitboxNotifyState : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    // Local-space offset from the owning actor used to position the hitbox center
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    FVector LocalOffset = FVector(50.f, 0.f, 50.f);

    // Half-size of the box used for sweep tests
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    FVector BoxExtent = FVector(30.f, 30.f, 30.f);

    // Damage applied to targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    float Damage = 10.f;

    // Base knockback force
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
    float KnockbackForce = 600.f;

    void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
    void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};


