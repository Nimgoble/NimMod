// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "NimMod.h"
/*#include "UI/Menu/NimModIngameMenu.h"
#include "UI/Style/NimModStyle.h"*/
#include "NimModPlayerController.h"
#include "NimModCharacter.h"
#include "NimModPlayerCameraManager.h"
#include "NimModWeapon.h"
#include "NimModPlayerState.h"
#include "NimModLocalPlayer.h"
#include "NimModGameViewportClient.h"
#include "Runtime/Online/OnlineSubsystem/Public/Online.h"
#include "Runtime/Online/OnlineSubsystem/Public/Interfaces/OnlineAchievementsInterface.h"
#include "Runtime/Online/OnlineSubsystem/Public/Interfaces/OnlineEventsInterface.h"
#include "Runtime/Online/OnlineSubsystem/Public/Interfaces/OnlineIdentityInterface.h"
#include "Runtime/Online/OnlineSubsystem/Public/Interfaces/OnlineSessionInterface.h"

#define  ACH_FRAG_SOMEONE	TEXT("ACH_FRAG_SOMEONE")
#define  ACH_SOME_KILLS		TEXT("ACH_SOME_KILLS")
#define  ACH_LOTS_KILLS		TEXT("ACH_LOTS_KILLS")
#define  ACH_FINISH_MATCH	TEXT("ACH_FINISH_MATCH")
#define  ACH_LOTS_MATCHES	TEXT("ACH_LOTS_MATCHES")
#define  ACH_FIRST_WIN		TEXT("ACH_FIRST_WIN")
#define  ACH_LOTS_WIN		TEXT("ACH_LOTS_WIN")
#define  ACH_MANY_WIN		TEXT("ACH_MANY_WIN")
#define  ACH_SHOOT_BULLETS	TEXT("ACH_SHOOT_BULLETS")
#define  ACH_SHOOT_ROCKETS	TEXT("ACH_SHOOT_ROCKETS")
#define  ACH_GOOD_SCORE		TEXT("ACH_GOOD_SCORE")
#define  ACH_GREAT_SCORE	TEXT("ACH_GREAT_SCORE")
#define  ACH_PLAY_SANCTUARY	TEXT("ACH_PLAY_SANCTUARY")
#define  ACH_PLAY_HIGHRISE	TEXT("ACH_PLAY_HIGHRISE")

static const int32 SomeKillsCount = 10;
static const int32 LotsKillsCount = 20;
static const int32 LotsMatchesCount = 5;
static const int32 LotsWinsCount = 3;
static const int32 ManyWinsCount = 5;
static const int32 LotsBulletsCount = 100;
static const int32 LotsRocketsCount = 10;
static const int32 GoodScoreCount = 10;
static const int32 GreatScoreCount = 15;

ANimModPlayerController::ANimModPlayerController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerCameraManagerClass = ANimModPlayerCameraManager::StaticClass();
	//CheatClass = UNimModCheatManager::StaticClass();
	bAllowGameActions = true;
	bGameEndedFrame = false;
	LastDeathLocation = FVector::ZeroVector;

	ServerSayString = TEXT("Say");
	NimModFriendUpdateTimer = 0.0f;
	bHasSentStartEvents = false;
}

void ANimModPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// UI input
	InputComponent->BindAction("InGameMenu", IE_Pressed, this, &ANimModPlayerController::OnToggleInGameMenu);
	InputComponent->BindAction("Scoreboard", IE_Pressed, this, &ANimModPlayerController::OnShowScoreboard);
	InputComponent->BindAction("Scoreboard", IE_Released, this, &ANimModPlayerController::OnHideScoreboard);
	InputComponent->BindAction("ConditionalCloseScoreboard", IE_Pressed, this, &ANimModPlayerController::OnConditionalCloseScoreboard);
	InputComponent->BindAction("ToggleScoreboard", IE_Pressed, this, &ANimModPlayerController::OnToggleScoreboard);

	// voice chat
	InputComponent->BindAction("PushToTalk", IE_Pressed, this, &APlayerController::StartTalking);
	InputComponent->BindAction("PushToTalk", IE_Released, this, &APlayerController::StopTalking);

	InputComponent->BindAction("ToggleChat", IE_Pressed, this, &ANimModPlayerController::ToggleChatWindow);
}


void ANimModPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	//FNimModStyle::Initialize();
	NimModFriendUpdateTimer = 0;
}

void ANimModPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ANimModPlayerController::TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	if (IsGameMenuVisible())
	{
		if (NimModFriendUpdateTimer > 0)
		{
			NimModFriendUpdateTimer -= DeltaTime;
		}
		else
		{
			//TSharedPtr<class FNimModFriends> NimModFriends = NimModIngameMenu->GetNimModFriends();
			//ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
			//if (NimModFriends.IsValid() && LocalPlayer && LocalPlayer->GetControllerId() >= 0)
			//{
			//	NimModFriends->UpdateFriends(LocalPlayer->GetControllerId());
			//}
			//NimModFriendUpdateTimer = 4; //make sure the time between calls is long enough that we won't trigger (0x80552C81) and not exceed the web api rate limit
		}
	}

	// Is this the first frame after the game has ended
	if (bGameEndedFrame)
	{
		bGameEndedFrame = false;

		// ONLY PUT CODE HERE WHICH YOU DON'T WANT TO BE DONE DUE TO HOST LOSS

		// Do we need to show the end of round scoreboard?
		if (IsPrimaryPlayer())
		{
			ANimModHUD* NimModHUD = GetNimModHUD();
			if (NimModHUD)
			{
				/*NimModHUD->ShowScoreboard(true, true);*/
			}
		}
	}
};

