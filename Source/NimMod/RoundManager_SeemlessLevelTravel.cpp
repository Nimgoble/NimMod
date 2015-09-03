#include "NimMod.h"
#include "RoundManager_SeemlessLevelTravel.h"


ARoundManager_SeemlessLevelTravel::ARoundManager_SeemlessLevelTravel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RoundCount = 0;
}

void ARoundManager_SeemlessLevelTravel::RestartRound()
{
	InternalRestartRound();
}

void ARoundManager_SeemlessLevelTravel::InternalRestartRound_Implementation()
{
	UWorld *world = GetWorld();
	if (world != nullptr)
	{
		world->SeamlessTravel(GetOriginalMapName());
	}
}