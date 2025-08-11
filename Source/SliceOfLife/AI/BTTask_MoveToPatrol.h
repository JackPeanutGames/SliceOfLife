#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveToPatrol.generated.h"

UCLASS()
class SLICEOFLIFE_API UBTTask_MoveToPatrol : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_MoveToPatrol();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};


