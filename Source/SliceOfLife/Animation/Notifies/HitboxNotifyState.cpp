#include "HitboxNotifyState.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "SliceOfLife/Components/CombatComponent.h"

void USOL_HitboxNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp)
    {
        return;
    }
    if (AActor* Owner = MeshComp->GetOwner())
    {
        if (UCombatComponent* Combat = Owner->FindComponentByClass<UCombatComponent>())
        {
            Combat->SpawnHitboxParams(LocalOffset, BoxExtent, Damage, KnockbackForce);
            return;
        }
    }
}

void USOL_HitboxNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    // Nothing to tear down for simple one-shot hitbox; kept for extensibility
}


