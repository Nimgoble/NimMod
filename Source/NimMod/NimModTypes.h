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

#define NIMMOD_SURFACE_Default		SurfaceType_Default
#define NIMMOD_SURFACE_Concrete	SurfaceType1
#define NIMMOD_SURFACE_Dirt		SurfaceType2
#define NIMMOD_SURFACE_Water		SurfaceType3
#define NIMMOD_SURFACE_Metal		SurfaceType4
#define NIMMOD_SURFACE_Wood		SurfaceType5
#define NIMMOD_SURFACE_Grass		SurfaceType6
#define NIMMOD_SURFACE_Glass		SurfaceType7
#define NIMMOD_SURFACE_Flesh		SurfaceType8