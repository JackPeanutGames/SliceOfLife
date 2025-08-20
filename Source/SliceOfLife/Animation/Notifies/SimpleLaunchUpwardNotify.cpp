#include "SimpleLaunchUpwardNotify.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"

USimpleLaunchUpwardNotify::USimpleLaunchUpwardNotify()
{
	// Set default values
	LaunchForce = 800.0f;
	bPreserveHorizontalVelocity = true;
}

void USimpleLaunchUpwardNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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

	// Apply the launch
	MovementComp->Velocity = LaunchVelocity;

	// Log the launch for debugging
	UE_LOG(LogTemp, Log, TEXT("SimpleLaunchUpwardNotify: Launched character %s with force %.1f"), 
		*Character->GetName(), LaunchForce);
}
