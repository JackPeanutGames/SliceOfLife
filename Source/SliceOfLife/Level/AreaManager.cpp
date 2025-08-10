#include "AreaManager.h"
#include "SliceOfLife/Characters/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "TimerManager.h"

AAreaManager::AAreaManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// Initialize default values
	CurrentArea = TEXT("StartingArea");
	LoadedAreas.Empty();
	UnlockedAreas.Empty();
	CurrentPlayer = nullptr;
}

void AAreaManager::BeginPlay()
{
	Super::BeginPlay();

	// Find the player
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		CurrentPlayer = Player;
	}

	// Initialize areas
	InitializeAreas();

	// Load starting area
	LoadArea(CurrentArea);
}

void AAreaManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Area manager logic can go here
}

void AAreaManager::InitializeAreas()
{
	// Add starting area to unlocked areas
	UnlockedAreas.Add(TEXT("StartingArea"));

	// Load area data from data table if available
	if (AreaDataTable)
	{
		TArray<FName> RowNames = AreaDataTable->GetRowNames();
		for (FName RowName : RowNames)
		{
			if (FAreaData* AreaData = AreaDataTable->FindRow<FAreaData>(RowName, TEXT("")))
			{
				// Check if area should be unlocked by default
				if (AreaData->bIsUnlocked)
				{
					UnlockedAreas.Add(AreaData->AreaName);
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Area Manager initialized with %d unlocked areas"), UnlockedAreas.Num());
}

void AAreaManager::LoadArea(const FString& AreaName)
{
	if (!IsAreaUnlocked(AreaName))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot load area %s: not unlocked"), *AreaName);
		return;
	}

	// Unload current area if different
	if (CurrentArea != AreaName)
	{
		UnloadArea(CurrentArea);
	}

	// Load new area
	CurrentArea = AreaName;
	LoadAreaContent(AreaName);
	LoadedAreas.AddUnique(AreaName);

	// Apply area effects
	ApplyAreaEffects(AreaName);

	// Call blueprint event
	OnAreaLoaded(AreaName);

	UE_LOG(LogTemp, Log, TEXT("Loaded area: %s"), *AreaName);
}

void AAreaManager::UnloadArea(const FString& AreaName)
{
	if (LoadedAreas.Contains(AreaName))
	{
		// Remove area effects
		RemoveAreaEffects(AreaName);

		// Unload area content
		UnloadAreaContent(AreaName);

		// Remove from loaded areas
		LoadedAreas.Remove(AreaName);

		// Call blueprint event
		OnAreaUnloaded(AreaName);

		UE_LOG(LogTemp, Log, TEXT("Unloaded area: %s"), *AreaName);
	}
}

void AAreaManager::TransitionToArea(const FString& AreaName, const FVector& PlayerDestination, const FRotator& PlayerRotation)
{
	if (!CanTransitionToArea(AreaName))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot transition to area %s"), *AreaName);
		return;
	}

	FString FromArea = CurrentArea;

	// Load the new area
	LoadArea(AreaName);

	// Teleport player to destination
	if (CurrentPlayer)
	{
		CurrentPlayer->SetActorLocationAndRotation(PlayerDestination, PlayerRotation);
	}

	// Call transition event
	OnAreaTransition(FromArea, AreaName);

	UE_LOG(LogTemp, Log, TEXT("Transitioned from %s to %s"), *FromArea, *AreaName);
}

bool AAreaManager::IsAreaUnlocked(const FString& AreaName) const
{
	return UnlockedAreas.Contains(AreaName);
}

TArray<FString> AAreaManager::GetConnectedAreas(const FString& AreaName) const
{
	FAreaData AreaData;
	if (GetAreaData(AreaName, AreaData))
	{
		return AreaData.ConnectedAreas;
	}
	return TArray<FString>();
}

bool AAreaManager::GetAreaData(const FString& AreaName, FAreaData& OutAreaData) const
{
	if (AreaDataTable)
	{
		TArray<FName> RowNames = AreaDataTable->GetRowNames();
		for (FName RowName : RowNames)
		{
			if (const FAreaData* AreaData = AreaDataTable->FindRow<FAreaData>(RowName, TEXT("")))
			{
				if (AreaData->AreaName == AreaName)
				{
					OutAreaData = *AreaData;
					return true;
				}
			}
		}
	}
	return false;
}

void AAreaManager::UnlockArea(const FString& AreaName)
{
	if (!UnlockedAreas.Contains(AreaName))
	{
		UnlockedAreas.Add(AreaName);

		// Call blueprint event
		OnAreaUnlocked(AreaName);

		UE_LOG(LogTemp, Log, TEXT("Unlocked area: %s"), *AreaName);
	}
}

void AAreaManager::CheckAreaUnlocks()
{
	if (!AreaDataTable)
	{
		return;
	}

	TArray<FName> RowNames = AreaDataTable->GetRowNames();
	for (FName RowName : RowNames)
	{
		if (FAreaData* AreaData = AreaDataTable->FindRow<FAreaData>(RowName, TEXT("")))
		{
			// Skip if already unlocked
			if (UnlockedAreas.Contains(AreaData->AreaName))
			{
				continue;
			}

			// Check if player has required power-ups
			bool bHasRequiredPowerUps = true;
			for (const FString& RequiredPowerUp : AreaData->RequiredPowerUps)
			{
				// This would need to be implemented based on your power-up system
				// For now, we'll assume all areas can be unlocked
				bHasRequiredPowerUps = true;
			}

			if (bHasRequiredPowerUps)
			{
				UnlockArea(AreaData->AreaName);
			}
		}
	}
}

void AAreaManager::LoadAreaContent(const FString& AreaName)
{
	// This function would handle loading area-specific content
	// Such as spawning enemies, setting up level geometry, etc.
	// Implementation depends on your specific needs

	UE_LOG(LogTemp, Log, TEXT("Loading content for area: %s"), *AreaName);
}

void AAreaManager::UnloadAreaContent(const FString& AreaName)
{
	// This function would handle unloading area-specific content
	// Such as destroying enemies, cleaning up level objects, etc.
	// Implementation depends on your specific needs

	UE_LOG(LogTemp, Log, TEXT("Unloading content for area: %s"), *AreaName);
}

bool AAreaManager::CanTransitionToArea(const FString& AreaName) const
{
	// Check if area is unlocked
	if (!IsAreaUnlocked(AreaName))
	{
		return false;
	}

	// Check if area is connected to current area
	TArray<FString> ConnectedAreas = GetConnectedAreas(CurrentArea);
	return ConnectedAreas.Contains(AreaName);
}

void AAreaManager::ApplyAreaEffects(const FString& AreaName)
{
	FAreaData AreaData;
	if (GetAreaData(AreaName, AreaData))
	{
		// Apply background music
		if (AreaData.BackgroundMusic)
		{
			// This would need to be implemented with your audio system
			UE_LOG(LogTemp, Log, TEXT("Playing background music for area: %s"), *AreaName);
		}

		// Apply ambient effects
		if (AreaData.AmbientEffect)
		{
			// This would need to be implemented with your particle system
			UE_LOG(LogTemp, Log, TEXT("Applying ambient effects for area: %s"), *AreaName);
		}
	}
}

void AAreaManager::RemoveAreaEffects(const FString& AreaName)
{
	FAreaData AreaData;
	if (GetAreaData(AreaName, AreaData))
	{
		// Stop background music
		if (AreaData.BackgroundMusic)
		{
			// This would need to be implemented with your audio system
			UE_LOG(LogTemp, Log, TEXT("Stopping background music for area: %s"), *AreaName);
		}

		// Remove ambient effects
		if (AreaData.AmbientEffect)
		{
			// This would need to be implemented with your particle system
			UE_LOG(LogTemp, Log, TEXT("Removing ambient effects for area: %s"), *AreaName);
		}
	}
}
