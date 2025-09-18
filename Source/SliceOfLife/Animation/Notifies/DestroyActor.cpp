#include "DestroyActor.h"

void UDestroyActor::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp)
	{
		return;
	}
	if (AActor* Owner = MeshComp->GetOwner())
	{
		Owner->SetLifeSpan(0.01f);
	}
}