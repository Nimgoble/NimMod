#include "NimMod.h"
#include "RoundManager_ForceRespawn.h"


ARoundManager_ForceRespawn::ARoundManager_ForceRespawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RoundCount = 0;
}

//void ARoundManager_ForceRespawn::BeginPlay()
//{
//	//InitializeRoundObjects();
//}

void ARoundManager_ForceRespawn::RestartRound()
{
	if (ISSERVER)
	{
		HandleRestartRound();
	}
	//TArray<AActor *> destroyActors;
	//TArray<AActor *> respawnActors;
	//for (FActorIterator It(GetWorld()); It; ++It)
	//{
	//	AActor* A = *It;
	//	if (A && A != this && !A->IsA<AController>() && ShouldReset(A))
	//	{
	//		if (currentRoundActors.Contains(A))
	//		{
	//			respawnActors.Add(A);
	//			currentRoundActors.Remove(A);
	//		}
	//		else
	//			destroyActors.Add(A);

	//		//We will, eventually, be destroying ALL of the actors that get this far.
	//	}
	//}

	////Whatever is left is something that is pending being deleted, or is deleted.
	////Respawn what we can.
	//for (AActor *A : currentRoundActors)
	//{
	//	if (A == nullptr)
	//	{
	//		destroyActors.Add(A);
	//		continue;
	//	}

	//	if (ShouldReset(A))
	//	{
	//		respawnActors.Add(A);
	//		//destroyActors.Add(A);
	//	}
	//}

	////Respawn the ones that we do want.
	//while (respawnActors.Num() > 0)
	//{
	//	AActor *A = respawnActors[0];
	//	if (A == nullptr)
	//	{
	//		respawnActors.RemoveAt(0);
	//		continue;
	//	}

	//	if (ISSERVER)
	//	{
	//		//Add a new actor as a copy of the 'reloaded' one, since the reloaded one is disappearing.
	//		FActorSpawnParameters spawnParams;
	//		spawnParams.Template = A;

	//		FString templateName = A->GetName();
	//		FString currentNameTail = FString::Printf(TEXT("_R%d"), RoundCount);
	//		FString newNameTail = FString::Printf(TEXT("_R%d"), (RoundCount + 1));

	//		if (templateName.Contains(currentNameTail))
	//			templateName.ReplaceInline(*currentNameTail, *newNameTail);
	//		else
	//			templateName += newNameTail;

	//		spawnParams.Name = FName(*(templateName));

	//		/*spawnParams.bNoFail = true;
	//		spawnParams.bNoCollisionFail = true;
	//		spawnParams.Name = NAME_None;*/
	//		//A->SetReplicates(true);
	//		FVector orig = A->GetActorLocation();
	//		FRotator rot = A->GetActorRotation();
	//		AActor *newActor = GetWorld()->SpawnActor
	//		(
	//			A->GetClass(),
	//			&orig,
	//			&rot,
	//			spawnParams
	//		);
	//		if (newActor == nullptr)
	//			continue;

	//		//newActor->SetOwner(A->GetOwner());

	//		currentRoundActors.Add(newActor);
	//	}

	//	if (currentRoundActors.Contains(A))
	//		currentRoundActors.Remove(A);

	//	//Remove the old one
	//	destroyActors.Add(A);
	//	respawnActors.RemoveAt(0);
	//}

	////Destroy all of the actors we don't want.
	//while (destroyActors.Num() > 0)
	//{
	//	AActor *actor = destroyActors[0];
	//	destroyActors.RemoveAt(0);
	//	//This can (theoretically) happen if we added something that has already been deleted.
	//	if (actor == nullptr)
	//		continue;

	//	actor->Destroy();
	//	actor = nullptr;
	//}

	//++RoundCount;
}

void ARoundManager_ForceRespawn::InitializeRoundObjects()
{
	/*for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* A = *It;
		if (A && A != this && !A->IsA<AController>() && ShouldReset(A))
		{
			currentRoundActors.Add(A);
		}
	}*/
}

