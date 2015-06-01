// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "NimMod.h"
#include "NimModGameMode.h"
#include "NimModHUD.h"
#include "NimModCharacter.h"
#include "NimModGameSession.h"
#include "NimModPlayerController.h"
#include "NimModPlayerState.h"
#include "NimModTeamStart.h"
#include "NimModSpectatorPawn.h"

ANimModGameMode::ANimModGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/NimMod/Blueprints/PlayerPawn"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	//HUDClass = ANimModHUD::StaticClass();

	/*static ConstructorHelpers::FClassFinder<APawn> PlayerPawnOb(TEXT("/Game/Blueprints/Pawns/PlayerPawn"));
	DefaultPawnClass = PlayerPawnOb.Class;*/

	/*static ConstructorHelpers::FClassFinder<APawn> BotPawnOb(TEXT("/Game/Blueprints/Pawns/BotPawn"));
	BotPawnClass = BotPawnOb.Class;*/

	HUDClass = ANimModHUD::StaticClass();
	/*static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassFinder(TEXT("/Game/Blueprints/OurNimModGameMode"));
	PlayerControllerClass = (PlayerControllerClassFinder.Succeeded() == false) ? ANimModPlayerController::StaticClass() : PlayerControllerClassFinder.Class;*/
	PlayerControllerClass = ANimModPlayerController::StaticClass();
	PlayerStateClass = ANimModPlayerState::StaticClass();
	SpectatorClass = ANimModSpectatorPawn::StaticClass();
	GameStateClass = ANimModGameState::StaticClass();

	MinRespawnDelay = 5.0f;

	/*bAllowBots = true;
	bNeedsBotCreation = true;*/
	bUseSeamlessTravel = true;
	bStartPlayersAsSpectators = true;
}

void ANimModGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	/*const int32 BotsCountOptionValue = GetIntOption(Options, GetBotsCountOptionName(), 0);*/
	Super::InitGame(MapName, Options, ErrorMessage);

	/*const UGameInstance* GI = GetGameInstance();
	if (GI && Cast<UNimModInstance>(GI)->GetIsOnline())
	{
		bPauseable = false;
	}*/
}

/** Returns game session class to use */
TSubclassOf<AGameSession> ANimModGameMode::GetGameSessionClass() const
{
	return ANimModGameSession::StaticClass();
}

void ANimModGameMode::DefaultTimer()
{
	Super::DefaultTimer();

	// don't update timers for Play In Editor mode, it's not real match
	//if (GetWorld()->IsPlayInEditor())
	//{
	//	// start match if necessary.
	//	if (GetMatchState() == MatchState::WaitingToStart)
	//	{
	//		StartMatch();
	//	}
	//	return;
	//}

	//ANimModGameState* const MyGameState = Cast<ANimModGameState>(GameState);
	//if (MyGameState && MyGameState->RemainingTime > 0 && !MyGameState->bTimerPaused)
	//{
	//	MyGameState->RemainingTime--;

	//	if (MyGameState->RemainingTime <= 0)
	//	{
	//		if (GetMatchState() == MatchState::WaitingPostMatch)
	//		{
	//			RestartGame();
	//		}
	//		else if (GetMatchState() == MatchState::InProgress)
	//		{
	//			FinishMatch();

	//			// Send end round events
	//			ANimModGameState* const MyGameState = Cast<ANimModGameState>(GameState);

	//			for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	//			{
	//				ANimModPlayerController* PlayerController = Cast<ANimModPlayerController>(*It);

	//				if (PlayerController && MyGameState)
	//				{
	//					ANimModPlayerState* PlayerState = Cast<ANimModPlayerState>((*It)->PlayerState);
	//					const bool bIsWinner = IsWinner(PlayerState);

	//					PlayerController->ClientSendRoundEndEvent(bIsWinner, MyGameState->ElapsedTime);
	//				}
	//			}
	//		}
	//		else if (GetMatchState() == MatchState::WaitingToStart)
	//		{
	//			StartMatch();
	//		}
	//	}
	//}
}

void ANimModGameMode::HandleMatchIsWaitingToStart()
{

	//if (bDelayedStart)
	//{
	//	// start warmup if needed
	//	ANimModGameState* const MyGameState = Cast<ANimModGameState>(GameState);
	//	if (MyGameState && MyGameState->RemainingTime == 0)
	//	{
	//		const bool bWantsMatchWarmup = !GetWorld()->IsPlayInEditor();
	//		if (bWantsMatchWarmup && WarmupTime > 0)
	//		{
	//			MyGameState->RemainingTime = WarmupTime;
	//		}
	//		else
	//		{
	//			MyGameState->RemainingTime = 0.0f;
	//		}
	//	}
	//}
}

void ANimModGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	//ANimModGameState* const MyGameState = Cast<ANimModGameState>(GameState);
	//MyGameState->RemainingTime = RoundTime;

	//// notify players
	//for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	//{
	//	ANimModPlayerController* PC = Cast<ANimModPlayerController>(*It);
	//	if (PC)
	//	{
	//		PC->ClientGameStarted();
	//	}
	//}
}

