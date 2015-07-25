// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerState.h"
#include "NimModTypes.h"
#include "NimModPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class NIMMOD_API ANimModPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ANimModPlayerState(const FObjectInitializer& ObjectInitializer);
	
	// Begin APlayerState interface
	/** clear scores */
	virtual void Reset() override;

	/**
	* Set the team
	*
	* @param	InController	The controller to initialize state with
	*/
	virtual void ClientInitialize(class AController* InController) override;

	virtual void UnregisterPlayerWithSession() override;

	// End APlayerState interface

	/**
	* Set new team and update pawn. Also updates player character team colors.
	*
	* @param	NewTeamNumber	Team we want to be on.
	*/
	void SetTeam(NimModTeam NewTeam);

	/** player killed someone */
	void ScoreKill(ANimModPlayerState* Victim, int32 Points);

	/** player died */
	void ScoreDeath(ANimModPlayerState* KilledBy, int32 Points);

	/** get current team */
	UFUNCTION(BlueprintCallable, Category = "NimMod|Player")
	NimModTeam GetTeam() const;

	/** get number of kills */
	UFUNCTION(BlueprintCallable, Category = "NimMod|Player")
	int32 GetKills() const;

	/** get number of deaths */
	UFUNCTION(BlueprintCallable, Category = "NimMod|Player")
	int32 GetDeaths() const;

	/** get number of points */
	UFUNCTION(BlueprintCallable, Category = "NimMod|Player")
	float GetTotalScore() const;

	/** get number of points */
	UFUNCTION(BlueprintCallable, Category = "NimMod|Player")
	float GetScore() const;

	/** get number of bullets fired this match */
	UFUNCTION(BlueprintCallable, Category = "NimMod|Player")
	int32 GetNumBulletsFired() const;

	/** get number of rockets fired this match */
	UFUNCTION(BlueprintCallable, Category = "NimMod|Player")
	int32 GetNumRocketsFired() const;

	/** get whether the player quit the match */
	bool IsQuitter() const;

	/** gets truncated player name to fit in death log and scoreboards */
	UFUNCTION(BlueprintCallable, Category = "NimMod|Player")
	FString GetShortPlayerName() const;

	/** Sends kill (excluding self) to clients */
	UFUNCTION(Reliable, Client)
	void InformAboutKill(class ANimModPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ANimModPlayerState* KilledPlayerState);

	/** broadcast death to local clients */
	UFUNCTION(Reliable, NetMulticast)
	void BroadcastDeath(class ANimModPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ANimModPlayerState* KilledPlayerState);

	/** replicate team colors. Updated the players mesh colors appropriately */
	UFUNCTION()
	void OnRep_TeamColor();

	//We don't need stats about amount of ammo fired to be server authenticated, so just increment these with local functions
	void AddBulletsFired(int32 NumBullets);
	void AddRocketsFired(int32 NumRockets);
	/** helper for scoring points */
	void ScorePoints(int32 Points);

	/** Set whether the player is a quitter */
	void SetQuitter(bool bInQuitter);

	virtual void CopyProperties(class APlayerState* PlayerState) override;
	virtual void SeamlessTravelTo(class APlayerState* NewPlayerState) override;
	//virtual void Reset() override;
protected:

	/** Set the mesh colors based on the current teamnum variable */
	void UpdateTeamColors();

	/** team number */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_TeamColor)
	NimModTeam Team;

	/** number of kills */
	UPROPERTY(Transient, Replicated)
	int32 TotalScore;

	/** number of kills */
	UPROPERTY(Transient, Replicated)
	int32 NumKills;

	/** number of deaths */
	UPROPERTY(Transient, Replicated)
	int32 NumDeaths;

	/** number of bullets fired this match */
	UPROPERTY()
	int32 NumBulletsFired;

	/** number of rockets fired this match */
	UPROPERTY()
	int32 NumRocketsFired;

	/** whether the user quit the match */
	UPROPERTY()
	uint8 bQuitter : 1;
};