void ANimModPlayerController::SetPlayer(UPlayer* InPlayer)
{
	Super::SetPlayer(InPlayer);

	//Build menu only after game is initialized
	/*NimModIngameMenu = MakeShareable(new FNimModIngameMenu());
	NimModIngameMenu->Construct(Cast<ULocalPlayer>(Player));*/
}

void ANimModPlayerController::QueryAchievements()
{
	// precache achievements
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer && LocalPlayer->GetControllerId() != -1)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
			if (Identity.IsValid())
			{
				TSharedPtr<FUniqueNetId> UserId = Identity->GetUniquePlayerId(LocalPlayer->GetControllerId());

				if (UserId.IsValid())
				{
					IOnlineAchievementsPtr Achievements = OnlineSub->GetAchievementsInterface();

					if (Achievements.IsValid())
					{
						Achievements->QueryAchievements(*UserId.Get(), FOnQueryAchievementsCompleteDelegate::CreateUObject(this, &ANimModPlayerController::OnQueryAchievementsComplete));
					}
				}
				else
				{
					UE_LOG(LogOnline, Warning, TEXT("No valid user id for this controller."));
				}
			}
			else
			{
				UE_LOG(LogOnline, Warning, TEXT("No valid identity interface."));
			}
		}
		else
		{
			UE_LOG(LogOnline, Warning, TEXT("No default online subsystem."));
		}
	}
	else
	{
		UE_LOG(LogOnline, Warning, TEXT("No local player, cannot read achievements."));
	}
}

void ANimModPlayerController::OnQueryAchievementsComplete(const FUniqueNetId& PlayerId, const bool bWasSuccessful)
{
	UE_LOG(LogOnline, Display, TEXT("ANimModPlayerController::OnQueryAchievementsComplete(bWasSuccessful = %s)"), bWasSuccessful ? TEXT("TRUE") : TEXT("FALSE"));
}

void ANimModPlayerController::UnFreeze()
{
	ServerRestartPlayer();
}

void ANimModPlayerController::FailedToSpawnPawn()
{
	if (StateName == NAME_Inactive)
	{
		BeginInactiveState();
	}
	Super::FailedToSpawnPawn();
}

void ANimModPlayerController::PawnPendingDestroy(APawn* P)
{
	LastDeathLocation = P->GetActorLocation();
	FVector CameraLocation = LastDeathLocation + FVector(0, 0, 300.0f);
	FRotator CameraRotation(-90.0f, 0.0f, 0.0f);
	FindDeathCameraSpot(CameraLocation, CameraRotation);

	Super::PawnPendingDestroy(P);

	ClientSetSpectatorCamera(CameraLocation, CameraRotation);
}

void ANimModPlayerController::GameHasEnded(class AActor* EndGameFocus, bool bIsWinner)
{
	UpdateSaveFileOnGameEnd(bIsWinner);
	UpdateAchievementsOnGameEnd();
	UpdateLeaderboardsOnGameEnd();

	Super::GameHasEnded(EndGameFocus, bIsWinner);
}

void ANimModPlayerController::ClientSetSpectatorCamera_Implementation(FVector CameraLocation, FRotator CameraRotation)
{
	SetInitialLocationAndRotation(CameraLocation, CameraRotation);
	SetViewTarget(this);
}

bool ANimModPlayerController::FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation)
{
	const FVector PawnLocation = GetPawn()->GetActorLocation();
	FRotator ViewDir = GetControlRotation();
	ViewDir.Pitch = -45.0f;

	const float YawOffsets[] = { 0.0f, -180.0f, 90.0f, -90.0f, 45.0f, -45.0f, 135.0f, -135.0f };
	const float CameraOffset = 600.0f;
	FCollisionQueryParams TraceParams(TEXT("DeathCamera"), true, GetPawn());

	for (int32 i = 0; i < ARRAY_COUNT(YawOffsets); i++)
	{
		FRotator CameraDir = ViewDir;
		CameraDir.Yaw += YawOffsets[i];
		CameraDir.Normalize();

		const FVector TestLocation = PawnLocation - CameraDir.Vector() * CameraOffset;
		const bool bBlocked = GetWorld()->LineTraceTest(PawnLocation, TestLocation, ECC_Camera, TraceParams);

		if (!bBlocked)
		{
			CameraLocation = TestLocation;
			CameraRotation = CameraDir;
			return true;
		}
	}

	return false;
}

bool ANimModPlayerController::ServerCheat_Validate(const FString& Msg)
{
	return true;
}

void ANimModPlayerController::ServerCheat_Implementation(const FString& Msg)
{
	if (CheatManager)
	{
		ClientMessage(ConsoleCommand(Msg));
	}
}

void ANimModPlayerController::SimulateInputKey(FKey Key, bool bPressed)
{
	InputKey(Key, bPressed ? IE_Pressed : IE_Released, 1, false);
}

