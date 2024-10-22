// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "NimMod.h"
#include "NimModCharacter.h"
#include "NimModProjectile.h"
#include "NimModPlayerState.h"
#include "NimModGameState.h"
#include "NimModGameMode.h"
#include "NimModPlayerController.h"
#include "NimModPlayerCameraManager.h"
#include "NimModWeapon.h"
#include "NimModDamageType.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

#define MAX_SLOTS_PER_INVENTORY_SLOT 3

//////////////////////////////////////////////////////////////////////////
// ANimModCharacter

ANimModCharacter::ANimModCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(46.f, 92.0f);

	/*CharacterCameraComponent = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FirstPersonCamera"));
	CharacterCameraComponent->AttachParent = GetCapsuleComponent();*/
	DefaultBaseEyeHeight = 71.f;
	BaseEyeHeight = DefaultBaseEyeHeight;
	//CharacterCameraComponent->RelativeLocation = FVector(0, 0, DefaultBaseEyeHeight); // Position the camera
	//CharacterCameraComponent->bUsePawnControlRotation = true;

	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("PawnMesh1P"));
	Mesh1P->AttachParent = GetCapsuleComponent();
	Mesh1P->SetOnlyOwnerSee(true);
	/*Mesh1P->bOwnerNoSee = false;*/
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh1P->PrimaryComponentTick.TickGroup = TG_PrePhysics;
	Mesh1P->bChartDistanceFactor = false;
	Mesh1P->SetCollisionObjectType(ECC_Pawn);
	Mesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh1P->SetCollisionResponseToAllChannels(ECR_Ignore);

	/*GetMesh()->bOnlyOwnerSee = false;
	GetMesh()->bOwnerNoSee = true;*/
	GetMesh()->SetOnlyOwnerSee(false);
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bReceivesDecals = false;
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	// Adjust jump to make it less floaty
	MoveComp->GravityScale = 1.5f;
	MoveComp->JumpZVelocity = 620;
	MoveComp->bCanWalkOffLedgesWhenCrouching = true;
	MoveComp->MaxWalkSpeedCrouched = 200;
	MoveComp->AirControl = 1;

	/* Ignore this channel or it will absorb the trace impacts instead of the skeletal mesh */
	//GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	// Enable crouching
	MoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;

	TargetingSpeedModifier = 1.5f;
	bIsTargeting = false;
	RunningSpeedModifier = 5.0f;
	bWantsToRun = false;
	bWantsToFire = false;
	LowHealthPercentage = 0.5f;

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
}

void ANimModCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Role == ROLE_Authority)
	{
		Health = GetMaxHealth();
		SpawnDefaultInventory();
	}

	// set initial mesh visibility (3rd person view)
	UpdatePawnMeshes();

	// create material instance for setting team colors (3rd person view)
	for (int32 iMat = 0; iMat < GetMesh()->GetNumMaterials(); iMat++)
	{
		MeshMIDs.Add(GetMesh()->CreateAndSetMaterialInstanceDynamic(iMat));
	}

	// play respawn effects
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (RespawnFX)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, RespawnFX, GetActorLocation(), GetActorRotation());
		}

		if (RespawnSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, RespawnSound, GetActorLocation());
		}
	}
}

void ANimModCharacter::Destroyed()
{
	Super::Destroyed();
	DestroyInventory();
}

void ANimModCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();

	// switch mesh to 1st person view
	UpdatePawnMeshes();

	// reattach weapon if needed
	SetCurrentWeapon(CurrentWeapon);

	// set team colors for 1st person view

	//The material's index that we want is 1, not 0.
	UMaterialInstanceDynamic* Mesh1PMID = Mesh1P->CreateAndSetMaterialInstanceDynamic(1);
	UpdateTeamColors(Mesh1PMID);
}

void ANimModCharacter::PossessedBy(class AController* InController)
{
	Super::PossessedBy(InController);

	// [server] as soon as PlayerState is assigned, set team colors of this pawn for local player
	UpdateTeamColorsAllMIDs();

	UCharacterMovementComponent *MoveComp = GetCharacterMovement();
	ANimModPlayerController *NimModController = GetNimModPlayerController();
	if (MoveComp && NimModController)
	{
		ANimModTeam *ourTeam = NimModController->GetPlayerTeam();
		if (ourTeam)
		{
			MoveComp->MaxWalkSpeed = ourTeam->TeamInfo.MaxSpeed;
		}
	}
}

void ANimModCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// [client] as soon as PlayerState is assigned, set team colors of this pawn for local player
	if (PlayerState != NULL)
	{
		UpdateTeamColorsAllMIDs();
	}
}

void ANimModCharacter::BeginPlay()
{
	//GetMesh()->SetOwnerNoSee(false); // compatibility with old content, we're doing this through UpdateHiddenComponents() now

	if (GetWorld()->GetNetMode() != NM_DedicatedServer)
	{
		APlayerController* PC = GEngine->GetFirstLocalPlayerController(GetWorld());
		if (PC != NULL && PC->MyHUD != NULL)
		{
			PC->MyHUD->AddPostRenderedActor(this);
		}
	}
	/*if (Health == 0 && Role == ROLE_Authority)
	{
		Health = HealthMax;
	}*/
	/*CharacterCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, DefaultBaseEyeHeight), false);
	if (CharacterCameraComponent->RelativeLocation.Size2D() > 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: CameraComponent shouldn't have X/Y translation!"), *GetName());
	}*/
	Super::BeginPlay();
}

void ANimModCharacter::SetPlayerDefaults()
{
	DestroyInventory();
	SpawnDefaultInventory();
}

class ANimModPlayerController *ANimModCharacter::GetNimModPlayerController()
{
	return Cast<ANimModPlayerController>(Controller);
}

//Custom override that removes the "crouched" checks.
bool ANimModCharacter::CanJumpInternal_Implementation() const
{
	const bool bCanHoldToJumpHigher = (GetJumpMaxHoldTime() > 0.0f) && IsJumpProvidingForce();
	return CharacterMovement && (CharacterMovement->IsMovingOnGround() || bCanHoldToJumpHigher) && CharacterMovement->IsJumpAllowed();
}

void ANimModCharacter::Falling()
{
	StartingFallHeight = GetActorLocation().Z;
}

void ANimModCharacter::Landed(const FHitResult& Hit)
{
	if (Hit.bBlockingHit)
	{
		float fallDistance = StartingFallHeight - GetActorLocation().Z;
		if (fallDistance > 75)
		{
			//Apply fall damage?
			int i = 0;
		}
	}

	Super::Landed(Hit);
}

