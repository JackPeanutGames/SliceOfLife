#include "SliceOfLife/Items/ItemDropActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "SliceOfLife/Characters/PlayerCharacter.h"

AItemDropActor::AItemDropActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(MeshComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->SetupAttachment(RootComponent);
	CollisionComponent->InitSphereRadius(24.f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->SetCollisionProfileName(TEXT("ItemPickup"));

	// Default to 2.5D constraint at construction time
	ApplyPlaneConstraintSettings();
	
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AItemDropActor::OnOverlapBegin);
}

void AItemDropActor::BeginPlay()
{
	Super::BeginPlay();
}

void AItemDropActor::SetPreparedState(EPreparedState NewState)
{
	PreparedState = NewState;
}

void AItemDropActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		OnPickedUp(Player);
		Destroy();
	}
}

void AItemDropActor::ApplyPlaneConstraintSettings()
{
	
}


