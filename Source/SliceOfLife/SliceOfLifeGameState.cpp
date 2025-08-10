#include "SliceOfLifeGameState.h"
#include "Net/UnrealNetwork.h"

ASliceOfLifeGameState::ASliceOfLifeGameState()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentAreaName = TEXT("StartingArea");
    GlobalTimer = 0.f;
    GlobalScore = 0;
}

void ASliceOfLifeGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASliceOfLifeGameState, CurrentAreaName);
    DOREPLIFETIME(ASliceOfLifeGameState, GlobalTimer);
    DOREPLIFETIME(ASliceOfLifeGameState, GlobalScore);
}


