#include "NimModTypes.generated.h"
#pragma once

namespace ENimModMatchState
{
	enum Type
	{
		Warmup,
		Playing,
		Won,
		Lost,
	};
}

namespace ENimModCrosshairDirection
{
	enum Type
	{
		Left = 0,
		Right = 1,
		Top = 2,
		Bottom = 3,
		Center = 4
	};
}

namespace ENimModHudPosition
{
	enum Type
	{
		Left = 0,
		FrontLeft = 1,
		Front = 2,
		FrontRight = 3,
		Right = 4,
		BackRight = 5,
		Back = 6,
		BackLeft = 7,
	};
}

/** keep in sync with NimModImpactEffect */
UENUM()
namespace ENimModPhysMaterialType
{
	enum Type
	{
		Unknown,
		Concrete,
		Dirt,
		Water,
		Metal,
		Wood,
		Grass,
		Glass,
		Flesh,
	};
}

namespace ENimModDialogType
{
	enum Type
	{
		None,
		Generic,
		ControllerDisconnected
	};
}

UENUM(BlueprintType)
enum class ENimModTeam : uint8
{
	INVALID UMETA(DisplayName = "Invalid"),
	ASSASSINS UMETA(DisplayName = "Assassins"),
	BODYGUARDS UMETA(DisplayName = "Bodyguards"),
	VIP UMETA(DisplayName = "VIP"),
	SPECTATORS UMETA(DisplayName = "Spectators")
};

USTRUCT(blueprintable, meta = (DisplayName = "NimMod Team Info"))
struct FNimModTeamInfo
{
	GENERATED_USTRUCT_BODY()

	FNimModTeamInfo()
	{
		TeamNumber = ENimModTeam::INVALID;
		TeamName = "INVALID";
		TeamColor = FLinearColor(0.0f, 0.0f, 0.0f, 1.0f);
		MaxSpeed = 0.0f;
	}

	FNimModTeamInfo(ENimModTeam InTeamNumber, FString InTeamName, FLinearColor InTeamColor, float InMaxSpeed)
	{
		TeamNumber = InTeamNumber;
		TeamName = InTeamName;
		TeamColor = InTeamColor;
		MaxSpeed = InMaxSpeed;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod|Team")
	ENimModTeam TeamNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod|Team")
	FString TeamName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod|Team")
	FLinearColor TeamColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod|Team")
	float MaxSpeed;

	//TODO: Implement later.
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod|Team")
	UNimModHUDLayoutWidget *HUDLayout;*/
};

//USTRUCT(blueprintable, meta = (DisplayName = "NimMod Team"))
//struct FNimModTeam
//{
//	GENERATED_USTRUCT_BODY()
//
//	FNimModTeam()
//	{
//	}
//
//	FNimModTeam(FNimModTeamInfo InTeamInfo) : TeamInfo(InTeamInfo)
//	{
//	}
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod|Team")
//	FNimModTeamInfo TeamInfo;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod|Team")
//	int32 TeamScore;
//};

UENUM(BlueprintType)
enum class ENimModHUDMessageType : uint8
{
	PublicChat UMETA(DisplayName = "Public Chat"),
	TeamChat UMETA(DisplayName = "Team Chat"),
	ServerMessage UMETA(DisplayName = "Server Message"),
	MarqueeMessage UMETA(DisplayName = "Marquee Message")
};

USTRUCT(blueprintable, meta = (DisplayName = "NimMod HUD Message"))
struct FNimModHUDMessage
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod|HUD")
	ENimModHUDMessageType MessageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod|HUD")
	class ANimModPlayerState *Sender;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod|HUD")
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NimMod|HUD")
	bool TeamOnly;

	FNimModHUDMessage()
	{
		Message = TEXT("");
		TeamOnly = false;
		Sender = nullptr;
		MessageType = ENimModHUDMessageType::MarqueeMessage;
	}
};

USTRUCT()
struct FProjectileWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** projectile class */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	TSubclassOf<class ANimModProjectile> ProjectileClass;

	/** life time */
	UPROPERTY(EditDefaultsOnly, Category = Projectile)
	float ProjectileLife;

	/** damage at impact point */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	int32 ExplosionDamage;

	/** radius of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	float ExplosionRadius;

	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category = WeaponStat)
	TSubclassOf<UDamageType> DamageType;

	/** defaults */
	FProjectileWeaponData()
	{
		ProjectileClass = NULL;
		ProjectileLife = 10.0f;
		ExplosionDamage = 100;
		ExplosionRadius = 300.0f;
		DamageType = UDamageType::StaticClass();
	}
};

#define NIMMOD_SURFACE_Default		SurfaceType_Default
#define NIMMOD_SURFACE_Concrete	SurfaceType1
#define NIMMOD_SURFACE_Dirt		SurfaceType2
#define NIMMOD_SURFACE_Water		SurfaceType3
#define NIMMOD_SURFACE_Metal		SurfaceType4
#define NIMMOD_SURFACE_Wood		SurfaceType5
#define NIMMOD_SURFACE_Grass		SurfaceType6
#define NIMMOD_SURFACE_Glass		SurfaceType7
#define NIMMOD_SURFACE_Flesh		SurfaceType8