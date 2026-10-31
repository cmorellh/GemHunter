// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#include "GemYieldLibrary.h"

#include "GemDatabaseSubsystem.h"
#include "GemHunterCore.h"

#define LOCTEXT_NAMESPACE "GemHunterYield"

bool UGemYieldLibrary::RollGemId(const FLocalityRow& Locality, FRandomStream& Rng, FName& OutGemId)
{
	OutGemId = NAME_None;

	// Kashmir and sites like it: mined out, so the default yield is nothing.
	// A rare-event override belongs in the mission layer, not here.
	if (Locality.bIsDepleted || Locality.FoundGems.Num() == 0)
	{
		return false;
	}

	float TotalWeight = 0.0f;
	for (const FGemWeight& Entry : Locality.FoundGems)
	{
		TotalWeight += FMath::Max(0.0f, Entry.Weight);
	}

	if (TotalWeight <= 0.0f)
	{
		UE_LOG(LogGemHunterCore, Warning,
			TEXT("Locality '%s' has a gem table whose weights sum to zero."), *Locality.Id.ToString());
		return false;
	}

	// Standard weighted pick: walk the table subtracting weights until the roll runs out.
	float Roll = Rng.FRandRange(0.0f, TotalWeight);
	for (const FGemWeight& Entry : Locality.FoundGems)
	{
		const float Weight = FMath::Max(0.0f, Entry.Weight);
		if (Roll < Weight)
		{
			OutGemId = Entry.GemId;
			return !OutGemId.IsNone();
		}
		Roll -= Weight;
	}

	// Floating-point drift can leave the roll just past the end; the last entry is correct.
	OutGemId = Locality.FoundGems.Last().GemId;
	return !OutGemId.IsNone();
}

float UGemYieldLibrary::GetQualitySkew(EGemRarity Rarity)
{
	// Exponent on a 0..1 roll. Higher means more of the distribution sits near zero,
	// so top-grade stones get rarer as the tier climbs.
	switch (Rarity)
	{
	case EGemRarity::Common:    return 1.2f;
	case EGemRarity::Uncommon:  return 1.8f;
	case EGemRarity::Rare:      return 2.6f;
	case EGemRarity::Legendary: return 4.0f;
	default:                    return 1.0f;
	}
}

float UGemYieldLibrary::GetBaseCarat(EGemRarity Rarity)
{
	// Rarer gems are found smaller. A 5 ct ruby is a museum piece; a 5 ct
	// amethyst is gravel.
	switch (Rarity)
	{
	case EGemRarity::Common:    return 6.0f;
	case EGemRarity::Uncommon:  return 3.0f;
	case EGemRarity::Rare:      return 1.5f;
	case EGemRarity::Legendary: return 0.8f;
	default:                    return 1.0f;
	}
}

FFoundSpecimen UGemYieldLibrary::MakeSpecimen(const FGemRow& Gem, FName LocalityId, FRandomStream& Rng)
{
	FFoundSpecimen Specimen;
	Specimen.GemId = Gem.Id;
	Specimen.LocalityId = LocalityId;
	Specimen.FoundAt = FDateTime::UtcNow();

	// Origin starts unknown. The late-game lab tool is what confirms it and
	// doubles the specimen's value, so it must not be free at pickup.
	Specimen.bOriginIdentified = false;

	Specimen.Quality = FMath::Pow(Rng.FRand(), GetQualitySkew(Gem.Rarity));

	// Spread carat around the tier's base, with quality nudging size up: the same
	// crystal that grew cleanly tends to have grown bigger.
	const float Spread = 0.35f + (Rng.FRand() * 1.3f);
	Specimen.CaratWeight = GetBaseCarat(Gem.Rarity) * Spread * (0.7f + (Specimen.Quality * 0.6f));
	Specimen.CaratWeight = FMath::Max(0.05f, FMath::RoundToFloat(Specimen.CaratWeight * 100.0f) / 100.0f);

	return Specimen;
}

