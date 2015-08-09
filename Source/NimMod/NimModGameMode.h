// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include "NimModGameState.h"
#include "NimModTypes.h"
#include "Runtime/CoreUObject/Public/Serialization/ArchiveUObject.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "NimModGameMode.generated.h"

//class FReloadObjectArc;

UCLASS(minimalapi)
class ANimModGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ANimModGameMode(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	/** Initialize the game. This is called before actors' PreInitializeComponents. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/** Accept or reject a player attempting to join the server.  Fails login if you set the ErrorMessage to a non-empty string. */
	virtual void PreLogin(const FString& Options, const FString& Address, const TSharedPtr<class FUniqueNetId>& UniqueId, FString& ErrorMessage) override;

	/** starts match warmup */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting);

	virtual void PostSeamlessTravel() override;

	/** @return true if it's valid to call RestartPlayer. Will call Player->CanRestartPlayer */
	bool PlayerCanRestart_Implementation(APlayerController* Player) override;

	/** select best spawn point for player */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/** always pick new random spawn */
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;

	/** returns default pawn class for given controller */
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	/** prevents friendly fire */
	virtual float ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;

	/** notify about kills */
	virtual void Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType);

	/** can players damage each other? */
	virtual bool CanDealDamage(class ANimModPlayerState* DamageInstigator, class ANimModPlayerState* DamagedPlayer) const;

	/** always create cheat manager */
	virtual bool AllowCheats(APlayerController* P) override;

	/** update remaining time */
	//virtual void DefaultTimer() override;

	/** called before startmatch */
	virtual void HandleMatchIsWaitingToStart() override;

	/** starts new match */
	virtual void HandleMatchHasStarted() override;

	/** hides the onscreen hud and restarts the map */
	virtual void RestartGame() override;

	ANimModGameState * GetGameState(){ return Cast<ANimModGameState>(GameState); }

	/*UFUNCTION(BlueprintCallable, Category = "NimMod|Online")
	void RegisterServer(FString serverName, FString mapName, int32 maxNumberOfPlayers, bool isLAN);
	UFUNCTION(BlueprintCallable, Category = "NimMod|Online")
	void UnRegisterServer();*/

	void BroadcastHUDMessage(class ANimModPlayerController *controller, FNimModHUDMessage message);

	virtual void GetSeamlessTravelActorList(bool bToEntry, TArray<AActor*>& ActorList);
protected:

	bool ShouldActorTravel(AActor *Actor);

	void UpdateServerPlayerCount();
	/*UPROPERTY()
	FString ServerIP;

	bool GetServerIP();
	void GetServerIPResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);*/

	/** delay between first player login and starting match */
	UPROPERTY(config)
	int32 WarmupTime;

	/** match duration */
	UPROPERTY(config)
	int32 RoundTime;

	UPROPERTY(config)
	int32 TimeBetweenMatches;

	/** score for kill */
	UPROPERTY(config)
	int32 KillScore;

	/** score for death */
	UPROPERTY(config)
	int32 DeathScore;

	/** scale for self instigated damage */
	UPROPERTY(config)
	float DamageSelfScale;

	/** check who won */
	virtual void DetermineMatchWinner();

	/** check if PlayerState is a winner */
	virtual bool IsWinner(class ANimModPlayerState* PlayerState) const;

	/** check if player can use spawnpoint */
	virtual bool IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const;

	/** check if player should use spawnpoint */
	virtual bool IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Player) const;

	/** Returns game session class to use */
	virtual TSubclassOf<AGameSession> GetGameSessionClass() const override;

public:

	/** Does end of game handling for the online layer */
	virtual void RestartPlayer(class AController* NewPlayer);

	/** finish current match and lock players */
	UFUNCTION(exec)
	void FinishMatch();

	/*Finishes the match and bumps everyone to main menu.*/
	/*Only GameInstance should call this function */
	void RequestFinishAndExitToMainMenu();

	/** get the name of the bots count option used in server travel URL */
	static FString GetBotsCountOptionName();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NimMod|Team")
	TArray<FNimModTeamInfo> TeamDefinitions;

private:
	TArray<ANimModTeam *> Teams;

	class ARoundManager_ForceRespawn *RoundManager;
};



