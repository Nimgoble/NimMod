// Fill out your copyright notice in the Description page of Project Settings.

#include "NimMod.h"
#include "NimModGameState.h"
#include "NimModGameMode.h"
#include "NimModPlayerController.h"

ANimModGameState::ANimModGameState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NumTeams = 4;
	RemainingTime = 0;
	bTimerPaused = false;
	for (int i = 0; i <= (int32)NimModTeam::VIP; ++i)
	{
		TeamScores.Add(0);
	}
}

void ANimModGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANimModGameState, NumTeams);
	DOREPLIFETIME(ANimModGameState, RemainingTime);
	DOREPLIFETIME(ANimModGameState, bTimerPaused);
	DOREPLIFETIME(ANimModGameState, TeamScores);
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
	TriggerRoundRestart();
}

void ANimModGameState::VIPKilled()
{
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
	RestartRound();
}

void ANimModGameState::SendClientsMessage(FString message)
{
#if WITH_SERVER_CODE
	UWorld *world = GetWorld();
	if (!world)
		return;

	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		AController* Controller = *Iterator;
		ANimModPlayerController* PlayerController = Cast<ANimModPlayerController>(Controller);
		if (PlayerController)
		{
			PlayerController->ClientMessage(message, NAME_None, 3.0f);
		}
	}
#endif
}

void ANimModGameState::RestartRound_Implementation()
{
	UWorld *world = GetWorld();
	if (world != nullptr)
	{
		originalMapName = world->GetCurrentLevel()->GetOutermost()->GetName();
		if (GetWorld()->IsPlayInEditor())
		{
			FWorldContext WorldContext = GEngine->GetWorldContextFromWorldChecked(world);
			originalMapName = world->StripPIEPrefixFromPackageName(originalMapName, world->BuildPIEPackagePrefix(WorldContext.PIEInstance));
		}
		world->SeamlessTravel(originalMapName);
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
			PlayerController->SetIgnoreMoveInput(true);
			PlayerController->SetIgnoreLookInput(true);
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
			PlayerController->SetIgnoreMoveInput(false);
			PlayerController->SetIgnoreLookInput(false);
		}
	}
}


