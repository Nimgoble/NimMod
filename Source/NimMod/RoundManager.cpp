#include "NimMod.h"
#include "RoundManager.h"


ARoundManager::ARoundManager(const FObjectInitializer& ObjectInitializer)
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ARoundManager::RestartRound()
{
}