#pragma once

#include "GameFramework/Info.h"
#include "RoundManager.generated.h"

UCLASS(Abstract, Blueprintable)
class NIMMOD_API ARoundManager : public AInfo
{
	GENERATED_BODY()
public:
	ARoundManager(const FObjectInitializer& ObjectInitializer);
	virtual void RestartRound();

protected:
	UFUNCTION(BlueprintCallable, Category="NimMod|RoundManager")
	bool ShouldReset(AActor* ActorToReset);

	static bool ShouldReset(UWorld *world, AActor* ActorToReset);
	FString GetOriginalMapName();

	UPROPERTY(BlueprintReadWrite, Category = "NimMod|RoundManager")
	int32 RoundCount;

private:
	// Hidden functions that don't make sense to use on this class.
	HIDE_ACTOR_TRANSFORM_FUNCTIONS();
};