float UGemYieldLibrary::GetScoopFindChance(const FRockLayerRow& Layer, EGemRarity Rarity)
{
	// Whether a layer holds gems at all is data, not a side effect of how soft it
	// is: barren topsoil is soft, and so is the gem-rich illam beneath it. Keying
	// this on hardness alone made overburden out-yield the gravels, which is
	// backwards.
	if (Layer.Fertility <= 0.0f)
	{
		return 0.0f;
	}

	// Hardness still matters, but only as dig difficulty: a scoop of soft ground
	// moves more material than a scoop of hard rock. Written out rather than using
	// FMath::GetMappedRangeValueClamped, whose FVector2f/FVector2D overloads do not
	// deduce cleanly from float literals.
	const float HardnessAlpha = FMath::Clamp((Layer.Hardness - 2.0f) / (8.0f - 2.0f), 0.0f, 1.0f);
	const float DigFactor = FMath::Lerp(0.55f, 0.20f, HardnessAlpha);
	const float LayerFactor = DigFactor * FMath::Clamp(Layer.Fertility, 0.0f, 1.0f);

	float RarityFactor = 1.0f;
	switch (Rarity)
	{
	case EGemRarity::Common:    RarityFactor = 1.0f;  break;
	case EGemRarity::Uncommon:  RarityFactor = 0.6f;  break;
	case EGemRarity::Rare:      RarityFactor = 0.25f; break;
	case EGemRarity::Legendary: RarityFactor = 0.05f; break;
	}

	return FMath::Clamp(LayerFactor * RarityFactor, 0.0f, 1.0f);
}

FName UGemYieldLibrary::FindLayerAtDepth(
	const UGemDatabaseSubsystem* Database,
	const TArray<FName>& LayerIds,
	float DepthMeters)
{
	if (!Database)
	{
		return NAME_None;
	}

	for (const FName& LayerId : LayerIds)
	{
		const FRockLayerRow* Layer = Database->FindRockLayer(LayerId);
		if (Layer && DepthMeters >= Layer->DepthMin && DepthMeters <= Layer->DepthMax)
		{
			return LayerId;
		}
	}

	return NAME_None;
}

bool UGemYieldLibrary::RollScoop(
	const UGemDatabaseSubsystem* Database,
	FName LocalityId,
	FName LayerId,
	EToolTier EquippedTier,
	FRandomStream& Rng,
	FFoundSpecimen& OutSpecimen,
	FText& OutFailReason)
{
	OutSpecimen = FFoundSpecimen();
	OutFailReason = FText::GetEmpty();

	if (!Database || !Database->IsLoaded())
	{
		OutFailReason = LOCTEXT("NoDatabase", "The gem database is not loaded.");
		return false;
	}

	const FLocalityRow* Locality = Database->FindLocality(LocalityId);
	if (!Locality)
	{
		OutFailReason = FText::Format(LOCTEXT("NoLocality", "No locality row for '{0}'."),
			FText::FromName(LocalityId));
		return false;
	}

	if (Locality->bIsDepleted)
	{
		OutFailReason = FText::Format(LOCTEXT("Depleted", "{0} is mined out."), Locality->Name);
		return false;
	}

	if (LayerId.IsNone())
	{
		// The backend reported no geology at that point: outside the dig volume,
		// or past the deepest band it knows about.
		OutFailReason = LOCTEXT("NoLayer", "Nothing to work here.");
		return false;
	}

	// Both gates, and they are the same ones the database already owns, so the
	// rule lives in exactly one place.
	if (!Database->CanToolTierWorkLayer(EquippedTier, LayerId))
	{
		const FRockLayerRow* Layer = Database->FindRockLayer(LayerId);
		OutFailReason = FText::Format(LOCTEXT("ToolTooWeak", "Your tool cannot work {0}."),
			Layer ? Layer->Name : FText::FromName(LayerId));
		return false;
	}

	FName GemId;
	if (!RollGemId(*Locality, Rng, GemId))
	{
		OutFailReason = FText::Format(LOCTEXT("NoGemTable", "{0} has nothing to find."), Locality->Name);
		return false;
	}

	const FGemRow* Gem = Database->FindGem(GemId);
	if (!Gem)
	{
		// A locality referenced a gem id that is not in Gems.csv. That is a data
		// bug worth shouting about, not a quiet miss.
		UE_LOG(LogGemHunterCore, Error,
			TEXT("Locality '%s' lists gem '%s', which has no row in the gem table."),
			*LocalityId.ToString(), *GemId.ToString());
		OutFailReason = LOCTEXT("BadGemRef", "Data error: unknown gem id.");
		return false;
	}

	// The weighted pick chose *which* gem; this decides whether the scoop held one
	// at all. Doing it in this order keeps rarity meaningful: a legendary site
	// still mostly yields nothing rather than yielding commons instead.
	const FRockLayerRow* Layer = Database->FindRockLayer(LayerId);
	const float FindChance = Layer ? GetScoopFindChance(*Layer, Gem->Rarity) : 0.0f;
	if (Rng.FRand() > FindChance)
	{
		OutFailReason = LOCTEXT("EmptyScoop", "Nothing in this scoop.");
		return false;
	}

	OutSpecimen = MakeSpecimen(*Gem, LocalityId, Rng);
	return true;
}

#undef LOCTEXT_NAMESPACE
