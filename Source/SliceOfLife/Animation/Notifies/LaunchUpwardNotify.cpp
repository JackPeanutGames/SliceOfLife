#include "LaunchUpwardNotify.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SliceOfLife/Characters/PlayerCharacter.h"

ULaunchUpwardNotify::ULaunchUpwardNotify()
{
	// Set default values
	LaunchForce = 800.0f;
	bPreserveHorizontalVelocity = true;
	bAddForwardPush = false;
	ForwardPushForce = 200.0f;
	bEnableAirControl = true;
	AirControlDuration = 1.0f;
}

void ULaunchUpwardNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	// Get the character that owns this mesh
	ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (!Character)
	{
		return;
	}

	// Get the character movement component
	UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
	if (!MovementComp)
	{
		return;
	}

	// Calculate the launch velocity
	FVector LaunchVelocity = FVector::ZeroVector;

	// Add upward force
	LaunchVelocity.Z = LaunchForce;

	// Handle horizontal velocity
	if (bPreserveHorizontalVelocity)
	{
		// Preserve existing horizontal velocity
		FVector CurrentVelocity = MovementComp->Velocity;
		LaunchVelocity.X = CurrentVelocity.X;
		LaunchVelocity.Y = CurrentVelocity.Y;
	}

	// Add forward push if enabled
	if (bAddForwardPush)
	{
		FVector ForwardDirection = Character->GetActorForwardVector();
		LaunchVelocity += ForwardDirection * ForwardPushForce;
	}

	// Apply the launch
	MovementComp->Velocity = LaunchVelocity;

	// Enable air control if requested
	if (bEnableAirControl)
	{
		// Store original air control settings
		float OriginalAirControl = MovementComp->AirControl;
		float OriginalAirControlBoostMultiplier = MovementComp->AirControlBoostMultiplier;
		float OriginalAirControlBoostVelocityThreshold = MovementComp->AirControlBoostVelocityThreshold;

		// Enable enhanced air control
		MovementComp->AirControl = 1.0f;
		MovementComp->AirControlBoostMultiplier = 2.0f;
		MovementComp->AirControlBoostVelocityThreshold = 0.0f;

		// Set a timer to restore original air control settings
		FTimerHandle AirControlTimerHandle;
		Character->GetWorld()->GetTimerManager().SetTimer(
			AirControlTimerHandle,
			[MovementComp, OriginalAirControl, OriginalAirControlBoostMultiplier, OriginalAirControlBoostVelocityThreshold]()
			{
				if (MovementComp)
				{
					MovementComp->AirControl = OriginalAirControl;
					MovementComp->AirControlBoostMultiplier = OriginalAirControlBoostMultiplier;
					MovementComp->AirControlBoostVelocityThreshold = OriginalAirControlBoostVelocityThreshold;
				}
			},
			AirControlDuration,
			false
		);
	}

	// Log the launch for debugging
	UE_LOG(LogTemp, Log, TEXT("LaunchUpwardNotify: Launched character %s with force %.1f"), 
		*Character->GetName(), LaunchForce);

	// Special handling for PlayerCharacter if it's a custom class
	if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(Character))
	{
		// You can add any player-specific launch logic here
		// For example, triggering special effects, sounds, or state changes
		UE_LOG(LogTemp, Log, TEXT("LaunchUpwardNotify: Applied to PlayerCharacter %s"), *PlayerChar->GetName());
	}
}
