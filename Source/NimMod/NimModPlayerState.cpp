// Fill out your copyright notice in the Description page of Project Settings.

#include "NimMod.h"
#include "NimModPlayerState.h"
#include "NimModCharacter.h"
#include "NimModGameState.h"
#include "NimModPlayerController.h"

ANimModPlayerState::ANimModPlayerState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Team = NimModTeam::SPECTATORS;
	NumKills = 0;
	NumDeaths = 0;
	NumBulletsFired = 0;
	NumRocketsFired = 0;
	bQuitter = false;
}

void ANimModPlayerState::Reset()
{
	Super::Reset();

	//PlayerStates persist across seamless travel.  Keep the same teams as previous match.
	//SetTeamNum(0);
	NumKills = 0;
	NumDeaths = 0;
	NumBulletsFired = 0;
	NumRocketsFired = 0;
	bQuitter = false;
}

void ANimModPlayerState::UnregisterPlayerWithSession()
{
	if (!bFromPreviousLevel)
	{
		Super::UnregisterPlayerWithSession();
	}
}

void ANimModPlayerState::ClientInitialize(class AController* InController)
{
	Super::ClientInitialize(InController);

	UpdateTeamColors();
}

void ANimModPlayerState::SetTeam(NimModTeam NewTeam)
{
	Team = NewTeam;

	UpdateTeamColors();
}

void ANimModPlayerState::OnRep_TeamColor()
{
	UpdateTeamColors();
}

void ANimModPlayerState::AddBulletsFired(int32 NumBullets)
{
	NumBulletsFired += NumBullets;
}

void ANimModPlayerState::AddRocketsFired(int32 NumRockets)
{
	NumRocketsFired += NumRockets;
}

void ANimModPlayerState::SetQuitter(bool bInQuitter)
{
	bQuitter = bInQuitter;
}

void ANimModPlayerState::CopyProperties(class APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	ANimModPlayerState* NimModPlayer = Cast<ANimModPlayerState>(PlayerState);
	if (NimModPlayer)
	{
		NimModPlayer->Team = Team;
	}
}

void ANimModPlayerState::UpdateTeamColors()
{
	AController* OwnerController = Cast<AController>(GetOwner());
	if (OwnerController != NULL)
	{
		ANimModCharacter* NimModCharacter = Cast<ANimModCharacter>(OwnerController->GetCharacter());
		if (NimModCharacter != NULL)
		{
			NimModCharacter->UpdateTeamColorsAllMIDs();
		}
	}
}

NimModTeam ANimModPlayerState::GetTeam() const
{
	return Team;
}

int32 ANimModPlayerState::GetKills() const
{
	return NumKills;
}

int32 ANimModPlayerState::GetDeaths() const
{
	return NumDeaths;
}

float ANimModPlayerState::GetScore() const
{
	return Score;
}

int32 ANimModPlayerState::GetNumBulletsFired() const
{
	return NumBulletsFired;
}

int32 ANimModPlayerState::GetNumRocketsFired() const
{
	return NumRocketsFired;
}

bool ANimModPlayerState::IsQuitter() const
{
	return bQuitter;
}

void ANimModPlayerState::ScoreKill(ANimModPlayerState* Victim, int32 Points)
{
	NumKills++;
	ScorePoints(Points);
}

void ANimModPlayerState::ScoreDeath(ANimModPlayerState* KilledBy, int32 Points)
{
	NumDeaths++;
	ScorePoints(Points);
}

void ANimModPlayerState::ScorePoints(int32 Points)
{
	ANimModGameState* const MyGameState = Cast<ANimModGameState>(GetWorld()->GameState);
	uint8 teamNumber = (uint8)Team;
	if (MyGameState && teamNumber >= 0)
	{
		if (teamNumber >= MyGameState->TeamScores.Num())
		{
			MyGameState->TeamScores.AddZeroed(teamNumber - MyGameState->TeamScores.Num() + 1);
		}

		MyGameState->TeamScores[teamNumber] += Points;
	}

	Score += Points;
}

void ANimModPlayerState::InformAboutKill_Implementation(class ANimModPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ANimModPlayerState* KilledPlayerState)
{
	//id can be null for bots
	if (KillerPlayerState->UniqueId.IsValid())
	{
		//search for the actual killer before calling OnKill()	
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			ANimModPlayerController* TestPC = Cast<ANimModPlayerController>(*It);
			if (TestPC && TestPC->IsLocalController())
			{
				// a local player might not have an ID if it was created with CreateDebugPlayer.
				ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(TestPC->Player);
				TSharedPtr<FUniqueNetId> LocalID = LocalPlayer->GetCachedUniqueNetId();
				if (LocalID.IsValid() && *LocalPlayer->GetCachedUniqueNetId() == *KillerPlayerState->UniqueId)
				{
					TestPC->OnKill();
				}
			}
		}
	}
}

void ANimModPlayerState::BroadcastDeath_Implementation(class ANimModPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class ANimModPlayerState* KilledPlayerState)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		// all local players get death messages so they can update their huds.
		ANimModPlayerController* TestPC = Cast<ANimModPlayerController>(*It);
		if (TestPC && TestPC->IsLocalController())
		{
			TestPC->OnDeathMessage(KillerPlayerState, this, KillerDamageType);
		}
	}
}

void ANimModPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANimModPlayerState, Team);
	DOREPLIFETIME(ANimModPlayerState, NumKills);
	DOREPLIFETIME(ANimModPlayerState, NumDeaths);
}

FString ANimModPlayerState::GetShortPlayerName() const
{
	if (PlayerName.Len() > MAX_PLAYER_NAME_LENGTH)
	{
		return PlayerName.Left(MAX_PLAYER_NAME_LENGTH) + "...";
	}
	return PlayerName;
}
