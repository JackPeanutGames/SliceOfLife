#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SimpleLaunchUpwardNotify.generated.h"

/**
 * Simple animation notify that launches the player character upwards.
 * Lightweight version for basic upward launches in up attack animations.
 */
UCLASS()
class SLICEOFLIFE_API USimpleLaunchUpwardNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	USimpleLaunchUpwardNotify();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	// The upward force to apply (in Unreal units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launch", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
	float LaunchForce = 800.0f;

	// Whether to preserve existing horizontal velocity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launch")
	bool bPreserveHorizontalVelocity = true;
};
