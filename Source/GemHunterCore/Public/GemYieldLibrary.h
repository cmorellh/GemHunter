// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GemDataRows.h"
#include "GemHunterSaveGame.h"
#include "GemYieldLibrary.generated.h"

class UGemDatabaseSubsystem;

/**
 * What a scoop of ground yields.
 *
 * Stateless on purpose: every rule here is a pure function of a DataTable row
 * plus a seed, so a locality's output is a data question and the same seed always
 * gives the same result. That keeps "new gem = new CSV row" true for yields too,
 * and makes the progression simulable without running the game.
 */
UCLASS()
class GEMHUNTERCORE_API UGemYieldLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Pick one gem from a locality's weighted FoundGems table.
	 *
	 * Returns false, and leaves OutGemId as NAME_None, when the site is depleted or
	 * has no gem table — a Kashmir-style site pays out nothing by default.
	 */
	static bool RollGemId(const FLocalityRow& Locality, FRandomStream& Rng, FName& OutGemId);

	/**
	 * Build a specimen of a gem found at a locality.
	 *
	 * Rarity skews both size and grade: a legendary stone is usually small and
	 * flawed, which is what makes a good one worth re-digging a site for.
	 */
	static FFoundSpecimen MakeSpecimen(const FGemRow& Gem, FName LocalityId, FRandomStream& Rng);

	/**
	 * The full yield for one scoop, or false if the scoop produced nothing.
	 *
	 * Takes the layer the scoop hit rather than a depth, because that is what
	 * IDiggableTerrain reports. Gameplay therefore never needs to know how a
	 * backend maps position to geology, which is what lets the sieve bed and a
	 * voxel volume be interchangeable here.
	 */
	static bool RollScoop(
		const UGemDatabaseSubsystem* Database,
		FName LocalityId,
		FName LayerId,
		EToolTier EquippedTier,
		FRandomStream& Rng,
		FFoundSpecimen& OutSpecimen,
		FText& OutFailReason);

	/** Which of LayerIds covers DepthMeters, or NAME_None if none does. */
	static FName FindLayerAtDepth(
		const UGemDatabaseSubsystem* Database,
		const TArray<FName>& LayerIds,
		float DepthMeters);

	/** 0..1 chance that a scoop of this layer contains anything at all. */
	static float GetScoopFindChance(const FRockLayerRow& Layer, EGemRarity Rarity);

private:
	/** Rarity drives how harshly the quality roll is skewed towards poor stones. */
	static float GetQualitySkew(EGemRarity Rarity);

	/** Typical carat for a rarity tier, before the random spread. */
	static float GetBaseCarat(EGemRarity Rarity);
};
