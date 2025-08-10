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
}


