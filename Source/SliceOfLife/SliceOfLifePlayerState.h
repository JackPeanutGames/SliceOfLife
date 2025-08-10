#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SliceOfLifePlayerState.generated.h"

UCLASS(Blueprintable)
class SLICEOFLIFE_API ASliceOfLifePlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ASliceOfLifePlayerState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Persistent per-player data
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
    float DamagePercent;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
    int32 Lives;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
    int32 Collectibles;
};


