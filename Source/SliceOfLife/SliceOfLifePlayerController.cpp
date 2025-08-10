#include "SliceOfLifePlayerController.h"
#include "EnhancedInputSubsystems.h"

void ASliceOfLifePlayerController::BeginPlay()
{
    Super::BeginPlay();
    // Ensure Enhanced Input subsystem exists; character will add mapping context
    if (ULocalPlayer* LP = GetLocalPlayer())
    {
        UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP);
        (void)Subsystem; // intentionally unused beyond ensuring creation
    }

    // Let BP create and add the widgets
    CreateAndAddDebugWidgets();
}

void ASliceOfLifePlayerController::UpdatePlayerStateDisplay(float DamagePercent, const FString& StateLabel, FVector2D Velocity)
{
    // Implemented in BP. This native function serves as a callable hook from C++ if needed
}


