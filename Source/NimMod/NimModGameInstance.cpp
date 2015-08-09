// Fill out your copyright notice in the Description page of Project Settings.

#include "NimMod.h"
#include "NimModGameInstance.h"
#include "NimModGameMode.h"

void UNimModGameInstance::Init()
{
	IsDedicatedServerValid = false;
	parseManager = UVaRestParseManager::ConstructParseRequest(GetWorld(), ERequestVerb::POST, ERequestContentType::json);
	parseManager->SetParseAuthData(TEXT("vYZ1D0SmrASsZv9ZhZd8wWEJ4fMIJ6bQqiLNG6hA"), TEXT("MyRnXcDSZPzRPIXOV8IWz4VW936rmnME9zKLRf2D"));
	//IsParseManagerReady = true;
	GetServerIP();
}
void UNimModGameInstance::RegisterServer(FString serverName, FString mapName, int32 maxNumberOfPlayers, bool isLAN)
{
	/*if (GetWorld()->GetNetMode() != NM_DedicatedServer)
		return;*/

	//We don't register LAN servers with the master server.
	if (isLAN)
		return;

	ServerInfo.ServerName = serverName;
	ServerInfo.MaxNumberOfPlayers = maxNumberOfPlayers;
	FServerMessage message;
	message.MessageType = ENimModServerMessage::Register;
	message.ServerInfo = ServerInfo;
	PreProcessMessage(message);
}
void UNimModGameInstance::DeregisterServer()
{
	FServerMessage message;
	message.MessageType = ENimModServerMessage::Deregister;
	message.ServerInfo = ServerInfo;
	PreProcessMessage(message);

	/*if (GetWorld()->GetNetMode() != NM_DedicatedServer)
		return;*/
}
void UNimModGameInstance::UpdateServer(int32 currentNumberOfPlayers)
{
	ServerInfo.MapName = GetCurrentMapName();
	ServerInfo.CurrentNumberOfPlayers = currentNumberOfPlayers;
	FServerMessage message;
	message.MessageType = ENimModServerMessage::Update;
	message.ServerInfo = ServerInfo;
	PreProcessMessage(message);
}
void UNimModGameInstance::ServerHeartbeat()
{

}
void UNimModGameInstance::RegisterServerComplete()
{
	{
		FScopeLock sl(&validServerLock);
		IsDedicatedServerValid = true;
	}
	UVaRestJsonObject *responseObject = parseManager->GetResponseObject();
	ServerInfo.DedicatedServerID = responseObject->GetStringField(TEXT("objectId"));
	UnhookManagerCallbacks(ENimModServerMessage::Register);
	CurrentMessageDone();
	ProcessPendingMessages();
}
void UNimModGameInstance::RegisterServerFailure()
{
	{
		FScopeLock sl(&validServerLock);
		IsDedicatedServerValid = false;
	}
	UnhookManagerCallbacks(ENimModServerMessage::Register);
	CurrentMessageDone();
	ProcessPendingMessages();
}
void UNimModGameInstance::DeregisterServerComplete()
{
	{
		FScopeLock sl(&validServerLock);
		IsDedicatedServerValid = false;
	}
	UnhookManagerCallbacks(ENimModServerMessage::Deregister);
	CurrentMessageDone();
	ProcessPendingMessages();
}
void UNimModGameInstance::UpdateServerComplete()
{
	UnhookManagerCallbacks(ENimModServerMessage::Update);
	CurrentMessageDone();
	ProcessPendingMessages();
}
void UNimModGameInstance::UnhookManagerCallbacks(ENimModServerMessage type)
{
	switch (type)
	{
	case ENimModServerMessage::Register:
		parseManager->OnRequestComplete.RemoveDynamic(this, &UNimModGameInstance::RegisterServerComplete);
		parseManager->OnRequestFail.RemoveDynamic(this, &UNimModGameInstance::RegisterServerFailure);
		break;
	case ENimModServerMessage::Update:
		parseManager->OnRequestComplete.RemoveDynamic(this, &UNimModGameInstance::UpdateServerComplete);
		parseManager->OnRequestFail.RemoveDynamic(this, &UNimModGameInstance::UpdateServerComplete);
		break;

	case ENimModServerMessage::Deregister:
		parseManager->OnRequestComplete.RemoveDynamic(this, &UNimModGameInstance::DeregisterServerComplete);
		parseManager->OnRequestFail.RemoveDynamic(this, &UNimModGameInstance::DeregisterServerComplete);
		break;
	}
}
void UNimModGameInstance::CurrentMessageDone()
{
	{
		FScopeLock sl(&messageLock);
		FServerMessage message;
		MessageQueue.Dequeue(message);
	}
}
void UNimModGameInstance::CleanUpManager(UVaRestParseManager *manager)
{
	UWorld *world = GetWorld();
	if (manager && world)
	{
		manager->ConditionalBeginDestroy();
		manager = nullptr;
	}
}

