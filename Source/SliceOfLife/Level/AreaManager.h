#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "AreaManager.generated.h"

class APlayerCharacter;
class UBoxComponent;
class USoundBase;
class UParticleSystem;

USTRUCT(BlueprintType)
struct FAreaData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area")
	FString AreaName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area")
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area")
	bool bIsUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area")
	TArray<FString> RequiredPowerUps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area")
	TArray<FString> ConnectedAreas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area")
	FVector PlayerSpawnLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area")
	FRotator PlayerSpawnRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area")
	USoundBase* BackgroundMusic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area")
	UParticleSystem* AmbientEffect;
};

USTRUCT(BlueprintType)
struct FAreaTransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	FString FromArea;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	FString ToArea;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	FVector TransitionLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	FVector PlayerDestination;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	FRotator PlayerDestinationRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
	bool bRequiresLoading = false;
};

UCLASS(Blueprintable)
class SLICEOFLIFE_API AAreaManager : public AActor
{
	GENERATED_BODY()

public:
	AAreaManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Area Management
	UFUNCTION(BlueprintCallable, Category = "Area Manager")
	void LoadArea(const FString& AreaName);

	UFUNCTION(BlueprintCallable, Category = "Area Manager")
	void UnloadArea(const FString& AreaName);

	UFUNCTION(BlueprintCallable, Category = "Area Manager")
	void TransitionToArea(const FString& AreaName, const FVector& PlayerDestination, const FRotator& PlayerRotation);

	// Area Queries
	UFUNCTION(BlueprintPure, Category = "Area Manager")
	FString GetCurrentArea() const { return CurrentArea; }

	UFUNCTION(BlueprintPure, Category = "Area Manager")
	bool IsAreaUnlocked(const FString& AreaName) const;

	UFUNCTION(BlueprintPure, Category = "Area Manager")
	TArray<FString> GetConnectedAreas(const FString& AreaName) const;

	// Note: Use an out parameter instead of exposing a pointer to a USTRUCT
	UFUNCTION(BlueprintPure, Category = "Area Manager")
	bool GetAreaData(const FString& AreaName, FAreaData& OutAreaData) const;

	// Power-up Integration
	UFUNCTION(BlueprintCallable, Category = "Area Manager")
	void UnlockArea(const FString& AreaName);

	UFUNCTION(BlueprintCallable, Category = "Area Manager")
	void CheckAreaUnlocks();

	// Events
	UFUNCTION(BlueprintImplementableEvent, Category = "Area Manager")
	void OnAreaLoaded(const FString& AreaName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Area Manager")
	void OnAreaUnloaded(const FString& AreaName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Area Manager")
	void OnAreaTransition(const FString& FromArea, const FString& ToArea);

	UFUNCTION(BlueprintImplementableEvent, Category = "Area Manager")
	void OnAreaUnlocked(const FString& AreaName);

protected:
	// Area Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area Manager")
	UDataTable* AreaDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Area Manager")
	TArray<FAreaTransition> AreaTransitions;

	// Current State
	UPROPERTY(BlueprintReadOnly, Category = "Area Manager")
	FString CurrentArea;

	UPROPERTY(BlueprintReadOnly, Category = "Area Manager")
	TArray<FString> LoadedAreas;

	UPROPERTY(BlueprintReadOnly, Category = "Area Manager")
	TArray<FString> UnlockedAreas;

	// Player Reference
	UPROPERTY(BlueprintReadOnly, Category = "Area Manager")
	APlayerCharacter* CurrentPlayer;

	// Internal Functions
	void InitializeAreas();
	void LoadAreaContent(const FString& AreaName);
	void UnloadAreaContent(const FString& AreaName);
	bool CanTransitionToArea(const FString& AreaName) const;
	void ApplyAreaEffects(const FString& AreaName);
	void RemoveAreaEffects(const FString& AreaName);
};