void ANimModPlayerController::OnKill()
{
	UpdateAchievementProgress(ACH_FRAG_SOMEONE, 100.0f);

	const auto Events = Online::GetEventsInterface();
	const auto Identity = Online::GetIdentityInterface();

	if (Events.IsValid() && Identity.IsValid())
	{
		ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
		if (LocalPlayer)
		{
			int32 UserIndex = LocalPlayer->GetControllerId();
			TSharedPtr<FUniqueNetId> UniqueID = Identity->GetUniquePlayerId(UserIndex);
			if (UniqueID.IsValid())
			{
				ANimModCharacter* Pawn = Cast<ANimModCharacter>(GetCharacter());
				// If player is dead, use location stored during pawn cleanup.
				FVector Location = Pawn ? Pawn->GetActorLocation() : LastDeathLocation;
				ANimModWeapon* Weapon = Pawn ? Pawn->GetWeapon() : 0;
				int32 WeaponType = Weapon ? (int32)Weapon->GetAmmoType() : 0;

				FOnlineEventParms Params;

				Params.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
				Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
				Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused

				Params.Add(TEXT("PlayerRoleId"), FVariantData((int32)0)); // unused
				Params.Add(TEXT("PlayerWeaponId"), FVariantData((int32)WeaponType));
				Params.Add(TEXT("EnemyRoleId"), FVariantData((int32)0)); // unused
				Params.Add(TEXT("EnemyWeaponId"), FVariantData((int32)0)); // untracked			
				Params.Add(TEXT("KillTypeId"), FVariantData((int32)0)); // unused
				Params.Add(TEXT("LocationX"), FVariantData(Location.X));
				Params.Add(TEXT("LocationY"), FVariantData(Location.Y));
				Params.Add(TEXT("LocationZ"), FVariantData(Location.Z));

				Events->TriggerEvent(*UniqueID, TEXT("KillOponent"), Params);
			}
		}
	}
}

void ANimModPlayerController::OnDeathMessage(class ANimModPlayerState* KillerPlayerState, class ANimModPlayerState* KilledPlayerState, const UDamageType* KillerDamageType)
{
	/*ANimModHUD* NimModHUD = GetNimModHUD();
	if (NimModHUD)
	{
		NimModHUD->ShowDeathMessage(KillerPlayerState, KilledPlayerState, KillerDamageType);
	}*/

	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer && LocalPlayer->GetCachedUniqueNetId().IsValid() && KilledPlayerState->UniqueId.IsValid())
	{
		// if this controller is the player who died, update the hero stat.
		if (*LocalPlayer->GetCachedUniqueNetId() == *KilledPlayerState->UniqueId)
		{
			const auto Events = Online::GetEventsInterface();
			const auto Identity = Online::GetIdentityInterface();

			if (Events.IsValid() && Identity.IsValid())
			{
				const int32 UserIndex = LocalPlayer->GetControllerId();
				TSharedPtr<FUniqueNetId> UniqueID = Identity->GetUniquePlayerId(UserIndex);
				if (UniqueID.IsValid())
				{
					ANimModCharacter* Pawn = Cast<ANimModCharacter>(GetCharacter());
					ANimModWeapon* Weapon = Pawn ? Pawn->GetWeapon() : NULL;

					FVector Location = Pawn ? Pawn->GetActorLocation() : FVector::ZeroVector;
					int32 WeaponType = Weapon ? (int32)Weapon->GetAmmoType() : 0;

					FOnlineEventParms Params;
					Params.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
					Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
					Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused

					Params.Add(TEXT("PlayerRoleId"), FVariantData((int32)0)); // unused
					Params.Add(TEXT("PlayerWeaponId"), FVariantData((int32)WeaponType));
					Params.Add(TEXT("EnemyRoleId"), FVariantData((int32)0)); // unused
					Params.Add(TEXT("EnemyWeaponId"), FVariantData((int32)0)); // untracked

					Params.Add(TEXT("LocationX"), FVariantData(Location.X));
					Params.Add(TEXT("LocationY"), FVariantData(Location.Y));
					Params.Add(TEXT("LocationZ"), FVariantData(Location.Z));

					Events->TriggerEvent(*UniqueID, TEXT("PlayerDeath"), Params);
				}
			}
		}
	}
}

void ANimModPlayerController::UpdateAchievementProgress(const FString& Id, float Percent)
{
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
			if (Identity.IsValid())
			{
				TSharedPtr<FUniqueNetId> UserId = LocalPlayer->GetCachedUniqueNetId();

				if (UserId.IsValid())
				{

					IOnlineAchievementsPtr Achievements = OnlineSub->GetAchievementsInterface();
					if (Achievements.IsValid() && (!WriteObject.IsValid() || WriteObject->WriteState != EOnlineAsyncTaskState::InProgress))
					{
						WriteObject = MakeShareable(new FOnlineAchievementsWrite());
						WriteObject->SetFloatStat(*Id, Percent);

						FOnlineAchievementsWriteRef WriteObjectRef = WriteObject.ToSharedRef();
						Achievements->WriteAchievements(*UserId, WriteObjectRef);
					}
					else
					{
						UE_LOG(LogOnline, Warning, TEXT("No valid achievement interface or another write is in progress."));
					}
				}
				else
				{
					UE_LOG(LogOnline, Warning, TEXT("No valid user id for this controller."));
				}
			}
			else
			{
				UE_LOG(LogOnline, Warning, TEXT("No valid identity interface."));
			}
		}
		else
		{
			UE_LOG(LogOnline, Warning, TEXT("No default online subsystem."));
		}
	}
	else
	{
		UE_LOG(LogOnline, Warning, TEXT("No local player, cannot update achievements."));
	}
}

void ANimModPlayerController::OnToggleInGameMenu()
{
	// this is not ideal, but necessary to prevent both players from pausing at the same time on the same frame
	UWorld* GameWorld = GEngine->GameViewport->GetWorld();

	for (auto It = GameWorld->GetControllerIterator(); It; ++It)
	{
		ANimModPlayerController* Controller = Cast<ANimModPlayerController>(*It);
		if (Controller && Controller->IsPaused())
		{
			return;
		}
	}

	// if no one's paused, pause
	/*if (NimModIngameMenu.IsValid())
	{
		NimModIngameMenu->ToggleGameMenu();
	}*/
}