FRotator ANimModCharacter::GetAimOffsets() const
{
	const FVector AimDirWS = GetBaseAimRotation().Vector();
	const FVector AimDirLS = ActorToWorld().InverseTransformVectorNoScale(AimDirWS);
	const FRotator AimRotLS = AimDirLS.Rotation();

	return AimRotLS;
}

bool ANimModCharacter::IsEnemyFor(AController* TestPC) const
{
	if (TestPC == Controller || TestPC == NULL)
	{
		return false;
	}

	ANimModPlayerState* TestPlayerState = Cast<ANimModPlayerState>(TestPC->PlayerState);
	ANimModPlayerState* MyPlayerState = Cast<ANimModPlayerState>(PlayerState);

	bool bIsEnemy = true;
	if (GetWorld()->GameState && GetWorld()->GameState->GameModeClass)
	{
		const ANimModGameMode* DefGame = GetWorld()->GameState->GameModeClass->GetDefaultObject<ANimModGameMode>();
		if (DefGame && MyPlayerState && TestPlayerState)
		{
			bIsEnemy = DefGame->CanDealDamage(TestPlayerState, MyPlayerState);
		}
	}

	return bIsEnemy;
}


//////////////////////////////////////////////////////////////////////////
// Meshes

void ANimModCharacter::UpdatePawnMeshes()
{
	bool const bFirstPerson = IsFirstPerson();
	if (Mesh1P != nullptr)
	{
		Mesh1P->MeshComponentUpdateFlag = !bFirstPerson ? EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered : EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;
		Mesh1P->SetOwnerNoSee(!bFirstPerson);
	}

	GetMesh()->MeshComponentUpdateFlag = bFirstPerson ? EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered : EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;
	GetMesh()->SetOwnerNoSee(bFirstPerson);
}

void ANimModCharacter::UpdateTeamColors(UMaterialInstanceDynamic* UseMID)
{
	if (UseMID)
	{
		ANimModPlayerState* MyPlayerState = Cast<ANimModPlayerState>(PlayerState);
		ANimModGameState *gameState = Cast<ANimModGameState>(GetWorld()->GetGameState());
		if (MyPlayerState && gameState)
		{
			ANimModTeam *ourTeam = gameState->GetTeam(MyPlayerState->GetTeam());
			if (ourTeam)
			{
				UseMID->SetVectorParameterValue(TEXT("AccentColor"), ourTeam->TeamInfo.TeamColor);
				UseMID->SetVectorParameterValue(TEXT("TextColor"), ourTeam->TeamInfo.TeamColor);
			}	
		}
		//ANimModPlayerState* MyPlayerState = Cast<ANimModPlayerState>(PlayerState);
		//if (MyPlayerState != NULL)
		//{
		//	float MaterialParam = (float)MyPlayerState->GetTeam();
		//	UseMID->SetScalarParameterValue(TEXT("Team Color Index"), MaterialParam);
		//	
		//}
	}
}

void ANimModCharacter::OnCameraUpdate(const FVector& PreviousCameraLocation, const FVector& CameraLocation, const FRotator& CameraRotation)
{
	USkeletalMeshComponent* DefMesh1P = Cast<USkeletalMeshComponent>(GetClass()->GetDefaultSubobjectByName(TEXT("PawnMesh1P")));
	//USkeletalMeshComponent* DefMesh1P = Mesh1P;
	const FMatrix DefMeshLS = FRotationTranslationMatrix(DefMesh1P->RelativeRotation, DefMesh1P->RelativeLocation);
	const FMatrix LocalToWorld = ActorToWorld().ToMatrixWithScale();

	// Mesh rotating code expect uniform scale in LocalToWorld matrix

	const FRotator RotCameraPitch(CameraRotation.Pitch, 0.0f, 0.0f);
	const FRotator RotCameraYaw(0.0f, CameraRotation.Yaw, 0.0f);

	const FMatrix LeveledCameraLS = FRotationTranslationMatrix(RotCameraYaw, CameraLocation) * LocalToWorld.Inverse();
	const FMatrix PitchedCameraLS = FRotationMatrix(RotCameraPitch) * LeveledCameraLS;
	const FMatrix MeshRelativeToCamera = DefMeshLS * LeveledCameraLS.Inverse();
	const FMatrix PitchedMesh = MeshRelativeToCamera * PitchedCameraLS;

	//FVector origin = PitchedMesh.GetOrigin();
	FVector origin = Mesh1P->GetRelativeTransform().GetLocation();
	//FVector originalLocation = Mesh1P->GetRelativeTransform().GetLocation();
	////FVector origin = Mesh1P->ComponentToWorld.GetLocation();
	//if (bIsCrouched)
	//{
	//	//origin.Z = (origin.Z - (DefaultBaseEyeHeight - DefMesh1P->RelativeLocation.Z));
	//	origin.Z = (PreviousCameraLocation.Z - CameraLocation.Z);
	//}

	Mesh1P->SetRelativeLocationAndRotation(origin, PitchedMesh.Rotator(), false, nullptr, ETeleportType::TeleportPhysics);
	/*FVector newLocation = Mesh1P->GetRelativeTransform().GetLocation();
	int i = -1;*/
}


//////////////////////////////////////////////////////////////////////////
// Damage & death


void ANimModCharacter::FellOutOfWorld(const class UDamageType& dmgType)
{
	Die(Health, FDamageEvent(dmgType.GetClass()), NULL, NULL);
}

void ANimModCharacter::Suicide()
{
	KilledBy(this);
}

void ANimModCharacter::KilledBy(APawn* EventInstigator)
{
	if (Role == ROLE_Authority && !bIsDying)
	{
		AController* Killer = NULL;
		if (EventInstigator != NULL)
		{
			Killer = EventInstigator->Controller;
			LastHitBy = NULL;
		}

		Die(Health, FDamageEvent(UDamageType::StaticClass()), Killer, NULL);
	}
}


float ANimModCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	ANimModPlayerController* MyPC = Cast<ANimModPlayerController>(Controller);
	if (MyPC && MyPC->HasGodMode())
	{
		return 0.f;
	}

	if (Health <= 0)
	{
		return 0;
	}

	// Modify based on game rules.
	ANimModGameMode* const Game = GetWorld()->GetAuthGameMode<ANimModGameMode>();
	Damage = Game ? Game->ModifyDamage(Damage, this, DamageEvent, EventInstigator, DamageCauser) : 0.f;

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage > 0.f)
	{
		Health -= (int)ActualDamage;
		if (Health <= 0)
		{
			Die(ActualDamage, DamageEvent, EventInstigator, DamageCauser);
		}
		else
		{
			PlayHit(ActualDamage, DamageEvent, EventInstigator ? EventInstigator->GetPawn() : NULL, DamageCauser);
		}

		MakeNoise(1.0f, EventInstigator ? EventInstigator->GetPawn() : this);
	}

	return ActualDamage;
}


