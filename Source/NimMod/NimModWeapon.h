// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "NimModWeapon.generated.h"

namespace EWeaponState
{
	enum Type
	{
		Idle,
		Firing,
		Reloading,
		Equipping,
	};
}

UENUM(BlueprintType)
enum class ENimModFireMode : uint8
{
	SemiAutomatic UMETA(DisplayName = "Semi-Automatic"),
	Automatic UMETA(DisplayName = "Automatic"),
	BurstFire UMETA(DisplayName = "Burst Fire")
};

USTRUCT()
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** inifite ammo for reloads */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	bool bInfiniteAmmo;

	/** infinite ammo in clip, no reload required */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	bool bInfiniteClip;

	/** max ammo */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 MaxAmmo;

	/** clip size */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 AmmoPerClip;

	/** initial clips */
	UPROPERTY(EditDefaultsOnly, Category = Ammo)
	int32 InitialClips;

	/** The slot this weapon will be in */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat, meta = (UIMin = 0, ClampMin = 0, UIMax = 9, ClampMax = 9))
	int32 InventorySlot;

	/** The order of this weapon in its slot */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat, meta = (UIMin = 0, ClampMin = 0, UIMax = 9, ClampMax = 9))
	int32 InventorySlotOrder;

	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	TArray<ENimModFireMode> FireModes;

	/** time between two consecutive shots */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float TimeBetweenShots;

	/** failsafe reload duration if weapon doesn't have any animation for it */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float NoAnimReloadDuration;

	/** defaults */
	FWeaponData()
	{
		bInfiniteAmmo = false;
		bInfiniteClip = false;
		MaxAmmo = 100;
		AmmoPerClip = 20;
		InitialClips = 4;
		TimeBetweenShots = 0.2f;
		NoAnimReloadDuration = 1.0f;
		InventorySlot = 0;
		InventorySlotOrder = 0;
	}
};

USTRUCT(blueprintable)
struct FWeaponAnim
{
	GENERATED_USTRUCT_BODY()

	/** animation played on pawn (1st person view) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Pawn1P;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Weapon1P;

	/** animation played on pawn (3rd person view) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Pawn3P;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	UAnimMontage* Weapon3P;
};

UCLASS(Abstract, Blueprintable)
class NIMMOD_API ANimModWeapon : public AActor
{
	GENERATED_BODY()
	
public:	

	ANimModWeapon(const FObjectInitializer& ObjectInitializer);

	/** perform initial setup */
	virtual void PostInitializeComponents() override;

	virtual void Destroyed() override;

	//////////////////////////////////////////////////////////////////////////
	// Ammo

	enum class EAmmoType
	{
		EBullet,
		ERocket,
		EMax,
	};

	/** [server] add ammo */
	void GiveAmmo(int AddAmount);

	/** consume a bullet */
	void UseAmmo();

	/** query ammo type */
	virtual EAmmoType GetAmmoType() const
	{
		return EAmmoType::EBullet;
	}

	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/** weapon is being equipped by owner pawn */
	virtual void OnEquip();

	UFUNCTION(BlueprintImplementableEvent, Category = "NimMod|Weapon")
	void HandleOnEquip();

	/** weapon is now equipped by owner pawn */
	virtual void OnEquipFinished();

	UFUNCTION(BlueprintImplementableEvent, Category = "NimMod|Weapon")
	void HandleOnEquipFinished();

	/** weapon is holstered by owner pawn */
	virtual void OnUnEquip();

	UFUNCTION(BlueprintImplementableEvent, Category = "NimMod|Weapon")
	void HandleOnUnEquip();

	/** [server] weapon was added to pawn's inventory */
	virtual void OnEnterInventory(ANimModCharacter* NewOwner);

	/** [server] weapon was removed from pawn's inventory */
	virtual void OnLeaveInventory();

	/** check if it's currently equipped */
	bool IsEquipped() const;

