#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PerformAttack.generated.h"

UCLASS()
class SLICEOFLIFE_API UBTTask_PerformAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_PerformAttack();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};