bool ANimModCharacter::CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const
{
	if (bIsDying										// already dying
		|| IsPendingKill()								// already destroyed
		|| Role != ROLE_Authority						// not authority
		|| GetWorld()->GetAuthGameMode() == NULL
		|| GetWorld()->GetAuthGameMode()->GetMatchState() == MatchState::LeavingMap)	// level transition occurring
	{
		return false;
	}

	return true;
}


bool ANimModCharacter::Die(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser)
{
	if (!CanDie(KillingDamage, DamageEvent, Killer, DamageCauser))
	{
		return false;
	}

	Health = FMath::Min(0, Health);

	// if this is an environmental death then refer to the previous killer so that they receive credit (knocked into lava pits, etc)
	UDamageType const* const DamageType = DamageEvent.DamageTypeClass ? DamageEvent.DamageTypeClass->GetDefaultObject<UDamageType>() : GetDefault<UDamageType>();
	Killer = GetDamageInstigator(Killer, *DamageType);

	AController* const KilledPlayer = (Controller != NULL) ? Controller : Cast<AController>(GetOwner());
	GetWorld()->GetAuthGameMode<ANimModGameMode>()->Killed(Killer, KilledPlayer, this, DamageType);

	NetUpdateFrequency = GetDefault<ANimModCharacter>()->NetUpdateFrequency;
	GetCharacterMovement()->ForceReplicationUpdate();

	OnDeath(KillingDamage, DamageEvent, Killer ? Killer->GetPawn() : NULL, DamageCauser);
	return true;
}


void ANimModCharacter::OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	if (bIsDying)
	{
		return;
	}

	bReplicateMovement = false;
	bTearOff = true;
	bIsDying = true;

	if (Role == ROLE_Authority)
	{
		ReplicateHit(KillingDamage, DamageEvent, PawnInstigator, DamageCauser, true);

		// play the force feedback effect on the client player controller
		APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC && DamageEvent.DamageTypeClass)
		{
			UNimModDamageType *DamageType = Cast<UNimModDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
			if (DamageType && DamageType->KilledForceFeedback)
			{
				PC->ClientPlayForceFeedback(DamageType->KilledForceFeedback, false, "Damage");
			}
		}
	}

	// cannot use IsLocallyControlled here, because even local client's controller may be NULL here
	if (GetNetMode() != NM_DedicatedServer && DeathSound && Mesh1P && Mesh1P->IsVisible())
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}

	ANimModPlayerController *controller = GetNimModPlayerController();
	if (controller)
	{
		ANimModPlayerCameraManager *cameraManager = controller->GetNimModPlayerCameraManager();
		if (cameraManager)
			cameraManager->ResetFOV();
	}

	// remove all weapons
	DestroyInventory();

	// switch back to 3rd person view
	UpdatePawnMeshes();

	DetachFromControllerPendingDestroy();
	StopAllAnimMontages();

	if (LowHealthWarningPlayer && LowHealthWarningPlayer->IsPlaying())
	{
		LowHealthWarningPlayer->Stop();
	}

	if (RunLoopAC)
	{
		RunLoopAC->Stop();
	}

	// disable collisions on capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	if (GetMesh())
	{
		static FName CollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetCollisionProfileName(CollisionProfileName);
	}
	SetActorEnableCollision(true);

	// Death anim
	float DeathAnimDuration = PlayAnimMontage(DeathAnim);

	// Ragdoll
	if (DeathAnimDuration > 0.f)
	{
		// Use a local timer handle as we don't need to store it for later but we don't need to look for something to clear
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ANimModCharacter::SetRagdollPhysics, FMath::Min(0.1f, DeathAnimDuration), false);
	}
	else
	{
		SetRagdollPhysics();
	}
}

void ANimModCharacter::PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser)
{
	if (Role == ROLE_Authority)
	{
		ReplicateHit(DamageTaken, DamageEvent, PawnInstigator, DamageCauser, false);

		// play the force feedback effect on the client player controller
		APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC && DamageEvent.DamageTypeClass)
		{
			UNimModDamageType *DamageType = Cast<UNimModDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
			if (DamageType && DamageType->HitForceFeedback)
			{
				PC->ClientPlayForceFeedback(DamageType->HitForceFeedback, false, "Damage");
			}
		}
	}

	if (DamageTaken > 0.f)
	{
		ApplyDamageMomentum(DamageTaken, DamageEvent, PawnInstigator, DamageCauser);
	}

	ANimModPlayerController* MyPC = Cast<ANimModPlayerController>(Controller);
	ANimModHUD* MyHUD = MyPC ? Cast<ANimModHUD>(MyPC->GetHUD()) : NULL;
	if (MyHUD)
	{
		MyHUD->HandleHit(DamageTaken, DamageEvent, PawnInstigator);
	}
	/*
	if (PawnInstigator && PawnInstigator != this && PawnInstigator->IsLocallyControlled())
	{
		ANimModPlayerController* InstigatorPC = Cast<ANimModPlayerController>(PawnInstigator->Controller);
		ANimModHUD* InstigatorHUD = InstigatorPC ? Cast<ANimModHUD>(InstigatorPC->GetHUD()) : NULL;
		if (InstigatorHUD)
		{
			InstigatorHUD->NotifyEnemyHit();
		}
	}*/
}


void ANimModCharacter::SetRagdollPhysics()
{
	bool bInRagdoll = false;

	if (IsPendingKill())
	{
		bInRagdoll = false;
	}
	else if (!GetMesh() || !GetMesh()->GetPhysicsAsset())
	{
		bInRagdoll = false;
	}
	else
	{
		// initialize physics/etc
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;

		bInRagdoll = true;
	}

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);

	if (!bInRagdoll)
	{
		// hide and set short lifespan
		TurnOff();
		SetActorHiddenInGame(true);
		SetLifeSpan(1.0f);
	}
	else
	{
		SetLifeSpan(10.0f);
	}
}