	/** check if mesh is already attached */
	bool IsAttachedToPawn() const;


	//////////////////////////////////////////////////////////////////////////
	// Input

	/** [local + server] start weapon fire */
	virtual void StartPrimaryFire();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "StartSecondaryFire"))
	void OnStartPrimaryFire();

	/** [local + server] stop weapon fire */
	virtual void StopPrimaryFire();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "StartSecondaryFire"))
	void OnStopPrimaryFire();

	/** [local + server] start weapon fire */
	virtual void StartSecondaryFire();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "StartSecondaryFire"))
	void OnStartSecondaryFire();

	/** [local + server] stop weapon fire */
	virtual void StopSecondaryFire();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "StopSecondaryFire"))
	void OnStopSecondaryFire();

	/** [all] start weapon reload */
	virtual void StartReload(bool bFromReplication = false);

	/** [local + server] interrupt weapon reload */
	virtual void StopReload();

	/** [server] performs actual reload */
	virtual void ReloadWeapon();

	/** trigger reload from server */
	UFUNCTION(reliable, client)
	void ClientStartReload();

	UFUNCTION(BlueprintImplementableEvent, Category = "NimMod|Weapon|HUD")
	void OnDrawHUD(class ANimModHUD *HUD, float width, float height);

	//////////////////////////////////////////////////////////////////////////
	// Control

	/** check if weapon can fire */
	bool CanFire() const;

	/** check if weapon can be reloaded */
	bool CanReload() const;


	//////////////////////////////////////////////////////////////////////////
	// Reading data

	/** get current weapon state */
	EWeaponState::Type GetCurrentState() const;

	/** get current ammo amount (total) */
	UFUNCTION(BlueprintCallable, Category = Ammo)
	int32 GetCurrentAmmo() const;

	/** get current ammo amount (clip) */
	UFUNCTION(BlueprintCallable, Category = Ammo)
	int32 GetCurrentAmmoInClip() const;

	/** get clip size */
	UFUNCTION(BlueprintCallable, Category = Ammo)
	int32 GetAmmoPerClip() const;

	/** get max ammo amount */
	UFUNCTION(BlueprintCallable, Category = Ammo)
	int32 GetMaxAmmo() const;

	UFUNCTION(BlueprintCallable, Category = Ammo)
	int32 GetInventorySlot() const;

	UFUNCTION(BlueprintCallable, Category = Ammo)
	int32 GetInventorySlotOrder() const;

	/** get weapon mesh (needs pawn owner to determine variant) */
	USkeletalMeshComponent* GetWeaponMesh() const;

	/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	class ANimModCharacter* GetPawnOwner() const;

	UFUNCTION(BlueprintCallable, Category = Animation)
	UAnimMontage *GetIdleAnimation();

	/** icon displayed on the HUD when weapon is equipped as primary */
	UPROPERTY(EditDefaultsOnly, Category = HUD)
	FCanvasIcon PrimaryIcon;

	/** icon displayed on the HUD when weapon is secondary */
	UPROPERTY(EditDefaultsOnly, Category = HUD)
	FCanvasIcon SecondaryIcon;

	/** bullet icon used to draw current clip (left side) */
	UPROPERTY(EditDefaultsOnly, Category = HUD)
	FCanvasIcon PrimaryClipIcon;

	/** bullet icon used to draw secondary clip (left side) */
	UPROPERTY(EditDefaultsOnly, Category = HUD)
	FCanvasIcon SecondaryClipIcon;

	/** how many icons to draw per clip */
	UPROPERTY(EditDefaultsOnly, Category = HUD)
	float AmmoIconsCount;

	/** defines spacing between primary ammo icons (left side) */
	UPROPERTY(EditDefaultsOnly, Category = HUD)
	int32 PrimaryClipIconOffset;

	/** defines spacing between secondary ammo icons (left side) */
	UPROPERTY(EditDefaultsOnly, Category = HUD)
	int32 SecondaryClipIconOffset;

	/** crosshair parts icons (left, top, right, bottom and center) */
	UPROPERTY(EditDefaultsOnly, Category = HUD)
	FCanvasIcon Crosshair[5];

	/** crosshair parts icons when targeting (left, top, right, bottom and center) */
	UPROPERTY(EditDefaultsOnly, Category = HUD)
	FCanvasIcon AimingCrosshair[5];

	/** only use red colored center part of aiming crosshair */
	UPROPERTY(EditDefaultsOnly, Category = HUD)
	bool UseLaserDot;

	/** false = default crosshair */
	UPROPERTY(EditDefaultsOnly, Category = HUD)
	bool UseCustomCrosshair;

	/** false = use custom one if set, otherwise default crosshair */
	UPROPERTY(EditDefaultsOnly, Category = HUD)
	bool UseCustomAimingCrosshair;

	/** true - crosshair will not be shown unless aiming with the weapon */
	UPROPERTY(EditDefaultsOnly, Category = HUD)
	bool bHideCrosshairWhileNotAiming;

	/** check if weapon has infinite ammo (include owner's cheats) */
	bool HasInfiniteAmmo() const;

	/** check if weapon has infinite clip (include owner's cheats) */
	bool HasInfiniteClip() const;

	/** set the weapon's owning pawn */
	void SetOwningPawn(ANimModCharacter* ANimModCharacter);

	/** gets last time when this weapon was switched to */
	float GetEquipStartedTime() const;

	/** gets the duration of equipping weapon*/
	float GetEquipDuration() const;

