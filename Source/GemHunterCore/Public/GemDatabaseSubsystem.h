// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GemDataRows.h"
#include "GemDatabaseSubsystem.generated.h"

class UDataTable;

/**
 * Read-only access to the gem database. Loads the DataTables named in
 * UGemHunterDataSettings and hands out rows by Id.
 *
 * Gameplay should never hold a UDataTable pointer of its own; it asks here.
 * That is what keeps "new gem = new CSV row + texture" true.
 */
UCLASS()
class GEMHUNTERCORE_API UGemDatabaseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** False if any configured table failed to load; the HUD can then show a clear error. */
	UFUNCTION(BlueprintPure, Category = "Gem Hunter|Database")
	bool IsLoaded() const { return bLoaded; }

	/** Re-reads the tables after the settings change. Editor convenience. */
	UFUNCTION(BlueprintCallable, Category = "Gem Hunter|Database")
	void ReloadTables();

	// Row lookups. All return nullptr when the id is unknown.
	const FGemRow* FindGem(FName GemId) const;
	const FLocalityRow* FindLocality(FName LocalityId) const;
	const FRockLayerRow* FindRockLayer(FName RockLayerId) const;
	const FToolTierRow* FindToolTier(EToolTier Tier) const;
	const FMissionRow* FindMission(FName MissionId) const;

	// Blueprint-facing copies, since Blueprints cannot hold a raw row pointer.
	UFUNCTION(BlueprintCallable, Category = "Gem Hunter|Database", meta = (DisplayName = "Find Gem"))
	bool GetGem(FName GemId, FGemRow& OutGem) const;

	UFUNCTION(BlueprintCallable, Category = "Gem Hunter|Database", meta = (DisplayName = "Find Locality"))
	bool GetLocality(FName LocalityId, FLocalityRow& OutLocality) const;

	UFUNCTION(BlueprintCallable, Category = "Gem Hunter|Database", meta = (DisplayName = "Find Mission"))
	bool GetMission(FName MissionId, FMissionRow& OutMission) const;

	UFUNCTION(BlueprintPure, Category = "Gem Hunter|Database")
	TArray<FName> GetAllGemIds() const;

	UFUNCTION(BlueprintPure, Category = "Gem Hunter|Database")
	TArray<FName> GetAllLocalityIds() const;

	/** Missions belonging to one site, unsorted. */
	UFUNCTION(BlueprintPure, Category = "Gem Hunter|Database")
	TArray<FName> GetMissionIdsForLocality(FName LocalityId) const;

	/**
	 * Whether a tool tier can work a rock layer. Both gates must pass:
	 * the tier's MaxHardness must meet the layer's Hardness, and its
	 * MaxDepth must reach the layer's DepthMin.
	 */
	UFUNCTION(BlueprintPure, Category = "Gem Hunter|Database")
	bool CanToolTierWorkLayer(EToolTier Tier, FName RockLayerId) const;

private:
	/** Loads the soft-pointed tables synchronously and validates their row structs. */
	void LoadTables();

	/** Warns if a table's row struct is not what the subsystem expects. */
	bool ValidateTable(const UDataTable* Table, const UScriptStruct* ExpectedStruct, const TCHAR* Label) const;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> GemTable;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> LocalityTable;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> RockLayerTable;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> ToolTierTable;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> MissionTable;

	bool bLoaded = false;
};
