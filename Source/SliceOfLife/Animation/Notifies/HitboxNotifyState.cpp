#include "HitboxNotifyState.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "SliceOfLife/Components/CombatComponent.h"
#include "SliceOfLife/Characters/PlayerCharacter.h"
#include "SliceOfLife/Weapons/WeaponBase.h"
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
            // If the owner has an equipped weapon, enable its hitbox instead of spawning generic
            bool bEnabledWeaponHitbox = false;
            if (APlayerCharacter* Player = Cast<APlayerCharacter>(Owner))
            {
                TArray<AActor*> Attached;
                Player->GetAttachedActors(Attached);
                for (AActor* A : Attached)
                {
                    if (AWeaponBase* Weapon = Cast<AWeaponBase>(A))
                    {
                        Weapon->SetAttackParams(Damage, KnockbackForce);
                        Weapon->EnableHitbox();
                        bEnabledWeaponHitbox = true;
                        break;
                    }
                }
            }
            if (!bEnabledWeaponHitbox)
            {
                // Fist fallback: spawn generic hitbox (no weapon equipped)
                Combat->SpawnHitboxParams(LocalOffset, BoxExtent, Damage, KnockbackForce);
                SpawnedHitbox = NewObject<UBoxComponent>(Owner);
                if (SpawnedHitbox)
                {
                    SpawnedHitbox->AttachToComponent(MeshComp, FAttachmentTransformRules::KeepRelativeTransform);
                    SpawnedHitbox->RegisterComponent();
                    SpawnedHitbox->ShapeColor = FColor::Red;
                    SpawnedHitbox->SetHiddenInGame(false);
                    SpawnedHitbox->SetBoxExtent(BoxExtent, true);
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
        // If a weapon was used for the hitbox, disable it now
        if (APlayerCharacter* Player = Cast<APlayerCharacter>(Owner))
        {
            TArray<AActor*> Attached;
            Player->GetAttachedActors(Attached);
            for (AActor* A : Attached)
            {
                if (AWeaponBase* Weapon = Cast<AWeaponBase>(A))
                {
                    Weapon->DisableHitbox();
                    break;
                }
            }
        }
    }
    if (SpawnedHitbox)
    {
        SpawnedHitbox->DestroyComponent();
        SpawnedHitbox = nullptr;
    }
}