void ANimModCharacter::ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser, bool bKilled)
{
	const float TimeoutTime = GetWorld()->GetTimeSeconds() + 0.5f;

	FDamageEvent const& LastDamageEvent = LastTakeHitInfo.GetDamageEvent();
	if ((PawnInstigator == LastTakeHitInfo.PawnInstigator.Get()) && (LastDamageEvent.DamageTypeClass == LastTakeHitInfo.DamageTypeClass) && (LastTakeHitTimeTimeout == TimeoutTime))
	{
		// same frame damage
		if (bKilled && LastTakeHitInfo.bKilled)
		{
			// Redundant death take hit, just ignore it
			return;
		}

		// otherwise, accumulate damage done this frame
		Damage += LastTakeHitInfo.ActualDamage;
	}

	LastTakeHitInfo.ActualDamage = Damage;
	LastTakeHitInfo.PawnInstigator = Cast<ANimModCharacter>(PawnInstigator);
	LastTakeHitInfo.DamageCauser = DamageCauser;
	LastTakeHitInfo.SetDamageEvent(DamageEvent);
	LastTakeHitInfo.bKilled = bKilled;
	LastTakeHitInfo.EnsureReplication();

	LastTakeHitTimeTimeout = TimeoutTime;
}

void ANimModCharacter::OnRep_LastTakeHitInfo()
{
	if (LastTakeHitInfo.bKilled)
	{
		OnDeath(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get());
	}
	else
	{
		PlayHit(LastTakeHitInfo.ActualDamage, LastTakeHitInfo.GetDamageEvent(), LastTakeHitInfo.PawnInstigator.Get(), LastTakeHitInfo.DamageCauser.Get());
	}
}

//Pawn::PlayDying sets this lifespan, but when that function is called on client, dead pawn's role is still SimulatedProxy despite bTearOff being true. 
void ANimModCharacter::TornOff()
{
	SetLifeSpan(25.f);
}


//////////////////////////////////////////////////////////////////////////
// Inventory

void ANimModCharacter::SpawnDefaultInventory()
{
	if (Role < ROLE_Authority)
	{
		return;
	}

	//Initialize the empty array of our inventory.
	int32 maxFlatInventoryCount = 10 * MAX_SLOTS_PER_INVENTORY_SLOT;
	for (int32 i = 0; i < maxFlatInventoryCount; ++i)
		Inventory.Add(nullptr);

	int32 NumWeaponClasses = DefaultInventoryClasses.Num();
	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (DefaultInventoryClasses[i])
		{
			FActorSpawnParameters SpawnInfo;
			SpawnInfo.bNoCollisionFail = true;
			ANimModWeapon* NewWeapon = GetWorld()->SpawnActor<ANimModWeapon>(DefaultInventoryClasses[i], SpawnInfo);
			AddWeapon(NewWeapon);
		}
	}


	for (auto *weapon : Inventory)
	{
		if (weapon != nullptr)
		{
			EquipWeapon(weapon);
			break;
		}
	}
	// equip first weapon in inventory
	/*if (Inventory.Num() > 0)
	{
		if (Inventory[0].Num() > 0)
			EquipWeapon(Inventory[0][0]);
	}*/
}

void ANimModCharacter::DestroyInventory()
{
	for (ANimModWeapon *nimModWeapon : Inventory)
	{
		if (nimModWeapon)
		{
			RemoveWeapon(nimModWeapon);
			if (Role == ROLE_Authority)
				nimModWeapon->Destroy();
		}
		/*InventorySlot slot = slotKVP.Value;
		while (slot.Num() != 0)
		{
			InventorySlot::TIterator iter = slot.CreateIterator();
			ANimModWeapon *weapon = iter.Value;
			if (weapon)
			{
				RemoveWeapon(weapon);
				weapon->Destroy();
			}
		}*/
	}
}

void ANimModCharacter::AddWeapon(ANimModWeapon* Weapon)
{
	if (Weapon && Role == ROLE_Authority)
	{
		int32 weaponIndex = GetWeaponInventoryIndex(Weapon);
		if (Inventory[weaponIndex] != nullptr)
			return;

		Inventory[weaponIndex] = Weapon;
		//slot.KeySort(TLess<uint32>());
		Weapon->OnEnterInventory(this);
	}
}

void ANimModCharacter::RemoveWeapon(ANimModWeapon* Weapon)
{
	if (Weapon && CurrentWeapon == Weapon)
		Weapon->OnUnEquip();

	if (Weapon && Role == ROLE_Authority)
	{
		int32 weaponIndex = GetWeaponInventoryIndex(Weapon);
		Inventory[weaponIndex] = nullptr;
		Weapon->OnLeaveInventory();
	}
}

int32 ANimModCharacter::GetWeaponInventoryIndex(class ANimModWeapon *weapon)
{
	return (weapon->GetInventorySlot() * MAX_SLOTS_PER_INVENTORY_SLOT) + weapon->GetInventorySlotOrder();
}

bool ANimModCharacter::DoesSlotHaveWeapons(int32 slot)
{
	int32 slotStart, slotEnd;
	GetSlotStartAndEnd(slot, slotStart, slotEnd);
	for (int32 index = slotStart; index < slotEnd; ++index)
	{
		if (Inventory[index] != nullptr)
			return true;
	}
	return false;
}

void ANimModCharacter::GetSlotStartAndEnd(int32 slot, int32 &start, int32 &end)
{
	start = slot * MAX_SLOTS_PER_INVENTORY_SLOT;
	end = start + MAX_SLOTS_PER_INVENTORY_SLOT;
}

//Is this needed?
ANimModWeapon* ANimModCharacter::FindWeapon(TSubclassOf<ANimModWeapon> WeaponClass)
{
	/*for (int32 i = 0; i < Inventory.Num(); i++)
	{
		if (Inventory[i] && Inventory[i]->IsA(WeaponClass))
		{
			return Inventory[i];
		}
	}*/

	return NULL;
}

void ANimModCharacter::EquipWeapon(ANimModWeapon* Weapon)
{
	if (Weapon)
	{
		if (Role == ROLE_Authority)
		{
			SetCurrentWeapon(Weapon);
		}
		else
		{
			ServerEquipWeapon(Weapon);
		}
	}
}

bool ANimModCharacter::ServerEquipWeapon_Validate(ANimModWeapon* Weapon)
{
	return true;
}

void ANimModCharacter::ServerEquipWeapon_Implementation(ANimModWeapon* Weapon)
{
	EquipWeapon(Weapon);
}

void ANimModCharacter::OnRep_CurrentWeapon(ANimModWeapon* LastWeapon)
{
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}

