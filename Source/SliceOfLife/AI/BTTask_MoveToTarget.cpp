#include "BTTask_MoveToTarget.h"
#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_MoveToTarget::UBTTask_MoveToTarget()
{
    NodeName = TEXT("Move To Target");
}

EBTNodeResult::Type UBTTask_MoveToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AEnemyAIController* Controller = OwnerComp.GetAIOwner() ? Cast<AEnemyAIController>(OwnerComp.GetAIOwner()) : nullptr;
    UBlackboardComponent* BB = Controller ? Controller->GetBlackboardComponent() : nullptr;
    if (!Controller || !BB)
    {
        return EBTNodeResult::Failed;
    }

    if (AActor* Target = Cast<AActor>(BB->GetValueAsObject(AEnemyAIController::TargetActorKey)))
    {
        Controller->MoveToLocation(Target->GetActorLocation(), 75.f, true, true, true, false, 0, true);
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}


