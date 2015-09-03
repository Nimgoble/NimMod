#pragma once

#include "RoundManager.h"
#include "RoundManager_LoadStreamingLevel.generated.h"

UCLASS(minimalapi)
class ARoundManager_LoadStreamingLevel : public ARoundManager
{
	GENERATED_BODY()

public:
	ARoundManager_LoadStreamingLevel(const FObjectInitializer& ObjectInitializer);
	virtual void RestartRound() override;
private:
	UPROPERTY()
	class ULevelStreamingKismet *currentRoundLevel;

	UPROPERTY()
	class ULevelStreamingKismet *nextRoundLevel;

	//void RestartRound_LoadStreamingLevel();

	class ULevelStreamingKismet *LoadRoundLevel();

	UFUNCTION()
	void OnRoundLevelLoaded();

	/*UFUNCTION(Reliable, NetMulticast)
	void SetLoadedLevel(ULevelStreamingKismet *level);*/
};