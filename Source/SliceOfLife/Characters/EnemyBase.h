#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SliceOfLife/Components/HealthComponent.h"
#include "SliceOfLife/Components/CombatComponent.h"
#include "SliceOfLife/Weapons/WeaponBase.h"
#include "EnemyBase.generated.h"

class UBehaviorTree;
class UBlackboardData;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UStaticMeshComponent;
class USphereComponent;
class UCapsuleComponent;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle,
	Patrolling,
	Chasing,
	Attacking,
	Stunned,
	Dead
};

USTRUCT(BlueprintType)
struct FEnemyStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	float PatrolSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	float ChaseSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	float DetectionRange = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	float AttackRange = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	float PatrolRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	float StunDuration = 1.0f;
};

UCLASS(Blueprintable)
class SLICEOFLIFE_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
    

	// AI Behavior
	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void SetEnemyState(EEnemyState NewState);

	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void StartPatrolling();

	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void StartChasing(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void StartAttacking(AActor* Target);

	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void StunEnemy(float Duration);

	// State Queries
	UFUNCTION(BlueprintPure, Category = "Enemy AI")
	EEnemyState GetEnemyState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Enemy AI")
	bool IsStunned() const { return CurrentState == EEnemyState::Stunned; }

	UFUNCTION(BlueprintPure, Category = "Enemy AI")
	bool IsDead() const { return CurrentState == EEnemyState::Dead; }

	UFUNCTION(BlueprintPure, Category = "Enemy AI")
	AActor* GetCurrentTarget() const { return CurrentTarget; }

	// Movement
	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void MoveToLocation(FVector Location, float AcceptanceRadius = 50.0f);

	UFUNCTION(BlueprintCallable, Category = "Enemy AI")
	void StopMovement();

	// Combat
	UFUNCTION(BlueprintCallable, Category = "Enemy Combat")
	void PerformAttack();

	UFUNCTION(BlueprintCallable, Category = "Enemy Combat")
	bool CanAttack() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ReactToHit();

    // AI Controller & BT
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI")
    TSubclassOf<class AAIController> AIControllerClassOverride;

    // Accessors for AI assets and tunables
    UFUNCTION(BlueprintPure, Category = "Enemy AI")
    const UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

    UFUNCTION(BlueprintPure, Category = "Enemy AI")
    const UBlackboardData* GetBlackboardData() const { return BlackboardData; }

    UFUNCTION(BlueprintPure, Category = "Enemy Stats")
    const FEnemyStats& GetEnemyStats() const { return EnemyStats; }

	// Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy Events")
	void OnEnemyStateChanged(EEnemyState OldState, EEnemyState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy Events")
	void OnEnemyDamaged(float Damage, AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy Events")
	void OnEnemyDeath();

	// Hit tracking and death management
	UFUNCTION(BlueprintCallable, Category = "Enemy Combat")
	void ResetSwingHits();
	
	UFUNCTION(BlueprintCallable, Category = "Enemy Combat")
	void Die();
	
	// Debug and info methods
	UFUNCTION(BlueprintPure, Category = "Enemy|Debug")
	int32 GetRemainingDrops() const { return RemainingDrops; }

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCombatComponent* CombatComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY()
    UAISenseConfig_Sight* SightConfig;

    // Placeholder visual: 2m tall human enemy, sphere body (2m diameter) with a small head sphere
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
    UStaticMeshComponent* PlaceholderBody;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
    UStaticMeshComponent* PlaceholderHead;

    // Body part colliders for hit detection
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hitboxes")
    USphereComponent* HeadCollider;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hitboxes")
    UCapsuleComponent* TorsoCollider;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hitboxes")
    UCapsuleComponent* LegCollider;

    // Weapon visual attached to socket (e.g., Skewer)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FName WeaponSocketName = TEXT("SkewerSocket");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    UStaticMesh* WeaponMesh;

    UPROPERTY()
    UStaticMeshComponent* WeaponMeshComponent;

    // Item drops
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drops")
    class UDataTable* ItemDropTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drops")
    int32 HitsRemaining = 2;

	// AI Behavior Tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI")
	UBehaviorTree* BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI")
	UBlackboardData* BlackboardData;

    // Animation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Combat")
    class UAnimMontage* AttackMontage;

    // 2.5D constraint toggle — if true, constrains movement to Y=0 plane
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|PlaneConstraint")
    bool bConstrainToYPlane = true;

	// Enemy State
	UPROPERTY(BlueprintReadOnly, Category = "Enemy AI")
	EEnemyState CurrentState;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy AI")
	AActor* CurrentTarget;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy AI")
	float StateTimer;

	// Patrol System
	UPROPERTY(BlueprintReadOnly, Category = "Enemy AI")
	FVector PatrolCenter;

	UPROPERTY(BlueprintReadOnly, Category = "Enemy AI")
	FVector CurrentPatrolTarget;

	// Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Stats")
	FEnemyStats EnemyStats;

	// Internal Functions
	void UpdateAI(float DeltaTime);
	void UpdatePatrol(float DeltaTime);
	void UpdateChase(float DeltaTime);
	void UpdateAttack(float DeltaTime);
	void UpdateStun(float DeltaTime);
    void FindNewPatrolTarget();
    bool IsTargetInRange(AActor* Target, float Range) const;

    // Perception callback
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Debug draw hurt box
    void DebugDrawHurtCapsule();
    void DebugDrawBodyColliders();

    // Must be UFUNCTION for AddDynamic binding (matches FOnDamageReceived signature)
    UFUNCTION()
    void OnDamageReceived(float Damage, FVector KnockbackDirection, float KnockbackForce);

	// Movement speed helper
	float GetMovementSpeed() const;

	FTimerHandle HitReactTimerHandle;

    // Debug hit timestamps for red flash
    float LastHeadHitTime = -1000.f;
    float LastTorsoHitTime = -1000.f;
    float LastLegHitTime = -1000.f;
    
    // Track weapons that have hit this enemy during the current attack swing
    TSet<TWeakObjectPtr<AWeaponBase>> HitByWeaponsThisSwing;
    
    // Track remaining drops for this enemy
    int32 RemainingDrops = 0;

protected:
    UFUNCTION()
    void OnBodyPartOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
