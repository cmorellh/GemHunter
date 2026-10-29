// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#include "GemCollectionSubsystem.h"

#include "GemHunterCore.h"
#include "GemHunterDataSettings.h"
#include "Kismet/GameplayStatics.h"

void UGemCollectionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!LoadCollection())
	{
		// No save yet: start an empty vault rather than leaving SaveData null,
		// so every accessor below can assume it exists.
		SaveData = NewObject<UGemHunterSaveGame>(this);
	}
}

FString UGemCollectionSubsystem::GetSaveSlotName() const
{
	const UGemHunterDataSettings* Settings = GetDefault<UGemHunterDataSettings>();
	return (Settings && !Settings->SaveSlotName.IsEmpty()) ? Settings->SaveSlotName : TEXT("GemHunterCollection");
}

void UGemCollectionSubsystem::RecordFind(const FFoundSpecimen& Specimen)
{
	if (!SaveData)
	{
		SaveData = NewObject<UGemHunterSaveGame>(this);
	}

	if (Specimen.GemId.IsNone())
	{
		UE_LOG(LogGemHunterCore, Warning, TEXT("RecordFind ignored a specimen with no GemId."));
		return;
	}

	FFoundSpecimen Stored = Specimen;
	if (Stored.FoundAt == FDateTime())
	{
		Stored.FoundAt = FDateTime::UtcNow();
	}

	SaveData->FoundSpecimens.Add(Stored);
	OnSpecimenFound.Broadcast(Stored);

	// Phase 0 stub: autosaving on every find is fine while the vault is tiny.
	// Revisit once specimen counts grow or quality grading lands.
	SaveCollection();
}

bool UGemCollectionSubsystem::HasFound(FName GemId) const
{
	if (!SaveData)
	{
		return false;
	}

	return SaveData->FoundSpecimens.ContainsByPredicate(
		[GemId](const FFoundSpecimen& Specimen) { return Specimen.GemId == GemId; });
}

int32 UGemCollectionSubsystem::GetFoundCount(FName GemId) const
{
	if (!SaveData)
	{
		return 0;
	}

	int32 Count = 0;
	for (const FFoundSpecimen& Specimen : SaveData->FoundSpecimens)
	{
		if (Specimen.GemId == GemId)
		{
			++Count;
		}
	}
	return Count;
}

int32 UGemCollectionSubsystem::GetUniqueGemCount() const
{
	if (!SaveData)
	{
		return 0;
	}

	TSet<FName> Unique;
	for (const FFoundSpecimen& Specimen : SaveData->FoundSpecimens)
	{
		Unique.Add(Specimen.GemId);
	}
	return Unique.Num();
}

bool UGemCollectionSubsystem::GetBestSpecimen(FName GemId, FFoundSpecimen& OutSpecimen) const
{
	if (!SaveData)
	{
		return false;
	}

	const FFoundSpecimen* Best = nullptr;
	for (const FFoundSpecimen& Specimen : SaveData->FoundSpecimens)
	{
		if (Specimen.GemId != GemId)
		{
			continue;
		}

		// Quality first, carat as the tiebreak: a cleaner small stone beats a
		// big cloudy one on a museum shelf.
		if (!Best
			|| Specimen.Quality > Best->Quality
			|| (Specimen.Quality == Best->Quality && Specimen.CaratWeight > Best->CaratWeight))
		{
			Best = &Specimen;
		}
	}

	if (!Best)
	{
		return false;
	}

	OutSpecimen = *Best;
	return true;
}

TArray<FFoundSpecimen> UGemCollectionSubsystem::GetAllSpecimens() const
{
	return SaveData ? SaveData->FoundSpecimens : TArray<FFoundSpecimen>();
}

bool UGemCollectionSubsystem::SaveCollection()
{
	if (!SaveData)
	{
		return false;
	}

	return UGameplayStatics::SaveGameToSlot(SaveData, GetSaveSlotName(), 0);
}

bool UGemCollectionSubsystem::LoadCollection()
{
	const FString SlotName = GetSaveSlotName();
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		return false;
	}

	UGemHunterSaveGame* Loaded = Cast<UGemHunterSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!Loaded)
	{
		UE_LOG(LogGemHunterCore, Warning, TEXT("Save slot '%s' exists but could not be read as a UGemHunterSaveGame."), *SlotName);
		return false;
	}

	SaveData = Loaded;
	UE_LOG(LogGemHunterCore, Log, TEXT("Vault loaded: %d specimens, %d unique gems."),
		SaveData->FoundSpecimens.Num(), GetUniqueGemCount());
	return true;
}
