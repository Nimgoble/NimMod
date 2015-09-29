// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NimModWeapon.h"
#include "NimModWeapon_Instant.generated.h"

USTRUCT()
struct FInstantHitInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector Origin;

	UPROPERTY()
	float ReticleSpread;

	UPROPERTY()
	int32 RandomSeed;
};

USTRUCT()
struct FWeaponSpread
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Weapon|Spread")
	float Min;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Spread")
	float Max;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Spread")
	float MaxTotal;
};

USTRUCT()
struct FWeaponRecoil
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	float MinHorizontal;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	float MaxHorizontal;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	float MaxTotalHorizontal;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	float MinVertical;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	float MaxVertical;
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	float MaxTotalVertical;
};

USTRUCT()
struct FWeaponSpreadAndRecoilState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Spread")
	FWeaponSpread WeaponSpreadInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil")
	FWeaponRecoil WeaponRecoilInfo;
};

USTRUCT()
struct FWeaponSpreadAndRecoilStates
{
	GENERATED_USTRUCT_BODY()

	//Standing still
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FWeaponSpreadAndRecoilState StandingStillInfo;
	//Moving
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FWeaponSpreadAndRecoilState MovingInfo;
	//Crouching
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FWeaponSpreadAndRecoilState CrouchingInfo;
	//Jumping/Falling
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FWeaponSpreadAndRecoilState InAirInfo;
};

USTRUCT()
struct FInstantWeaponData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	int32 NumberOfBulletsPerShot;

	/** base weapon spread (degrees) */
	/*UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float WeaponSpread;*/

	/** targeting spread modifier */
	/*UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	float TargetingSpreadMod;*/

	/** continuous firing: spread increment */
	//UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	//float FiringSpreadIncrement;

	///** continuous firing: max increment */
	//UPROPERTY(EditDefaultsOnly, Category = Accuracy)
	//float FiringSpreadMax;

	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	FWeaponSpreadAndRecoilStates SpreadAndRecoilStates;

	/** weapon range */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float WeaponRange;

	/** damage amount */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	int32 HitDamage;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	TSubclassOf<UDamageType> DamageType;

	/** hit verification: scale for bounding box of hit actor */
	UPROPERTY(EditDefaultsOnly, Category = HitVerification)
	float ClientSideHitLeeway;

	/** hit verification: threshold for dot product between view direction and hit direction */
	UPROPERTY(EditDefaultsOnly, Category = HitVerification)
	float AllowedViewDotHitDir;

	/** defaults */
	FInstantWeaponData()
	{
		/*WeaponSpread = 5.0f;
		TargetingSpreadMod = 0.25f;
		FiringSpreadIncrement = 1.0f;
		FiringSpreadMax = 10.0f;*/
		WeaponRange = 10000.0f;
		HitDamage = 10;
		DamageType = UDamageType::StaticClass();
		ClientSideHitLeeway = 200.0f;
		AllowedViewDotHitDir = 0.8f;
	}
};

/**
*
*/
UCLASS()
class NIMMOD_API ANimModWeapon_Instant : public ANimModWeapon
{
	GENERATED_BODY()

	/** get current spread */
	float GetCurrentSpread(const FWeaponSpreadAndRecoilState &spreadAndRecoilStateInformation);

	const FWeaponSpreadAndRecoilState& GetCurrentRecoilAndSpreadState();

	FVector2D CalculateNextViewPunch(const FWeaponSpreadAndRecoilState &spreadAndRecoilStateInformation);

	UPROPERTY(transient)
	FVector2D CurrentViewPunchTotal;

public:
	ANimModWeapon_Instant(const FObjectInitializer& ObjectInitializer);

protected:

	virtual EAmmoType GetAmmoType() const override
	{
		return EAmmoType::EBullet;
	}

	/** weapon config */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	FInstantWeaponData InstantConfig;

	/** impact effects */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<class ANimModImpactEffect> ImpactTemplate;

	/** smoke trail */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* TrailFX;

	/** param name for beam target in smoke trail */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName TrailTargetParam;

	/** instant hit notify for replication */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_HitNotify)
	FInstantHitInfo HitNotify;

	/** current spread from continuous firing */
	float CurrentFiringSpread;

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** server notified of hit from client to verify */
	UFUNCTION(reliable, server, WithValidation)
	void ServerNotifyHit(const FHitResult Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);

	/** server notified of miss to show trail FX */
	UFUNCTION(unreliable, server, WithValidation)
	void ServerNotifyMiss(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);

	/** process the instant hit and notify the server if necessary */
	void ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	/** continue processing the instant hit, as if it has been confirmed by the server */
	void ProcessInstantHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	/** check if weapon should deal damage to actor */
	bool ShouldDealDamage(AActor* TestActor) const;

	/** handle damage */
	void DealDamage(const FHitResult& Impact, const FVector& ShootDir);

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() override;

	/** [local + server] update spread on firing */
	virtual void OnBurstFinished() override;


	//////////////////////////////////////////////////////////////////////////
	// Effects replication

	UFUNCTION()
	void OnRep_HitNotify();

	/** called in network play to do the cosmetic fx  */
	void SimulateInstantHit(const FVector& Origin, int32 RandomSeed, float ReticleSpread);

	/** spawn effects for impact */
	void SpawnImpactEffects(const FHitResult& Impact);

	/** spawn trail effect */
	void SpawnTrailEffect(const FVector& EndPoint, const FRotator& rotation);
};
