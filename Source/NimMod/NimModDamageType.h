// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/DamageType.h"
#include "NimModDamageType.generated.h"

/**
 * 
 */
UCLASS()
class NIMMOD_API UNimModDamageType : public UDamageType
{
	GENERATED_BODY()

public:
	UNimModDamageType(const FObjectInitializer& ObjectInitializer);

	/** icon displayed in death messages log when killed with this weapon */
	UPROPERTY(EditDefaultsOnly, Category = HUD)
	FCanvasIcon KillIcon;

	/** force feedback effect to play on a player hit by this damage type */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UForceFeedbackEffect *HitForceFeedback;

	/** force feedback effect to play on a player killed by this damage type */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UForceFeedbackEffect *KilledForceFeedback;
	
};
