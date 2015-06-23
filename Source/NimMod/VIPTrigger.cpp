// Fill out your copyright notice in the Description page of Project Settings.

#include "NimMod.h"
#include "NimModCharacter.h"
#include "NimModPlayerController.h"
#include "NimModGameMode.h"
#include "NimModTypes.h"
#include "NimModRoundManager.h"
#include "VIPTrigger.h"

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

	if (pc->GetPlayerTeam() == NimModTeam::VIP)
	{
		//Add to their score
		ANimModGameState *gameState = Cast<ANimModGameState>(GetWorld()->GetGameState());
		int32 teamIndex = ((int32)NimModTeam::VIP);
		gameState->TeamScores[teamIndex] += 10;

		if (RoundManager != nullptr)
		{
			RoundManager->VIPEscaped();
		}

		////Freeze the players and start the countdown to round restart
		//if (RoundManager != nullptr)
		//{
		//	RoundManager->FreezePlayers();
		//}
		//else
		//{
		//	ANimModGameMode *gameMode = Cast<ANimModGameMode>(GetWorld()->GetAuthGameMode());
		//	gameMode->FreezePlayers();
		//}

		///*if (ISSERVER)
		//{*/
		//	GetWorld()->GetTimerManager().SetTimer(restartHandle, this, &AVIPTrigger::RestartLevel, 3.0f);
		////}
	}
}

//Restart, reset; Whatever.
//void AVIPTrigger::RestartLevel()
//{
//	GetWorld()->GetTimerManager().ClearTimer(restartHandle);
//
//	if (RoundManager != nullptr)
//	{
//		RoundManager->RestartRound();
//	}
//	else
//	{
//		ANimModGameMode *gameMode = Cast<ANimModGameMode>(GetWorld()->GetAuthGameMode());
//		//Let's hope this works...
//		gameMode->RestartRound();
//	}
//}

