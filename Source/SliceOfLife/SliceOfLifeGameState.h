#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SliceOfLifeGameState.generated.h"

UCLASS(Blueprintable)
class SLICEOFLIFE_API ASliceOfLifeGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    ASliceOfLifeGameState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Replicated match-wide data
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
    FString CurrentAreaName;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
    float GlobalTimer;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "GameState")
    int32 GlobalScore;
};


