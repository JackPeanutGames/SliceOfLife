// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "DestroyActor.generated.h"

/**
 * 
 */
UCLASS()
class SLICEOFLIFE_API UDestroyActor : public UAnimNotify
{
	GENERATED_BODY()

	void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation);
};
