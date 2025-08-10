#include "PlayerMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"

UPlayerMovementComponent::UPlayerMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
	
	// Initialize default values
	CurrentMovementState = EMovementState::Idle;
	JumpCount = 0;
	bCanJump = true;
	bIsDashing = false;
	bIsCrouching = false;
	DashTimer = 0.0f;
	MovementInput = FVector2D::ZeroVector;
	
	// Set default movement settings
	MovementSettings = FMovementSettings();
	
	// Configure character movement defaults
	MaxAcceleration = 2048.0f;
	BrakingDecelerationWalking = 2048.0f;
	GroundFriction = 8.0f;
	JumpZVelocity = MovementSettings.JumpVelocity;
	AirControl = MovementSettings.AirControl;
	GravityScale = MovementSettings.GravityScale;
}

void UPlayerMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (UCapsuleComponent* Capsule = OwnerCharacter->GetCapsuleComponent())
		{
			OriginalCapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
		}
	}
}

void UPlayerMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
    HandleDash(DeltaTime);
    UpdateMovementState();
    // Movement is handled by CharacterMovement + AddMovementInput in the Character
}

void UPlayerMovementComponent::SetMovementInput(FVector2D Input)
{
	// Clamp input magnitude to 1.0
	const double Size = Input.Size();
	MovementInput = Size > 1.0 ? Input / Size : Input;
}

void UPlayerMovementComponent::JumpPressed()
{
	if (bCanJump)
	{
		HandleJump();
	}
}

void UPlayerMovementComponent::JumpReleased()
{
    // Variable jump height using CharacterMovement's gravity: cut Z velocity when releasing
    if (IsFalling() && Velocity.Z > 0.0f)
    {
        Velocity.Z *= 0.5f;
    }
}

void UPlayerMovementComponent::DashPressed()
{
	if (!bIsDashing && IsGrounded() && !MovementInput.IsNearlyZero())
	{
		bIsDashing = true;
		DashTimer = MovementSettings.DashDuration;
		DashDirection = FVector(MovementInput.X, MovementInput.Y, 0.0f).GetSafeNormal();
		
        // Apply dash by setting a horizontal impulse
        FVector NewVel = DashDirection * MovementSettings.DashSpeed;
        NewVel.Z = Velocity.Z;
        Velocity = NewVel;
	}
}

void UPlayerMovementComponent::CrouchPressed()
{
	if (IsGrounded() && !bIsCrouching)
	{
		bIsCrouching = true;
		HandleCrouch();
	}
}

void UPlayerMovementComponent::CrouchReleased()
{
	if (bIsCrouching)
	{
		bIsCrouching = false;
		HandleCrouch();
	}
}

bool UPlayerMovementComponent::IsGrounded() const
{
	return IsMovingOnGround();
}

bool UPlayerMovementComponent::IsInAir() const
{
	return IsFalling();
}

bool UPlayerMovementComponent::IsDashing() const
{
	return bIsDashing;
}

bool UPlayerMovementComponent::IsCrouching() const
{
	return bIsCrouching;
}

void UPlayerMovementComponent::UpdateMovementState()
{
	if (bIsDashing)
	{
		CurrentMovementState = EMovementState::Dashing;
	}
	else if (bIsCrouching)
	{
		CurrentMovementState = EMovementState::Crouching;
	}
	else if (IsInAir())
	{
		CurrentMovementState = Velocity.Z > 0.0f ? EMovementState::Jumping : EMovementState::Falling;
	}
	else if (!MovementInput.IsNearlyZero())
	{
		CurrentMovementState = FMath::Abs(MovementInput.X) > 0.8f || FMath::Abs(MovementInput.Y) > 0.8f ? 
			EMovementState::Running : EMovementState::Walking;
	}
	else
	{
		CurrentMovementState = EMovementState::Idle;
	}
}

void UPlayerMovementComponent::HandleJump()
{
	if (IsGrounded())
	{
		// Ground jump
		JumpCount = 1;
		Velocity.Z = MovementSettings.JumpVelocity;
		bCanJump = false;
	}
	else if (JumpCount < 2)
	{
		// Double jump
		JumpCount++;
		Velocity.Z = MovementSettings.DoubleJumpVelocity;
	}
	
	// Reset jump ability when landing
	if (IsGrounded())
	{
		bCanJump = true;
		JumpCount = 0;
	}
}

void UPlayerMovementComponent::HandleDash(float DeltaTime)
{
	if (bIsDashing)
	{
		DashTimer -= DeltaTime;
		
		if (DashTimer <= 0.0f)
		{
			bIsDashing = false;
			// Apply some deceleration after dash
			Velocity *= 0.5f;
		}
	}
}

void UPlayerMovementComponent::HandleCrouch()
{
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (UCapsuleComponent* Capsule = OwnerCharacter->GetCapsuleComponent())
		{
            if (bIsCrouching)
            {
                Capsule->SetCapsuleHalfHeight(OriginalCapsuleHalfHeight * 0.5f);
                MaxWalkSpeed = MovementSettings.CrouchSpeed;
            }
            else
            {
                Capsule->SetCapsuleHalfHeight(OriginalCapsuleHalfHeight);
                MaxWalkSpeed = MovementSettings.WalkSpeed;
            }
		}
	}
}

// Manual velocity-based movement removed in favor of CharacterMovement input handling