void ANimModCharacter::SetCurrentWeapon(class ANimModWeapon* NewWeapon, class ANimModWeapon* LastWeapon)
{
	ANimModWeapon* LocalLastWeapon = NULL;

	if (LastWeapon != NULL)
	{
		LocalLastWeapon = LastWeapon;
	}
	else if (NewWeapon != CurrentWeapon)
	{
		LocalLastWeapon = CurrentWeapon;
	}

	// unequip previous
	if (LocalLastWeapon)
	{
		LocalLastWeapon->OnUnEquip();
	}

	CurrentWeapon = NewWeapon;

	// equip new one
	if (NewWeapon)
	{
		NewWeapon->SetOwningPawn(this);	// Make sure weapon's MyPawn is pointing back to us. During replication, we can't guarantee APawn::CurrentWeapon will rep after AWeapon::MyPawn!
		NewWeapon->OnEquip();
	}
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage

void ANimModCharacter::StartPrimaryWeaponFire()
{
	//UE_LOG(LogNimMod, Warning, TEXT("ANimModCharacter::StartWeaponFire()"));
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		PreviousViewPunchStep = FVector2D::ZeroVector;
		if (CurrentWeapon)
		{
			CurrentWeapon->StartPrimaryFire();
		}
	}
}

void ANimModCharacter::StopPrimaryWeaponFire()
{
	//UE_LOG(LogNimMod, Warning, TEXT("ANimModCharacter::StopWeaponFire()"));
	if (bWantsToFire)
	{
		bWantsToFire = false;
		PreviousViewPunchStep = TotalViewPunch;
		if (CurrentWeapon)
		{
			CurrentWeapon->StopPrimaryFire();
		}
	}
}

void ANimModCharacter::StartSecondaryWeaponFire()
{
	//UE_LOG(LogNimMod, Warning, TEXT("ANimModCharacter::StartWeaponFire()"));
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		if (CurrentWeapon)
		{
			CurrentWeapon->StartSecondaryFire();
		}
	}
}

void ANimModCharacter::StopSecondaryWeaponFire()
{
	//UE_LOG(LogNimMod, Warning, TEXT("ANimModCharacter::StopWeaponFire()"));
	if (bWantsToFire)
	{
		bWantsToFire = false;
		if (CurrentWeapon)
		{
			CurrentWeapon->StopSecondaryFire();
		}
	}
}

bool ANimModCharacter::CanFire() const
{
	ANimModPlayerController* MyPC = Cast<ANimModPlayerController>(Controller);
	return (MyPC) ? !MyPC->IsFrozen() && IsAlive() : IsAlive();
}

bool ANimModCharacter::CanReload() const
{
	return true;
}

void ANimModCharacter::AddViewPunch(FVector2D punch)
{
	ANimModPlayerController* MyPC = Cast<ANimModPlayerController>(Controller);
	if (MyPC)
	{
		MyPC->AddPitchInput(punch.Y);
		MyPC->AddYawInput(punch.X);
		TotalViewPunch += punch;
	}
}

void ANimModCharacter::SetTargeting(bool bNewTargeting)
{
	bIsTargeting = bNewTargeting;

	if (TargetingSound)
	{
		UGameplayStatics::PlaySoundAttached(TargetingSound, GetRootComponent());
	}

	if (Role < ROLE_Authority)
	{
		ServerSetTargeting(bNewTargeting);
	}
}

bool ANimModCharacter::ServerSetTargeting_Validate(bool bNewTargeting)
{
	return true;
}

void ANimModCharacter::ServerSetTargeting_Implementation(bool bNewTargeting)
{
	SetTargeting(bNewTargeting);
}

//////////////////////////////////////////////////////////////////////////
// Movement

void ANimModCharacter::SetRunning(bool bNewRunning, bool bToggle)
{
	bWantsToRun = bNewRunning;
	bWantsToRunToggled = bNewRunning && bToggle;

	if (Role < ROLE_Authority)
	{
		ServerSetRunning(bNewRunning, bToggle);
	}

	UpdateRunSounds(bNewRunning);
}

bool ANimModCharacter::ServerSetRunning_Validate(bool bNewRunning, bool bToggle)
{
	return true;
}

void ANimModCharacter::ServerSetRunning_Implementation(bool bNewRunning, bool bToggle)
{
	SetRunning(bNewRunning, bToggle);
}

void ANimModCharacter::UpdateRunSounds(bool bNewRunning)
{
	if (bNewRunning)
	{
		if (!RunLoopAC && RunLoopSound)
		{
			RunLoopAC = UGameplayStatics::PlaySoundAttached(RunLoopSound, GetRootComponent());
			if (RunLoopAC)
			{
				RunLoopAC->bAutoDestroy = false;
			}

		}
		else if (RunLoopAC)
		{
			RunLoopAC->Play();
		}
	}
	else
	{
		if (RunLoopAC)
		{
			RunLoopAC->Stop();
		}

		if (RunStopSound)
		{
			UGameplayStatics::PlaySoundAttached(RunStopSound, GetRootComponent());
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Animations

float ANimModCharacter::PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate, FName StartSectionName)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance)
	{
		bool isPlaying = UseMesh->AnimScriptInstance->Montage_IsPlaying(AnimMontage);
		if (!isPlaying)
			UseMesh->AnimScriptInstance->Montage_Stop(0.0f, AnimMontage);

		float rtn = UseMesh->AnimScriptInstance->Montage_Play(AnimMontage, InPlayRate);

		/*FOnMontageEnded EndDelegate;
		EndDelegate.BindUObject(this, &ANimModCharacter::OnMontageEnded);
		UseMesh->AnimScriptInstance->Montage_SetEndDelegate(EndDelegate, AnimMontage);*/
		return rtn;
	}

	return 0.0f;
}

void ANimModCharacter::StopAnimMontage(class UAnimMontage* AnimMontage)
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (AnimMontage && UseMesh && UseMesh->AnimScriptInstance &&
		UseMesh->AnimScriptInstance->Montage_IsPlaying(AnimMontage))
	{
		UseMesh->AnimScriptInstance->Montage_Stop(AnimMontage->BlendOutTime);
	}
}

void ANimModCharacter::StopAllAnimMontages()
{
	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (UseMesh && UseMesh->AnimScriptInstance)
	{
		UseMesh->AnimScriptInstance->Montage_Stop(0.0f);
	}
}

