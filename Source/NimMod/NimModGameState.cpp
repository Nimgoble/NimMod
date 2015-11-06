// Fill out your copyright notice in the Description page of Project Settings.

#include "NimMod.h"
#include "NimModGameState.h"
#include "NimModGameMode.h"
#include "NimModPlayerController.h"
#include "RoundManager.h"
#include "NimModGameInstance.h"
//Includes for the ShouldReset
#include "NimModHUD.h"
#include "NimModCharacter.h"
#include "NimModGameSession.h"
#include "NimModPlayerState.h"
#include "NimModTeamStart.h"
#include "NimModSpectatorPawn.h"
#include "VIPTrigger.h"
#include "Runtime/Engine/Classes/GameFramework/GameNetworkManager.h"
//#include "Runtime/Engine/Classes/Particles/ParticleEventManager.h"
#include "Runtime/Engine/Classes/Lightmass/LightmassImportanceVolume.h"
#include "Runtime/Engine/Classes/AI/Navigation/NavigationData.h"
#include "Runtime/Engine/Classes/Engine/LevelStreaming.h"
//#include "NimModRoundManager.h"

ANimModGameState::ANimModGameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	RemainingTime = 0;
	bTimerPaused = false;
	TeamsReady = false;
}

void ANimModGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ANimModGameState::BeginPlay()
{
	Super::BeginPlay();
}

void ANimModGameState::OnRep_Teams(TArray<ANimModTeam *> replicatedTeams)
{
	TeamsReady = true;
}

ANimModTeam *ANimModGameState::GetTeam(ENimModTeam teamEnum)
{
	if (Teams.Num() == 0)
		return nullptr;

	if (!TeamsReady)
		return nullptr;

	return Teams[(int)teamEnum];
}

void ANimModGameState::SetTeams(TArray<ANimModTeam *> InTeams)
{
	Teams.Empty();
	Teams.Append(InTeams);
	if (AuthorityGameMode)
		TeamsReady = true;
}

void ANimModGameState::SetRoundManager(class ARoundManager *NewRoundManager)
{
	RoundManager = NewRoundManager;
}

int32 ANimModGameState::GetTeamScore(ENimModTeam team)
{
	ANimModTeam *aTeam = GetTeam(team);
	return (aTeam == nullptr) ? -1 : aTeam->TeamScore;
}

void ANimModGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ANimModGameState, NumTeams);
	DOREPLIFETIME(ANimModGameState, RemainingTime);
	DOREPLIFETIME(ANimModGameState, bTimerPaused);
	DOREPLIFETIME(ANimModGameState, Teams);
	DOREPLIFETIME(ANimModGameState, RoundManager);
	//DOREPLIFETIME(ANimModGameState, TeamScores);
}

//void ANimModGameState::GetRankedMap(int32 TeamIndex, RankedPlayerMap& OutRankedMap) const
//{
//	OutRankedMap.Empty();
//
//	//first, we need to go over all the PlayerStates, grab their score, and rank them
//	TMultiMap<int32, ANimModPlayerState*> SortedMap;
//	for (int32 i = 0; i < PlayerArray.Num(); ++i)
//	{
//		int32 Score = 0;
//		ANimModPlayerState* CurPlayerState = Cast<ANimModPlayerState>(PlayerArray[i]);
//		if (CurPlayerState && (CurPlayerState->GetTeamNum() == TeamIndex))
//		{
//			SortedMap.Add(FMath::TruncToInt(CurPlayerState->Score), CurPlayerState);
//		}
//	}
//
//	//sort by the keys
//	SortedMap.KeySort(TGreater<int32>());
//
//	//now, add them back to the ranked map
//	OutRankedMap.Empty();
//
//	int32 Rank = 0;
//	for (TMultiMap<int32, ANimModPlayerState*>::TIterator It(SortedMap); It; ++It)
//	{
//		OutRankedMap.Add(Rank++, It.Value());
//	}
//
//}


void ANimModGameState::RequestFinishAndExitToMainMenu()
{
	if (AuthorityGameMode)
	{
		// we are server, tell the gamemode
		ANimModGameMode* const GameMode = Cast<ANimModGameMode>(AuthorityGameMode);
		if (GameMode)
		{
			GameMode->RequestFinishAndExitToMainMenu();
		}
	}
	else
	{
		// we are client, handle our own business
		/*UNimModGameInstance* GI = Cast<UNimModGameInstance>(GetGameInstance());
		if (GI)
		{
			GI->RemoveSplitScreenPlayers();
		}*/

		/*ANimModPlayerController* const PrimaryPC = Cast<ANimModPlayerController>(GetGameInstance()->GetFirstLocalPlayerController());
		if (PrimaryPC)
		{
			check(PrimaryPC->GetNetMode() == ENetMode::NM_Client);
			PrimaryPC->HandleReturnToMainMenu();
		}*/
	}

}

