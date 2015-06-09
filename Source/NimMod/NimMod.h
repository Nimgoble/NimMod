// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#ifndef __NIMMOD_H__
#define __NIMMOD_H__

#include "Engine.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "ParticleDefinitions.h"
#include "SoundDefinitions.h"
#include "Net/UnrealNetwork.h"
/*#include "EngineMinimal.h"*/

/** when you modify this, please note that this information can be saved with instances
* also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list **/
#define COLLISION_WEAPON		ECC_GameTraceChannel1
#define COLLISION_PROJECTILE	ECC_GameTraceChannel2
#define COLLISION_PICKUP		ECC_GameTraceChannel3

DECLARE_LOG_CATEGORY_EXTERN(LogNimMod, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogNimModWeapon, Log, All);

#define MAX_PLAYER_NAME_LENGTH 16

#define ISDEDICATED (GEngine->GetNetMode(GetWorld()) == NM_DedicatedServer)
#define ISLISTEN (GEngine->GetNetMode(GetWorld()) == NM_ListenServer)
#define ISSTANDALONE (GEngine->GetNetMode(GetWorld()) == NM_Standalone)
#define ISSERVER (GEngine->GetNetMode(GetWorld()) < NM_Client)
#define ISCLIENT (GEngine->GetNetMode(GetWorld()) == NM_Client)


#endif
