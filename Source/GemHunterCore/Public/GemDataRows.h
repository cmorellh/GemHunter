// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GemHunterTypes.h"
#include "GemDataRows.generated.h"

/**
 * Every row struct here backs a CSV in data/ and imports as a DataTable.
 * The CSV header names must match these property names exactly, except
 * for column 0, which Unreal always consumes as the DataTable row name.
 *
 * Adding a gem or a locality is a CSV row plus a texture. Never C++.
 */

/** One gem variety. Ruby and blue sapphire are both Corundum, hence Species + Variety. */
USTRUCT(BlueprintType)
struct GEMHUNTERCORE_API FGemRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Stable lowercase slug, e.g. "ruby", "sapphire_blue". Referenced by FGemWeight::GemId. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	FText DisplayName;

	/** Mineral species, e.g. "Corundum", "Beryl", "Quartz". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	FString MineralSpecies;

	/** Variety within the species, e.g. "Ruby", "Blue Sapphire", "Amethyst". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	FString Variety;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	EGemRarity Rarity = EGemRarity::Common;

	/** Mohs hardness band. Gating compares a tool's MaxHardness against HardnessMax. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	float HardnessMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	float HardnessMax = 0.0f;

	/** Rough colour for placeholder materials and codex text, e.g. "deep red". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	FString ColorHint;

	/**
	 * Where the gem formed, in kilometres. Diamonds form 150 km+ down.
	 * Kept separate from FindDepthMeters on purpose: the gap between the two
	 * is the codex's teaching moment (see docs/Decisions.md D2).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Depth")
	float FormationDepthKm = 0.0f;

	/** Typical depth below surface the player actually finds it at, in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem|Depth")
	float FindDepthMeters = 0.0f;

	/** Codex entry, written in our own words. Never copied from GIA or any other source. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem", meta = (MultiLine = true))
	FText CodexText;

	/** Semicolon-separated citations, e.g. "USGS MRDS; Smithsonian Open Access". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem")
	FString SourceRefs;
};

/** A real place on Earth that becomes a node on the overworld map. */
USTRUCT(BlueprintType)
struct GEMHUNTERCORE_API FLocalityRow : public FTableRowBase
{
	GENERATED_BODY()

	/** Stable lowercase slug, e.g. "ratnapura". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locality")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locality")
	FText Name;

	/** Continent / Country / Region drive the overworld map hierarchy (docs/Decisions.md D1). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locality|Map")
	FString Continent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locality|Map")
	FString Country;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locality|Map")
	FString Region;

	/** WGS84 decimal degrees. 0,0 means "not filled in yet" - see Notes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locality|Map")
	float Latitude = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locality|Map")
	float Longitude = 0.0f;

	/** e.g. "Alluvial gravels", "Marble", "Amphibolite". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locality|Geology")
	FString HostRock;

	/** e.g. "Placer", "Metamorphic", "Pegmatite", "Kimberlite pipe". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locality|Geology")
	FString DepositType;

	/** Weighted gem table for this site. Weights are relative, not probabilities. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locality|Geology")
	TArray<FGemWeight> FoundGems;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locality")
	EToolTier RequiredToolTier = EToolTier::Hands;

	/** Kashmir-style site: pays out once, or only on rare events. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locality")
	bool bIsDepleted = false;

	/** Free-text TODOs and provenance, e.g. "TODO coords from Mindat". */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locality")
	FString Notes;
};

/** A depth band of rock the player digs through. */
USTRUCT(BlueprintType)
struct GEMHUNTERCORE_API FRockLayerRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock Layer")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock Layer")
	FText Name;

	/** Metres below the surface. Real depths, fictional tools (docs/Decisions.md D2). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock Layer")
	float DepthMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock Layer")
	float DepthMax = 0.0f;

	/** Mohs hardness of the host rock, not of the gems inside it. Drives how hard
	 *  the layer is to dig, and which tool tier can work it at all. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock Layer")
	float Hardness = 0.0f;

	/**
	 * How gem-bearing this layer is, 0..1. Independent of Hardness: barren
	 * overburden is soft and yields nothing, while a placer gravel is soft and
	 * yields plenty. 0 means a scoop of it never contains a gem.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock Layer")
	float Fertility = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rock Layer")
	EToolTier RequiredToolTier = EToolTier::Hands;
};

/** A tool tier. MaxHardness and MaxDepth together are the whole gate. */
USTRUCT(BlueprintType)
struct GEMHUNTERCORE_API FToolTierRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tool")
	EToolTier Tier = EToolTier::Hands;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tool")
	FText Name;

	/** Highest Mohs hardness this tier can break. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tool")
	float MaxHardness = 0.0f;

	/** Deepest reach in metres. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tool")
	float MaxDepth = 0.0f;
};

/** A mission at a site: guided first, then harder and rarer (docs/Decisions.md D1). */
USTRUCT(BlueprintType)
struct GEMHUNTERCORE_API FMissionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FName Id;

	/** Matches FLocalityRow::Id. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FName LocalityId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FText Name;

	/** 1 = first mission at a site, rising from there. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	int32 DifficultyTier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	EMissionHintLevel HintLevel = EMissionHintLevel::Guided;

	/** Gems that satisfy the objective, with relative weights for spawning. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	TArray<FGemWeight> TargetGems;

	/** Multiplier on the site's base gem density. Lower means sparser and harder. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	float GemDensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	float DepthMin = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	float DepthMax = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mission")
	FText Reward;
};
