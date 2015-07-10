// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

/**
* General session settings for a NimMod game
*/
class FNimModOnlineSessionSettings : public FOnlineSessionSettings
{
public:

	FNimModOnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4);
	virtual ~FNimModOnlineSessionSettings() {}
};

/**
* General search setting for a NimMod game
*/
class FNimModOnlineSearchSettings : public FOnlineSessionSearch
{
public:
	FNimModOnlineSearchSettings(bool bSearchingLAN = false, bool bSearchingPresence = false);

	virtual ~FNimModOnlineSearchSettings() {}
};

/**
* Search settings for an empty dedicated server to host a match
*/
class FNimModOnlineSearchSettingsEmptyDedicated : public FNimModOnlineSearchSettings
{
public:
	FNimModOnlineSearchSettingsEmptyDedicated(bool bSearchingLAN = false, bool bSearchingPresence = false);

	virtual ~FNimModOnlineSearchSettingsEmptyDedicated() {}
};
