#include "HitboxNotifyState.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "SliceOfLife/Components/CombatComponent.h"
#include "Components/BoxComponent.h"

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
            Combat->BeginAttackWindow();
            // Spawn a box hitbox attached to mesh so it mirrors flip automatically
            SpawnedHitbox = NewObject<UBoxComponent>(Owner);
            if (SpawnedHitbox)
            {
                SpawnedHitbox->AttachToComponent(MeshComp, FAttachmentTransformRules::KeepRelativeTransform);
                SpawnedHitbox->RegisterComponent();
                SpawnedHitbox->SetBoxExtent(BoxExtent, true);
                // Position from local offset (relative to mesh)
                const FVector RelLoc = LocalOffset;
                SpawnedHitbox->SetRelativeLocation(RelLoc);
                SpawnedHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                SpawnedHitbox->SetCollisionObjectType(ECC_WorldDynamic);
                SpawnedHitbox->SetGenerateOverlapEvents(true);
                SpawnedHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
                SpawnedHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

                SpawnedHitbox->OnComponentBeginOverlap.AddDynamic(Combat, &UCombatComponent::OnHitboxBeginOverlap);
            }
            return;
        }
    }
}

void USOL_HitboxNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr)
    {
        if (UCombatComponent* Combat = Owner->FindComponentByClass<UCombatComponent>())
        {
            Combat->EndAttackWindow();
        }
    }
    if (SpawnedHitbox)
    {
        SpawnedHitbox->DestroyComponent();
        SpawnedHitbox = nullptr;
    }
}


