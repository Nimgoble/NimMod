// Fill out your copyright notice in the Description page of Project Settings.

#include "NimMod.h"
#include "NimModPlayerCameraManager.h"
#include "NimModCharacter.h"

ANimModPlayerCameraManager::ANimModPlayerCameraManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CurrentFOV = NormalFOV = 90.0f;
	TargetingFOV = 60.0f;
	ViewPitchMin = -87.0f;
	ViewPitchMax = 87.0f;
	bAlwaysApplyModifiers = true;
}

void ANimModPlayerCameraManager::UpdateCamera(float DeltaTime)
{
	ANimModCharacter* MyPawn = PCOwner ? Cast<ANimModCharacter>(PCOwner->GetPawn()) : NULL;
	if (MyPawn && MyPawn->IsFirstPerson())
	{
		//const float TargetFOV = MyPawn->IsTargeting() ? TargetingFOV : NormalFOV;
		DefaultFOV = CurrentFOV;//FMath::FInterpTo(DefaultFOV, TargetFOV, DeltaTime, 20.0f);
	}

	Super::UpdateCamera(DeltaTime);

	if (MyPawn && MyPawn->IsFirstPerson())
	{
		MyPawn->OnCameraUpdate(GetCameraLocation(), GetCameraRotation());
	}
}

void ANimModPlayerCameraManager::ResetFOV()
{
	CurrentFOV = NormalFOV;
}