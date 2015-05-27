// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once 
#include "GameFramework/HUD.h"
#include "NimModHUDLayoutWidget.h"
#include "NimModHUD.generated.h"

UCLASS()
class ANimModHUD : public AHUD
{
	GENERATED_BODY()

public:
	ANimModHUD(const FObjectInitializer& ObjectInitializer);

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	UPROPERTY(EditDefaultsOnly, Category = Layout)
	TSubclassOf<class UNimModHUDLayoutWidget> LayoutWidget;

	// Add any of the blueprint based hud widgets
	virtual void BeginPlay();

	virtual void PostInitializeComponents();

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;
	class UNimModHUDLayoutWidget *layoutWidget;
};

