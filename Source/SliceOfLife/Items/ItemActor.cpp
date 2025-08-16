#include "SliceOfLife/Items/ItemActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

AItemActor::AItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(12.f);
	Collision->SetSimulatePhysics(true);
	Collision->SetCollisionProfileName(TEXT("PhysicsActor"));

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComp->SetupAttachment(Collision);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AItemActor::SetItemMesh(UStaticMesh* Mesh)
{
	if (MeshComp && Mesh)
	{
		MeshComp->SetStaticMesh(Mesh);
	}
}

void AItemActor::ImpulseBackward(const FVector& Direction, float Strength)
{
	if (Collision)
	{
		Collision->AddImpulse(Direction.GetSafeNormal() * Strength, NAME_None, true);
	}
}


