// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Camera/PlayerCameraManager.h"
#include "NimModPlayerCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class NIMMOD_API ANimModPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
	
public:

	ANimModPlayerCameraManager(const FObjectInitializer& ObjectInitializer);

	/** normal FOV */
	float NormalFOV;

	/** targeting FOV */
	float TargetingFOV;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "NimMod|Camera")
	float CurrentFOV;

	/** After updating camera, inform pawn to update 1p mesh to match camera's location&rotation */
	virtual void UpdateCamera(float DeltaTime) override;

	/*UFUNCTION(BlueprintCallable, Category = "NimMod|Camera")
	void SetFOV(float fov);*/

	UFUNCTION(BlueprintCallable, Category = "NimMod|Camera")
	void ResetFOV();
};
