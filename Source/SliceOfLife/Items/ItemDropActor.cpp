#include "SliceOfLife/Items/ItemDropActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "SliceOfLife/Characters/PlayerCharacter.h"

AItemDropActor::AItemDropActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create collision component as root (handles gameplay interactions)
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(CollisionComponent);
	CollisionComponent->InitSphereRadius(24.f);
	
	// Create mesh component (visual only, attached to collision)
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(CollisionComponent);
	
	// Configure mesh collision (no physics, just visual)
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetSimulatePhysics(false);
	
	// Configure collision rules for item drops
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);    // keep on ground
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap); // overlap weapons, enemies, other items
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);        // allow player pickup

	// Default to 2.5D constraint at construction time
	ApplyPlaneConstraintSettings();
	
	// Create and configure projectile movement component
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent; // Use collision component for movement
	ProjectileMovement->bRotationFollowsVelocity = false;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	
	// Enable plane constraint (lock movement along Y)
	ProjectileMovement->bConstrainToPlane = true;
	ProjectileMovement->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f));
	ProjectileMovement->SetPlaneConstraintOrigin(FVector::ZeroVector);
	
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AItemDropActor::OnOverlapBegin);
}

void AItemDropActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Set bounciness after UPROPERTY values are initialized
	if (ProjectileMovement)
	{
		ProjectileMovement->Bounciness = BounceVelocity;
	}
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
		// Add the item to the player's inventory based on its category
		Player->AddItemToInventory(Category);
		
		// Call the Blueprint event for any additional pickup effects
		OnPickedUp(Player);
		
		// Destroy the item drop
		Destroy();
	}
}

void AItemDropActor::ApplyPlaneConstraintSettings()
{
	if (ProjectileMovement)
	{
		// Apply plane constraint to the projectile movement component
		// This constrains the item to the Y=0 plane (2.5D movement)
		ProjectileMovement->bConstrainToPlane = bConstrainToYPlane;
		ProjectileMovement->SetPlaneConstraintNormal(FVector(0.f, 1.f, 0.f));
		ProjectileMovement->SetPlaneConstraintOrigin(FVector::ZeroVector);
	}
}

void AItemDropActor::LaunchItem(const FVector& Direction, float Speed)
{
	if (ProjectileMovement)
	{
		// Use provided speed or random speed within designer range
		float LaunchSpeed = Speed;
		if (Speed <= 0.0f)
		{
			LaunchSpeed = FMath::RandRange(MinLaunchSpeed, MaxLaunchSpeed);
		}
		
		// Normalize direction and apply speed
		FVector LaunchDirection = Direction.GetSafeNormal();
		ProjectileMovement->Velocity = LaunchDirection * LaunchSpeed;
		
		// Activate the movement component
		ProjectileMovement->Activate();
	}
}

void AItemDropActor::LaunchItemWithVelocity(const FVector& Velocity)
{
	if (ProjectileMovement)
	{
		// Set the velocity directly
		ProjectileMovement->Velocity = Velocity;
		
		// Activate the movement component
		ProjectileMovement->Activate();
	}
}


