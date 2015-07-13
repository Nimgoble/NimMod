// Fill out your copyright notice in the Description page of Project Settings.

#include "NimMod.h"
#include "NimModHUD.h"
#include "NimModHUDLayoutWidget.h"
#include "NimModPlayerController.h"

ANimModHUD *UNimModHUDLayoutWidget::GetOwningHUD()
{
	return OwningHUD;
}

//void UNimModHUDLayoutWidget::HandleHUDMessage(const FString& message, ENimModHUDMessageType type)
//{
//
//}

void UNimModHUDLayoutWidget::SendChatMessage(FNimModHUDMessage chatMessage)
{
	ANimModPlayerController *pc = GetOwningNimModPlayerController();
	if (pc)
		pc->SendHUDMessage(chatMessage);

	HandleHUDMessage(chatMessage);
}

ANimModPlayerController *UNimModHUDLayoutWidget::GetOwningNimModPlayerController()
{
	return Cast<ANimModPlayerController>(GetOwningPlayer());
}