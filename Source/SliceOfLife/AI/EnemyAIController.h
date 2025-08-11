#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;

UCLASS()
class SLICEOFLIFE_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController();

    virtual void OnPossess(APawn* InPawn) override;

    UBlackboardComponent* GetBlackboardComponent() const { return BlackboardComp; }
    UBehaviorTreeComponent* GetBehaviorComponent() const { return BehaviorComp; }

    // Blackboard keys used by BT assets
    static const FName TargetActorKey;
    static const FName PatrolLocationKey;
    static const FName IsInAttackRangeKey;

protected:
    UPROPERTY(Transient)
    UBehaviorTreeComponent* BehaviorComp;

    UPROPERTY(Transient)
    UBlackboardComponent* BlackboardComp;
};


