#include "EndAttackNotify.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "SliceOfLife/Components/CombatComponent.h"

void USOL_EndAttackNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp)
    {
        return;
    }
    if (AActor* Owner = MeshComp->GetOwner())
    {
        if (UCombatComponent* Combat = Owner->FindComponentByClass<UCombatComponent>())
        {
            Combat->EndAttackNow();
        }
    }
}


