#include "SliceOfLifeGameMode.h"
#include "SliceOfLife/Characters/PlayerCharacter.h"
#include "SliceOfLife/Components/HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

ASliceOfLifeGameMode::ASliceOfLifeGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Set default values
	bGamePaused = false;
	bGameOver = false;
	CurrentArea = TEXT("StartingArea");
	bCheckpointSet = false;
	CurrentPlayer = nullptr;
	
	// Set default player class
	DefaultPawnClass = APlayerCharacter::StaticClass();
}

void ASliceOfLifeGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeGame();
}

void ASliceOfLifeGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Game logic updates can go here
}

void ASliceOfLifeGameMode::InitializeGame()
{
	// Set initial checkpoint to player start
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (APlayerCharacter* Player = Cast<APlayerCharacter>(PC->GetPawn()))
		{
			CurrentPlayer = Player;
			CheckpointLocation = Player->GetActorLocation();
			CheckpointRotation = Player->GetActorRotation();
			bCheckpointSet = true;
			
			// Call blueprint event
			OnPlayerSpawned(Player);
		}
	}
}

void ASliceOfLifeGameMode::SetupPlayer()
{
	// This can be called to setup additional player properties
	if (CurrentPlayer)
	{
		// Setup player-specific game mode logic here
	}
}

void ASliceOfLifeGameMode::RespawnPlayer()
{
	if (bCheckpointSet && CurrentPlayer)
	{
		// Teleport player to checkpoint
		CurrentPlayer->SetActorLocationAndRotation(CheckpointLocation, CheckpointRotation);
		
		// Reset player state
		if (UHealthComponent* HealthComp = CurrentPlayer->GetHealthComponent())
		{
			HealthComp->ResetHealth();
		}
		
		UE_LOG(LogTemp, Log, TEXT("Player respawned at checkpoint"));
	}
	else
	{
		// No checkpoint set, restart level
		RestartLevel();
	}
}

void ASliceOfLifeGameMode::RestartLevel()
{
	UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}

void ASliceOfLifeGameMode::PauseGame()
{
	if (!bGamePaused)
	{
		bGamePaused = true;
		UGameplayStatics::SetGamePaused(this, true);
		UE_LOG(LogTemp, Log, TEXT("Game paused"));
	}
}

void ASliceOfLifeGameMode::ResumeGame()
{
	if (bGamePaused)
	{
		bGamePaused = false;
		UGameplayStatics::SetGamePaused(this, false);
		UE_LOG(LogTemp, Log, TEXT("Game resumed"));
	}
}

void ASliceOfLifeGameMode::GameOver()
{
	if (!bGameOver)
	{
		bGameOver = true;
		PauseGame();
		OnGameOver();
		UE_LOG(LogTemp, Warning, TEXT("Game Over"));
	}
}

void ASliceOfLifeGameMode::SetCheckpoint(FVector Location, FRotator Rotation)
{
	CheckpointLocation = Location;
	CheckpointRotation = Rotation;
	bCheckpointSet = true;
	
	UE_LOG(LogTemp, Log, TEXT("Checkpoint set at location: %s"), *Location.ToString());
}

void ASliceOfLifeGameMode::LoadCheckpoint()
{
	if (bCheckpointSet)
	{
		RespawnPlayer();
	}
}

void ASliceOfLifeGameMode::ChangeArea(const FString& AreaName)
{
	if (AreaName != CurrentArea)
	{
		FString OldArea = CurrentArea;
		CurrentArea = AreaName;
		
		// Save game state before area change
		SaveGameState();
		
		// Call blueprint event
		OnAreaChanged(AreaName);
		
		UE_LOG(LogTemp, Log, TEXT("Area changed from %s to %s"), *OldArea, *AreaName);
	}
}

void ASliceOfLifeGameMode::SaveGameState()
{
	// Save current game state (checkpoint, area, etc.)
	// This can be expanded to save to a save file
	UE_LOG(LogTemp, Log, TEXT("Game state saved"));
}

void ASliceOfLifeGameMode::LoadGameState()
{
	// Load saved game state
	// This can be expanded to load from a save file
	UE_LOG(LogTemp, Log, TEXT("Game state loaded"));
}
