// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PhysicsVolume.h"
#include "VIPTrigger.generated.h"

/**
 * 
 */
UCLASS()
class NIMMOD_API AVIPTrigger : public APhysicsVolume
{
	GENERATED_BODY()
	
	AVIPTrigger(const class FObjectInitializer& PCIP);
public:
	virtual void ActorEnteredVolume(class AActor* Other) override;
	
};
