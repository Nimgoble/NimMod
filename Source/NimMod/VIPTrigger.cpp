// Fill out your copyright notice in the Description page of Project Settings.

#include "NimMod.h"
#include "NimModCharacter.h"
#include "NimModPlayerController.h"
#include "NimModPlayerState.h"
#include "NimModGameMode.h"
#include "NimModTypes.h"
#include "VIPTrigger.h"
#include "NimModLevelScriptActor.h"

AVIPTrigger::AVIPTrigger(const class FObjectInitializer& PCIP) : Super(PCIP)
{

}

void AVIPTrigger::ActorEnteredVolume(class AActor* Other)
{
	Super::ActorEnteredVolume(Other);

	/*if (!ISSERVER)
		return;*/

	ANimModCharacter *character = Cast<ANimModCharacter>(Other);
 	if (!character)
		return;

	ANimModPlayerController *pc = character->GetNimModPlayerController();
	if (!pc || !pc->PlayerState)
		return;

	if (pc->GetPlayerTeamNumber() == ENimModTeam::VIP)
	{
		pc->GetNimModPlayerState()->ScorePoints(10);
		//Add to their score
		ANimModGameState *gameState = Cast<ANimModGameState>(GetWorld()->GetGameState());
		int32 teamIndex = ((int32)ENimModTeam::BODYGUARDS);
		gameState->Teams[teamIndex]->TeamScore += 1;
		gameState->VIPEscaped();

	}
}
