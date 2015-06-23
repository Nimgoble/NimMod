// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
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

private:
	UPROPERTY()
	class ANimModHUD *OwningHUD;
};
