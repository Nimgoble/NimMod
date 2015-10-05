// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NimModTypes.h"
#include "NimModWeapon_Projectile.generated.h"

// A weapon that fires a visible projectile
UCLASS(Abstract)
class ANimModWeapon_Projectile : public ANimModWeapon
{
	GENERATED_UCLASS_BODY()

	/** apply config on projectile */
	void ApplyWeaponConfig(FProjectileWeaponData& Data);

protected:

	virtual EAmmoType GetAmmoType() const override
	{
		return EAmmoType::ERocket;
	}

	/** weapon config */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	FProjectileWeaponData ProjectileConfig;

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() override;

	/** spawn projectile on server */
	UFUNCTION(reliable, server, WithValidation)
	void ServerFireProjectile(FVector Origin, FVector_NetQuantizeNormal ShootDir, FRotator Rotation);
};
