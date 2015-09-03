#pragma once

#include "RoundManager.h"
#include "RoundManager_SeemlessLevelTravel.generated.h"

UCLASS(minimalapi)
class ARoundManager_SeemlessLevelTravel : public ARoundManager
{
	GENERATED_BODY()

public:
	ARoundManager_SeemlessLevelTravel(const FObjectInitializer& ObjectInitializer);
	virtual void RestartRound() override;

private:
	UFUNCTION(Reliable, NetMulticast)
	void InternalRestartRound();
};