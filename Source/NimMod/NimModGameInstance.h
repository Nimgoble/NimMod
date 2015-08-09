// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NimModTeam.h"
#include "NimModTypes.h"
#include "Engine/GameInstance.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "../../Plugins/VaRest/Source/VaRestPlugin/Private/VaRestPluginPrivatePCH.h"
#include "Queue.h"
#include "Array.h"
#include "NimModGameInstance.generated.h"

UENUM()
enum class ENimModServerMessage : uint8
{
	Register,
	Deregister,
	Update
};

USTRUCT(blueprintable, meta = (DisplayName = "NimMod Server Information"))
struct FNimModServerInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod Server Info")
	FString DedicatedServerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod Server Info")
	FString ServerIP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod Server Info")
	FString ServerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod Server Info")
	FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod Server Info")
	int32 MaxNumberOfPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod Server Info")
	int32 CurrentNumberOfPlayers;
};

USTRUCT()
struct FServerMessage
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	ENimModServerMessage MessageType;

	UPROPERTY()
	FNimModServerInfo ServerInfo;
};
/**
 * 
 */
UCLASS()
class NIMMOD_API UNimModGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "NimMod|Online")
	void RegisterServer(FString serverName, FString mapName, int32 maxNumberOfPlayers, bool isLAN);
	UFUNCTION(BlueprintCallable, Category = "NimMod|Online")
	void DeregisterServer();

	void UpdateServer(int32 currentNumberOfPlayers);

	/** virtual function to allow custom GameInstances an opportunity to set up what it needs */
	virtual void Init();
	/** virtual function to allow custom GameInstances an opportunity to do cleanup when shutting down */
	virtual void Shutdown();

	/*void SaveTeamScoresForRoundRestart(TArray<int32> teamScores);
	TArray<int32> GetSavedTeamScores();*/

	void SaveTeamsForRoundRestart(TArray<ANimModTeam *> teams);
	TArray<ANimModTeam *> GetSavedTeams();

	/*UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "UpdateServer"))
	void HandleUpdateServer(FNimModServerInfo info);*/

private:

	//Internal calls
	void Internal_RegisterServer();
	void Internal_UpdateServer(const FNimModServerInfo &info);
	void Internal_DeregisterServer();

	bool GetServerIP();
	void GetServerIPResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

public:
	void ServerHeartbeat();

protected:
	UFUNCTION()
	void RegisterServerComplete();
	UFUNCTION()
	void RegisterServerFailure();
	UFUNCTION()
	void DeregisterServerComplete();
	UFUNCTION()
	void UpdateServerComplete();

private:
	void UnhookManagerCallbacks(ENimModServerMessage type);
	void CurrentMessageDone();
	void ProcessPendingMessages();
	void PreProcessMessage(const FServerMessage &message);
	void ProcessMessage(const FServerMessage &message);
	void QueueMessage(const FServerMessage &message);
	void CleanUpManager(UVaRestParseManager *manager);
	FString GetCurrentMapName();

	/*UPROPERTY()
	bool IsParseManagerReady;
	FCriticalSection isParseManagerReadyLock;
	void SetParseManagerReady(bool ready);*/

	UPROPERTY()
	bool HasValidIP;
	FCriticalSection validIPLock;

	UPROPERTY()
	bool IsDedicatedServerValid;
	FCriticalSection validServerLock;

	UPROPERTY()
	FNimModServerInfo ServerInfo;

	UPROPERTY()
	UVaRestParseManager *parseManager;

	TQueue<FServerMessage, EQueueMode::Type::Mpsc> MessageQueue;
	FCriticalSection messageLock;

	/*UPROPERTY()
	TArray<int32> SavedTeamScores;*/

	UPROPERTY()
	TArray<ANimModTeam *> SavedTeams;
};