void ANimModPlayerController::OnConditionalCloseScoreboard()
{
	/*ANimModHUD* NimModHUD = GetNimModHUD();
	if (NimModHUD && (NimModHUD->IsMatchOver() == false))
	{
		NimModHUD->ConditionalCloseScoreboard();
	}*/
}

void ANimModPlayerController::OnToggleScoreboard()
{
	/*ANimModHUD* NimModHUD = GetNimModHUD();
	if (NimModHUD && (NimModHUD->IsMatchOver() == false))
	{
		NimModHUD->ToggleScoreboard();
	}*/
}

void ANimModPlayerController::OnShowScoreboard()
{
	/*ANimModHUD* NimModHUD = GetNimModHUD();
	if (NimModHUD)
	{
		NimModHUD->ShowScoreboard(true);
	}*/
}

void ANimModPlayerController::OnHideScoreboard()
{
	//ANimModHUD* NimModHUD = GetNimModHUD();
	//// If have a valid match and the match is over - hide the scoreboard
	//if ((NimModHUD != NULL) && (NimModHUD->IsMatchOver() == false))
	//{
	//	NimModHUD->ShowScoreboard(false);
	//}
}

bool ANimModPlayerController::IsGameMenuVisible() const
{
	bool Result = false;
	/*if (NimModIngameMenu.IsValid())
	{
		Result = NimModIngameMenu->GetIsGameMenuUp();
	}*/

	return Result;
}

void ANimModPlayerController::SetInfiniteAmmo(bool bEnable)
{
	bInfiniteAmmo = bEnable;
}

void ANimModPlayerController::SetInfiniteClip(bool bEnable)
{
	bInfiniteClip = bEnable;
}

void ANimModPlayerController::SetHealthRegen(bool bEnable)
{
	bHealthRegen = bEnable;
}

void ANimModPlayerController::SetGodMode(bool bEnable)
{
	bGodMode = bEnable;
}

void ANimModPlayerController::ClientGameStarted_Implementation()
{
	bAllowGameActions = true;

	// Enable controls mode now the game has started
	SetIgnoreMoveInput(false);

	/*ANimModHUD* NimModHUD = GetNimModHUD();
	if (NimModHUD)
	{
		NimModHUD->SetMatchState(ENimModMatchState::Playing);
		NimModHUD->ShowScoreboard(false);
	}*/
	bGameEndedFrame = false;

	QueryAchievements();

	// Send round start event
	const auto Events = Online::GetEventsInterface();
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);

	if (LocalPlayer != nullptr && Events.IsValid())
	{
		auto UniqueId = LocalPlayer->GetPreferredUniqueNetId();

		if (UniqueId.IsValid())
		{
			// Generate a new session id
			Events->SetPlayerSessionId(*UniqueId, FGuid::NewGuid());

			FString MapName = *FPackageName::GetShortName(GetWorld()->PersistentLevel->GetOutermost()->GetName());

			// Fire session start event for all cases
			FOnlineEventParms Params;
			Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
			Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
			Params.Add(TEXT("MapName"), FVariantData(MapName));

			Events->TriggerEvent(*UniqueId, TEXT("PlayerSessionStart"), Params);

			//// Online matches require the MultiplayerRoundStart event as well
			//UNimModGameInstance* SGI = GetWorld() != NULL ? Cast<UNimModGameInstance>(GetWorld()->GetGameInstance()) : NULL;

			//if (SGI->GetIsOnline())
			//{
			//	FOnlineEventParms MultiplayerParams;

			//	// @todo: fill in with real values
			//	MultiplayerParams.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
			//	MultiplayerParams.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
			//	MultiplayerParams.Add(TEXT("MatchTypeId"), FVariantData((int32)1)); // @todo abstract the specific meaning of this value across platforms
			//	MultiplayerParams.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused

			//	Events->TriggerEvent(*UniqueId, TEXT("MultiplayerRoundStart"), MultiplayerParams);
			//}

			bHasSentStartEvents = true;
		}
	}
}

/** Starts the online game using the session name in the PlayerState */
void ANimModPlayerController::ClientStartOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	ANimModPlayerState* NimModPlayerState = Cast<ANimModPlayerState>(PlayerState);
	if (NimModPlayerState)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				UE_LOG(LogOnline, Log, TEXT("Starting session %s on client"), *NimModPlayerState->SessionName.ToString());
				Sessions->StartSession(NimModPlayerState->SessionName);
			}
		}
	}
	else
	{
		// Keep retrying until player state is replicated
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ClientStartOnlineGame, this, &ANimModPlayerController::ClientStartOnlineGame_Implementation, 0.2f, false);
	}
}

/** Ends the online game using the session name in the PlayerState */
void ANimModPlayerController::ClientEndOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	ANimModPlayerState* NimModPlayerState = Cast<ANimModPlayerState>(PlayerState);
	if (NimModPlayerState)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				UE_LOG(LogOnline, Log, TEXT("Ending session %s on client"), *NimModPlayerState->SessionName.ToString());
				Sessions->EndSession(NimModPlayerState->SessionName);
			}
		}
	}
}

void ANimModPlayerController::HandleReturnToMainMenu()
{
	OnHideScoreboard();
	CleanupSessionOnReturnToMenu();
}

