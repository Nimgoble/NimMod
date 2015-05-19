// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "NimMod.h"
#include "NimModImpactEffect.h"
#include "NimModTypes.h"

ANimModImpactEffect::ANimModImpactEffect(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bAutoDestroyWhenFinished = true;
}

void ANimModImpactEffect::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UPhysicalMaterial* HitPhysMat = SurfaceHit.PhysMaterial.Get();
	EPhysicalSurface HitSurfaceType = UPhysicalMaterial::DetermineSurfaceType(HitPhysMat);

	// show particles
	UParticleSystem* ImpactFX = GetImpactFX(HitSurfaceType);
	if (ImpactFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactFX, GetActorLocation(), GetActorRotation());
	}

	// play sound
	USoundCue* ImpactSound = GetImpactSound(HitSurfaceType);
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}

	if (DefaultDecal.DecalMaterial)
	{
		FRotator RandomDecalRotation = SurfaceHit.ImpactNormal.Rotation();
		RandomDecalRotation.Roll = FMath::FRandRange(-180.0f, 180.0f);

		UGameplayStatics::SpawnDecalAttached(DefaultDecal.DecalMaterial, FVector(DefaultDecal.DecalSize, DefaultDecal.DecalSize, 1.0f),
			SurfaceHit.Component.Get(), SurfaceHit.BoneName,
			SurfaceHit.ImpactPoint, RandomDecalRotation, EAttachLocation::KeepWorldPosition,
			DefaultDecal.LifeSpan);
	}
}

UParticleSystem* ANimModImpactEffect::GetImpactFX(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	UParticleSystem* ImpactFX = NULL;

	switch (SurfaceType)
	{
	case NIMMOD_SURFACE_Concrete:	ImpactFX = ConcreteFX; break;
	case NIMMOD_SURFACE_Dirt:		ImpactFX = DirtFX; break;
	case NIMMOD_SURFACE_Water:		ImpactFX = WaterFX; break;
	case NIMMOD_SURFACE_Metal:		ImpactFX = MetalFX; break;
	case NIMMOD_SURFACE_Wood:		ImpactFX = WoodFX; break;
	case NIMMOD_SURFACE_Grass:		ImpactFX = GrassFX; break;
	case NIMMOD_SURFACE_Glass:		ImpactFX = GlassFX; break;
	case NIMMOD_SURFACE_Flesh:		ImpactFX = FleshFX; break;
	default:						ImpactFX = DefaultFX; break;
	}

	return ImpactFX;
}

USoundCue* ANimModImpactEffect::GetImpactSound(TEnumAsByte<EPhysicalSurface> SurfaceType) const
{
	USoundCue* ImpactSound = NULL;

	switch (SurfaceType)
	{
	case NIMMOD_SURFACE_Concrete:	ImpactSound = ConcreteSound; break;
	case NIMMOD_SURFACE_Dirt:		ImpactSound = DirtSound; break;
	case NIMMOD_SURFACE_Water:		ImpactSound = WaterSound; break;
	case NIMMOD_SURFACE_Metal:		ImpactSound = MetalSound; break;
	case NIMMOD_SURFACE_Wood:		ImpactSound = WoodSound; break;
	case NIMMOD_SURFACE_Grass:		ImpactSound = GrassSound; break;
	case NIMMOD_SURFACE_Glass:		ImpactSound = GlassSound; break;
	case NIMMOD_SURFACE_Flesh:		ImpactSound = FleshSound; break;
	default:						ImpactSound = DefaultSound; break;
	}

	return ImpactSound;
}
