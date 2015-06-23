// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "NimMod.h"
#include "NimModRoundManager.h"
#include "NimModGameMode.h"
#include "NimModHUD.h"
#include "NimModCharacter.h"
#include "NimModGameSession.h"
#include "NimModPlayerController.h"
#include "NimModPlayerState.h"
#include "NimModTeamStart.h"
#include "NimModSpectatorPawn.h"
#include "VIPTrigger.h"
#include "Runtime/Engine/Classes/GameFramework/GameNetworkManager.h"
//#include "Runtime/Engine/Classes/Particles/ParticleEventManager.h"
#include "Runtime/Engine/Classes/Lightmass/LightmassImportanceVolume.h"
#include "Runtime/Engine/Classes/AI/Navigation/NavigationData.h"
#include "Runtime/Engine/Classes/Engine/LevelStreaming.h"

//#ifdef DEBUG
#include "Developer/GameplayDebugger/Classes/GameplayDebuggingReplicator.h"
//#endif

ANimModRoundManager::ANimModRoundManager(const FObjectInitializer& ObjectInitializer) : 
	Super(ObjectInitializer/*.DoNotCreateDefaultSubobject(TEXT("Sprite"))*/)
{
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bReplicateMovement = true;
	bAlwaysRelevant = true;
}

ANimModRoundManager::~ANimModRoundManager()
{
	ReloadArchiveObjectType::TIterator iter = reloadObjectArchives.CreateIterator();
	for (iter; iter; ++iter)
	{
		FReloadObjectArc *reloadArchive = (iter->Value);
		if (reloadArchive == nullptr)
			continue;

		reloadArchive->Close();
		reloadArchive->Flush();
		delete reloadArchive;
		reloadArchive = nullptr;
	}
}

void ANimModRoundManager::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld() != nullptr)
	{
		originalMapName = GetWorld()->GetCurrentLevel()->GetOutermost()->GetName();
		if (GetWorld()->IsPlayInEditor())
		{
			FWorldContext WorldContext = GEngine->GetWorldContextFromWorldChecked(GetWorld());
			originalMapName = GetWorld()->StripPIEPrefixFromPackageName(originalMapName, GetWorld()->BuildPIEPackagePrefix(WorldContext.PIEInstance));
		}
		AGameMode *genericGameMode = GetWorld()->GetAuthGameMode();
		if (genericGameMode == nullptr)
			return;

		ANimModGameMode *gameMode = Cast<ANimModGameMode>(genericGameMode);
		if (gameMode)
			gameMode->SetCurrentRoundManager(this);
	}

	//InitializeRoundObjects_OverwriteWithArchive();
	//InitializeRoundObjects_ForceRespawn();
}

void ANimModRoundManager::VIPEscaped()
{
	TriggerRoundRestart();
}

void ANimModRoundManager::VIPKilled()
{
	TriggerRoundRestart();
}

void ANimModRoundManager::TriggerRoundRestart_Implementation()
{
	FreezePlayers();
	GetWorld()->GetTimerManager().SetTimer(restartHandle, this, &ANimModRoundManager::OnRestartTimerExpired, 3.0f);
}

void ANimModRoundManager::OnRestartTimerExpired()
{
	GetWorld()->GetTimerManager().ClearTimer(restartHandle);
	RestartRound();
}

void ANimModRoundManager::SendClientsMessage(FString message)
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

bool ANimModRoundManager::ShouldReset(AActor* ActorToReset)
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
		actorClass->IsChildOf(AGameplayDebuggingReplicator::StaticClass()) ||
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

//bool ANimModRoundManager::RestartRound_Validate()
//{
//	return true;
//}

void ANimModRoundManager::RestartRound_Implementation()
{
	GetWorld()->SeamlessTravel(originalMapName);
	//GetWorld()->SeamlessTravel(TEXT("?Restart"));
	return;
	//Players should already be frozen
	//FreezePlayers();

	/*RestartRound_LoadStreamingLevel();
	return;*/

	// Reset all actors (except controllers, the GameMode, and any other actors specified by ShouldReset())
	//TODO: This doesn't handle actors that are in currentRoundActors that have been destroyed before this point.
	//We need to find a way to respawn those, as well.

	//RestartRound_OverwriteWithArchive();

	//RestartRound_ForceRespawn();

	//GetWorld()->ForceGarbageCollection(true);

	// reset the GameMode
	//Reset();

	/*if (!ISSERVER)
	return;*/

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

	//Unfreeze the players.
	UnfreezePlayers();
}

void ANimModRoundManager::FreezePlayers_Implementation()
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

