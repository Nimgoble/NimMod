// Fill out your copyright notice in the Description page of Project Settings.

#include "NimMod.h"
#include "NimModPlayerController.h"
#include "NimModWidgetBase.h"


UNimModWidgetBase::UNimModWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	IsModal = false;
	ShowCursor = false;
	ZOrder = 1.0f;
}

void UNimModWidgetBase::ShowWidget()
{
	AddToViewport(ZOrder);
	if (!IsInViewport())
		return;
	
	if (IsModal)
		LockPlayerInputToUI();
}

void UNimModWidgetBase::RemoveWidget()
{	
	RemoveFromParent();
}

void UNimModWidgetBase::ToggleWidget()
{
	if (IsInViewport())
		RemoveWidget();
	else
		ShowWidget();
}

void UNimModWidgetBase::RemoveFromParent()
{
	Super::RemoveFromParent();

	//The check to IsInViewport() doesn't work, because it isn't removed right away.
	if (/*!IsInViewport() &&*/ IsModal)
		UnlockPlayerInput();
}

ANimModPlayerController *UNimModWidgetBase::GetNimModPlayerController()
{
	return Cast<ANimModPlayerController>(GetOwningPlayer());
}

void UNimModWidgetBase::LockPlayerInputToUI()
{
	ANimModPlayerController *playerController = GetNimModPlayerController();
	if (!playerController)
		return;

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewport(true);
	InputMode.SetWidgetToFocus(this->TakeWidget());
	playerController->SetInputMode(InputMode);
	playerController->bShowMouseCursor = ShowCursor;
}

void UNimModWidgetBase::UnlockPlayerInput()
{
	ANimModPlayerController *playerController = GetNimModPlayerController();
	if (!playerController)
		return;
	FInputModeGameOnly InputMode;
	//InputMode.SetLockMouseToViewport(false);
	playerController->SetInputMode(InputMode);
	playerController->bShowMouseCursor = false;
}

