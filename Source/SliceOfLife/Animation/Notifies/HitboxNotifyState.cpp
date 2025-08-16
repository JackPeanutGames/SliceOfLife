#include "HitboxNotifyState.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
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
            Combat->SpawnHitboxParams(LocalOffset, BoxExtent, Damage, KnockbackForce);
            // Spawn a box hitbox attached to mesh so it mirrors flip automatically
            SpawnedHitbox = NewObject<UBoxComponent>(Owner);
            if (SpawnedHitbox)
            {
                SpawnedHitbox->AttachToComponent(MeshComp, FAttachmentTransformRules::KeepRelativeTransform);
                SpawnedHitbox->RegisterComponent();
                // Make the hitbox visible and bright red for debug visibility
                SpawnedHitbox->ShapeColor = FColor::Red;
                SpawnedHitbox->SetHiddenInGame(false);
                SpawnedHitbox->SetBoxExtent(BoxExtent, true);
                // Position from local offset (relative to mesh, henc X instead of Y) but center on the 2.5D plane (X=0)
                FVector AdjustedOffset = LocalOffset;
                AdjustedOffset.X = 0.f;
                SpawnedHitbox->SetRelativeLocation(AdjustedOffset);
                SpawnedHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                SpawnedHitbox->SetCollisionObjectType(ECC_WorldDynamic);
                SpawnedHitbox->SetGenerateOverlapEvents(false);
                SpawnedHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
                SpawnedHitbox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
                SpawnedHitbox->IgnoreActorWhenMoving(Owner, true);
                SpawnedHitbox->UpdateOverlaps();

                FTimerHandle EnableCollisionHandle;
                UWorld* World = SpawnedHitbox->GetWorld();
                World->GetTimerManager().SetTimer(
                    EnableCollisionHandle,
                    [this]()
                    {
                        if (SpawnedHitbox)
                        {
                            SpawnedHitbox->SetGenerateOverlapEvents(true);
                            SpawnedHitbox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                        }
                    },
                    0.1f,
                    false);
                
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


