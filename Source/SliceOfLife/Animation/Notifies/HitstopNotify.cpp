#include "HitstopNotify.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "SliceOfLife/Components/HealthComponent.h"

void USOL_HitstopNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp)
    {
        return;
    }
    if (AActor* Owner = MeshComp->GetOwner())
    {
        if (UHealthComponent* Health = Owner->FindComponentByClass<UHealthComponent>())
        {
            // Reuse hitstun system to approximate hitstop on attacker or victim as needed
            Health->SetHitstun(Duration);
        }
    }
}


