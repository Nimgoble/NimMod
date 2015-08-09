// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"
#include "NimModTypes.h"
#include "NimModTeam.h"
#include "NimModGameState.generated.h"

/**
 * 
 */
UCLASS()
class NIMMOD_API ANimModGameState : public AGameState
{
	GENERATED_BODY()
	
public:

	ANimModGameState(const FObjectInitializer& ObjectInitializer);

	virtual void PostInitializeComponents() override;
	virtual void BeginPlay();

	//UPROPERTY(Transient, Replicated, ReplicatedUsing = OnRep_Teams)
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Teams)
	TArray<ANimModTeam *> Teams;

	UFUNCTION(BlueprintCallable, Category = "NimMod|Team")
	ANimModTeam *GetTeam(ENimModTeam teamEnum);

	UFUNCTION()
	void SetTeams(TArray<ANimModTeam *> InTeams);

	UFUNCTION()
	void SetRoundManager(class ARoundManager_ForceRespawn *NewRoundManager);

	/** number of teams in current game (doesn't deprecate when no players are left in a team) */
	/*UPROPERTY(Transient, Replicated)
	int32 NumTeams;*/

	/** accumulated score per team */
	/*UPROPERTY(Transient, Replicated)
	TArray<int32> TeamScores;*/

	UFUNCTION(BlueprintCallable, Category = "NimMod|Team")
	int32 GetTeamScore(ENimModTeam team);

	/** time left for warmup / match */
	UPROPERTY(Transient, Replicated)
	int32 RemainingTime;

	/** is timer paused? */
	UPROPERTY(Transient, Replicated)
	bool bTimerPaused;

	/** gets ranked PlayerState map for specific team */
	//void GetRankedMap(int32 TeamIndex, RankedPlayerMap& OutRankedMap) const;

	void RequestFinishAndExitToMainMenu();

	void VIPEscaped();

	void VIPKilled();

private:
	UFUNCTION()
	void OnRep_Teams(TArray<ANimModTeam *> replicatedTeams);

	FTimerHandle restartHandle;

	UPROPERTY(Transient)
	bool TeamsReady;

	UFUNCTION(Reliable, NetMulticast)
	void TriggerRoundRestart();

	UFUNCTION()
	void OnRestartTimerExpired();

	void SendClientsMessage(FString message);

	UFUNCTION(Reliable, NetMulticast)
	void RestartRound();

	UFUNCTION(Reliable, NetMulticast)
	void FreezePlayers();

	UFUNCTION(Reliable, NetMulticast)
	void UnfreezePlayers();

	UPROPERTY(Transient, Replicated)
	FString originalMapName;

	UPROPERTY(Transient, Replicated)
	class ARoundManager_ForceRespawn *RoundManager;
};