void ANimModGameMode::FinishMatch()
{
	//ANimModGameState* const MyGameState = GetGameState();
	//if (IsMatchInProgress())
	//{
	//	EndMatch();
	//	DetermineMatchWinner();

	//	// notify players
	//	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	//	{
	//		ANimModPlayerState* PlayerState = Cast<ANimModPlayerState>((*It)->PlayerState);
	//		const bool bIsWinner = IsWinner(PlayerState);

	//		(*It)->GameHasEnded(NULL, bIsWinner);
	//	}

	//	// lock all pawns
	//	// pawns are not marked as keep for seamless travel, so we will create new pawns on the next match rather than
	//	// turning these back on.
	//	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	//	{
	//		(*It)->TurnOff();
	//	}

	//	// set up to restart the match
	//	MyGameState->RemainingTime = TimeBetweenMatches;
	//}
}

void ANimModGameMode::RequestFinishAndExitToMainMenu()
{
	FinishMatch();

	//UNimModInstance* const GI = Cast<UNimModInstance>(GetGameInstance());
	//if (GI)
	//{
	//	GI->RemoveSplitScreenPlayers();
	//}

	//ANimModPlayerController* LocalPrimaryController = nullptr;
	//for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	//{
	//	ANimModPlayerController* Controller = Cast<ANimModPlayerController>(*Iterator);

	//	if (Controller == NULL)
	//	{
	//		continue;
	//	}

	//	if (!Controller->IsLocalController())
	//	{
	//		const FString RemoteReturnReason = NSLOCTEXT("NetworkErrors", "HostHasLeft", "Host has left the game.").ToString();
	//		Controller->ClientReturnToMainMenu(RemoteReturnReason);
	//	}
	//	else
	//	{
	//		LocalPrimaryController = Controller;
	//	}
	//}

	//// GameInstance should be calling this from an EndState.  So call the PC function that performs cleanup, not the one that sets GI state.
	//if (LocalPrimaryController != NULL)
	//{
	//	LocalPrimaryController->HandleReturnToMainMenu();
	//}
}

void ANimModGameMode::DetermineMatchWinner()
{
	// nothing to do here
}

bool ANimModGameMode::IsWinner(class ANimModPlayerState* PlayerState) const
{
	return false;
}

void ANimModGameMode::PreLogin(const FString& Options, const FString& Address, const TSharedPtr<FUniqueNetId>& UniqueId, FString& ErrorMessage)
{
	ANimModGameState* const MyGameState = Cast<ANimModGameState>(GameState);
	const bool bMatchIsOver = MyGameState && MyGameState->HasMatchEnded();
	if (bMatchIsOver)
	{
		ErrorMessage = TEXT("Match is over!");
	}
	else
	{
		// GameSession can be NULL if the match is over
		Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	}
}


void ANimModGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// update spectator location for client
	ANimModPlayerController* NewPC = Cast<ANimModPlayerController>(NewPlayer);
	if (NewPC /*&& NewPC->GetPawn() == NULL*/)
	{
		NewPC->ClientSetSpectatorCamera(NewPC->GetSpawnLocation(), NewPC->GetControlRotation());
		NewPC->ClientPutInServer();
		//NewPC->OnShowTeamMenu();
	}

	// notify new player if match is already in progress
	if (NewPC && IsMatchInProgress())
	{
		NewPC->ClientGameStarted();
		NewPC->ClientStartOnlineGame();
	}
}

bool ANimModGameMode::PlayerCanRestart(APlayerController* Player)
{
	ANimModPlayerController* NewPC = Cast<ANimModPlayerController>(Player);
	if (NewPC)
	{
		if (NewPC->GetPlayerTeam() == NimModTeam::SPECTATORS && !NewPC->HasHadInitialSpawn())
			return false;
	}

	return Super::PlayerCanRestart(Player);
}

void ANimModGameMode::Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	ANimModPlayerState* KillerPlayerState = Killer ? Cast<ANimModPlayerState>(Killer->PlayerState) : NULL;
	ANimModPlayerState* VictimPlayerState = KilledPlayer ? Cast<ANimModPlayerState>(KilledPlayer->PlayerState) : NULL;

	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		KillerPlayerState->ScoreKill(VictimPlayerState, KillScore);
		KillerPlayerState->InformAboutKill(KillerPlayerState, DamageType, VictimPlayerState);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->ScoreDeath(KillerPlayerState, DeathScore);
		VictimPlayerState->BroadcastDeath(KillerPlayerState, DamageType, VictimPlayerState);
	}
}

