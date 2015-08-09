#include "NimMod.h"
#include "NimModTeam.h"

ANimModTeam::ANimModTeam(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
//	bool networkingSupported = UObject::IsSupportedForNetworking();
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ANimModTeam::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANimModTeam, TeamInfo);
	DOREPLIFETIME(ANimModTeam, TeamScore);
}