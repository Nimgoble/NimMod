// Fill out your copyright notice in the Description page of Project Settings.

#include "NimMod.h"
#include "NimModLocalPlayer.h"
#include "NimModPlayerController.h"
#include "NimModGameViewportClient.h"

void UNimModGameViewportClient::AddViewportWidgetContent(TSharedRef<class SWidget> ViewportContent, const int32 ZOrder)
{
	UE_LOG(LogPlayerManagement, Log, TEXT("UNimModGameViewportClient::AddViewportWidgetContent: %p"), &ViewportContent.Get());

	//if ((DialogWidget.IsValid() || LoadingScreenWidget.IsValid()) && ViewportContent != DialogWidget && ViewportContent != LoadingScreenWidget)
	//{
	//	// Add to hidden list, and don't show until we hide the dialog widget
	//	HiddenViewportContentStack.AddUnique(ViewportContent);
	//	return;
	//}

	if (ViewportContentStack.Contains(ViewportContent))
	{
		return;
	}

	ViewportContentStack.AddUnique(ViewportContent);

	//UpdatePlayerInputMode(ViewportContent, false);
	Super::AddViewportWidgetContent(ViewportContent, 0);
}

void UNimModGameViewportClient::RemoveViewportWidgetContent(TSharedRef<class SWidget> ViewportContent)
{
	UE_LOG(LogPlayerManagement, Log, TEXT("UNimModGameViewportClient::RemoveViewportWidgetContent: %p"), &ViewportContent.Get());

	ViewportContentStack.Remove(ViewportContent);
	//HiddenViewportContentStack.Remove(ViewportContent);
	//UpdatePlayerInputMode(ViewportContent, true);
	Super::RemoveViewportWidgetContent(ViewportContent);
}

void UNimModGameViewportClient::UpdatePlayerInputMode(TSharedRef<class SWidget> ViewPortContent, bool removed)
{
#if !UE_SERVER
	//UGameEngine* engine = Cast<UGameEngine>(GEngine);
	//UNimModLocalPlayer* FirstPlayer = Cast<UNimModLocalPlayer>(GEngine->GetLocalPlayerFromControllerId(this, 0));	// Grab the first local player.
	//if (FirstPlayer != nullptr)
	//{
	//	ANimModPlayerController *playerController = Cast<ANimModPlayerController>(FirstPlayer->PlayerController);
	//	if (playerController)
	//	{
	//		UUserWidget* userWidget = Cast<UUserWidget>(ViewPortContent.Get());
	//		if (removed || userWidget == nullptr)
	//		{
	//			FInputModeGameAndUI InputMode;
	//			InputMode.SetLockMouseToViewport(true);
	//			if (!removed)
	//				InputMode.SetWidgetToFocus(ViewPortContent);
	//			playerController->SetInputMode(InputMode);
	//			playerController->bShowMouseCursor = (removed == false);
	//		}
	//		else
	//		{
	//			FInputModeUIOnly InputMode;
	//			InputMode.SetLockMouseToViewport(true);
	//			InputMode.SetWidgetToFocus(ViewPortContent);
	//			playerController->SetInputMode(InputMode);
	//			playerController->bShowMouseCursor = true;
	//		}
	//	}
	//}
#endif
}