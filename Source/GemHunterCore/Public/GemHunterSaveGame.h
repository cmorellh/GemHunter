// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GemHunterSaveGame.generated.h"

/**
 * One specimen in the vault. Quality varies, so the same gem found again at a
 * better grade is still worth digging for, and the vault shows your best.
 */
USTRUCT(BlueprintType)
struct GEMHUNTERCORE_API FFoundSpecimen
{
	GENERATED_BODY()

	/** Matches FGemRow::Id. */
	UPROPERTY(BlueprintReadOnly, Category = "Specimen")
	FName GemId;

	/** Matches FLocalityRow::Id. Origin is half the point of the collection. */
	UPROPERTY(BlueprintReadOnly, Category = "Specimen")
	FName LocalityId;

	UPROPERTY(BlueprintReadOnly, Category = "Specimen")
	float CaratWeight = 0.0f;

	/** 0..1 composite of clarity and colour. */
	UPROPERTY(BlueprintReadOnly, Category = "Specimen")
	float Quality = 0.0f;

	/** True once the late-game lab tool has confirmed origin (doubles value). */
	UPROPERTY(BlueprintReadOnly, Category = "Specimen")
	bool bOriginIdentified = false;

	UPROPERTY(BlueprintReadOnly, Category = "Specimen")
	FDateTime FoundAt = FDateTime();
};

UCLASS(BlueprintType)
class GEMHUNTERCORE_API UGemHunterSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** Bumped whenever FFoundSpecimen changes shape, so old saves can be migrated. */
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	int32 SaveVersion = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Save")
	TArray<FFoundSpecimen> FoundSpecimens;

	/** Highest tool tier unlocked, stored as the enum's underlying value. */
	UPROPERTY(BlueprintReadOnly, Category = "Save")
	uint8 UnlockedToolTier = 0;
};
