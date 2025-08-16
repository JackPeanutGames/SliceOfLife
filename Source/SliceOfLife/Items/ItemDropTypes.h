#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ItemDropTypes.generated.h"

UENUM(BlueprintType)
enum class ERarity : uint8
{
	Common,
	Uncommon,
	Rare
};

UENUM(BlueprintType)
enum class EBodyPart : uint8
{
	Head,
	Torso,
	Leg
};

UENUM(BlueprintType)
enum class EPreparedBy : uint8
{
	None,
	Skewer,
	Crusher,
	Slicer
};

USTRUCT(BlueprintType)
struct FItemDrop : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* ItemMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERarity Rarity = ERarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Category; // Limb, Appendage, Organ

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBodyPart BodyPart = EBodyPart::Torso;

	// Optional: default prepared state in the table; final prepared state is determined at drop time
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPreparedBy PreparedBy = EPreparedBy::None;
};


