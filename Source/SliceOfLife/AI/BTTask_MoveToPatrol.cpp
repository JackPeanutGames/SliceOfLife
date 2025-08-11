#include "BTTask_MoveToPatrol.h"
#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTTask_MoveToPatrol::UBTTask_MoveToPatrol()
{
    NodeName = TEXT("Move To Patrol");
}

EBTNodeResult::Type UBTTask_MoveToPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AEnemyAIController* Controller = OwnerComp.GetAIOwner() ? Cast<AEnemyAIController>(OwnerComp.GetAIOwner()) : nullptr;
    UBlackboardComponent* BB = Controller ? Controller->GetBlackboardComponent() : nullptr;
    if (!Controller || !BB)
    {
        return EBTNodeResult::Failed;
    }

    const FVector PatrolLocation = BB->GetValueAsVector(AEnemyAIController::PatrolLocationKey);
    Controller->MoveToLocation(PatrolLocation, 50.f, true, true, true, false, 0, true);
    return EBTNodeResult::Succeeded; // fire-and-forget; service/next tick will choose next state
}