protected:

	UPROPERTY(Transient, Replicated)
	ENimModFireMode CurrentFireMode;

	/** pawn owner */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_MyPawn)
	class ANimModCharacter* MyPawn;

	/** weapon data */
	UPROPERTY(EditDefaultsOnly, Category = Config)
	FWeaponData WeaponConfig;

private:
	/** weapon mesh: 1st person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** weapon mesh: 3rd person view */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* Mesh3P;
protected:

	/** firing audio (bLoopedFireSound set) */
	UPROPERTY(Transient)
	UAudioComponent* FireAC;

	/** name of bone/socket for muzzle in weapon mesh */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	FName MuzzleAttachPoint;

	/** FX for muzzle flash */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* MuzzleFX;

	/** spawned component for muzzle FX */
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

	/** spawned component for second muzzle FX (Needed for split screen) */
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSCSecondary;

	/** camera shake on firing */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<UCameraShake> FireCameraShake;

	/** force feedback effect to play when the weapon is fired */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UForceFeedbackEffect *FireForceFeedback;

	/** single fire sound (bLoopedFireSound not set) */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireSound;

	/** looped fire sound (bLoopedFireSound set) */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireLoopSound;

	/** finished burst sound (bLoopedFireSound set) */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* FireFinishSound;

	/** out of ammo sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* OutOfAmmoSound;

	/** reload sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* ReloadSound;

	/** reload animations */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FWeaponAnim ReloadAnim;

	/** equip sound */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	USoundCue* EquipSound;

	/** equip animations */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FWeaponAnim EquipAnim;

	/** fire animations */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	FWeaponAnim FireAnim;

	/** fire animations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	FWeaponAnim IdleAnim;

	/** is muzzle FX looped? */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	uint32 bLoopedMuzzleFX : 1;

	/** is fire sound looped? */
	UPROPERTY(EditDefaultsOnly, Category = Sound)
	uint32 bLoopedFireSound : 1;

	/** is fire animation looped? */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	uint32 bLoopedFireAnim : 1;

	/** is fire animation playing? */
	uint32 bPlayingFireAnim : 1;

	/** is weapon currently equipped? */
	uint32 bIsEquipped : 1;

	/** is weapon fire active? */
	uint32 bWantsToFire : 1;

	/** is reload animation playing? */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Reload)
	uint32 bPendingReload : 1;

	/** is equip animation playing? */
	uint32 bPendingEquip : 1;

	/** weapon is refiring */
	uint32 bRefiring;

	/** current weapon state */
	EWeaponState::Type CurrentState;

	/** time of last successful weapon fire */
	float LastFireTime;

	/** last time when this weapon was switched to */
	float EquipStartedTime;

	/** how much time weapon needs to be equipped */
	float EquipDuration;

	/** current total ammo */
	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmo;

	/** current ammo - inside clip */
	UPROPERTY(Transient, Replicated)
	int32 CurrentAmmoInClip;

	/** burst counter, used for replicating fire events to remote clients */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_BurstCounter)
	int32 BurstCounter;

	/** Handle for efficient management of OnEquipFinished timer */
	FTimerHandle TimerHandle_OnEquipFinished;

	/** Handle for efficient management of StopReload timer */
	FTimerHandle TimerHandle_StopReload;

	/** Handle for efficient management of ReloadWeapon timer */
	FTimerHandle TimerHandle_ReloadWeapon;

	/** Handle for efficient management of HandleFiring timer */
	FTimerHandle TimerHandle_HandleFiring;

	//////////////////////////////////////////////////////////////////////////
	// Input - server side

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartPrimaryFire();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopPrimaryFire();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartSecondaryFire();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopSecondaryFire();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStartReload();

	UFUNCTION(reliable, server, WithValidation)
	void ServerStopReload();


	//////////////////////////////////////////////////////////////////////////
	// Replication & effects

	UFUNCTION()
	void OnRep_MyPawn();

	UFUNCTION()
	void OnRep_BurstCounter();

	UFUNCTION()
	void OnRep_Reload();

	/** Called in network play to do the cosmetic fx for firing */
	virtual void SimulateWeaponFire();

	/** Called in network play to stop cosmetic fx (e.g. for a looping shot). */
	virtual void StopSimulatingWeaponFire();


	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() PURE_VIRTUAL(ANimModWeapon::FireWeapon, );

	/** [server] fire & update ammo */
	UFUNCTION(reliable, server, WithValidation)
	void ServerHandleFiring();

	/** [local + server] handle weapon fire */
	void HandleFiring();

	/** [local + server] firing started */
	virtual void OnBurstStarted();

	/** [local + server] firing finished */
	virtual void OnBurstFinished();

	/** update weapon state */
	void SetWeaponState(EWeaponState::Type NewState);

	/** determine current weapon state */
	void DetermineWeaponState();


	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/** attaches weapon mesh to pawn's mesh */
	void AttachMeshToPawn();

	/** detaches weapon mesh from pawn */
	void DetachMeshFromPawn();


	//////////////////////////////////////////////////////////////////////////
	// Weapon usage helpers

	/** play weapon sounds */
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);

	/** play weapon animations */
	float PlayWeaponAnimation(const FWeaponAnim& Animation);

	/** stop playing weapon animations */
	void StopWeaponAnimation(const FWeaponAnim& Animation);

	/** Get the aim of the weapon, allowing for adjustments to be made by the weapon */
	virtual FVector GetAdjustedAim() const;

	/** Get the aim of the camera */
	FVector GetCameraAim() const;

	/** get the originating location for camera damage */
	FVector GetCameraDamageStartLocation(const FVector& AimDir) const;

	/** get the muzzle location of the weapon */
	FVector GetMuzzleLocation() const;

	/** get direction of weapon's muzzle */
	FVector GetMuzzleDirection() const;

	/** find hit */
	FHitResult WeaponTrace(const FVector& TraceFrom, const FVector& TraceTo) const;

protected:
	/** Returns Mesh1P subobject **/
	FORCEINLINE USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns Mesh3P subobject **/
	FORCEINLINE USkeletalMeshComponent* GetMesh3P() const { return Mesh3P; }
	
};
