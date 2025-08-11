#include "BTService_CheckPlayerDistance.h"
#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "SliceOfLife/Characters/EnemyBase.h"
#include "Kismet/GameplayStatics.h"

UBTService_CheckPlayerDistance::UBTService_CheckPlayerDistance()
{
    bNotifyBecomeRelevant = true;
    bNotifyCeaseRelevant = false;
    Interval = 0.2f;
    RandomDeviation = 0.0f;
}

void UBTService_CheckPlayerDistance::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    AEnemyAIController* Controller = OwnerComp.GetAIOwner() ? Cast<AEnemyAIController>(OwnerComp.GetAIOwner()) : nullptr;
    UBlackboardComponent* BB = Controller ? Controller->GetBlackboardComponent() : nullptr;
    APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;

    if (!BB || !Pawn)
    {
        return;
    }

    // Find nearest player pawn
    AActor* NearestPlayer = nullptr;
    float NearestDistSq = FLT_MAX;
    for (FConstPlayerControllerIterator It = Pawn->GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (APlayerController* PC = It->Get())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                const float DistSq = FVector::DistSquared(PlayerPawn->GetActorLocation(), Pawn->GetActorLocation());
                if (DistSq < NearestDistSq)
                {
                    NearestDistSq = DistSq;
                    NearestPlayer = PlayerPawn;
                }
            }
        }
    }

    BB->SetValueAsObject(AEnemyAIController::TargetActorKey, NearestPlayer);

    bool bInRange = false;
    if (AEnemyBase* Enemy = Cast<AEnemyBase>(Pawn))
    {
        if (NearestPlayer)
        {
            bInRange = (FVector::Dist(NearestPlayer->GetActorLocation(), Pawn->GetActorLocation()) <= Enemy->GetEnemyStats().AttackRange);
        }
    }
    BB->SetValueAsBool(AEnemyAIController::IsInAttackRangeKey, bInRange);
}

