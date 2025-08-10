#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SliceOfLifePlayerController.generated.h"

UCLASS()
class SLICEOFLIFE_API ASliceOfLifePlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;

    // UI hooks for designers to implement in BP
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void CreateAndAddDebugWidgets();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdatePlayerStateDisplay(float DamagePercent, const FString& StateLabel, FVector2D Velocity);
};


