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
};


