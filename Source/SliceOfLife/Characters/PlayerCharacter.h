#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SliceOfLife/Components/CombatComponent.h" // for EAttackState and FAttackData accessors
#include "SliceOfLife/Items/ItemDropTypes.h" // for ECategory enum
#include "PlayerCharacter.generated.h"

class UPlayerMovementComponent;
class UCombatComponent;
class UHealthComponent;
class UInputMappingContext;
class UInputAction;
class UStaticMeshComponent;
class AWeaponBase;

UCLASS(Blueprintable)
class SLICEOFLIFE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
    APlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
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

    // 2.5D constraint toggle — if true, constrains movement to Y=0 plane
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|PlaneConstraint")
    bool bConstrainToYPlane = true;

    // Facing state for 2.5D
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Facing")
    bool bFacingRight = true;

    UFUNCTION(BlueprintCallable, Category = "Facing")
    void SetFacing(bool bRight);

    // Designer-tunable movement settings applied to CharacterMovement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tuning")
    float DesignerMaxWalkSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tuning")
    float DesignerMaxWalkSpeedCrouched = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tuning")
    float DesignerJumpZVelocity = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tuning")
    float DesignerAirControl = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Tuning")
    float DesignerGravityScale = 1.0f;

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

    // Input lock flag (driven by anim notifies or montage events)
    UPROPERTY(BlueprintReadOnly, Category = "Input")
    bool bIsInputLocked = false;

	// Health State
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	bool bIsInHitstun;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	float CurrentHealthPercent;

	// Internal
	void UpdateCombatState();
    void AddDefaultMappingContext();
    void ApplyPlaneConstraintSettings();

    // Weapons
    UPROPERTY(EditDefaultsOnly, Category = "Weapon")
    TSubclassOf<AWeaponBase> SkewerClass;

    UPROPERTY(VisibleAnywhere, Category = "Weapon")
    AWeaponBase* CurrentWeapon;

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

    // Facing helpers for gameplay math (world X +/-)
    UFUNCTION(BlueprintCallable, Category = "Facing")
    FVector GetFacingVector() const { return FVector(bFacingRight ? 1.f : -1.f, 0.f, 0.f); }

    UFUNCTION(BlueprintCallable, Category = "Facing")
    int32 GetFacingSign() const { return bFacingRight ? 1 : -1; }

    // Input locking during attack montages
    UFUNCTION(BlueprintCallable, Category = "Input")
    void SetInputLocked(bool bLocked) { bIsInputLocked = bLocked; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsInHitstun() const { return bIsInHitstun; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const { return CurrentHealthPercent; }

	// Inventory Counters
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 NumLimbs = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 NumAppendages = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 NumOrgans = 0;

	// Inventory Methods
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddItemToInventory(ECategory Category);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetNumLimbs() const { return NumLimbs; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetNumAppendages() const { return NumAppendages; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetNumOrgans() const { return NumOrgans; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetTotalInventoryCount() const { return NumLimbs + NumAppendages + NumOrgans; }

	// Debug method to reset inventory (useful for testing)
	UFUNCTION(BlueprintCallable, Category = "Inventory|Debug")
	void ResetInventory();

	// Debug method to display current inventory status
	UFUNCTION(BlueprintCallable, Category = "Inventory|Debug")
	void DisplayInventoryStatus();

	// Get formatted inventory string for UI display
	UFUNCTION(BlueprintPure, Category = "Inventory|UI")
	FString GetInventoryDisplayString() const;

	// Check if player has enough items for recipes/quests
	UFUNCTION(BlueprintPure, Category = "Inventory|Quests")
	bool HasEnoughItems(int32 RequiredLimbs, int32 RequiredAppendages, int32 RequiredOrgans) const;

	// Consume items for crafting (returns false if not enough items)
	UFUNCTION(BlueprintCallable, Category = "Inventory|Crafting")
	bool ConsumeItems(int32 LimbsToConsume, int32 AppendagesToConsume, int32 OrgansToConsume);
};
