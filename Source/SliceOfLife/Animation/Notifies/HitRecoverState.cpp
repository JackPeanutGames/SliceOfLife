#include "HitRecoverState.h"
#include "Components/SkeletalMeshComponent.h"
#include "SliceOfLife/Characters/EnemyBase.h"

void UHitRecoverState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    if (MeshComp)
    {
        if (AEnemyBase* Enemy = Cast<AEnemyBase>(MeshComp->GetOwner()))
        {
            Enemy->SetRecovering(true);
        }
    }
}

void UHitRecoverState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (MeshComp)
    {
        if (AEnemyBase* Enemy = Cast<AEnemyBase>(MeshComp->GetOwner()))
        {
            Enemy->SetRecovering(false);
        }
    }
}


