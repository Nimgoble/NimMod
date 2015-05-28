// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameViewportClient.h"
#include "NimModGameViewportClient.generated.h"

/**
 * 
 */
UCLASS()
class NIMMOD_API UNimModGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()
	
	virtual void AddViewportWidgetContent(TSharedRef<class SWidget> ViewportContent, const int32 ZOrder = 0) override;
	virtual void RemoveViewportWidgetContent(TSharedRef<class SWidget> ViewportContent) override;
	
protected:
	TArray<TSharedRef<class SWidget>> ViewportContentStack;

	void UpdatePlayerInputMode(TSharedRef<class SWidget>, bool removed);
};
