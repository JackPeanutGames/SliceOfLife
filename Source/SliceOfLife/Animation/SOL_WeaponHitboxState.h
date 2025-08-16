#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SOL_WeaponHitboxState.generated.h"

class APlayerCharacter;
class AWeaponBase;

UCLASS()
class SLICEOFLIFE_API USOL_WeaponHitboxState : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
    virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
    void ToggleWeaponHitbox(APlayerCharacter* Player, bool bEnable) const;
    void DrawWeaponHitboxDebug(APlayerCharacter* Player) const;
};


