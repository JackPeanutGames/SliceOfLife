#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerMovementComponent.generated.h"

UENUM(BlueprintType)
enum class EMovementState : uint8
{
	Idle,
	Walking,
	Running,
	Jumping,
	Falling,
	Dashing,
	Crouching
};

USTRUCT(BlueprintType)
struct FMovementSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float JumpVelocity = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DoubleJumpVelocity = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DashSpeed = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DashDuration = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CrouchSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AirControl = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float FastFallMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float GravityScale = 1.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SLICEOFLIFE_API UPlayerMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UPlayerMovementComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Movement Input Functions
    // Deprecated: movement input is handled by ACharacter::AddMovementInput
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetMovementInput(FVector2D Input);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void JumpPressed();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void JumpReleased();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DashPressed();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void CrouchPressed();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void CrouchReleased();

	// State Queries
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsGrounded() const;

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsInAir() const;

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsDashing() const;

	UFUNCTION(BlueprintPure, Category = "Movement")
	int32 GetJumpCount() const;

	// Note: This overrides UNavMovementComponent::IsCrouching; do not use UFUNCTION here
	virtual bool IsCrouching() const override;

	UFUNCTION(BlueprintPure, Category = "Movement")
	EMovementState GetMovementState() const { return CurrentMovementState; }

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FMovementSettings MovementSettings;

protected:
	// Movement State
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	EMovementState CurrentMovementState;

	// Input
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector2D MovementInput;

	// Jump System
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	int32 JumpCount;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bCanJump;

	// Dash System
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsDashing;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float DashTimer;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FVector DashDirection;

	// Crouch System
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsCrouching;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float OriginalCapsuleHalfHeight;

	// Internal Functions
	void UpdateMovementState();
    void HandleJump();
    void HandleDash(float DeltaTime);
    void HandleCrouch();

    // Reset jump state when landing
    virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode = 0) override;
};
