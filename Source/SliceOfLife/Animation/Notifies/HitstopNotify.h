#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "HitstopNotify.generated.h"

UCLASS(DisplayName = "SOL_HitstopNotify")
class SLICEOFLIFE_API USOL_HitstopNotify : public UAnimNotify
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitstop")
    float Duration = 0.05f;

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};


