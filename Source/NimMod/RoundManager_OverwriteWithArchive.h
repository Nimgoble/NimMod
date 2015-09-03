#pragma once

#include "RoundManager.h"
#include "RoundManager_OverwriteWithArchive.generated.h"

UCLASS(minimalapi)
class ARoundManager_OverwriteWithArchive : public ARoundManager
{
	GENERATED_BODY()

public:
	ARoundManager_OverwriteWithArchive(const FObjectInitializer& ObjectInitializer);
	~ARoundManager_OverwriteWithArchive();
	virtual void BeginPlay();
	virtual void RestartRound() override;
private:
	/**
	* @return true if ActorToReset should have Reset() called on it while restarting the game,
	*		   false if the GameMode will manually reset it or if the actor does not need to be reset
	*/
	void InitializeRoundObjects();

	typedef TMap<UObject *, FReloadObjectArc *> ReloadArchiveObjectType;
	ReloadArchiveObjectType reloadObjectArchives;

	UPROPERTY()
	TArray<AActor *> currentRoundActors;
};