// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NimMod.h"
#include "Runtime/Online/OnlineSubsystem/Public/Online.h"
#include "NimModHUD.h"
#include "NimModLocalPlayer.h"
#include "NimModTeam.h"
#include "NimModTypes.h"
#include "NimModWidgetBase.h"
#include "NimModPlayerController.generated.h"

UCLASS(config = Game)
class ANimModPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

public:
	/** sets spectator location and rotation */
	UFUNCTION(reliable, client)
	void ClientSetSpectatorCamera(FVector CameraLocation, FRotator CameraRotation);

	/** notify player about started match */
	UFUNCTION(reliable, client)
	void ClientGameStarted();

	/** Starts the online game using the session name in the PlayerState */
	UFUNCTION(reliable, client)
	void ClientStartOnlineGame();

	/** Ends the online game using the session name in the PlayerState */
	UFUNCTION(reliable, client)
	void ClientEndOnlineGame();

	UFUNCTION(reliable, client)
	void ClientRestartRound();

	/** notify player about finished match */
	virtual void ClientGameEnded_Implementation(class AActor* EndGameFocus, bool bIsWinner);

	/** Notifies clients to send the end-of-round event */
	UFUNCTION(reliable, client)
	void ClientSendRoundEndEvent(bool bIsWinner, int32 ExpendedTimeInSeconds);

	/** used for input simulation from blueprint (for automatic perf tests) */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void SimulateInputKey(FKey Key, bool bPressed = true);

	/** sends cheat message */
	UFUNCTION(reliable, server, WithValidation)
	void ServerCheat(const FString& Msg);

	/* Overriden Message implementation. */
	//virtual void ClientTeamMessage_Implementation(APlayerState* SenderPlayerState, const FString& S, FName Type, float MsgLifeTime) override;

	UFUNCTION(Reliable, Client)
	void ClientHUDMessage(const FNimModHUDMessage &message);
	//void ClientHUDMessage_Implementation(const FNimModHUDMessage &message);

	void SendHUDMessage(const FNimModHUDMessage &message);

	UFUNCTION(unreliable, server, WithValidation)
	void ServerHUDMessage(const FNimModHUDMessage &message);
	/*bool ServerHUDMessage_Validate(const FNimModHUDMessage &message);
	void ServerHUDMessage_Implementation(const FNimModHUDMessage &message);*/

	/* Tell the HUD to toggle the chat window. */

	void ToggleChatWindow(bool teamChat);

	template<bool IsTeamChat>
	void ToggleChatWindow();

	void CommitChatMessage();

	/** Local function say a string */
	UFUNCTION(exec)
	virtual void Say(const FString& Msg);

	/** RPC for clients to talk to server */
	UFUNCTION(unreliable, server, WithValidation)
	void ServerSay(const FString& Msg);

	/** Local function run an emote */
	// 	UFUNCTION(exec)
	// 	virtual void Emote(const FString& Msg);

	/** notify local client about deaths */
	void OnDeathMessage(class ANimModPlayerState* KillerPlayerState, class ANimModPlayerState* KilledPlayerState, const UDamageType* KillerDamageType);

	/** toggle InGameMenu handler */
	void OnToggleInGameMenu();

	/** Show the in-game menu if it's not already showing */
	void ShowInGameMenu();

	/** Hides scoreboard if currently diplayed */
	void OnConditionalCloseScoreboard();

	/** Toggles scoreboard */
	void OnToggleScoreboard();

	/** shows scoreboard */
	void OnShowScoreboard();

	/** hides scoreboard */
	void OnHideScoreboard();

	/** Do the initialization of the client-side stuff here **/
	UFUNCTION(reliable, client)
	void ClientPutInServer();

	/** Toggles the Team Menu **/
	void OnToggleTeamMenu();
	void OnShowTeamMenu();
	void OnHideTeamMenu();
	void InitializeClientWidgets();

	/*The type of UUserWidget that we should use as our team menu*/
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<class UNimModWidgetBase> TeamMenuWidget;
	/*The instance of our TeamMenuWidget*/
	UPROPERTY()
	UNimModWidgetBase *TeamMenu;

	/*The type of UUserWidget that we should use as our team menu*/
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<class UNimModWidgetBase> ScoreBoardWidget;
	/*The instance of our TeamMenuWidget*/
	UPROPERTY()
	UNimModWidgetBase *ScoreBoard;

	UFUNCTION(BlueprintCallable, Server, reliable, WithValidation, Category = "UI")
	void SetPlayerTeam(ENimModTeam team);

	UFUNCTION(BlueprintCallable, Category = "NimMod|Player|Team")
	ANimModTeam *GetPlayerTeam();

	UFUNCTION(BlueprintCallable, Category = "NimMod|Player|Team")
	ENimModTeam GetPlayerTeamNumber();

	void StartSpectating();
	bool IsSpectating();
	void SpecNext();
	void SpecPrev();

	UFUNCTION(BlueprintCallable, Category = "NimMod|Player")
	class ANimModPlayerCameraManager *GetNimModPlayerCameraManager();

	/** set infinite ammo cheat */
	void SetInfiniteAmmo(bool bEnable);

	/** set infinite clip cheat */
	void SetInfiniteClip(bool bEnable);

	/** set health regen cheat */
	void SetHealthRegen(bool bEnable);

	/** set god mode cheat */
	UFUNCTION(exec)
	void SetGodMode(bool bEnable);

	/** get infinite ammo cheat */
	bool HasInfiniteAmmo() const;

	/** get infinite clip cheat */
	bool HasInfiniteClip() const;

	/** get health regen cheat */
	bool HasHealthRegen() const;

	/** get gode mode cheat */
	bool HasGodMode() const;

	/** check if gameplay related actions (movement, weapon usage, etc) are allowed right now */
	bool IsGameInputAllowed() const;

	/** is game menu currently active? */
	bool IsGameMenuVisible() const;

	/** Ends and/or destroys game session */
	void CleanupSessionOnReturnToMenu();

	bool HasHadInitialSpawn() { return bHasHadInitialSpawn; }

	/**
	* Called when the read achievements request from the server is complete
	*
	* @param PlayerId The player id who is responsible for this delegate being fired
	* @param bWasSuccessful true if the server responded successfully to the request
	*/
	void OnQueryAchievementsComplete(const FUniqueNetId& PlayerId, const bool bWasSuccessful);

	// Begin APlayerController interface

	/** handle weapon visibility */
	virtual void SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning) override;

	/** Returns true if movement input is ignored. Overridden to always allow spectators to move. */
	virtual bool IsMoveInputIgnored() const override;

	/** Returns true if look input is ignored. Overridden to always allow spectators to look around. */
	virtual bool IsLookInputIgnored() const override;

	/** initialize the input system from the player settings */
	virtual void InitInputSystem() override;

	virtual bool SetPause(bool bPause, FCanUnpause CanUnpauseDelegate = FCanUnpause()) override;

	// End APlayerController interface

	// begin ANimModPlayerController-specific

	/**
	* Reads achievements to precache them before first use
	*/
	void QueryAchievements();

	/**
	* Writes a single achievement (unless another write is in progress).
	*
	* @param Id achievement id (string)
	* @param Percent number 1 to 100
	*/
	void UpdateAchievementProgress(const FString& Id, float Percent);

	/** Returns a pointer to the shooter game hud. May return NULL. */
	UFUNCTION(BlueprintCallable, Category = "NimMod|Player")
	ANimModHUD* GetNimModHUD() const;

	UFUNCTION(BlueprintCallable, Category = "NimMod|Player")
	ANimModPlayerState *GetNimModPlayerState() const;

	/** Returns the persistent user record associated with this player, or null if there is't one. */
	class UNimModPersistentUser* GetPersistentUser() const;

	UFUNCTION(BlueprintCallable, Category = "NimMod|Player")
	class ANimModCharacter *GetNimModCharacter() const;

	/** Informs that player fragged someone */
	void OnKill();

	/** Cleans up any resources necessary to return to main menu.  Does not modify GameInstance state. */
	virtual void HandleReturnToMainMenu();

	/** Associate a new UPlayer with this PlayerController. */
	virtual void SetPlayer(UPlayer* Player);

	UNimModLocalPlayer *GetLocalPlayer();

	void SetFrozen(bool frozen);
	bool IsFrozen();

	void SetIsVIP(bool vip);
	bool IsVIP();

	// end ANimModPlayerController-specific

	virtual void PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel) override;

