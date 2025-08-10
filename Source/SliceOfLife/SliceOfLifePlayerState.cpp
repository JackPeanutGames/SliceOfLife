#include "SliceOfLifePlayerState.h"
#include "Net/UnrealNetwork.h"

ASliceOfLifePlayerState::ASliceOfLifePlayerState()
{
    DamagePercent = 0.f;
    Lives = 3;
    Collectibles = 0;
}

void ASliceOfLifePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASliceOfLifePlayerState, DamagePercent);
    DOREPLIFETIME(ASliceOfLifePlayerState, Lives);
    DOREPLIFETIME(ASliceOfLifePlayerState, Collectibles);
}

void ASliceOfLifePlayerState::SetDamagePercent(float NewPercent)
{
    DamagePercent = FMath::Max(0.f, NewPercent);
    OnDamagePercentChanged.Broadcast(DamagePercent);
}

void ASliceOfLifePlayerState::AddDamagePercent(float DeltaPercent)
{
    SetDamagePercent(DamagePercent + DeltaPercent);
}


