// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Actor.h"
#include "NimModTypes.h"
#include "NimModProjectile.generated.h"

UCLASS(Abstract, Blueprintable)
class ANimModProjectile : public AActor
{
	GENERATED_BODY()

	/** life time */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	bool ShouldBounce;

	/** life time */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float GravityScale;

	/** life time */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float MaxSpeed;

	/** force of explosion */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float RadialForce;

public:
	ANimModProjectile(const FObjectInitializer& ObjectInitializer);

	/** initial setup */
	virtual void PostInitializeComponents() override;

	/** setup velocity */
	void InitVelocity(FVector& ShootDirection);

	/** handle hit */
	UFUNCTION()
	void OnImpact(const FHitResult& HitResult);

private:
	/** movement component */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UProjectileMovementComponent* MovementComp;

	/** collisions */
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	UParticleSystemComponent* ParticleComp;

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	URadialForceComponent *RadialForceComp;
protected:

	/** effects for explosion */
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	TSubclassOf<class ANimModExplosionEffect> ExplosionTemplate;

	/** controller that fired me (cache for damage calculations) */
	TWeakObjectPtr<AController> MyController;

	/** projectile data */
	FProjectileWeaponData WeaponConfig;

	/** did it explode? */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_Exploded)
	bool bExploded;

	/** [client] explosion happened */
	UFUNCTION()
	void OnRep_Exploded();

	/** trigger explosion */
	void Explode(const FHitResult& Impact);

	/** shutdown projectile and prepare for destruction */
	void DisableAndDestroy();

	/** update velocity on client */
	virtual void PostNetReceiveVelocity(const FVector& NewVelocity) override;

protected:
	/** Returns MovementComp subobject **/
	FORCEINLINE UProjectileMovementComponent* GetMovementComp() const { return MovementComp; }
	/** Returns CollisionComp subobject **/
	FORCEINLINE USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ParticleComp subobject **/
	FORCEINLINE UParticleSystemComponent* GetParticleComp() const { return ParticleComp; }
	/** Returns RadialForceComp subobject **/
	FORCEINLINE URadialForceComponent *GetRadialForceComp() const { return RadialForceComp; }
};