float ANimModGameMode::ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	float ActualDamage = Damage;

	ANimModCharacter* DamagedPawn = Cast<ANimModCharacter>(DamagedActor);
	if (DamagedPawn && EventInstigator)
	{
		ANimModPlayerState* DamagedPlayerState = Cast<ANimModPlayerState>(DamagedPawn->PlayerState);
		ANimModPlayerState* InstigatorPlayerState = Cast<ANimModPlayerState>(EventInstigator->PlayerState);

		// disable friendly fire
		if (!CanDealDamage(InstigatorPlayerState, DamagedPlayerState))
		{
			ActualDamage = 0.0f;
		}

		// scale self instigated damage
		if (InstigatorPlayerState == DamagedPlayerState)
		{
			ActualDamage *= DamageSelfScale;
		}
	}

	return ActualDamage;
}

bool ANimModGameMode::CanDealDamage(class ANimModPlayerState* DamageInstigator, class ANimModPlayerState* DamagedPlayer) const
{
	return true;
}

bool ANimModGameMode::AllowCheats(APlayerController* P)
{
	return true;
}

bool ANimModGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

UClass* ANimModGameMode::GetDefaultPawnClassForController(AController* InController)
{
	/*if (Cast<ANimModAIController>(InController))
	{
		return BotPawnClass;
	}*/

	return Super::GetDefaultPawnClassForController(InController);
}

AActor* ANimModGameMode::ChoosePlayerStart(AController* Player)
{
	TArray<APlayerStart*> PreferredSpawns;
	TArray<APlayerStart*> FallbackSpawns;

	APlayerStart* BestStart = NULL;
	for (int32 i = 0; i < PlayerStarts.Num(); i++)
	{
		APlayerStart* TestSpawn = PlayerStarts[i];
		if (Cast<APlayerStartPIE>(TestSpawn) != NULL)
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			BestStart = TestSpawn;
			break;
		}
		else if (TestSpawn != NULL)
		{
			if (IsSpawnpointAllowed(TestSpawn, Player))
			{
				if (IsSpawnpointPreferred(TestSpawn, Player))
				{
					PreferredSpawns.Add(TestSpawn);
				}
				else
				{
					FallbackSpawns.Add(TestSpawn);
				}
			}
		}
	}


	if (BestStart == NULL)
	{
		if (PreferredSpawns.Num() > 0)
		{
			BestStart = PreferredSpawns[FMath::RandHelper(PreferredSpawns.Num())];
		}
		else if (FallbackSpawns.Num() > 0)
		{
			BestStart = FallbackSpawns[FMath::RandHelper(FallbackSpawns.Num())];
		}
	}

	return BestStart ? BestStart : Super::ChoosePlayerStart(Player);
}

bool ANimModGameMode::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
{
	ANimModTeamStart* NimModSpawnPoint = Cast<ANimModTeamStart>(SpawnPoint);
	ANimModPlayerController *nimModPlayerController = Cast<ANimModPlayerController>(Player);

	if (NimModSpawnPoint && nimModPlayerController)
	{
		if (NimModSpawnPoint->SpawnTeam == nimModPlayerController->GetNimModPlayerState()->GetTeam())
			return true;
		/*ANimModAIController* AIController = Cast<ANimModAIController>(Player);
		if (NimModSpawnPoint->bNotForBots && AIController)
		{
			return false;
		}

		if (NimModSpawnPoint->bNotForPlayers && AIController == NULL)
		{
			return false;
		}*/
		return false;
	}

	return false;
}

bool ANimModGameMode::IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Player) const
{
	ACharacter* MyPawn = Cast<ACharacter>((*DefaultPawnClass)->GetDefaultObject<ACharacter>());
	/*ANimModAIController* AIController = Cast<ANimModAIController>(Player);
	if (AIController != nullptr)
	{
		MyPawn = Cast<ACharacter>(BotPawnClass->GetDefaultObject<ACharacter>());
	}*/

	if (MyPawn)
	{
		const FVector SpawnLocation = SpawnPoint->GetActorLocation();
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			ACharacter* OtherPawn = Cast<ACharacter>(*It);
			if (OtherPawn && OtherPawn != MyPawn)
			{
				const float CombinedHeight = (MyPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * 2.0f;
				const float CombinedRadius = MyPawn->GetCapsuleComponent()->GetScaledCapsuleRadius() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleRadius();
				const FVector OtherLocation = OtherPawn->GetActorLocation();

				// check if player start overlaps this pawn
				if (FMath::Abs(SpawnLocation.Z - OtherLocation.Z) < CombinedHeight && (SpawnLocation - OtherLocation).Size2D() < CombinedRadius)
				{
					return false;
				}
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

void ANimModGameMode::RestartGame()
{
	// Hide the scoreboard too !
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		ANimModPlayerController* PlayerController = Cast<ANimModPlayerController>(*It);
		if (PlayerController != nullptr)
		{
			ANimModHUD* NimModHUD = Cast<ANimModHUD>(PlayerController->GetHUD());
			if (NimModHUD != nullptr)
			{
				// Passing true to bFocus here ensures that focus is returned to the game viewport.
				//NimModHUD->ShowScoreboard(false, true);
			}
		}
	}

	Super::RestartGame();
}

