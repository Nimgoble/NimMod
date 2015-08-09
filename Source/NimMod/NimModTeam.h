#pragma once

#include "NimModTypes.h"
#include "NimModTeam.generated.h"

UCLASS(blueprintable, meta = (DisplayName = "NimMod Team"))
class ANimModTeam : public AInfo
{
	GENERATED_BODY()
	ANimModTeam(const FObjectInitializer& ObjectInitializer);
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetTeamInfo(FNimModTeamInfo InTeamInfo){ TeamInfo = InTeamInfo; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "NimMod|Team")
	FNimModTeamInfo TeamInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "NimMod|Team")
	int32 TeamScore;

	HIDE_ACTOR_TRANSFORM_FUNCTIONS();
};
