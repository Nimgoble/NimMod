#include "NimMod.h"
#include "RoundManager.h"
#include "NimModGameMode.h"
#include "NimModHUD.h"
#include "NimModCharacter.h"
#include "NimModGameSession.h"
#include "NimModPlayerController.h"
#include "NimModPlayerState.h"
#include "NimModTeamStart.h"
#include "NimModSpectatorPawn.h"
#include "VIPTrigger.h"
#include "NimModTeam.h"
#include "Runtime/Engine/Classes/GameFramework/GameNetworkManager.h"
//#include "Runtime/Engine/Classes/Particles/ParticleEventManager.h"
#include "Runtime/Engine/Classes/Lightmass/LightmassImportanceVolume.h"
#include "Runtime/Engine/Classes/AI/Navigation/NavigationData.h"
#include "Runtime/Engine/Classes/Engine/LevelStreaming.h"


ARoundManager::ARoundManager(const FObjectInitializer& ObjectInitializer)
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ARoundManager::RestartRound()
{
}

bool ARoundManager::ShouldReset(AActor* ActorToReset)
{
	if (ActorToReset == this)
		return false;

	return ShouldReset(GetWorld(), ActorToReset);
}

bool ARoundManager::ShouldReset(UWorld *world, AActor* ActorToReset)
{
	if (ActorToReset == nullptr)
		return false;

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
		actorClass->IsChildOf(ANimModTeam::StaticClass()) ||
		/*#ifdef DEBUG*/
		/*actorClass->IsChildOf(AGameplayDebuggingReplicator::StaticClass()) ||*/
		/*#endif*/
		/*particleEventManager != nullptr ||*/
		(AActor *)world->MyParticleEventManager == ActorToReset ||
		networkManager != nullptr
	)
		return false;

	/*ENetMode ourNetMode = this->GetNetMode();
	if (ourNetMode == ENetMode::NM_Client)
	{
	if (ActorToReset->GetNetMode() != ourNetMode)
	return false;
	}*/

	/*if (currentRoundActors.Contains(ActorToReset) && ActorToReset->IsPendingKill())
	return true;*/

	if (ActorToReset->IsRootComponentStatic())
		return false; //I...think...

	return true;
}

FString ARoundManager::GetOriginalMapName()
{
	UWorld *world = GetWorld();
	FString originalMapName;
	if (world != nullptr)
	{
		originalMapName = world->GetCurrentLevel()->GetOutermost()->GetName();
		if (GetWorld()->IsPlayInEditor())
		{
			FWorldContext WorldContext = GEngine->GetWorldContextFromWorldChecked(world);
			originalMapName = world->StripPIEPrefixFromPackageName(originalMapName, world->BuildPIEPackagePrefix(WorldContext.PIEInstance));
		}
	}
	return originalMapName;
}