// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#pragma once

#include "CoreMinimal.h"
#include "GemHunterTypes.generated.h"

/**
 * Rarity ladder. These are a game-design proposal, not market data
 * (see docs/Game Design Plan.md, "Launch gem roster").
 */
UENUM(BlueprintType)
enum class EGemRarity : uint8
{
	Common			UMETA(DisplayName = "Common"),
	Uncommon		UMETA(DisplayName = "Uncommon"),
	Rare			UMETA(DisplayName = "Rare"),
	Legendary		UMETA(DisplayName = "Legendary")
};

/**
 * Tool progression. Tiers gate both rock hardness (Mohs) and depth,
 * which is what makes "tools gate depth" a data rule rather than code.
 */
UENUM(BlueprintType)
enum class EToolTier : uint8
{
	// Phase 0: panning and sieving river gravels at Ratnapura.
	Hands			UMETA(DisplayName = "T0 - Hands & Sieve"),
	HandTool		UMETA(DisplayName = "T1 - Pick & Shovel"),
	PowerTool		UMETA(DisplayName = "T2 - Power Drill"),
	Rig				UMETA(DisplayName = "T3 - Drill Rig"),
	Experimental	UMETA(DisplayName = "T4 - Experimental")
};

/** How much hand-holding a mission gives (see docs/Decisions.md D1). */
UENUM(BlueprintType)
enum class EMissionHintLevel : uint8
{
	Guided			UMETA(DisplayName = "Guided"),
	Hinted			UMETA(DisplayName = "Hinted"),
	Unguided		UMETA(DisplayName = "Unguided")
};

/**
 * One entry in a locality's "what can be found here" table.
 * Weight is relative within the locality, not a probability.
 */
USTRUCT(BlueprintType)
struct GEMHUNTERCORE_API FGemWeight
{
	GENERATED_BODY()

	/** Matches FGemRow::Id. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem Hunter")
	FName GemId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem Hunter")
	float Weight = 1.0f;
};