void ANimModCharacter::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (CurrentWeapon == nullptr)
		return;

	USkeletalMeshComponent* UseMesh = GetPawnMesh();
	if (UseMesh && UseMesh->AnimScriptInstance)
	{
		if (!UseMesh->AnimScriptInstance->Montage_IsPlaying(nullptr))
		{
			UAnimMontage *idleAnimation = CurrentWeapon->GetIdleAnimation();
			if (idleAnimation)
				PlayAnimMontage(idleAnimation);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// Input

void ANimModCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	check(InputComponent);
	InputComponent->BindAxis("MoveForward", this, &ANimModCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ANimModCharacter::MoveRight);
	InputComponent->BindAxis("MoveUp", this, &ANimModCharacter::MoveUp);
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ANimModCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &ANimModCharacter::LookUpAtRate);

	InputComponent->BindAction("PrimaryFire", IE_Pressed, this, &ANimModCharacter::OnStartPrimaryFire);
	InputComponent->BindAction("PrimaryFire", IE_Released, this, &ANimModCharacter::OnStopPrimaryFire);

	InputComponent->BindAction("SecondaryFire", IE_Pressed, this, &ANimModCharacter::OnStartSecondaryFire);
	InputComponent->BindAction("SecondaryFire", IE_Released, this, &ANimModCharacter::OnStopSecondaryFire);

	InputComponent->BindAction("Targeting", IE_Pressed, this, &ANimModCharacter::OnStartTargeting);
	InputComponent->BindAction("Targeting", IE_Released, this, &ANimModCharacter::OnStopTargeting);

	/*InputComponent->BindAction("NextWeapon", IE_Pressed, this, &ANimModCharacter::OnNextWeapon);
	InputComponent->BindAction("PrevWeapon", IE_Pressed, this, &ANimModCharacter::OnPrevWeapon);*/

	InputComponent->BindAction("Slot1", IE_Pressed, this, &ANimModCharacter::UseSlot<1>);
	InputComponent->BindAction("Slot2", IE_Pressed, this, &ANimModCharacter::UseSlot<2>);
	InputComponent->BindAction("Slot3", IE_Pressed, this, &ANimModCharacter::UseSlot<3>);
	InputComponent->BindAction("Slot4", IE_Pressed, this, &ANimModCharacter::UseSlot<4>);
	InputComponent->BindAction("Slot5", IE_Pressed, this, &ANimModCharacter::UseSlot<5>);
	InputComponent->BindAction("Slot6", IE_Pressed, this, &ANimModCharacter::UseSlot<6>);
	InputComponent->BindAction("Slot7", IE_Pressed, this, &ANimModCharacter::UseSlot<7>);
	InputComponent->BindAction("Slot8", IE_Pressed, this, &ANimModCharacter::UseSlot<8>);
	InputComponent->BindAction("Slot9", IE_Pressed, this, &ANimModCharacter::UseSlot<9>);
	InputComponent->BindAction("Slot0", IE_Pressed, this, &ANimModCharacter::UseSlot<0>);

	InputComponent->BindAction("Reload", IE_Pressed, this, &ANimModCharacter::OnReload);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ANimModCharacter::OnStartJump);
	InputComponent->BindAction("Jump", IE_Released, this, &ANimModCharacter::OnStopJump);

	InputComponent->BindAction("Run", IE_Pressed, this, &ANimModCharacter::OnStartRunning);
	InputComponent->BindAction("Run", IE_Released, this, &ANimModCharacter::OnStopRunning);
	InputComponent->BindAction("RunToggle", IE_Pressed, this, &ANimModCharacter::OnStartRunningToggle);
	
	InputComponent->BindAction("Crouch", IE_Pressed, this, &ANimModCharacter::OnCrouch);
	InputComponent->BindAction("Crouch", IE_Released, this, &ANimModCharacter::OnUnCrouch);
}


void ANimModCharacter::MoveForward(float Val)
{
	if (Controller && Val != 0.f)
	{
		// Limit pitch when walking or falling
		const bool bLimitRotation = (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling());
		const FRotator Rotation = bLimitRotation ? GetActorRotation() : Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, Val);
	}
}

void ANimModCharacter::MoveRight(float Val)
{
	if (Val != 0.f)
	{
		const FRotator Rotation = GetActorRotation();
		const FVector Direction = FRotationMatrix(Rotation).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, Val);
	}
}

void ANimModCharacter::MoveUp(float Val)
{
	if (Val != 0.f)
	{
		// Not when walking or falling.
		if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling())
		{
			return;
		}

		AddMovementInput(FVector::UpVector, Val);
	}
}

void ANimModCharacter::TurnAtRate(float Val)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Val * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ANimModCharacter::LookUpAtRate(float Val)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Val * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void ANimModCharacter::OnStartPrimaryFire()
{
	ANimModPlayerController* MyPC = Cast<ANimModPlayerController>(Controller);
	if (MyPC && !MyPC->IsFrozen() && MyPC->IsGameInputAllowed())
	{
		if (IsRunning())
		{
			SetRunning(false, false);
		}
		MyPC->StartFire();
		StartPrimaryWeaponFire();
	}
}

void ANimModCharacter::OnStopPrimaryFire()
{
	StopPrimaryWeaponFire();
}

void ANimModCharacter::OnStartSecondaryFire()
{
	ANimModPlayerController* MyPC = Cast<ANimModPlayerController>(Controller);
	if (MyPC && !MyPC->IsFrozen() && MyPC->IsGameInputAllowed())
	{
		if (IsRunning())
		{
			SetRunning(false, false);
		}
		StartSecondaryWeaponFire();
	}
}

void ANimModCharacter::OnStopSecondaryFire()
{
	StopSecondaryWeaponFire();
}

void ANimModCharacter::OnStartTargeting()
{
	ANimModPlayerController* MyPC = Cast<ANimModPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		if (IsRunning())
		{
			SetRunning(false, false);
		}
		SetTargeting(true);
	}
}

void ANimModCharacter::OnStopTargeting()
{
	SetTargeting(false);
}