void ANimModPlayerController::ClientReturnToMainMenu_Implementation(const FString& ReturnReason)
{
	//UNimModGameInstance* SGI = GetWorld() != NULL ? Cast<UNimModGameInstance>(GetWorld()->GetGameInstance()) : NULL;

	//if (!ensure(SGI != NULL))
	//{
	//	return;
	//}

	//if (GetNetMode() == NM_Client)
	//{
	//	const FText ReturnReason = NSLOCTEXT("NetworkErrors", "HostQuit", "The host has quit the match.");
	//	const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

	//	SGI->ShowMessageThenGotoState(ReturnReason, OKButton, FText::GetEmpty(), NimModGameInstanceState::MainMenu);
	//}
	//else
	//{
	//	SGI->GotoState(NimModGameInstanceState::MainMenu);
	//}

	//// Clear the flag so we don't do normal end of round stuff next
	//bGameEndedFrame = false;
}

/** Ends and/or destroys game session */
void ANimModPlayerController::CleanupSessionOnReturnToMenu()
{
	/*UNimModGameInstance * SGI = GetWorld() != NULL ? Cast<UNimModGameInstance>(GetWorld()->GetGameInstance()) : NULL;

	if (ensure(SGI != NULL))
	{
		SGI->CleanupSessionOnReturnToMenu();
	}*/
}

void ANimModPlayerController::ClientGameEnded_Implementation(class AActor* EndGameFocus, bool bIsWinner)
{
	Super::ClientGameEnded_Implementation(EndGameFocus, bIsWinner);

	//// Disable controls now the game has ended
	//SetIgnoreMoveInput(true);

	//bAllowGameActions = false;

	//// Make sure that we still have valid view target
	//SetViewTarget(GetPawn());

	//ANimModHUD* NimModHUD = GetNimModHUD();
	//if (NimModHUD)
	//{
	//	NimModHUD->SetMatchState(bIsWinner ? ENimModMatchState::Won : ENimModMatchState::Lost);
	//}

	//UpdateSaveFileOnGameEnd(bIsWinner);
	//UpdateAchievementsOnGameEnd();
	//UpdateLeaderboardsOnGameEnd();

	//// Flag that the game has just ended (if it's ended due to host loss we want to wait for ClientReturnToMainMenu_Implementation first, incase we don't want to process)
	//bGameEndedFrame = true;
}

void ANimModPlayerController::ClientSendRoundEndEvent_Implementation(bool bIsWinner, int32 ExpendedTimeInSeconds)
{
	//const auto Events = Online::GetEventsInterface();
	//ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);

	//if (bHasSentStartEvents && LocalPlayer != nullptr && Events.IsValid())
	//{
	//	auto UniqueId = LocalPlayer->GetPreferredUniqueNetId();

	//	if (UniqueId.IsValid())
	//	{
	//		FString MapName = *FPackageName::GetShortName(GetWorld()->PersistentLevel->GetOutermost()->GetName());
	//		ANimModPlayerState* NimModPlayerState = Cast<ANimModPlayerState>(PlayerState);
	//		int32 PlayerScore = NimModPlayerState ? NimModPlayerState->GetScore() : 0;

	//		// Fire session end event for all cases
	//		FOnlineEventParms Params;
	//		Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
	//		Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
	//		Params.Add(TEXT("ExitStatusId"), FVariantData((int32)0)); // unused
	//		Params.Add(TEXT("PlayerScore"), FVariantData((int32)PlayerScore));
	//		Params.Add(TEXT("PlayerWon"), FVariantData((bool)bIsWinner));
	//		Params.Add(TEXT("MapName"), FVariantData(MapName));
	//		Params.Add(TEXT("MapNameString"), FVariantData(MapName)); // @todo workaround for a bug in backend service, remove when fixed

	//		Events->TriggerEvent(*UniqueId, TEXT("PlayerSessionEnd"), Params);

	//		// Online matches require the MultiplayerRoundEnd event as well
	//		UNimModGameInstance* SGI = GetWorld() != NULL ? Cast<UNimModGameInstance>(GetWorld()->GetGameInstance()) : NULL;
	//		if (SGI->GetIsOnline())
	//		{
	//			FOnlineEventParms MultiplayerParams;

	//			ANimModGameState* const MyGameState = GetWorld() != NULL ? GetWorld()->GetGameState<ANimModGameState>() : NULL;
	//			if (ensure(MyGameState != nullptr))
	//			{
	//				MultiplayerParams.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
	//				MultiplayerParams.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
	//				MultiplayerParams.Add(TEXT("MatchTypeId"), FVariantData((int32)1)); // @todo abstract the specific meaning of this value across platforms
	//				MultiplayerParams.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
	//				MultiplayerParams.Add(TEXT("TimeInSeconds"), FVariantData((float)ExpendedTimeInSeconds));
	//				MultiplayerParams.Add(TEXT("ExitStatusId"), FVariantData((int32)0)); // unused

	//				Events->TriggerEvent(*UniqueId, TEXT("MultiplayerRoundEnd"), MultiplayerParams);
	//			}
	//		}
	//	}

	//	bHasSentStartEvents = false;
	//}
}

