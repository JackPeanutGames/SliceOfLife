#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SliceOfLife/Components/HealthComponent.h"
#include "SliceOfLife/Components/CombatComponent.h"
#include "EnemyBase.generated.h"

class UBehaviorTree;
class UBlackboardData;
class UPawnSensingComponent;
class UStaticMeshComponent;

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

	// Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy Events")
	void OnEnemyStateChanged(EEnemyState OldState, EEnemyState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy Events")
	void OnEnemyDamaged(float Damage, AActor* DamageCauser);

	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy Events")
	void OnEnemyDeath();

protected:
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy AI")
	UPawnSensingComponent* PawnSensingComponent;

    // Placeholder visual: 2m tall human enemy, sphere body (2m diameter) with a small head sphere
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
    UStaticMeshComponent* PlaceholderBody;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
    UStaticMeshComponent* PlaceholderHead;

	// AI Behavior Tree
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI")
	UBehaviorTree* BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy AI")
	UBlackboardData* BlackboardData;

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

    // Must be UFUNCTION for AddDynamic binding
    UFUNCTION()
    void OnPawnSeen(APawn* SeenPawn);

    // Must be UFUNCTION for AddDynamic binding (matches FOnDamageReceived signature)
    UFUNCTION()
    void OnDamageReceived(float Damage, FVector KnockbackDirection, float KnockbackForce);

	// Movement speed helper
	float GetMovementSpeed() const;
};
