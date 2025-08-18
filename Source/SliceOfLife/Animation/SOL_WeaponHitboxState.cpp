#include "SliceOfLife/Animation/SOL_WeaponHitboxState.h"
#include "Components/SkeletalMeshComponent.h"
#include "SliceOfLife/Characters/PlayerCharacter.h"
#include "SliceOfLife/Weapons/WeaponBase.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"
#include "Components/BoxComponent.h"

static bool ShouldDrawHitboxes()
{
    static const auto CVarShowHitboxes = IConsoleManager::Get().FindConsoleVariable(TEXT("SliceOfLife.ShowHitboxes"));
    return CVarShowHitboxes ? (CVarShowHitboxes->GetInt() != 0) : false;
}

void USOL_WeaponHitboxState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;
    if (APlayerCharacter* Player = Cast<APlayerCharacter>(MeshComp->GetOwner()))
    {
        // Configure and enable the current weapon's hitbox
        TArray<AActor*> Attached;
        Player->GetAttachedActors(Attached);
        for (AActor* A : Attached)
        {
            if (AWeaponBase* Weapon = Cast<AWeaponBase>(A))
            {
                Weapon->ConfigureHitbox(LocalOffset, BoxExtent);
                ToggleWeaponHitbox(Player, true);
                break;
            }
        }
    }
}

void USOL_WeaponHitboxState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;
    // Weapon draws its own debug in Tick when the cvar is on
}

void USOL_WeaponHitboxState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (!MeshComp) return;
    if (APlayerCharacter* Player = Cast<APlayerCharacter>(MeshComp->GetOwner()))
    {
        ToggleWeaponHitbox(Player, false);
    }
}

void USOL_WeaponHitboxState::ToggleWeaponHitbox(APlayerCharacter* Player, bool bEnable) const
{
    if (!Player) return;
    TArray<AActor*> Attached;
    Player->GetAttachedActors(Attached);
    for (AActor* A : Attached)
    {
        if (AWeaponBase* Weapon = Cast<AWeaponBase>(A))
        {
            if (bEnable) Weapon->EnableHitbox(); else Weapon->DisableHitbox();
            break;
        }
    }
}

void USOL_WeaponHitboxState::DrawWeaponHitboxDebug(APlayerCharacter* Player) const
{
    if (!Player || !ShouldDrawHitboxes()) return;
    TArray<AActor*> Attached;
    Player->GetAttachedActors(Attached);
    for (AActor* A : Attached)
    {
        if (AWeaponBase* Weapon = Cast<AWeaponBase>(A))
        {
            if (UPrimitiveComponent* Hitbox = Weapon->GetHitboxComponent())
            {
                const FTransform T = Hitbox->GetComponentTransform();
                if (const UBoxComponent* Box = Cast<UBoxComponent>(Hitbox))
                {
                    DrawDebugBox(Player->GetWorld(), T.GetLocation(), Box->GetUnscaledBoxExtent(), T.GetRotation().GetNormalized(), FColor::Red, false, 0.f, 0, 1.5f);
                }
                else
                {
                    DrawDebugSphere(Player->GetWorld(), T.GetLocation(), 20.f, 12, FColor::Red, false, 0.f, 0, 1.5f);
                }
            }
            break;
        }
    }
}