void ANimModRoundManager::UnfreezePlayers_Implementation()
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
void ANimModRoundManager::RestartRound_ForceRespawn()
{
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

			/*spawnParams.bNoFail = true;
			spawnParams.bNoCollisionFail = true;
			spawnParams.Name = NAME_None;*/
			//A->SetReplicates(true);
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

			//newActor->SetOwner(A->GetOwner());

			currentRoundActors.Add(newActor);
		}

		if (currentRoundActors.Contains(A))
			currentRoundActors.Remove(A);

		//Remove the old one
		destroyActors.Add(A);
		respawnActors.RemoveAt(0);
	}

	//Destroy all of the actors we don't want.
	while (destroyActors.Num() > 0)
	{
		AActor *actor = destroyActors[0];
		destroyActors.RemoveAt(0);
		//This can (theoretically) happen if we added something that has already been deleted.
		if (actor == nullptr)
			continue;

		actor->Destroy();
		actor = nullptr;
	}
}

void ANimModRoundManager::InitializeRoundObjects_ForceRespawn()
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

void ANimModRoundManager::InitializeRoundObjects_OverwriteWithArchive()
{
	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* A = *It;
		if (A && A != this && !A->IsA<AController>() && ShouldReset(A))
		{

			FReloadObjectArc *reloadArchive = new FReloadObjectArc();
			reloadArchive->ActivateWriter();
			reloadArchive->SerializeObject(A);
			reloadObjectArchives.Add(A, reloadArchive);
			reloadArchive->Reset();
			//reloadArchive << A;
		}
	}
}

void ANimModRoundManager::RestartRound_OverwriteWithArchive()
{
	TArray<AActor *> destroyActors;
	TArray<AActor *> respawnActors;
	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* A = *It;
		if (A && A != this && !A->IsA<AController>() && ShouldReset(A))
		{
			FReloadObjectArc **reloadArchivePtr = reloadObjectArchives.Find(A);
			if (reloadArchivePtr == nullptr)
			{
				destroyActors.Add(A);
				continue;
			}
			FReloadObjectArc *reloadArchive = *reloadArchivePtr;
			if (reloadArchive == nullptr)
			{
				destroyActors.Add(A);
				continue;
			}
			reloadArchive->Reset();

			if (reloadArchive->IsSaving())
				reloadArchive->ActivateReader();
			//A->Reset();
			//A->RerunConstructionScripts();

			//Shotgun approach:
			/*A->ClearInstanceComponents(true);
			A->Reset();
			A->DestroyConstructedComponents();*/

			reloadArchive->SerializeObject(A);
		}
	}

	//Whatever is left is something that is pending being deleted, or is deleted.
	//Respawn what we can.
	//for (AActor *A : currentRoundActors)
	//{
	//	if (A == nullptr)
	//	{
	//		destroyActors.Add(A);
	//		continue;
	//	}

	//	if (ShouldReset(A))
	//	{
	//		respawnActors.Add(A);
	//		//destroyActors.Add(A);
	//	}
	//}

	////Respawn the ones that we do want.
	//while (respawnActors.Num() > 0)
	//{
	//	AActor *A = respawnActors[0];
	//	if (A == nullptr)
	//	{
	//		respawnActors.RemoveAt(0);
	//		continue;
	//	}

	//	FReloadObjectArc *reloadArchive = *reloadObjectArchives.Find(A);
	//	/*A->Reset();*/

	//	//Add a new actor as a copy of the 'reloaded' one, since the reloaded one is disappearing.

	//	/*FActorSpawnParameters spawnParams;
	//	spawnParams.Template = A;*/

	//	/*spawnParams.bNoFail = true;
	//	spawnParams.bNoCollisionFail = true;
	//	spawnParams.Name = NAME_None;*/
	//	//A->SetReplicates(true);
	//	/*FVector orig = A->GetActorLocation();
	//	FRotator rot = A->GetActorRotation();
	//	AActor *newActor = GetWorld()->SpawnActor
	//	(
	//	A->GetClass(),
	//	&orig,
	//	&rot,
	//	spawnParams
	//	);
	//	if (newActor == nullptr)
	//	continue;*/

	//	//newActor->SetOwner(A->GetOwner());

	//	//Add the new one
	//	reloadObjectArchives.Add(newActor, reloadArchive);


	//	//Remove the old one
	//	reloadObjectArchives.Remove(A);
	//	destroyActors.Add(A);
	//	respawnActors.RemoveAt(0);
	//}

	//Destroy all of the actors we don't want.
	while (destroyActors.Num() > 0)
	{
		AActor *actor = destroyActors[0];
		destroyActors.RemoveAt(0);
		//This can (theoretically) happen if we added something that has already been deleted.
		if (actor == nullptr)
			continue;

		actor->Destroy();
		actor = nullptr;
	}
}

