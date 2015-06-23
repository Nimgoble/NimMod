// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "NimMod.h"
#include "NimModHUD.h"
#include "Engine/Canvas.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "NimModPlayerController.h"
#include "Blueprint/UserWidget.h"

ANimModHUD::ANimModHUD(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/NimMod/Textures/FirstPersonCrosshair"));
	CrosshairTex = CrosshairTexObj.Object;
}

void ANimModHUD::BeginPlay()
{
	Super::BeginPlay();

	if (LayoutWidget != nullptr)
	{
		ANimModPlayerController *pc = Cast<ANimModPlayerController>(this->GetOwningPlayerController());
		if (pc)
		{
			layoutWidget = CreateWidget<UNimModHUDLayoutWidget>(pc, LayoutWidget);
			layoutWidget->SetOwningHUD(this);
			layoutWidget->AddToViewport(1);
		}
	}
}

void ANimModHUD::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}


void ANimModHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw very simple crosshair

	// find center of the Canvas
	const FVector2D Center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

	// offset by half the texture's dimensions so that the center of the texture aligns with the center of the Canvas
	const FVector2D CrosshairDrawPosition( (Center.X - (CrosshairTex->GetSurfaceWidth() * 0.5)),
										   (Center.Y - (CrosshairTex->GetSurfaceHeight() * 0.5f)) );

	// draw the crosshair
	FCanvasTileItem TileItem( CrosshairDrawPosition, CrosshairTex->Resource, FLinearColor::White);
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem( TileItem );
}

