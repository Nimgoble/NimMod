// Fill out your copyright notice in the Description page of Project Settings.

#include "NimMod.h"
#include "NimModGameState.h"
#include "NimModGameMode.h"
#include "NimModPlayerController.h"
#include "RoundManager_ForceRespawn.h"
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
	//ARoundManager_ForceRespawn::StaticClass();
	NumTeams = 4;
	RemainingTime = 0;
	bTimerPaused = false;
	for (int i = 0; i <= (int32)NimModTeam::VIP; ++i)
	{
		TeamScores.Add(0);
	}

	//if (!HasAnyFlags(RF_TagGarbageTemp))
	//{
	//	//RoundManager = NewObject<ARoundManager_ForceRespawn>(GetWorld());
	//	RoundManager = NewObject<ANimModRoundManager>(GetWorld());
	//}
}

void ANimModGameState::BeginPlay()
{
	/*if (ISSERVER)
	{
		UE_LOG(LogNimMod, Warning, TEXT("Game State BeginPlay on Server"));
	}
	else
	{
		UE_LOG(LogNimMod, Warning, TEXT("Game State BeginPlay on Client"));
	}*/
	Super::BeginPlay();
	InitializeRoundObjects_ForceRespawn();
}

int32 ANimModGameState::GetTeamScore(NimModTeam team)
{
	return TeamScores[(int)team];
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
	SendClientsMessage("The VIP has escaped!");
	TriggerRoundRestart();
}

void ANimModGameState::VIPKilled()
{
	SendClientsMessage("The VIP has been assassinated!");
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
			gameInstance->SaveTeamScoresForRoundRestart(this->TeamScores);
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
	/*RestartRound_ForceRespawn();
	return;*/
	/*if (RoundManager != nullptr)
		RoundManager->RestartRound();*/

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

bool ANimModGameState::ShouldReset(AActor* ActorToReset)
{
	UClass *actorClass = ActorToReset->GetClass();
	AGameNetworkManager *networkManager = Cast<AGameNetworkManager>(ActorToReset);
	//AParticleEventManager *particleEventManager = Cast<AParticleEventManager>(ActorToReset);
	if
		(
		actorClass->IsChildOf(UWorld::StaticClass()) ||
		actorClass->IsChildOf(APlayerController::StaticClass()) ||
		actorClass->IsChildOf(ACharacter::StaticClass()) ||
		actorClass->IsChildOf(AGameMode::StaticClass()) ||
		actorClass->IsChildOf(APlayerCameraManager::StaticClass()) ||
		actorClass->IsChildOf(APlayerStart::StaticClass()) ||
		actorClass->IsChildOf(AHUD::StaticClass()) ||
		actorClass->IsChildOf(UGameViewportClient::StaticClass()) ||
		actorClass->IsChildOf(AGameSession::StaticClass()) ||
		/*actorClass->IsChildOf(AGameNetworkManager::StaticClass()) ||*/
		actorClass->IsChildOf(APlayerState::StaticClass()) ||
		actorClass->IsChildOf(AWorldSettings::StaticClass()) ||
		actorClass->IsChildOf(AGameState::StaticClass()) ||
		actorClass->IsChildOf(AVIPTrigger::StaticClass()) ||
		actorClass->IsChildOf(ULevel::StaticClass()) ||
		actorClass->IsChildOf(ALight::StaticClass()) ||
		actorClass->IsChildOf(ALevelScriptActor::StaticClass()) ||
		actorClass->IsChildOf(ALightmassImportanceVolume::StaticClass()) ||
		actorClass->IsChildOf(APostProcessVolume::StaticClass()) ||
		actorClass->IsChildOf(ANavigationData::StaticClass()) ||
		actorClass->IsChildOf(AAtmosphericFog::StaticClass()) ||
		actorClass->IsChildOf(ASpectatorPawn::StaticClass()) ||
		/*#ifdef DEBUG*/
		/*actorClass->IsChildOf(AGameplayDebuggingReplicator::StaticClass()) ||*/
		/*#endif*/
		/*particleEventManager != nullptr ||*/
		(AActor *)GetWorld()->MyParticleEventManager == ActorToReset ||
		networkManager != nullptr
		)
		return false;

	ENetMode ourNetMode = this->GetNetMode();
	if (ourNetMode == ENetMode::NM_Client)
	{
		if (ActorToReset->GetNetMode() != ourNetMode)
			return false;
	}

	if (currentRoundActors.Contains(ActorToReset) && ActorToReset->IsPendingKill())
		return true;

	if (ActorToReset->IsRootComponentStatic())
		return false; //I...think...

	return true;
}

void ANimModGameState::InitializeRoundObjects_ForceRespawn()
{
	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* A = *It;
		if (A && A != this && !A->IsA<AController>() && ShouldReset(A))
		{
			currentRoundActors.Add(A);
		}
	}
}
void ANimModGameState::RestartRound_ForceRespawn()
{
	if (ISSERVER)
	{
		UE_LOG(LogNimMod, Warning, TEXT("RestartRound_ForceRespawn on Server"));
	}
	else
	{
		UE_LOG(LogNimMod, Warning, TEXT("RestartRound_ForceRespawn on Client"));
	}
	TArray<AActor *> destroyActors;
	TArray<AActor *> respawnActors;
	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* A = *It;
		if (A && A != this && !A->IsA<AController>() && ShouldReset(A))
		{
			if (currentRoundActors.Contains(A))
			{
				respawnActors.Add(A);
				currentRoundActors.Remove(A);
			}
			else
				destroyActors.Add(A);

			//We will, eventually, be destroying ALL of the actors the get this far.
		}
	}

	//Whatever is left is something that is pending being deleted, or is deleted.
	//Respawn what we can.
	for (AActor *A : currentRoundActors)
	{
		if (A == nullptr)
		{
			destroyActors.Add(A);
			continue;
		}

		if (ShouldReset(A))
		{
			respawnActors.Add(A);
			//destroyActors.Add(A);
		}
	}

	//Respawn the ones that we do want.
	while (respawnActors.Num() > 0)
	{
		AActor *A = respawnActors[0];
		if (A == nullptr)
		{
			respawnActors.RemoveAt(0);
			continue;
		}

		if (ISSERVER)
		{
			//Add a new actor as a copy of the 'reloaded' one, since the reloaded one is disappearing.
			FActorSpawnParameters spawnParams;
			spawnParams.Template = A;

			spawnParams.bNoFail = true;
			spawnParams.bNoCollisionFail = true;
			spawnParams.Name = NAME_None;
			A->SetReplicates(true);
			FVector orig = A->GetActorLocation();
			FRotator rot = A->GetActorRotation();
			AActor *newActor = GetWorld()->SpawnActor
			(
				A->GetClass(),
				&orig,
				&rot,
				spawnParams
			);
			if (newActor == nullptr)
				continue;

			newActor->SetOwner(A->GetOwner());

			currentRoundActors.Add(newActor);
		}

		if (currentRoundActors.Contains(A))
			currentRoundActors.Remove(A);

		//Remove the old one
		destroyActors.Add(A);
		respawnActors.RemoveAt(0);
	}

	//Destroy all of the actors we don't want.
	//while (destroyActors.Num() > 0)
	//{
	//	AActor *actor = destroyActors[0];
	//	destroyActors.RemoveAt(0);
	//	//This can (theoretically) happen if we added something that has already been deleted.
	//	if (actor == nullptr)
	//		continue;

	//	actor->Destroy();
	//	actor = nullptr;
	//}

	// Notify the level script that the level has been reset
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

