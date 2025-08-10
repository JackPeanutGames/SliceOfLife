#include "PowerUpBase.h"
#include "SliceOfLife/Characters/PlayerCharacter.h"
#include "SliceOfLife/Components/HealthComponent.h"
#include "SliceOfLife/Components/PlayerMovementComponent.h"
#include "SliceOfLife/Components/CombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

APowerUpBase::APowerUpBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create components
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(RootComponent);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Initialize state
	bIsCollected = false;

	// Setup default power-up data
	PowerUpData = FPowerUpData();
	PowerUpData.PowerUpName = TEXT("Default Power Up");
	PowerUpData.PowerUpType = EPowerUpType::Health;
	PowerUpData.Value = 1.0f;
	PowerUpData.bIsPermanent = true;
	PowerUpData.Duration = 0.0f;
	PowerUpData.Description = TEXT("A mysterious power up");

	// Bind overlap event
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APowerUpBase::OnPlayerOverlap);
}

void APowerUpBase::BeginPlay()
{
	Super::BeginPlay();

	// Store initial location for bobbing effect
	InitialLocation = GetActorLocation();
}

void APowerUpBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsCollected)
	{
		UpdateVisualEffects(DeltaTime);
	}
}

void APowerUpBase::ApplyPowerUp(APlayerCharacter* Player)
{
	if (!Player || !CanApplyToPlayer(Player))
	{
		return;
	}

	// Apply power-up based on type
	switch (PowerUpData.PowerUpType)
	{
	case EPowerUpType::Health:
		if (UHealthComponent* HealthComp = Player->GetHealthComponent())
		{
			HealthComp->Heal(PowerUpData.Value);
		}
		break;

	case EPowerUpType::Movement:
		if (UPlayerMovementComponent* MovementComp = Player->GetPlayerMovementComponent())
		{
			// Apply movement speed boost
			// This could be expanded with more movement upgrades
		}
		break;

	case EPowerUpType::Combat:
		if (UCombatComponent* CombatComp = Player->GetCombatComponent())
		{
			// Apply combat upgrades
			// This could be expanded with damage boosts, new abilities, etc.
		}
		break;

	case EPowerUpType::Special:
		// Handle special power-ups (double jump, wall jump, etc.)
		break;

	default:
		break;
	}

	// Call blueprint event
	OnPowerUpApplied(Player);

	// Handle temporary power-ups
	if (!PowerUpData.bIsPermanent && PowerUpData.Duration > 0.0f)
	{
		// Set timer to remove power-up
		FTimerHandle RemoveTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(RemoveTimerHandle, [this, Player]()
		{
			RemovePowerUp(Player);
		}, PowerUpData.Duration, false);
	}

	UE_LOG(LogTemp, Log, TEXT("Applied power-up %s to player"), *PowerUpData.PowerUpName);
}

void APowerUpBase::RemovePowerUp(APlayerCharacter* Player)
{
	if (!Player)
	{
		return;
	}

	// Remove power-up effects based on type
	switch (PowerUpData.PowerUpType)
	{
	case EPowerUpType::Health:
		// Health power-ups are usually permanent
		break;

	case EPowerUpType::Movement:
		// Reset movement modifications
		break;

	case EPowerUpType::Combat:
		// Reset combat modifications
		break;

	case EPowerUpType::Special:
		// Remove special abilities
		break;

	default:
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("Removed power-up %s from player"), *PowerUpData.PowerUpName);
}

void APowerUpBase::OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsCollected)
	{
		return;
	}

	// Check if the overlapping actor is a player
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
	{
		// Collect the power-up
		bIsCollected = true;

		// Call blueprint event
		OnPowerUpCollected(Player);

		// Apply the power-up
		ApplyPowerUp(Player);

		// Play pickup effects
		PlayPickupEffects(Player);

		// Hide the power-up
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);

		// Destroy after a short delay to allow effects to play
		FTimerHandle DestroyTimerHandle;
		GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, [this]()
		{
			Destroy();
		}, 1.0f, false);
	}
}

void APowerUpBase::UpdateVisualEffects(float DeltaTime)
{
	// Rotate the power-up
	if (MeshComponent)
	{
		FRotator CurrentRotation = MeshComponent->GetRelativeRotation();
		CurrentRotation.Yaw += RotationSpeed * DeltaTime;
		MeshComponent->SetRelativeRotation(CurrentRotation);
	}

	// Bobbing effect
	if (CollisionComponent)
	{
		FVector CurrentLocation = GetActorLocation();
		float BobOffset = FMath::Sin(GetWorld()->GetTimeSeconds() * BobbingSpeed) * BobbingHeight;
		CurrentLocation.Z = InitialLocation.Z + BobOffset;
		SetActorLocation(CurrentLocation);
	}
}

void APowerUpBase::PlayPickupEffects(APlayerCharacter* Player)
{
	// Play pickup sound
	if (PowerUpData.PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PowerUpData.PickupSound, GetActorLocation());
	}

	// Spawn pickup effect
	if (PowerUpData.PickupEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, PowerUpData.PickupEffect, GetActorLocation(), GetActorRotation());
	}
}

bool APowerUpBase::CanApplyToPlayer(APlayerCharacter* Player) const
{
	if (!Player)
	{
		return false;
	}

	// Check if player already has this power-up (for permanent ones)
	if (PowerUpData.bIsPermanent)
	{
		// This could be expanded to check if the player already has this power-up
		// For now, we'll allow all power-ups
		return true;
	}

	// Temporary power-ups can always be applied
	return true;
}
