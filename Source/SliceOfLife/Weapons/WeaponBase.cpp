#include "SliceOfLife/Weapons/WeaponBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	UBoxComponent* Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Hitbox"));
	HitboxComponent = Box;
	Box->SetupAttachment(WeaponMesh);
	Box->SetBoxExtent(FVector(20.f, 5.f, 5.f));
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Box->SetCollisionObjectType(ECC_WorldDynamic);
	Box->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (HitboxComponent)
	{
		HitboxComponent->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnHitboxOverlap);
	}
}

void AWeaponBase::EnableHitbox()
{
	if (HitboxComponent)
	{
		HitboxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		HitboxComponent->SetGenerateOverlapEvents(true);
	}
}

void AWeaponBase::DisableHitbox()
{
	if (HitboxComponent)
	{
		HitboxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		HitboxComponent->SetGenerateOverlapEvents(false);
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

	// Knockback based on owner facing (X axis in 2.5D)
	const FVector Facing = OwnerCharacter->GetActorForwardVector();

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


