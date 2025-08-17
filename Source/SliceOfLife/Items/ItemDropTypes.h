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
enum class ECategory : uint8
{
	Limb,
	Appendage,
	Organ
};

UENUM(BlueprintType)
enum class EPreparedState : uint8
{
	None,
	Skewered,
	Crushed,
	Sliced
};

class AItemDropActor;

USTRUCT(BlueprintType)
struct FItemDropRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemName;

	// Blueprint class to spawn for this item
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AItemDropActor> ItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERarity Rarity = ERarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBodyPart BodyPart = EBodyPart::Torso;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECategory Category = ECategory::Organ; // Default; override per row
};