void ANimModPlayerController::SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning)
{
	Super::SetCinematicMode(bInCinematicMode, bHidePlayer, bAffectsHUD, bAffectsMovement, bAffectsTurning);

	// If we have a pawn we need to determine if we should show/hide the weapon
	ANimModCharacter* MyPawn = Cast<ANimModCharacter>(GetPawn());
	ANimModWeapon* MyWeapon = MyPawn ? MyPawn->GetWeapon() : NULL;
	if (MyWeapon)
	{
		if (bInCinematicMode && bHidePlayer)
		{
			MyWeapon->SetActorHiddenInGame(true);
		}
		else if (!bCinematicMode)
		{
			MyWeapon->SetActorHiddenInGame(false);
		}
	}
}

bool ANimModPlayerController::IsMoveInputIgnored() const
{
	if (IsInState(NAME_Spectating))
	{
		return false;
	}
	else
	{
		return Super::IsMoveInputIgnored();
	}
}

bool ANimModPlayerController::IsLookInputIgnored() const
{
	if (IsInState(NAME_Spectating))
	{
		return false;
	}
	else
	{
		return Super::IsLookInputIgnored();
	}
}

void ANimModPlayerController::InitInputSystem()
{
	Super::InitInputSystem();

	/*UNimModPersistentUser* PersistentUser = GetPersistentUser();
	if (PersistentUser)
	{
		PersistentUser->TellInputAboutKeybindings();
	}*/
}

void ANimModPlayerController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ANimModPlayerController, bInfiniteAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ANimModPlayerController, bInfiniteClip, COND_OwnerOnly);
}

void ANimModPlayerController::Suicide()
{
	if (IsInState(NAME_Playing))
	{
		ServerSuicide();
	}
}

bool ANimModPlayerController::ServerSuicide_Validate()
{
	return true;
}

void ANimModPlayerController::ServerSuicide_Implementation()
{
	if ((GetPawn() != NULL) && ((GetWorld()->TimeSeconds - GetPawn()->CreationTime > 1) || (GetNetMode() == NM_Standalone)))
	{
		ANimModCharacter* MyPawn = Cast<ANimModCharacter>(GetPawn());
		if (MyPawn)
		{
			MyPawn->Suicide();
		}
	}
}

bool ANimModPlayerController::HasInfiniteAmmo() const
{
	return bInfiniteAmmo;
}

bool ANimModPlayerController::HasInfiniteClip() const
{
	return bInfiniteClip;
}

bool ANimModPlayerController::HasHealthRegen() const
{
	return bHealthRegen;
}

bool ANimModPlayerController::HasGodMode() const
{
	return bGodMode;
}

bool ANimModPlayerController::IsGameInputAllowed() const
{
	return bAllowGameActions && !bCinematicMode;
}

void ANimModPlayerController::ToggleChatWindow()
{
	/*ANimModHUD* NimModHUD = Cast<ANimModHUD>(GetHUD());
	if (NimModHUD)
	{
		NimModHUD->ToggleChat();
	}*/
}

void ANimModPlayerController::ClientTeamMessage_Implementation(APlayerState* SenderPlayerState, const FString& S, FName Type, float MsgLifeTime)
{
	/*ANimModHUD* NimModHUD = Cast<ANimModHUD>(GetHUD());
	if (NimModHUD)
	{
		if (Type == ServerSayString)
		{
			if (SenderPlayerState != PlayerState)
			{
				NimModHUD->AddChatLine(S, false);
			}
		}
	}*/
}

void ANimModPlayerController::Say(const FString& Msg)
{
	ServerSay(Msg.Left(128));
}

bool ANimModPlayerController::ServerSay_Validate(const FString& Msg)
{
	return true;
}

void ANimModPlayerController::ServerSay_Implementation(const FString& Msg)
{
	GetWorld()->GetAuthGameMode()->Broadcast(this, Msg, ServerSayString);
}

ANimModHUD* ANimModPlayerController::GetNimModHUD() const
{
	return Cast<ANimModHUD>(GetHUD());
}


//UNimModPersistentUser* ANimModPlayerController::GetPersistentUser() const
//{
//	UNimModLocalPlayer* const NimModLP = Cast<UNimModLocalPlayer>(Player);
//	return NimModLP ? NimModLP->GetPersistentUser() : nullptr;
//}

bool ANimModPlayerController::SetPause(bool bPause, FCanUnpause CanUnpauseDelegate /*= FCanUnpause()*/)
{
	const bool Result = APlayerController::SetPause(bPause, CanUnpauseDelegate);

	// Update rich presence.
	const auto PresenceInterface = Online::GetPresenceInterface();
	const auto Events = Online::GetEventsInterface();
	const auto LocalPlayer = Cast<ULocalPlayer>(Player);
	TSharedPtr<FUniqueNetId> UserId = LocalPlayer ? LocalPlayer->GetCachedUniqueNetId() : nullptr;

	if (PresenceInterface.IsValid() && UserId.IsValid())
	{
		FOnlineUserPresenceStatus PresenceStatus;
		if (Result && bPause)
		{
			PresenceStatus.Properties.Add(DefaultPresenceKey, FString("Paused"));
		}
		else
		{
			PresenceStatus.Properties.Add(DefaultPresenceKey, FString("InGame"));
		}
		PresenceInterface->SetPresence(*UserId, PresenceStatus);

	}

	// Don't send pause events while online since the game doesn't actually pause
	if (GetNetMode() == NM_Standalone && Events.IsValid() && PlayerState->UniqueId.IsValid())
	{
		FOnlineEventParms Params;
		Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
		Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
		if (Result && bPause)
		{
			Events->TriggerEvent(*PlayerState->UniqueId, TEXT("PlayerSessionPause"), Params);
		}
		else
		{
			Events->TriggerEvent(*PlayerState->UniqueId, TEXT("PlayerSessionResume"), Params);
		}
	}

	return Result;
}

