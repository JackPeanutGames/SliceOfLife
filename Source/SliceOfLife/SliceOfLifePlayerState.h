#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Delegates/DelegateCombinations.h"
#include "SliceOfLifePlayerState.generated.h"

UCLASS(Blueprintable)
class SLICEOFLIFE_API ASliceOfLifePlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ASliceOfLifePlayerState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamagePercentChanged, float, NewDamagePercent);

    // Persistent per-player data
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
    float DamagePercent;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
    int32 Lives;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerState")
    int32 Collectibles;

    UFUNCTION(BlueprintCallable, Category = "PlayerState")
    void SetDamagePercent(float NewPercent);

    UFUNCTION(BlueprintCallable, Category = "PlayerState")
    void AddDamagePercent(float DeltaPercent);

    UPROPERTY(BlueprintAssignable, Category = "PlayerState")
    FOnDamagePercentChanged OnDamagePercentChanged;
};


