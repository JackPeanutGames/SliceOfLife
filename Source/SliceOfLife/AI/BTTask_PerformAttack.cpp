#include "BTTask_PerformAttack.h"
#include "EnemyAIController.h"
#include "SliceOfLife/Characters/EnemyBase.h"

UBTTask_PerformAttack::UBTTask_PerformAttack()
{
    NodeName = TEXT("Perform Attack");
}

EBTNodeResult::Type UBTTask_PerformAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    if (AEnemyAIController* Controller = OwnerComp.GetAIOwner() ? Cast<AEnemyAIController>(OwnerComp.GetAIOwner()) : nullptr)
    {
        if (AEnemyBase* Enemy = Cast<AEnemyBase>(Controller->GetPawn()))
        {
            Enemy->PerformAttack();
            return EBTNodeResult::Succeeded; // montage end transitions handled in enemy class
        }
    }
    return EBTNodeResult::Failed;
}