void ANimModPlayerController::ShowInGameMenu()
{
	/*ANimModHUD* NimModHUD = GetNimModHUD();
	if (NimModIngameMenu.IsValid() && !NimModIngameMenu->GetIsGameMenuUp() && NimModHUD && (NimModHUD->IsMatchOver() == false))
	{
		NimModIngameMenu->ToggleGameMenu();
	}*/
}
void ANimModPlayerController::UpdateAchievementsOnGameEnd()
{
	//ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	//if (LocalPlayer)
	//{
	//	ANimModPlayerState* NimModPlayerState = Cast<ANimModPlayerState>(PlayerState);
	//	if (NimModPlayerState)
	//	{
	//		const UNimModPersistentUser*  PersistentUser = GetPersistentUser();

	//		if (PersistentUser)
	//		{
	//			const int32 Wins = PersistentUser->GetWins();
	//			const int32 Losses = PersistentUser->GetLosses();
	//			const int32 Matches = Wins + Losses;

	//			const int32 TotalKills = PersistentUser->GetKills();
	//			const int32 MatchScore = (int32)NimModPlayerState->GetScore();

	//			const int32 TotalBulletsFired = PersistentUser->GetBulletsFired();
	//			const int32 TotalRocketsFired = PersistentUser->GetRocketsFired();

	//			float TotalGameAchievement = 0;
	//			float CurrentGameAchievement = 0;

	//			///////////////////////////////////////
	//			// Kill achievements
	//			if (TotalKills >= 1)
	//			{
	//				CurrentGameAchievement += 100.0f;
	//			}
	//			TotalGameAchievement += 100;

	//			{
	//				float fSomeKillPct = ((float)TotalKills / (float)SomeKillsCount) * 100.0f;
	//				fSomeKillPct = FMath::RoundToFloat(fSomeKillPct);
	//				UpdateAchievementProgress(ACH_SOME_KILLS, fSomeKillPct);

	//				CurrentGameAchievement += FMath::Min(fSomeKillPct, 100.0f);
	//				TotalGameAchievement += 100;
	//			}

	//			{
	//				float fLotsKillPct = ((float)TotalKills / (float)LotsKillsCount) * 100.0f;
	//				fLotsKillPct = FMath::RoundToFloat(fLotsKillPct);
	//				UpdateAchievementProgress(ACH_LOTS_KILLS, fLotsKillPct);

	//				CurrentGameAchievement += FMath::Min(fLotsKillPct, 100.0f);
	//				TotalGameAchievement += 100;
	//			}
	//			///////////////////////////////////////

	//			///////////////////////////////////////
	//			// Match Achievements
	//			{
	//				UpdateAchievementProgress(ACH_FINISH_MATCH, 100.0f);

	//				CurrentGameAchievement += 100;
	//				TotalGameAchievement += 100;
	//			}

	//			{
	//				float fLotsRoundsPct = ((float)Matches / (float)LotsMatchesCount) * 100.0f;
	//				fLotsRoundsPct = FMath::RoundToFloat(fLotsRoundsPct);
	//				UpdateAchievementProgress(ACH_LOTS_MATCHES, fLotsRoundsPct);

	//				CurrentGameAchievement += FMath::Min(fLotsRoundsPct, 100.0f);
	//				TotalGameAchievement += 100;
	//			}
	//			///////////////////////////////////////

	//			///////////////////////////////////////
	//			// Win Achievements
	//			if (Wins >= 1)
	//			{
	//				UpdateAchievementProgress(ACH_FIRST_WIN, 100.0f);

	//				CurrentGameAchievement += 100.0f;
	//			}
	//			TotalGameAchievement += 100;

	//			{
	//				float fLotsWinPct = ((float)Wins / (float)LotsWinsCount) * 100.0f;
	//				fLotsWinPct = FMath::RoundToInt(fLotsWinPct);
	//				UpdateAchievementProgress(ACH_LOTS_WIN, fLotsWinPct);

	//				CurrentGameAchievement += FMath::Min(fLotsWinPct, 100.0f);
	//				TotalGameAchievement += 100;
	//			}

	//			{
	//				float fManyWinPct = ((float)Wins / (float)ManyWinsCount) * 100.0f;
	//				fManyWinPct = FMath::RoundToInt(fManyWinPct);
	//				UpdateAchievementProgress(ACH_MANY_WIN, fManyWinPct);

	//				CurrentGameAchievement += FMath::Min(fManyWinPct, 100.0f);
	//				TotalGameAchievement += 100;
	//			}
	//			///////////////////////////////////////

	//			///////////////////////////////////////
	//			// Ammo Achievements
	//			{
	//				float fLotsBulletsPct = ((float)TotalBulletsFired / (float)LotsBulletsCount) * 100.0f;
	//				fLotsBulletsPct = FMath::RoundToFloat(fLotsBulletsPct);
	//				UpdateAchievementProgress(ACH_SHOOT_BULLETS, fLotsBulletsPct);

	//				CurrentGameAchievement += FMath::Min(fLotsBulletsPct, 100.0f);
	//				TotalGameAchievement += 100;
	//			}

	//			{
	//				float fLotsRocketsPct = ((float)TotalRocketsFired / (float)LotsRocketsCount) * 100.0f;
	//				fLotsRocketsPct = FMath::RoundToFloat(fLotsRocketsPct);
	//				UpdateAchievementProgress(ACH_SHOOT_ROCKETS, fLotsRocketsPct);

	//				CurrentGameAchievement += FMath::Min(fLotsRocketsPct, 100.0f);
	//				TotalGameAchievement += 100;
	//			}
	//			///////////////////////////////////////

	//			///////////////////////////////////////
	//			// Score Achievements
	//			{
	//				float fGoodScorePct = ((float)MatchScore / (float)GoodScoreCount) * 100.0f;
	//				fGoodScorePct = FMath::RoundToFloat(fGoodScorePct);
	//				UpdateAchievementProgress(ACH_GOOD_SCORE, fGoodScorePct);
	//			}

	//			{
	//				float fGreatScorePct = ((float)MatchScore / (float)GreatScoreCount) * 100.0f;
	//				fGreatScorePct = FMath::RoundToFloat(fGreatScorePct);
	//				UpdateAchievementProgress(ACH_GREAT_SCORE, fGreatScorePct);
	//			}
	//			///////////////////////////////////////

	//			///////////////////////////////////////
	//			// Map Play Achievements
	//			UWorld* World = GetWorld();
	//			if (World)
	//			{
	//				FString MapName = *FPackageName::GetShortName(World->PersistentLevel->GetOutermost()->GetName());
	//				if (MapName.Find(TEXT("Highrise")) != -1)
	//				{
	//					UpdateAchievementProgress(ACH_PLAY_HIGHRISE, 100.0f);
	//				}
	//				else if (MapName.Find(TEXT("Sanctuary")) != -1)
	//				{
	//					UpdateAchievementProgress(ACH_PLAY_SANCTUARY, 100.0f);
	//				}
	//			}
	//			///////////////////////////////////////			

	//			const auto Events = Online::GetEventsInterface();
	//			const auto Identity = Online::GetIdentityInterface();

	//			if (Events.IsValid() && Identity.IsValid())
	//			{
	//				const int32 UserIndex = LocalPlayer->GetControllerId();
	//				TSharedPtr<FUniqueNetId> UniqueID = Identity->GetUniquePlayerId(UserIndex);
	//				if (UniqueID.IsValid())
	//				{
	//					FOnlineEventParms Params;

	//					float fGamePct = (CurrentGameAchievement / TotalGameAchievement) * 100.0f;
	//					fGamePct = FMath::RoundToFloat(fGamePct);
	//					Params.Add(TEXT("CompletionPercent"), FVariantData((float)fGamePct));
	//					if (UniqueID.IsValid())
	//					{
	//						Events->TriggerEvent(*UniqueID, TEXT("GameProgress"), Params);
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
}