void UNimModGameInstance::Internal_RegisterServer()
{
	//SetParseManagerReady(false);
	parseManager->ResetData();
	parseManager->SetVerb(ERequestVerb::POST);
	parseManager->SetContentType(ERequestContentType::json);

	UVaRestJsonObject* requestObject = parseManager->GetRequestObject();
	requestObject->SetStringField(TEXT("ServerName"), ServerInfo.ServerName);
	requestObject->SetStringField(TEXT("IP"), ServerInfo.ServerIP);
	requestObject->SetStringField(TEXT("Map"), ServerInfo.MapName);
	requestObject->SetNumberField(TEXT("CurrentNumberOfPlayers"), (float)ServerInfo.CurrentNumberOfPlayers);
	requestObject->SetNumberField(TEXT("MaxNumberOfPlayers"), (float)ServerInfo.MaxNumberOfPlayers);
	parseManager->OnRequestComplete.AddDynamic(this, &UNimModGameInstance::RegisterServerComplete);
	parseManager->OnRequestFail.AddDynamic(this, &UNimModGameInstance::RegisterServerFailure);
	parseManager->ProcessParseURL(TEXT("classes"), TEXT("ServerInstance"));
}
void UNimModGameInstance::Internal_UpdateServer(const FNimModServerInfo &info)
{
	//SetParseManagerReady(false);
	parseManager->ResetData();
	parseManager->SetVerb(ERequestVerb::PUT);
	parseManager->SetContentType(ERequestContentType::json);

	UVaRestJsonObject* requestObject = parseManager->GetRequestObject();
	requestObject->SetStringField(TEXT("ServerName"), info.ServerName);
	requestObject->SetStringField(TEXT("IP"), info.ServerIP);
	requestObject->SetStringField(TEXT("Map"), info.MapName);
	requestObject->SetNumberField(TEXT("CurrentNumberOfPlayers"), (float)info.CurrentNumberOfPlayers);
	requestObject->SetNumberField(TEXT("MaxNumberOfPlayers"), (float)info.MaxNumberOfPlayers);
	parseManager->OnRequestComplete.AddDynamic(this, &UNimModGameInstance::UpdateServerComplete);
	parseManager->OnRequestFail.AddDynamic(this, &UNimModGameInstance::UpdateServerComplete);
	parseManager->ProcessParseURL(TEXT("classes"), TEXT("ServerInstance"), ServerInfo.DedicatedServerID);
}
void UNimModGameInstance::Internal_DeregisterServer()
{
	//SetParseManagerReady(false);
	parseManager->ResetData();
	parseManager->SetVerb(ERequestVerb::DEL);
	parseManager->SetContentType(ERequestContentType::x_www_form_urlencoded);
	parseManager->OnRequestComplete.AddDynamic(this, &UNimModGameInstance::DeregisterServerComplete);
	parseManager->OnRequestFail.AddDynamic(this, &UNimModGameInstance::DeregisterServerComplete);
	parseManager->ProcessParseURL(TEXT("classes"), TEXT("ServerInstance"), ServerInfo.DedicatedServerID);
}
///RAMA
bool UNimModGameInstance::GetServerIP()
{
	FHttpModule* Http = &FHttpModule::Get();

	if (!Http)
	{
		return false;
	}

	if (!Http->IsHttpEnabled())
	{
		return false;
	}
	//~~~~~~~~~~~~~~~~~~~

	FString TargetHost = "http://api.ipify.org";
	TSharedRef < IHttpRequest > Request = Http->CreateRequest();
	Request->SetVerb("GET");
	Request->SetURL(TargetHost);
	Request->SetHeader("User-Agent", "NimMod/1.0");
	Request->SetHeader("Content-Type", "text/html");

	Request->OnProcessRequestComplete().BindUObject(this, &UNimModGameInstance::GetServerIPResponseReceived);
	if (!Request->ProcessRequest())
	{
		return false;
	}

	return true;
}

