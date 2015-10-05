// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "NimMod.h"
#include "NimModWeapon_Projectile.h"
#include "NimModProjectile.h"

ANimModWeapon_Projectile::ANimModWeapon_Projectile(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

//////////////////////////////////////////////////////////////////////////
// Weapon usage

void ANimModWeapon_Projectile::FireWeapon()
{
	FVector ShootDir = GetAdjustedAim().GetSafeNormal();
	//ShootDir = FRotationMatrix(RootComponent->GetComponentToWorld().Rotator()).GetScaledAxis(EAxis::X);
	FVector Origin = GetMuzzleLocation();

	AActor *ownerOwner = GetOwner();
	ACharacter *Character = Cast<ACharacter>(ownerOwner);
	FRotator rotation = ShootDir.GetSafeNormal().Rotation();

	//None of these work
	/*rotation = FRotationMatrix(ShootDir.Rotation()).GetScaledAxis(EAxis::X).Rotation();
	if (RootComponent)
	{
		rotation = RootComponent->GetComponentToWorld().Rotator();
		rotation = RootComponent->GetComponentToWorld().Rotator().GetNormalized();
		rotation = FRotationMatrix(RootComponent->GetComponentToWorld().Rotator()).GetScaledAxis(EAxis::X).Rotation();
	}*/

	//THIS WORKS
	if (Character)
	{
		rotation = Character->GetViewRotation().GetNormalized();
	}

	//else
	//{
	//	rotation = ShootDir.GetSafeNormal().Rotation();
	//	//rotation = FRotationMatrix(ShootDir.Rotation()).GetScaledAxis(EAxis::X).Rotation();
	//}
	

	// trace from camera to check what's under crosshair
	//const float ProjectileAdjustRange = 10000.0f;
	//const FVector StartTrace = GetCameraDamageStartLocation(ShootDir);
	//const FVector EndTrace = StartTrace + ShootDir * ProjectileAdjustRange;
	//FHitResult Impact = WeaponTrace(StartTrace, EndTrace);

	//// and adjust directions to hit that actor
	//if (Impact.bBlockingHit)
	//{
	//	const FVector AdjustedDir = (Impact.ImpactPoint - Origin).GetSafeNormal();
	//	bool bWeaponPenetration = false;

	//	const float DirectionDot = FVector::DotProduct(AdjustedDir, ShootDir);
	//	if (DirectionDot < 0.0f)
	//	{
	//		// shooting backwards = weapon is penetrating
	//		bWeaponPenetration = true;
	//	}
	//	else if (DirectionDot < 0.5f)
	//	{
	//		// check for weapon penetration if angle difference is big enough
	//		// raycast along weapon mesh to check if there's blocking hit

	//		FVector MuzzleStartTrace = Origin - GetMuzzleDirection() * 150.0f;
	//		FVector MuzzleEndTrace = Origin;
	//		FHitResult MuzzleImpact = WeaponTrace(MuzzleStartTrace, MuzzleEndTrace);

	//		if (MuzzleImpact.bBlockingHit)
	//		{
	//			bWeaponPenetration = true;
	//		}
	//	}

	//	if (bWeaponPenetration)
	//	{
	//		// spawn at crosshair position
	//		Origin = Impact.ImpactPoint - ShootDir * 10.0f;
	//	}
	//	else
	//	{
	//		// adjust direction to hit
	//		ShootDir = AdjustedDir;
	//	}
	//}

	ServerFireProjectile(Origin, ShootDir, rotation);
}

bool ANimModWeapon_Projectile::ServerFireProjectile_Validate(FVector Origin, FVector_NetQuantizeNormal ShootDir, FRotator Rotation)
{
	return true;
}

void ANimModWeapon_Projectile::ServerFireProjectile_Implementation(FVector Origin, FVector_NetQuantizeNormal ShootDir, FRotator Rotation)
{
	FTransform SpawnTM(Rotation, Origin);
	ANimModProjectile* Projectile = Cast<ANimModProjectile>(UGameplayStatics::BeginSpawningActorFromClass(this, ProjectileConfig.ProjectileClass, SpawnTM));
	if (Projectile)
	{
		Projectile->Instigator = Instigator;
		Projectile->SetOwner(this);
		//Projectile->SetActorRotation(Rotation);
		//Projectile->InitVelocity(ShootDir);

		UGameplayStatics::FinishSpawningActor(Projectile, SpawnTM);
	}
}

void ANimModWeapon_Projectile::ApplyWeaponConfig(FProjectileWeaponData& Data)
{
	Data = ProjectileConfig;
}
