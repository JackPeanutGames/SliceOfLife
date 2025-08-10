#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SliceOfLife/Components/CombatComponent.h" // for EAttackState and FAttackData accessors
#include "PlayerCharacter.generated.h"

class UPlayerMovementComponent;
class UCombatComponent;
class UHealthComponent;
class UInputMappingContext;
class UInputAction;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class SLICEOFLIFE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
    APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_Controller() override;

protected:
		// Placeholder visual (Brocky): simple sphere body sized to 1m tall
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
		UStaticMeshComponent* PlaceholderBody;

	// Enhanced Input Actions
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LightAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* TiltAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* AerialAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SmashAttackAction;

	// Input Handlers
	UFUNCTION()
	void OnMove(const FInputActionValue& Value);

	UFUNCTION()
	void OnJump(const FInputActionValue& Value);

	UFUNCTION()
	void OnJumpReleased(const FInputActionValue& Value);

	UFUNCTION()
	void OnDash(const FInputActionValue& Value);

	UFUNCTION()
	void OnCrouch(const FInputActionValue& Value);

	UFUNCTION()
	void OnCrouchReleased(const FInputActionValue& Value);

	UFUNCTION()
	void OnLightAttack(const FInputActionValue& Value);

	UFUNCTION()
	void OnTiltAttack(const FInputActionValue& Value);

	UFUNCTION()
	void OnAerialAttack(const FInputActionValue& Value);

	UFUNCTION()
	void OnSmashAttackStart(const FInputActionValue& Value);

	UFUNCTION()
	void OnSmashAttackRelease(const FInputActionValue& Value);

	// Health/Hitstun delegate handlers
    // Must be UFUNCTION for AddDynamic binding
    UFUNCTION()
    void OnHealthChanged(float NewHealth);

    // Must be UFUNCTION for AddDynamic binding
    UFUNCTION()
    void OnHitstunChanged(bool bInHitstun);

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPlayerMovementComponent* PlayerMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	// Camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;

	// Movement State
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsMoving;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsRunning;

	// Combat State
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsAttacking;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsCharging;

    // Internal C++ hit detection trackers
    EAttackState PreviousAttackState;
    bool bHasHitThisSwing;

	// Health State
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	bool bIsInHitstun;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float CurrentHealthPercent;

	// Internal
	void UpdateCombatState();
    void AddDefaultMappingContext();

public:
	// Component Getters
	UFUNCTION(BlueprintPure, Category = "Components")
	UPlayerMovementComponent* GetPlayerMovementComponent() const { return PlayerMovementComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	UCombatComponent* GetCombatComponent() const { return CombatComponent; }

	UFUNCTION(BlueprintPure, Category = "Components")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	// State Getters
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsMoving() const { return bIsMoving; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsRunning() const { return bIsRunning; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttacking() const { return bIsAttacking; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsCharging() const { return bIsCharging; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsInHitstun() const { return bIsInHitstun; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const { return CurrentHealthPercent; }
};
