#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveToTarget.generated.h"

UCLASS()
class SLICEOFLIFE_API UBTTask_MoveToTarget : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_MoveToTarget();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};