void ANimModGameState::VIPEscaped()
{
	SendClientsMessage("The VIP has escaped!");
	TriggerRoundRestart();
}

void ANimModGameState::VIPKilled()
{
	SendClientsMessage("The VIP has been assassinated!");
	TriggerRoundRestart();
}

void ANimModGameState::VIPLeft()
{
	SendClientsMessage("The VIP has left.  Restarting round...");
	TriggerRoundRestart();
}

void ANimModGameState::TriggerRoundRestart_Implementation()
{
	FreezePlayers();
	GetWorld()->GetTimerManager().SetTimer(restartHandle, this, &ANimModGameState::OnRestartTimerExpired, 3.0f);
}

void ANimModGameState::OnRestartTimerExpired()
{
	GetWorld()->GetTimerManager().ClearTimer(restartHandle);

	if (ISSERVER)
	{
		UNimModGameInstance *gameInstance = Cast<UNimModGameInstance>(GetGameInstance());
		if (gameInstance)
		{
			gameInstance->SaveTeamsForRoundRestart(this->Teams);
		}
	}

	UWorld *world = GetWorld();
	if (world != nullptr)
	{
		for (FConstControllerIterator Iterator = world->GetControllerIterator(); Iterator; ++Iterator)
		{
			AController* Controller = *Iterator;
			ANimModPlayerController* PlayerController = Cast<ANimModPlayerController>(Controller);
			if (PlayerController)
				PlayerController->ClientRestartRound();
		}
	}

	RestartRound();
	UnfreezePlayers();
}

void ANimModGameState::SendClientsMessage(FString message)
{
	if (AuthorityGameMode)
	{
		// we are server, tell the gamemode
		ANimModGameMode* const GameMode = Cast<ANimModGameMode>(AuthorityGameMode);
		if (GameMode)
		{
			FNimModHUDMessage hudMessage;
			hudMessage.MessageType = ENimModHUDMessageType::MarqueeMessage;
			hudMessage.Message = message;
			hudMessage.TeamOnly = false;
			GameMode->BroadcastHUDMessage(nullptr, hudMessage);
		}
	}
//#if WITH_SERVER_CODE
//	UWorld *world = GetWorld();
//	if (!world)
//		return;
//
//	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
//	{
//		AController* Controller = *Iterator;
//		ANimModPlayerController* PlayerController = Cast<ANimModPlayerController>(Controller);
//		if (PlayerController)
//		{
//			PlayerController->ClientMessage(message, NAME_None, 3.0f);
//		}
//	}
//#endif
}

void ANimModGameState::RestartRound_Implementation()
{

	if (ISSERVER && RoundManager != nullptr)
	{
		RoundManager->RestartRound();
		//return;
	}

	ALevelScriptActor* LevelScript = GetWorld()->GetLevelScriptActor();
	if (LevelScript)
	{
		LevelScript->LevelReset();
	}

	if (ISSERVER)
	{
		//Reset the level
		for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
		{
			AController* Controller = *Iterator;
			ANimModPlayerController* PlayerController = Cast<ANimModPlayerController>(Controller);
			if (PlayerController)
			{
				ANimModGameMode *gameMode = Cast<ANimModGameMode>(GetWorld()->GetAuthGameMode());
				gameMode->RestartPlayer(PlayerController);
				//PlayerController->ClientRestartRound();
				//PlayerController->ServerRestartPlayer();
			}
			else
				Controller->Reset();
		}
	}
}

void ANimModGameState::FreezePlayers_Implementation()
{
	/*if (!ISSERVER)
	return;*/

	//Freeze the players
	UWorld *world = GetWorld();
	if (!world)
		return;

	for (FConstControllerIterator Iterator = world->GetControllerIterator(); Iterator; ++Iterator)
	{
		AController* Controller = *Iterator;
		ANimModPlayerController* PlayerController = Cast<ANimModPlayerController>(Controller);
		if (PlayerController)
		{
			PlayerController->SetFrozen(true);
			/*PlayerController->SetIgnoreMoveInput(true);
			PlayerController->SetIgnoreLookInput(true);*/
		}
	}
}

void ANimModGameState::UnfreezePlayers_Implementation()
{
	/*if (!ISSERVER)
	return;*/

	//Unfreeze the players
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		AController* Controller = *Iterator;
		ANimModPlayerController* PlayerController = Cast<ANimModPlayerController>(Controller);
		if (PlayerController)
		{
			PlayerController->SetFrozen(false);
			/*PlayerController->SetIgnoreMoveInput(false);
			PlayerController->SetIgnoreLookInput(false);*/
		}
	}
}
