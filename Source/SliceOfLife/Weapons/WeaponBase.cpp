#include "SliceOfLife/Weapons/WeaponBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"
#include "HAL/IConsoleManager.h"
#include "SliceOfLife/Components/CombatComponent.h"
#include "SliceOfLife/Characters/PlayerCharacter.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UBoxComponent* Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Hitbox"));
	HitboxComponent = Box;
	Box->SetupAttachment(WeaponMesh);
	Box->SetBoxExtent(FVector(20.f, 5.f, 5.f));
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Box->SetCollisionProfileName(TEXT("WeaponHitbox"));
	Box->SetGenerateOverlapEvents(false);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (HitboxComponent)
	{
		HitboxComponent->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnHitboxOverlap);
	}
}

void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	static const auto CVarShowHitboxes = IConsoleManager::Get().FindConsoleVariable(TEXT("SliceOfLife.ShowHitboxes"));
	const bool bShow = CVarShowHitboxes ? (CVarShowHitboxes->GetInt() != 0) : false;
	if (!bShow)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		const FColor DebugColor = bHitboxActive ? FColor::Red : FColor::Yellow;
		if (auto* Box = Cast<UBoxComponent>(HitboxComponent))
		{
			DrawDebugBox(World, Box->GetComponentLocation(),
				     Box->GetScaledBoxExtent(), Box->GetComponentRotation().Quaternion(),
				     DebugColor, false, 0.f, 0, 2.f);
		}
		else if (auto* Sphere = Cast<USphereComponent>(HitboxComponent))
		{
			DrawDebugSphere(World, Sphere->GetComponentLocation(),
					    Sphere->GetScaledSphereRadius(), 16,
					    DebugColor, false, 0.f, 0, 2.f);
		}
	}
}

void AWeaponBase::EnableHitbox()
{
	if (HitboxComponent)
	{
		HitboxComponent->SetCollisionProfileName(TEXT("WeaponHitbox"));
		HitboxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		HitboxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		HitboxComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
		HitboxComponent->SetGenerateOverlapEvents(true);
		bHitboxActive = true;
	}
}

void AWeaponBase::DisableHitbox()
{
	if (HitboxComponent)
	{
		HitboxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitboxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
		HitboxComponent->SetGenerateOverlapEvents(false);
		bHitboxActive = false;
	}
}

void AWeaponBase::ConfigureHitbox(const FVector& LocalOffset, const FVector& BoxExtent)
{
	if (UBoxComponent* Box = Cast<UBoxComponent>(HitboxComponent))
	{
		Box->SetBoxExtent(BoxExtent, true);
		FVector AdjustedOffset = LocalOffset;
		AdjustedOffset.X = 0.f; // center on 2.5D plane like legacy path
		Box->SetRelativeLocation(AdjustedOffset);
	}
}

void AWeaponBase::SetAttackParams(float InDamage, float InKnockback)
{
	Damage = InDamage;
	KnockbackForce = InKnockback;
}

void AWeaponBase::OnHitboxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OtherActor || OtherActor == OwnerActor)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(OwnerActor);
	if (!OwnerCharacter)
	{
		return;
	}

	// Knockback based on player facing vector if available
	FVector Facing = OwnerCharacter->GetActorForwardVector();
	if (const APlayerCharacter* Player = Cast<APlayerCharacter>(OwnerCharacter))
	{
		Facing = Player->GetFacingVector();
	}

	if (UCombatComponent* Combat = OwnerCharacter->FindComponentByClass<UCombatComponent>())
	{
		Combat->ApplyDamageAndKnockback(OtherActor, Damage, Facing, KnockbackForce);
	}
	else
	{
		// Fallback to generic damage application if no combat component
		AController* InstigatorController = OwnerCharacter->GetController();
		FPointDamageEvent PointEvent;
		OtherActor->TakeDamage(Damage, PointEvent, InstigatorController, OwnerActor);
		if (ACharacter* HitChar = Cast<ACharacter>(OtherActor))
		{
			const FVector LaunchVel = Facing * KnockbackForce;
			if (UCharacterMovementComponent* Move = HitChar->GetCharacterMovement())
			{
				Move->DisableMovement();
				Move->Velocity = LaunchVel;
			}
			else
			{
				HitChar->LaunchCharacter(LaunchVel, true, true);
			}
		}
	}
}


