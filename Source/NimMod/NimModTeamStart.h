// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NimModTypes.h"
#include "NimModTeamStart.generated.h"

UCLASS()
class ANimModTeamStart : public APlayerStart
{
	GENERATED_UCLASS_BODY()

	/** Which team can start at this point */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = Team)
	ENimModTeam SpawnTeam;

	/** Whether players can start at this point */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = Team)
	uint32 bNotForPlayers : 1;

	/** Whether bots can start at this point */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = Team)
	uint32 bNotForBots : 1;
};
