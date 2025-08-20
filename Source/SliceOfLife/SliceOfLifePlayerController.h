#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SliceOfLifePlayerController.generated.h"

class UUserWidget;
class ASliceOfLifePlayerState;

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
    
    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdatePlayerHealthDisplay(float HealthPercent);

    // Debug/tuning widget creation in C++ (optional path)
    UFUNCTION(BlueprintCallable, Category = "UI")
    void CreateDebugWidgetsNative();

    // Slider callbacks (examples)
    UFUNCTION(BlueprintCallable, Category = "UI|Tuning")
    void OnGravityChanged(float NewValue);

    UFUNCTION(BlueprintCallable, Category = "UI|Tuning")
    void OnFrictionChanged(float NewValue);

    UFUNCTION(BlueprintCallable, Category = "UI|Tuning")
    void OnPlayerSpeedChanged(float NewValue);

protected:
    // Optional native widget classes (set in BP defaults if desired)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> StateDisplayClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> TuningPanelClass;

    UPROPERTY(Transient)
    UUserWidget* StateDisplayWidget;

    UPROPERTY(Transient)
    UUserWidget* TuningPanelWidget;

    UFUNCTION()
    void OnPlayerDamagePercentChanged(float NewDamagePercent);
};