void ANimModCharacter::UseSlot(int32 slot)
{
	//Make sure we have some weapons
	if (Inventory.Num() == 0)
		return;

	//Make sure the slot has weapons
	if (!DoesSlotHaveWeapons(slot))
		return;

	int32 slotStart, slotEnd;
	GetSlotStartAndEnd(slot, slotStart, slotEnd);

	if (CurrentWeapon == nullptr || slot != CurrentWeapon->GetInventorySlot())
	{
		//Find the first weapon in this slot and assign it.
		ANimModWeapon *weapon = nullptr;
		for( int32 index = slotStart; index < slotEnd; ++index)
		{
			weapon = Inventory[index];
			if (weapon)
			{
				EquipWeapon(weapon);
				break;
			}
		}
	}
	else
	{

		//Determine if we're cycling through a slot, or switching to a new one.
		int32 currentWeaponIndex = GetWeaponInventoryIndex(CurrentWeapon);
		ANimModWeapon *newWeapon = nullptr;
		//Cycling.  Find the next weapon in this slot
		for (int32 index = currentWeaponIndex + 1; index < slotEnd; ++index)
		{
			newWeapon = Inventory[index];
			if (newWeapon != nullptr)
				break;
		}

		//No weapons in this slot after our current weapon?  Check the weapons before this weapon
		if (newWeapon == nullptr && currentWeaponIndex != slotStart)
		{
			for (int32 index = slotStart; index < currentWeaponIndex; ++index)
			{
				newWeapon = Inventory[index];
				if (newWeapon != nullptr)
					break;
			}
		}

		//Did we find anything?
		if (newWeapon != nullptr)
			EquipWeapon(newWeapon);

		/*if (slot == CurrentWeapon->GetInventorySlot())
		{
			
		}*/
		//else
		//{
		//	//Find the first weapon in this slot and assign it.
		//	ANimModWeapon *weapon = nullptr;
		//	for (int index = slotStart; index < slotEnd; ++index)
		//	{
		//		weapon = Inventory[index];
		//		if (weapon)
		//		{
		//			EquipWeapon(weapon);
		//			break;
		//		}
		//	}
		//}
	}
}

template<int32 Index>
void ANimModCharacter::UseSlot()
{
	UseSlot(Index);
}

//void ANimModCharacter::OnNextWeapon()
//{
//	ANimModPlayerController* MyPC = Cast<ANimModPlayerController>(Controller);
//	if (MyPC && MyPC->IsGameInputAllowed())
//	{
//		if (CurrentWeapon != NULL)
//		{
//
//		}
//		if (Inventory.Num() >= 2 && (CurrentWeapon == NULL || CurrentWeapon->GetCurrentState() != EWeaponState::Equipping))
//		{
//			const int32 CurrentWeaponIdx = Inventory.IndexOfByKey(CurrentWeapon);
//			ANimModWeapon* NextWeapon = Inventory[(CurrentWeaponIdx + 1) % Inventory.Num()];
//			EquipWeapon(NextWeapon);
//		}
//	}
//}
//
//void ANimModCharacter::OnPrevWeapon()
//{
//	ANimModPlayerController* MyPC = Cast<ANimModPlayerController>(Controller);
//	if (MyPC && MyPC->IsGameInputAllowed())
//	{
//		if (Inventory.Num() >= 2 && (CurrentWeapon == NULL || CurrentWeapon->GetCurrentState() != EWeaponState::Equipping))
//		{
//			const int32 CurrentWeaponIdx = Inventory.IndexOfByKey(CurrentWeapon);
//			ANimModWeapon* PrevWeapon = Inventory[(CurrentWeaponIdx - 1 + Inventory.Num()) % Inventory.Num()];
//			EquipWeapon(PrevWeapon);
//		}
//	}
//}

void ANimModCharacter::OnReload()
{
	ANimModPlayerController* MyPC = Cast<ANimModPlayerController>(Controller);
	if (MyPC && !MyPC->IsFrozen() && MyPC->IsGameInputAllowed())
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->StartReload();
		}
	}
}

void ANimModCharacter::OnStartRunning()
{
	/*ANimModPlayerController* MyPC = Cast<ANimModPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		if (IsTargeting())
		{
			SetTargeting(false);
		}
		StopWeaponFire();
		SetRunning(true, false);
	}*/
}

void ANimModCharacter::OnStartRunningToggle()
{
	/*ANimModPlayerController* MyPC = Cast<ANimModPlayerController>(Controller);
	if (MyPC && MyPC->IsGameInputAllowed())
	{
		if (IsTargeting())
		{
			SetTargeting(false);
		}
		StopWeaponFire();
		SetRunning(true, true);
	}*/
}

void ANimModCharacter::OnStopRunning()
{
	SetRunning(false, false);
}

void ANimModCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	//FVector origin = Mesh1P->ComponentToWorld.GetLocation();
	FTransform currentTransform = Mesh1P->GetRelativeTransform();
	FVector origin = currentTransform.GetLocation();
	origin.Z -= (DefaultBaseEyeHeight - CrouchedEyeHeight);// HalfHeightAdjust;
	FRotator rotation = currentTransform.GetRotation().Rotator();
	Mesh1P->SetRelativeLocationAndRotation(origin, rotation, false, nullptr, ETeleportType::TeleportPhysics);
}

void ANimModCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
	FTransform currentTransform = Mesh1P->GetRelativeTransform();
	FVector origin = currentTransform.GetLocation();
	origin.Z += (DefaultBaseEyeHeight - CrouchedEyeHeight);// HalfHeightAdjust; //This is different than the one passed in in OnStartCrouch...
	FRotator rotation = currentTransform.GetRotation().Rotator();
	Mesh1P->SetRelativeLocationAndRotation(origin, rotation, false, nullptr, ETeleportType::TeleportPhysics);
}

void ANimModCharacter::OnCrouch()
{
	ANimModPlayerController* MyPC = Cast<ANimModPlayerController>(Controller);
	if (MyPC && !MyPC->IsFrozen() && CanCrouch())
		Crouch();
}

void ANimModCharacter::OnUnCrouch()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp && MoveComp->IsCrouching())
		UnCrouch();
}

bool ANimModCharacter::IsCrouched() const
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
		return MoveComp->IsCrouching();

	return bIsCrouched;
}

bool ANimModCharacter::IsMoving() const
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		FVector velocity = GetVelocity();
		return MoveComp->IsMovingOnGround() && (velocity.X > 0 || velocity.Y > 0);
	}

	return false;
}

bool ANimModCharacter::IsJumping() const
{
	if (!GetCharacterMovement())
		return false;

	return GetVelocity().Z > 0;
}

bool ANimModCharacter::IsFalling() const
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
		MoveComp->IsFalling();

	return false;
}

bool ANimModCharacter::IsInAir() const
{
	return IsFalling() || IsJumping();
}

bool ANimModCharacter::IsRunning() const
{
	if (!GetCharacterMovement())
	{
		return false;
	}

	return (bWantsToRun || bWantsToRunToggled) && !GetVelocity().IsZero() && (GetVelocity().GetSafeNormal2D() | GetActorRotation().Vector()) > -0.1;
}

void ANimModCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bWantsToRunToggled && !IsRunning())
	{
		SetRunning(false, false);
	}
	ANimModPlayerController* MyPC = Cast<ANimModPlayerController>(Controller);
	if (MyPC)
	{
		if (MyPC->HasHealthRegen())
		{
			if (this->Health < this->GetMaxHealth())
			{
				this->Health += 5 * DeltaSeconds;
				if (Health > this->GetMaxHealth())
				{
					Health = this->GetMaxHealth();
				}
			}
		}

		if (!bWantsToFire)
		{
			if (PreviousViewPunchStep != FVector2D::ZeroVector /*&& CurrentViewPunchLERPCount < 10*/)
			{
				FVector2D nextViewPunchStep = FMath::Vector2DInterpConstantTo(PreviousViewPunchStep, FVector2D::ZeroVector, DeltaSeconds, 5.0f);
				FVector2D deltaStep = (PreviousViewPunchStep - nextViewPunchStep);
				AddViewPunch(-deltaStep);
				PreviousViewPunchStep = nextViewPunchStep;
			}
		}
		
		if (MyPC->GetPlayerTeamNumber() == ENimModTeam::ASSASSINS)
		{
			//FName parameterName = TEXT("NimMod_Opacity");
			//for (auto material : GetPawnMesh()->GetMaterials())
			//{
			//	if (material)
			//	{
			//		float TestValue; //not used but needed for GetScalarParameterValue call
			//		if (material->GetScalarParameterValue(parameterName, TestValue))
			//		{
			//			UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(material);
			//			if (!DynamicMaterial) //Is it already a UMaterialInstanceDynamic (ie we used it last tick)
			//			{
			//				continue;
			//			}
			//			DynamicMaterial->SetScalarParameterValue(parameterName, 0.25);
			//			break;
			//		}
			//	}
			//}
		}
	}

	if (LowHealthSound && GEngine->UseSound())
	{
		if ((this->Health > 0 && this->Health < this->GetMaxHealth() * LowHealthPercentage) && (!LowHealthWarningPlayer || !LowHealthWarningPlayer->IsPlaying()))
		{
			LowHealthWarningPlayer = UGameplayStatics::PlaySoundAttached(LowHealthSound, GetRootComponent(),
				NAME_None, FVector(ForceInit), EAttachLocation::KeepRelativeOffset, true);
			LowHealthWarningPlayer->SetVolumeMultiplier(0.0f);
		}
		else if ((this->Health > this->GetMaxHealth() * LowHealthPercentage || this->Health < 0) && LowHealthWarningPlayer && LowHealthWarningPlayer->IsPlaying())
		{
			LowHealthWarningPlayer->Stop();
		}
		if (LowHealthWarningPlayer && LowHealthWarningPlayer->IsPlaying())
		{
			const float MinVolume = 0.3f;
			const float VolumeMultiplier = (1.0f - (this->Health / (this->GetMaxHealth() * LowHealthPercentage)));
			LowHealthWarningPlayer->SetVolumeMultiplier(MinVolume + (1.0f - MinVolume) * VolumeMultiplier);
		}
	}
}

void ANimModCharacter::OnStartJump()
{
	ANimModPlayerController* MyPC = Cast<ANimModPlayerController>(Controller);
	if (MyPC && !MyPC->IsFrozen() && MyPC->IsGameInputAllowed())
	{
		bPressedJump = true;
	}
}

void ANimModCharacter::OnStopJump()
{
	bPressedJump = false;
}

//////////////////////////////////////////////////////////////////////////
// Replication

void ANimModCharacter::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	// Only replicate this property for a short duration after it changes so join in progress players don't get spammed with fx when joining late
	DOREPLIFETIME_ACTIVE_OVERRIDE(ANimModCharacter, LastTakeHitInfo, GetWorld() && GetWorld()->GetTimeSeconds() < LastTakeHitTimeTimeout);
}

void ANimModCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// only to local owner: weapon change requests are locally instigated, other clients don't need it
	DOREPLIFETIME_CONDITION(ANimModCharacter, Inventory, COND_OwnerOnly);

	// everyone except local owner: flag change is locally instigated
	DOREPLIFETIME_CONDITION(ANimModCharacter, bIsTargeting, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ANimModCharacter, bWantsToRun, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(ANimModCharacter, LastTakeHitInfo, COND_Custom);

	// everyone
	DOREPLIFETIME(ANimModCharacter, CurrentWeapon);
	DOREPLIFETIME(ANimModCharacter, Health);
}

ANimModWeapon* ANimModCharacter::GetWeapon() const
{
	return CurrentWeapon;
}

int32 ANimModCharacter::GetInventoryCount() const
{
	return Inventory.Num();
}

//ANimModWeapon* ANimModCharacter::GetInventoryWeapon(int32 index) const
//{
//	return Inventory[index];
//}

USkeletalMeshComponent* ANimModCharacter::GetPawnMesh() const
{
	return IsFirstPerson() ? Mesh1P : GetMesh();
}

USkeletalMeshComponent* ANimModCharacter::GetSpecifcPawnMesh(bool WantFirstPerson) const
{
	return WantFirstPerson == true ? Mesh1P : GetMesh();
}

FName ANimModCharacter::GetWeaponAttachPoint() const
{
	return WeaponAttachPoint;
}

FName ANimModCharacter::GetWeaponAttachPoint1P() const
{
	return WeaponAttachPoint1P;
}

float ANimModCharacter::GetTargetingSpeedModifier() const
{
	return TargetingSpeedModifier;
}

bool ANimModCharacter::IsTargeting() const
{
	return bIsTargeting;
}

float ANimModCharacter::GetRunningSpeedModifier() const
{
	return RunningSpeedModifier;
}

bool ANimModCharacter::IsFiring() const
{
	return bWantsToFire;
};

bool ANimModCharacter::IsFirstPerson() const
{
	return IsAlive() && Controller && Controller->IsLocalPlayerController();
}

int32 ANimModCharacter::GetMaxHealth() const
{
	return GetClass()->GetDefaultObject<ANimModCharacter>()->Health;
}

bool ANimModCharacter::IsAlive() const
{
	return Health > 0;
}

float ANimModCharacter::GetLowHealthPercentage() const
{
	return LowHealthPercentage;
}

void ANimModCharacter::UpdateTeamColorsAllMIDs()
{
	for (int32 i = 0; i < MeshMIDs.Num(); ++i)
	{
		UpdateTeamColors(MeshMIDs[i]);
	}
}

//UCameraComponent* ANimModCharacter::GetCameraComponent()
//{
//
//}
