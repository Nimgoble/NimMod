// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "NimModWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class NIMMOD_API UNimModWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UNimModWidgetBase(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, Category = "UI")
	bool IsModal;

	UPROPERTY(EditAnywhere, Category = "UI")
	bool ShowCursor;

	UPROPERTY(EditAnywhere, Category = "UI")
	int32 ZOrder;

	void ShowWidget();
	void RemoveWidget();
	void ToggleWidget();

	/** Removes the widget from it's parent widget, including the viewport if it was added to the viewport. */
	virtual void RemoveFromParent() override;

private:
	class ANimModPlayerController *GetNimModPlayerController();

	void LockPlayerInputToUI();
	void UnlockPlayerInput();
};
