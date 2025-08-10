#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "EndAttackNotify.generated.h"

UCLASS(DisplayName = "SOL_EndAttackNotify")
class SLICEOFLIFE_API USOL_EndAttackNotify : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};


