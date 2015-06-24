#pragma once

#include "Runtime/CoreUObject/Public/Serialization/ArchiveUObject.h"
#include "NimModRoundManager.generated.h"

UCLASS(minimalapi)
class ANimModRoundManager : /*public AActor*/ public AInfo
{
	GENERATED_BODY()

public:
	ANimModRoundManager(const FObjectInitializer& ObjectInitializer);
	~ANimModRoundManager();

	virtual void BeginPlay();

	//virtual void BeginDestroy() override;

	void VIPEscaped();

	void VIPKilled();

private:
	FTimerHandle restartHandle;

	UFUNCTION(Reliable, NetMulticast)
	void TriggerRoundRestart();

	UFUNCTION()
	void OnRestartTimerExpired();

	void SendClientsMessage(FString message);

	//UFUNCTION(Server, Reliable, WithValidation)
	UFUNCTION(Reliable, NetMulticast)
	void RestartRound();

	UFUNCTION(Reliable, NetMulticast)
	void FreezePlayers();

	UFUNCTION(Reliable, NetMulticast)
	void UnfreezePlayers();
	/**
	* @return true if ActorToReset should have Reset() called on it while restarting the game,
	*		   false if the GameMode will manually reset it or if the actor does not need to be reset
	*/
	bool ShouldReset(AActor* ActorToReset);

	//void RestartRound();

	/*UPROPERTY()
	TArray<class AActor *> originalActors;*/

	/*UPROPERTY()*/
	typedef TMap<UObject *, FReloadObjectArc *> ReloadArchiveObjectType;
	ReloadArchiveObjectType reloadObjectArchives;

	UPROPERTY()
	TArray<AActor *> currentRoundActors;

	void InitializeRoundObjects_ForceRespawn();

	void InitializeRoundObjects_OverwriteWithArchive();

	void RestartRound_ForceRespawn();

	void RestartRound_OverwriteWithArchive();

	UPROPERTY()
	FString originalMapName;

	UPROPERTY()
	int32 currentRoundNumber;

	UPROPERTY()
	ULevelStreamingKismet *currentRoundLevel;

	UPROPERTY()
	ULevelStreamingKismet *nextRoundLevel;

	void RestartRound_LoadStreamingLevel();

	ULevelStreamingKismet *LoadRoundLevel();

	UFUNCTION()
	void OnRoundLevelLoaded();

	/*UFUNCTION(Reliable, NetMulticast)
	void SetLoadedLevel(ULevelStreamingKismet *level);*/
};