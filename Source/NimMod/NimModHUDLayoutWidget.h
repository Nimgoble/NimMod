// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "NimModTypes.h"
#include "NimModHUDLayoutWidget.generated.h"

/**
 * 
 */

UCLASS()
class NIMMOD_API UNimModHUDLayoutWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetOwningHUD(class ANimModHUD *owningHUD){ OwningHUD = owningHUD; }
	class ANimModHUD *GetOwningHUD();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "HUDMessage"))
	void HandleHUDMessage(const FNimModHUDMessage &message);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ToggleChat"))
	void HandleToggleChat(bool isTeamChat);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "CommitChatMessage"))
	void HandleCommitChatMessage();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "RoundRestarting"))
	void HandleRoundRestarting();

	UFUNCTION(BlueprintCallable, Category = "NimMod|HUD")
	void SendChatMessage(FNimModHUDMessage chatMessage);

	UFUNCTION(BlueprintCallable, Category = "NimMod|HUD")
	class ANimModPlayerController *GetOwningNimModPlayerController();

private:
	UPROPERTY()
	class ANimModHUD *OwningHUD;
};
