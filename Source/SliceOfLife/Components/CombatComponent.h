#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "CombatComponent.generated.h"

class UHealthComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	Light,
	Tilt,
	Aerial,
	Smash
};

UENUM(BlueprintType)
enum class EAttackDirection : uint8
{
    Up,
    UpDiagonal,
    Forward,
    DownDiagonal,
    Down
};

UENUM(BlueprintType)
enum class EAttackState : uint8
{
	Idle,
	Charging,
	Attacking,
	Recovery
};

USTRUCT(BlueprintType)
struct FAttackData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FString AttackName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	EAttackType AttackType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float KnockbackForce = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FVector KnockbackDirection = FVector(1.0f, 0.0f, 0.5f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float HitstunDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float AttackDuration = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float RecoveryDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float ChargeTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float MaxChargeMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FVector HitboxOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FVector HitboxSize = FVector(50.0f, 50.0f, 50.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float HitboxDuration = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	FString AnimationMontageName;
};

USTRUCT(BlueprintType)
struct FMoveStaleData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Move Staling")
	FString MoveName;

	UPROPERTY(BlueprintReadOnly, Category = "Move Staling")
	int32 UseCount;

	UPROPERTY(BlueprintReadOnly, Category = "Move Staling")
	float LastUseTime;

	UPROPERTY(BlueprintReadOnly, Category = "Move Staling")
	float StaleMultiplier;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SLICEOFLIFE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Attack Input Functions
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void LightAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void TiltAttack(FVector2D Direction);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void AerialAttack();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SmashAttackStart();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SmashAttackRelease();

	// Combat State Queries
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool CanAttack() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttacking() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsCharging() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	EAttackState GetAttackState() const { return CurrentAttackState; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetChargeProgress() const;

    // C++ access to current attack data
    const FAttackData& GetCurrentAttack() const { return CurrentAttack; }

	// Move Staling
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ResetMoveStaling();

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetStaleMultiplier(const FString& MoveName) const;

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UDataTable* AttackDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float StaleDecayTime = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MinStaleMultiplier = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bEnableMoveStaling = true;

    // Animation montages for each attack type (assigned by designers)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
    class UAnimMontage* LightMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
    class UAnimMontage* TiltMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
    class UAnimMontage* AerialMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
    class UAnimMontage* SmashMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation")
    float MontagePlayRate = 1.0f;

    // Directional variants (optional). If not set, falls back to base montages above
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|Directional")
    class UAnimMontage* LightUpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|Directional")
    class UAnimMontage* LightForwardMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|Directional")
    class UAnimMontage* LightDownMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|Directional")
    class UAnimMontage* AerialUpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|Directional")
    class UAnimMontage* AerialForwardMontage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|Animation|Directional")
    class UAnimMontage* AerialDownMontage;

public:
    // Exposed helpers for animation notifies
    UFUNCTION(BlueprintCallable, Category = "Combat|Hitbox")
    void SpawnHitboxParams(const FVector& LocalOffset, const FVector& BoxExtent, float Damage, float KnockbackForce);

    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void EndAttackNow();

    // Notify-driven attack windows and overlap callback must be public for binding
    UFUNCTION()
    void OnHitboxBeginOverlap(class UPrimitiveComponent* OverlappedComp, AActor* OtherActor, class UPrimitiveComponent* OtherComp,
                              int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void BeginAttackWindow();

    UFUNCTION(BlueprintCallable, Category = "Combat|State")
    void EndAttackWindow();

protected:
	// Combat State
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	EAttackState CurrentAttackState;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FAttackData CurrentAttack;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float AttackTimer;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float ChargeTimer;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float CurrentChargeMultiplier;

	// Move Staling
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TArray<FMoveStaleData> StaleData;

	// Internal Functions
	void StartAttack(const FAttackData& AttackData);
	void UpdateAttack(float DeltaTime);
	void EndAttack();
  void DetectHits();
	float CalculateStaleMultiplier(const FString& MoveName);
	void UpdateMoveStaling(float DeltaTime);
	FAttackData* GetAttackData(EAttackType AttackType, const FVector2D& Direction = FVector2D::ZeroVector);

private:
    // Directional selection
    UPROPERTY(Transient)
    EAttackDirection CurrentAttackDirection = EAttackDirection::Forward;

    // Temporary hitbox parameters for overlap callback
    FVector PendingHitboxLocalOffset;
    FVector PendingHitboxExtent;
    float PendingHitboxDamage = 0.f;
    float PendingHitboxKnockback = 0.f;

    // One-hit-per-swing tracking
    TSet<TWeakObjectPtr<AActor>> ActorsHitThisSwing;


public:
    // Debug drawing for hitboxes/hurtboxes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bEnableHitboxDebug = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugHitboxDuration = 0.2f;

    // Optional hit impact FX (Cascade). If set, will be spawned at impact point on overlap
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|VFX")
    class UParticleSystem* HitImpactFX;

    /** When true, spawned attack hitboxes will be visible in game. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Debug)
    bool bShowOffensiveHitboxes = false;
};