void ANimModRoundManager::RestartRound_LoadStreamingLevel()
{
	/*if (!ISSERVER)
		return;*/

	UWorld *world = GetWorld();
	nextRoundLevel = LoadRoundLevel();
	nextRoundLevel->OnLevelShown.AddDynamic(this, &ANimModRoundManager::OnRoundLevelLoaded);
	nextRoundLevel->bShouldBeLoaded = true;
	nextRoundLevel->bShouldBeVisible = true;
}

ULevelStreamingKismet *ANimModRoundManager::LoadRoundLevel()
{
	/*ULevelStreamingKismet* StreamingLevel =
	static_cast<ULevelStreamingKismet*>
	(
	StaticConstructObject(ULevelStreamingKismet::StaticClass(), GetWorld(), NAME_None, RF_NoFlags, NULL)
	);*/

	ULevelStreamingKismet* StreamingLevel = NewObject<ULevelStreamingKismet>(GetWorld(), NAME_None, RF_NoFlags, NULL);
	currentRoundNumber++;
	FString uniqueEncounterName = FString::Printf(TEXT("%s_%d"), *originalMapName, currentRoundNumber);
	FName UniquePathName = FName(*uniqueEncounterName);
	StreamingLevel->SetWorldAssetByPackageName(UniquePathName);
	// Associate a package name.
	if (GetWorld()->IsPlayInEditor())
	{
		FWorldContext WorldContext = GEngine->GetWorldContextFromWorldChecked(GetWorld());
		StreamingLevel->RenameForPIE(WorldContext.PIEInstance);
	}

	StreamingLevel->LevelColor = FColor::MakeRandomColor();
	StreamingLevel->bShouldBeLoaded = false;
	StreamingLevel->bShouldBeVisible = false;
	StreamingLevel->bShouldBlockOnLoad = false;
	StreamingLevel->bInitiallyLoaded = false;
	StreamingLevel->bInitiallyVisible = false;

	//TODO: Calculate the transform of this new streaming level in our list
	FVector location = (currentRoundNumber % 2 == 0) ? FVector(0.0f, 0.0f, 0.0f) : FVector(1000.0f, 1000.0f, 1000.0f);
	StreamingLevel->LevelTransform = FTransform(location);// where to put it

	StreamingLevel->PackageNameToLoad = FName(*originalMapName);// PackageName containing level to load

	FString PackageFileName;
	if (!FPackageName::DoesPackageExist(StreamingLevel->PackageNameToLoad.ToString(), NULL, &PackageFileName))
	{
		//UE_LOG(LogStreamingLevel, Error, TEXT("Trying to load invalid level %s"), *StreamingLevel->PackageNameToLoad.ToString());
		return false;
	}

	StreamingLevel->PackageNameToLoad = FName(*FPackageName::FilenameToLongPackageName(PackageFileName));

	// Add the new level to world.
	GetWorld()->StreamingLevels.Add(StreamingLevel);

	return StreamingLevel;
}

void ANimModRoundManager::OnRoundLevelLoaded()
{
	//SetLoadedLevel(level)

	UWorld *world = GetWorld();
	//Get the level we just played in
	ULevel *currentLevel = (currentRoundLevel == nullptr) ? world->GetCurrentLevel() : currentRoundLevel->GetLoadedLevel();
	//Set the current level to the newly loaded level
	world->SetCurrentLevel(nextRoundLevel->GetLoadedLevel());
	//unload the old level
	world->RemoveLevel(currentLevel);
	//Update our pointers
	currentRoundLevel = nextRoundLevel;
	nextRoundLevel = nullptr;

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

	//Unfreeze the players.
	UnfreezePlayers();
}

//void ANimModRoundManager::SetLoadedLevel_Implementation(ULevelStreamingKismet *level)
//{
//	UWorld *world = GetWorld();
//	//Get the level we just played in
//	ULevel *currentLevel = (currentRoundLevel == nullptr) ? world->GetCurrentLevel() : currentRoundLevel->GetLoadedLevel();
//	//Set the current level to the newly loaded level
//	world->SetCurrentLevel(nextRoundLevel->GetLoadedLevel());
//	//unload the old level
//	world->RemoveLevel(currentLevel);
//	//Update our pointers
//	currentRoundLevel = nextRoundLevel;
//	nextRoundLevel = nullptr;
//}