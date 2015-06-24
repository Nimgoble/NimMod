// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameState.h"
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

	/** number of teams in current game (doesn't deprecate when no players are left in a team) */
	UPROPERTY(Transient, Replicated)
	int32 NumTeams;

	/** accumulated score per team */
	UPROPERTY(Transient, Replicated)
	TArray<int32> TeamScores;

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

	UPROPERTY(Transient, Replicated)
	FString originalMapName;
};
