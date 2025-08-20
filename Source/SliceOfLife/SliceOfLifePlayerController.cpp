#include "SliceOfLifePlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SliceOfLifePlayerState.h"

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

    // Also support native debug widgets if classes are assigned
    CreateDebugWidgetsNative();
}

void ASliceOfLifePlayerController::UpdatePlayerStateDisplay(float DamagePercent, const FString& StateLabel, FVector2D Velocity)
{
    // Implemented in BP. This native function serves as a callable hook from C++ if needed
}

void ASliceOfLifePlayerController::UpdatePlayerHealthDisplay(float HealthPercent)
{
    // Update the health display in the UI
    // This will be implemented in Blueprint to update the health bar
    UE_LOG(LogTemp, Log, TEXT("Player health updated: %.1f%%"), HealthPercent * 100.0f);
}

void ASliceOfLifePlayerController::CreateDebugWidgetsNative()
{
    if (StateDisplayClass && !StateDisplayWidget)
    {
        StateDisplayWidget = CreateWidget<UUserWidget>(this, StateDisplayClass);
        if (StateDisplayWidget)
        {
            StateDisplayWidget->AddToViewport(0);
        }
    }
    if (TuningPanelClass && !TuningPanelWidget)
    {
        TuningPanelWidget = CreateWidget<UUserWidget>(this, TuningPanelClass);
        if (TuningPanelWidget)
        {
            TuningPanelWidget->AddToViewport(1);
        }
    }

    if (ASliceOfLifePlayerState* SOLPS = GetPlayerState<ASliceOfLifePlayerState>())
    {
        SOLPS->OnDamagePercentChanged.AddDynamic(this, &ASliceOfLifePlayerController::OnPlayerDamagePercentChanged);
    }
}

void ASliceOfLifePlayerController::OnGravityChanged(float NewValue)
{
    if (ACharacter* Char = GetCharacter())
    {
        if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
        {
            Move->GravityScale = NewValue;
        }
    }
}

void ASliceOfLifePlayerController::OnFrictionChanged(float NewValue)
{
    if (ACharacter* Char = GetCharacter())
    {
        if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
        {
            Move->GroundFriction = NewValue;
        }
    }
}

void ASliceOfLifePlayerController::OnPlayerSpeedChanged(float NewValue)
{
    if (ACharacter* Char = GetCharacter())
    {
        if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
        {
            Move->MaxWalkSpeed = NewValue;
        }
    }
}

void ASliceOfLifePlayerController::OnPlayerDamagePercentChanged(float NewDamagePercent)
{
    FVector2D Vel2D = FVector2D::ZeroVector;
    FString StateLabel = TEXT("");
    if (ACharacter* Char = GetCharacter())
    {
        const FVector Vel = Char->GetVelocity();
        Vel2D = FVector2D(Vel.X, Vel.Z);
    }
    UpdatePlayerStateDisplay(NewDamagePercent, StateLabel, Vel2D);
}


