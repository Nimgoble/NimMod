// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/LevelScriptActor.h"
#include "NimModRoundManager.h"
#include "NimModLevelScriptActor.generated.h"

/**
*
*/
UCLASS()
class NIMMOD_API ANimModLevelScriptActor : public ALevelScriptActor
{
	GENERATED_BODY()
public:
	ANimModLevelScriptActor(const FObjectInitializer& ObjectInitializer);

	virtual void PreInitializeComponents() override;

	ANimModRoundManager *GetRoundManager(){ return RoundManager; }

private:
	UPROPERTY(Transient)
	ANimModRoundManager *RoundManager;
};
