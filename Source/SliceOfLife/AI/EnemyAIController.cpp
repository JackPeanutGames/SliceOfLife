#include "EnemyAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "SliceOfLife/Characters/EnemyBase.h"

const FName AEnemyAIController::TargetActorKey(TEXT("TargetActor"));
const FName AEnemyAIController::PatrolLocationKey(TEXT("PatrolLocation"));
const FName AEnemyAIController::IsInAttackRangeKey(TEXT("IsInAttackRange"));

AEnemyAIController::AEnemyAIController()
{
    BehaviorComp = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorComp"));
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (AEnemyBase* Enemy = Cast<AEnemyBase>(InPawn))
    {
        if (Enemy->GetBlackboardData() && BlackboardComp)
        {
            BlackboardComp->InitializeBlackboard(*const_cast<UBlackboardData*>(Enemy->GetBlackboardData()));
        }
        if (Enemy->GetBehaviorTree() && BehaviorComp)
        {
            BehaviorComp->StartTree(*const_cast<UBehaviorTree*>(Enemy->GetBehaviorTree()));
        }
    }
}


