#pragma once

#include "RoundManager.h"
#include "RoundManager_ForceRespawn.generated.h"

UCLASS(minimalapi)
class ARoundManager_ForceRespawn : public ARoundManager
{
	GENERATED_BODY()

public:
	ARoundManager_ForceRespawn(const FObjectInitializer& ObjectInitializer);
	//virtual void BeginPlay();
	virtual void RestartRound() override;
	
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Restart Round"))
	void HandleRestartRound();

protected:
	UPROPERTY(BlueprintReadWrite, Category = "NimMod|RoundManager")
	TArray<AActor *> CurrentRoundActors;

private:
	/**
	* @return true if ActorToReset should have Reset() called on it while restarting the game,
	*		   false if the GameMode will manually reset it or if the actor does not need to be reset
	*/
	void InitializeRoundObjects();
};