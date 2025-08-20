#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "LaunchUpwardNotify.generated.h"

/**
 * Animation notify that launches the player character upwards.
 * Perfect for up attack animations to create aerial combat feel.
 */
UCLASS()
class SLICEOFLIFE_API ULaunchUpwardNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	ULaunchUpwardNotify();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	// The upward force to apply (in Unreal units)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launch", meta = (ClampMin = "0.0", ClampMax = "2000.0"))
	float LaunchForce = 800.0f;

	// Whether to preserve existing horizontal velocity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launch")
	bool bPreserveHorizontalVelocity = true;

	// Whether to add a small forward push for better feel
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launch")
	bool bAddForwardPush = false;

	// Forward push force (only used if bAddForwardPush is true)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launch", meta = (EditCondition = "bAddForwardPush", ClampMin = "0.0", ClampMax = "500.0"))
	float ForwardPushForce = 200.0f;

	// Whether to enable air control during the launch
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launch")
	bool bEnableAirControl = true;

	// Duration to enable air control (in seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Launch", meta = (EditCondition = "bEnableAirControl", ClampMin = "0.1", ClampMax = "5.0"))
	float AirControlDuration = 1.0f;
};
