#include "NimMod.h"
#include "RoundManager_LoadStreamingLevel.h"


ARoundManager_LoadStreamingLevel::ARoundManager_LoadStreamingLevel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RoundCount = 0;
}

void ARoundManager_LoadStreamingLevel::RestartRound()
{
	UWorld *world = GetWorld();
	nextRoundLevel = LoadRoundLevel();
	nextRoundLevel->OnLevelShown.AddDynamic(this, &ARoundManager_LoadStreamingLevel::OnRoundLevelLoaded);
	nextRoundLevel->bShouldBeLoaded = true;
	nextRoundLevel->bShouldBeVisible = true;
}

ULevelStreamingKismet *ARoundManager_LoadStreamingLevel::LoadRoundLevel()
{
	/*ULevelStreamingKismet* StreamingLevel =
	static_cast<ULevelStreamingKismet*>
	(
	StaticConstructObject(ULevelStreamingKismet::StaticClass(), GetWorld(), NAME_None, RF_NoFlags, NULL)
	);*/

	ULevelStreamingKismet* StreamingLevel = NewObject<ULevelStreamingKismet>(GetWorld(), NAME_None, RF_NoFlags, NULL);
	FString originalMapName = GetOriginalMapName();
	int32 currentRoundNumber = RoundCount + 1;
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

void ARoundManager_LoadStreamingLevel::OnRoundLevelLoaded()
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

	//if (ISSERVER)
	//{
	//	//Reset the level
	//	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	//	{
	//		AController* Controller = *Iterator;
	//		ANimModPlayerController* PlayerController = Cast<ANimModPlayerController>(Controller);
	//		if (PlayerController)
	//		{
	//			ANimModGameMode *gameMode = Cast<ANimModGameMode>(GetWorld()->GetAuthGameMode());
	//			gameMode->RestartPlayer(PlayerController);
	//			//PlayerController->ClientRestartRound();
	//			//PlayerController->ServerRestartPlayer();
	//		}
	//		else
	//			Controller->Reset();
	//	}
	//}

	//Unfreeze the players.
	//UnfreezePlayers();
}

//void ARoundManager_LoadStreamingLevel::SetLoadedLevel_Implementation(ULevelStreamingKismet *level)
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