void ANimModPlayerController::UpdateLeaderboardsOnGameEnd()
{
	//ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	//if (LocalPlayer)
	//{
	//	// update leaderboards
	//	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	//	if (OnlineSub)
	//	{
	//		IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
	//		if (Identity.IsValid())
	//		{
	//			TSharedPtr<FUniqueNetId> UserId = Identity->GetUniquePlayerId(LocalPlayer->GetControllerId());
	//			if (UserId.IsValid())
	//			{
	//				IOnlineLeaderboardsPtr Leaderboards = OnlineSub->GetLeaderboardsInterface();
	//				if (Leaderboards.IsValid())
	//				{
	//					ANimModPlayerState* NimModPlayerState = Cast<ANimModPlayerState>(PlayerState);
	//					if (NimModPlayerState)
	//					{
	//						FNimModAllTimeMatchResultsWrite WriteObject;

	//						WriteObject.SetIntStat(LEADERBOARD_STAT_SCORE, NimModPlayerState->GetKills());
	//						WriteObject.SetIntStat(LEADERBOARD_STAT_KILLS, NimModPlayerState->GetKills());
	//						WriteObject.SetIntStat(LEADERBOARD_STAT_DEATHS, NimModPlayerState->GetDeaths());
	//						WriteObject.SetIntStat(LEADERBOARD_STAT_MATCHESPLAYED, 1);

	//						// the call will copy the user id and write object to its own memory
	//						Leaderboards->WriteLeaderboards(NimModPlayerState->SessionName, *UserId, WriteObject);
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
}

void ANimModPlayerController::UpdateSaveFileOnGameEnd(bool bIsWinner)
{
	//ANimModPlayerState* NimModPlayerState = Cast<ANimModPlayerState>(PlayerState);
	//if (NimModPlayerState)
	//{
	//	// update local saved profile
	//	UNimModPersistentUser* const PersistentUser = GetPersistentUser();
	//	if (PersistentUser)
	//	{
	//		PersistentUser->AddMatchResult(NimModPlayerState->GetKills(), NimModPlayerState->GetDeaths(), NimModPlayerState->GetNumBulletsFired(), NimModPlayerState->GetNumRocketsFired(), bIsWinner);
	//		PersistentUser->SaveIfDirty();
	//	}
	//}
}

void ANimModPlayerController::PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel)
{
	Super::PreClientTravel(PendingURL, TravelType, bIsSeamlessTravel);

	if (GetWorld() != NULL)
	{
		//UNimModGameViewportClient * NimModViewport = Cast<UNimModGameViewportClient>(GetWorld()->GetGameViewport());

		//if (NimModViewport != NULL)
		//{
		//	NimModViewport->ShowLoadingScreen();
		//}

		//ANimModHUD* NimModHUD = Cast<ANimModHUD>(GetHUD());
		//if (NimModHUD != nullptr)
		//{
		//	// Passing true to bFocus here ensures that focus is returned to the game viewport.
		//	NimModHUD->ShowScoreboard(false, true);
		//}
	}
}
