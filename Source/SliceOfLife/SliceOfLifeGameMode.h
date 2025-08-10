#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SliceOfLifeGameMode.generated.h"

class APlayerCharacter;
class ASliceOfLifeGameState;
class ASliceOfLifePlayerState;
class ASliceOfLifePlayerController;
class UGameplayStatics;

UCLASS(Blueprintable)
class SLICEOFLIFE_API ASliceOfLifeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASliceOfLifeGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Player Management
	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void RespawnPlayer();

	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void RestartLevel();

    // Game State Management
	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void PauseGame();

	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void ResumeGame();

	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void GameOver();

	// Checkpoint System
	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void SetCheckpoint(FVector Location, FRotator Rotation);

	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void LoadCheckpoint();

    // Area/Level Management
	UFUNCTION(BlueprintCallable, Category = "Game Mode")
	void ChangeArea(const FString& AreaName);

	UFUNCTION(BlueprintCallable, Category = "Game Mode")
    FString GetCurrentArea() const { return CurrentArea; }

	// Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Game Mode")
	void OnPlayerSpawned(APlayerCharacter* Player);

	UFUNCTION(BlueprintImplementableEvent, Category = "Game Mode")
	void OnGameOver();

	UFUNCTION(BlueprintImplementableEvent, Category = "Game Mode")
	void OnAreaChanged(const FString& NewArea);

protected:
    // Game State flags
    UPROPERTY(BlueprintReadOnly, Category = "Game Mode")
    bool bGamePaused;

	UPROPERTY(BlueprintReadOnly, Category = "Game Mode")
	bool bGameOver;

    // Legacy: kept for BP compatibility; value should mirror GameState's replicated value
    UPROPERTY(BlueprintReadOnly, Category = "Game Mode")
    FString CurrentArea;

	// Checkpoint System
	UPROPERTY(BlueprintReadOnly, Category = "Game Mode")
	FVector CheckpointLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Game Mode")
	FRotator CheckpointRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Game Mode")
	bool bCheckpointSet;

	// Player Reference
	UPROPERTY(BlueprintReadOnly, Category = "Game Mode")
	APlayerCharacter* CurrentPlayer;

	// Internal Functions
	void InitializeGame();
	void SetupPlayer();
	void SaveGameState();
	void LoadGameState();
};
