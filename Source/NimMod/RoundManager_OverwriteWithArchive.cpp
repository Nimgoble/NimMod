#include "NimMod.h"
#include "RoundManager_OverwriteWithArchive.h"


ARoundManager_OverwriteWithArchive::ARoundManager_OverwriteWithArchive(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RoundCount = 0;
}

ARoundManager_OverwriteWithArchive::~ARoundManager_OverwriteWithArchive()
{
	ReloadArchiveObjectType::TIterator iter = reloadObjectArchives.CreateIterator();
	for (iter; iter; ++iter)
	{
		FReloadObjectArc *reloadArchive = (iter->Value);
		if (reloadArchive == nullptr)
			continue;

		reloadArchive->Close();
		reloadArchive->Flush();
		delete reloadArchive;
		reloadArchive = nullptr;
	}
}

void ARoundManager_OverwriteWithArchive::BeginPlay()
{
	InitializeRoundObjects();
}

void ARoundManager_OverwriteWithArchive::InitializeRoundObjects()
{
	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* A = *It;
		if (A && A != this && !A->IsA<AController>() && ShouldReset(A))
		{

			FReloadObjectArc *reloadArchive = new FReloadObjectArc();
			reloadArchive->ActivateWriter();
			reloadArchive->SerializeObject(A);
			reloadObjectArchives.Add(A, reloadArchive);
			reloadArchive->Reset();
			//reloadArchive << A;
		}
	}
}

void ARoundManager_OverwriteWithArchive::RestartRound()
{
	TArray<AActor *> destroyActors;
	TArray<AActor *> respawnActors;
	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* A = *It;
		if (A && A != this && !A->IsA<AController>() && ShouldReset(A))
		{
			FReloadObjectArc **reloadArchivePtr = reloadObjectArchives.Find(A);
			if (reloadArchivePtr == nullptr)
			{
				destroyActors.Add(A);
				continue;
			}
			FReloadObjectArc *reloadArchive = *reloadArchivePtr;
			if (reloadArchive == nullptr)
			{
				destroyActors.Add(A);
				continue;
			}
			reloadArchive->Reset();

			if (reloadArchive->IsSaving())
				reloadArchive->ActivateReader();
			//A->Reset();
			//A->RerunConstructionScripts();

			//Shotgun approach:
			/*A->ClearInstanceComponents(true);
			A->Reset();
			A->DestroyConstructedComponents();*/

			reloadArchive->SerializeObject(A);
		}
	}

	//Whatever is left is something that is pending being deleted, or is deleted.
	//Respawn what we can.
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

	//	FReloadObjectArc *reloadArchive = *reloadObjectArchives.Find(A);
	//	/*A->Reset();*/

	//	//Add a new actor as a copy of the 'reloaded' one, since the reloaded one is disappearing.

	//	/*FActorSpawnParameters spawnParams;
	//	spawnParams.Template = A;*/

	//	/*spawnParams.bNoFail = true;
	//	spawnParams.bNoCollisionFail = true;
	//	spawnParams.Name = NAME_None;*/
	//	//A->SetReplicates(true);
	//	/*FVector orig = A->GetActorLocation();
	//	FRotator rot = A->GetActorRotation();
	//	AActor *newActor = GetWorld()->SpawnActor
	//	(
	//	A->GetClass(),
	//	&orig,
	//	&rot,
	//	spawnParams
	//	);
	//	if (newActor == nullptr)
	//	continue;*/

	//	//newActor->SetOwner(A->GetOwner());

	//	//Add the new one
	//	reloadObjectArchives.Add(newActor, reloadArchive);


	//	//Remove the old one
	//	reloadObjectArchives.Remove(A);
	//	destroyActors.Add(A);
	//	respawnActors.RemoveAt(0);
	//}

	//Destroy all of the actors we don't want.
	while (destroyActors.Num() > 0)
	{
		AActor *actor = destroyActors[0];
		destroyActors.RemoveAt(0);
		//This can (theoretically) happen if we added something that has already been deleted.
		if (actor == nullptr)
			continue;

		actor->Destroy();
		actor = nullptr;
	}
}