void UNimModGameInstance::GetServerIPResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	ServerInfo.ServerIP = Response->GetContentAsString();
	{
		FScopeLock sl(&validIPLock);
		HasValidIP = bWasSuccessful;
	}
	ProcessPendingMessages();
}

FString UNimModGameInstance::GetCurrentMapName()
{
	FString rtn;
	UWorld *world = GetWorld();
	if (world != nullptr)
	{
		rtn = world->GetCurrentLevel()->GetOutermost()->GetName();
		if (GetWorld()->IsPlayInEditor())
		{
			FWorldContext WorldContext = GEngine->GetWorldContextFromWorldChecked(world);
			rtn = world->StripPIEPrefixFromPackageName(rtn, world->BuildPIEPackagePrefix(WorldContext.PIEInstance));
		}
		rtn = FPackageName::GetShortName(rtn);
	}
	return rtn;
}
//void UNimModGameInstance::SetParseManagerReady(bool ready)
//{
//	{
//		FScopeLock sl(&isParseManagerReadyLock);
//		IsParseManagerReady = ready;
//	}
//}
void UNimModGameInstance::Shutdown()
{
	bool shouldProcess = false;
	{
		FScopeLock sl(&validServerLock);
		shouldProcess = IsDedicatedServerValid;
	}
	if (shouldProcess)
	{
		Internal_DeregisterServer();
		return;
	}
	CleanUpManager(parseManager);

	Super::Shutdown();
}

//void UNimModGameInstance::SaveTeamScoresForRoundRestart(TArray<int32> teamScores)
//{
//	SavedTeamScores = teamScores;
//}
//
//TArray<int32> UNimModGameInstance::GetSavedTeamScores()
//{
//	return SavedTeamScores;
//}

void UNimModGameInstance::SaveTeamsForRoundRestart(TArray<ANimModTeam *> teams)
{
	SavedTeams = teams;
}

TArray<ANimModTeam *> UNimModGameInstance::GetSavedTeams()
{
	return SavedTeams;
}

void UNimModGameInstance::ProcessPendingMessages()
{
	FServerMessage message;
	bool haveMessages = false;
	{
		FScopeLock sl(&messageLock);
		haveMessages = !MessageQueue.IsEmpty();
	}

	if (!haveMessages)
		return;

	/*while (haveMessages)
	{*/
		{
			FScopeLock sl(&messageLock);
			MessageQueue.Peek(message);
			//MessageQueue.Dequeue(message);
		}

		ProcessMessage(message);

		/*{
			FScopeLock sl(&messageLock);
			haveMessages = !MessageQueue.IsEmpty();
		}
	}*/
}
void UNimModGameInstance::PreProcessMessage(const FServerMessage &message)
{
	bool shouldProcess = false;
	//Are we currently processing a message?
	/*{
		FScopeLock sl(&isParseManagerReadyLock);
		shouldProcess = IsParseManagerReady;
	}

	if (!shouldProcess)
	{
		QueueMessage(message);
		return;
	}
	*/

	//Are there messages in the queue before us?
	{
		FScopeLock sl(&messageLock);
		shouldProcess = MessageQueue.IsEmpty();
	}

	QueueMessage(message);

	if (!shouldProcess)
		return;

	//Do we have a valid IP?
	{
		FScopeLock sl(&validIPLock);
		shouldProcess = HasValidIP;
	}

	if (!shouldProcess)
		return;

	//If we're an Update or Deregister, do we have a valid server entry?
	{
		FScopeLock sl(&validServerLock);
		shouldProcess = (message.MessageType == ENimModServerMessage::Register) ? !IsDedicatedServerValid : IsDedicatedServerValid;
	}

	if (!shouldProcess)
		return;

	ProcessMessage(message);
}
void UNimModGameInstance::ProcessMessage(const FServerMessage &message)
{
	switch (message.MessageType)
	{
	case ENimModServerMessage::Register:
		Internal_RegisterServer();
		break;
	case ENimModServerMessage::Deregister:
		Internal_DeregisterServer();
		break;
	case ENimModServerMessage::Update:
		Internal_UpdateServer(message.ServerInfo);
		break;
	}
}

void UNimModGameInstance::QueueMessage(const FServerMessage &message)
{
	{
		FScopeLock sl(&messageLock);
		MessageQueue.Enqueue(message);
	}
}

