// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GemHunterSaveGame.h"
#include "GemCollectionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpecimenFound, const FFoundSpecimen&, Specimen);

/**
 * The vault. Owns the player's found specimens and the save slot they live in.
 *
 * Phase 0 stub: recording and querying work, progression and valuation do not.
 */
UCLASS()
class GEMHUNTERCORE_API UGemCollectionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Fires on every find, including duplicates, so the HUD can react. */
	UPROPERTY(BlueprintAssignable, Category = "Gem Hunter|Collection")
	FOnSpecimenFound OnSpecimenFound;

	UFUNCTION(BlueprintCallable, Category = "Gem Hunter|Collection")
	void RecordFind(const FFoundSpecimen& Specimen);

	/** True if the gem has been found at least once, at any quality. */
	UFUNCTION(BlueprintPure, Category = "Gem Hunter|Collection")
	bool HasFound(FName GemId) const;

	UFUNCTION(BlueprintPure, Category = "Gem Hunter|Collection")
	int32 GetFoundCount(FName GemId) const;

	/** Distinct gems found, which is what the vault's completion meter shows. */
	UFUNCTION(BlueprintPure, Category = "Gem Hunter|Collection")
	int32 GetUniqueGemCount() const;

	/** The vault displays your best specimen of each gem, not your first. */
	UFUNCTION(BlueprintCallable, Category = "Gem Hunter|Collection")
	bool GetBestSpecimen(FName GemId, FFoundSpecimen& OutSpecimen) const;

	/** Returned by value: UFUNCTION return types cannot be const references. */
	UFUNCTION(BlueprintPure, Category = "Gem Hunter|Collection")
	TArray<FFoundSpecimen> GetAllSpecimens() const;

	UFUNCTION(BlueprintCallable, Category = "Gem Hunter|Collection")
	bool SaveCollection();

	UFUNCTION(BlueprintCallable, Category = "Gem Hunter|Collection")
	bool LoadCollection();

private:
	/** Reads the slot name from UGemHunterDataSettings. */
	FString GetSaveSlotName() const;

	UPROPERTY(Transient)
	TObjectPtr<UGemHunterSaveGame> SaveData;
};