protected:

	/** infinite ammo cheat */
	UPROPERTY(Transient, Replicated)
	uint8 bInfiniteAmmo : 1;

	/** infinite clip cheat */
	UPROPERTY(Transient, Replicated)
	uint8 bInfiniteClip : 1;

	/** health regen cheat */
	UPROPERTY(Transient, Replicated)
	uint8 bHealthRegen : 1;

	/** god mode cheat */
	UPROPERTY(Transient, Replicated)
	uint8 bGodMode : 1;

	/** frozen? */
	UPROPERTY(Transient, Replicated)
	uint8 bFrozen : 1;

	UPROPERTY(Transient, Replicated)
	uint8 bIsVIP : 1;

	/** if set, gameplay related actions (movement, weapn usage, etc) are allowed */
	uint8 bAllowGameActions : 1;

	/** true for the first frame after the game has ended */
	uint8 bGameEndedFrame : 1;

	/** stores pawn location at last player death, used where player scores a kill after they died **/
	FVector LastDeathLocation;

	/** shooter in-game menu */
	TSharedPtr<class FNimModIngameMenu> NimModIngameMenu;

	/** Achievements write object */
	FOnlineAchievementsWritePtr WriteObject;

	/** try to find spot for death cam */
	bool FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation);

	//Begin AActor interface

	/** after all game elements are created */
	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	//End AActor interface

	//Begin AController interface

	/** transition to dead state, retries spawning later */
	virtual void FailedToSpawnPawn() override;

	/** update camera when pawn dies */
	virtual void PawnPendingDestroy(APawn* P) override;

	//End AController interface

	// Begin APlayerController interface

	/** respawn after dying */
	virtual void UnFreeze() override;

	/** sets up input */
	virtual void SetupInputComponent() override;

	/**
	* Called from game info upon end of the game, used to transition to proper state.
	*
	* @param EndGameFocus Actor to set as the view target on end game
	* @param bIsWinner true if this controller is on winning team
	*/
	virtual void GameHasEnded(class AActor* EndGameFocus = NULL, bool bIsWinner = false) override;

	/** Return the client to the main menu gracefully.  ONLY sets GI state. */
	void ClientReturnToMainMenu_Implementation(const FString& ReturnReason) override;

	/** Causes the player to commit suicide */
	UFUNCTION(exec)
	virtual void Suicide();

	/** Notifies the server that the client has suicided */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSuicide();

	/** Updates achievements based on the PersistentUser stats at the end of a round */
	void UpdateAchievementsOnGameEnd();

	/** Updates leaderboard stats at the end of a round */
	void UpdateLeaderboardsOnGameEnd();

	/** Updates the save file at the end of a round */
	void UpdateSaveFileOnGameEnd(bool bIsWinner);

	// End APlayerController interface

	FName	ServerSayString;

	// Timer used for updating friends in the player tick.
	float NimModFriendUpdateTimer;

	// For tracking whether or not to send the end event
	bool bHasSentStartEvents;

	UPROPERTY(Transient, Replicated)
	uint8 bHasHadInitialSpawn : 1;

private:

	/** Handle for efficient management of ClientStartOnlineGame timer */
	FTimerHandle TimerHandle_ClientStartOnlineGame;
};

