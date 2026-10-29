// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#include "GemDatabaseSubsystem.h"

#include "Engine/DataTable.h"
#include "GemHunterCore.h"
#include "GemHunterDataSettings.h"

namespace
{
	const TCHAR* GemHunterDataContext = TEXT("GemHunter database lookup");

	/**
	 * Rows are addressed by their Id property, which the CSVs also use as the
	 * DataTable row name. Hitting the row-name index first keeps lookups O(1);
	 * the scan is only there to stay correct if someone renames a row in the
	 * editor and leaves the Id property alone.
	 */
	template <typename RowType, typename PredicateType>
	const RowType* FindRowByName(const UDataTable* Table, FName RowName, PredicateType Matches)
	{
		if (!Table || RowName.IsNone())
		{
			return nullptr;
		}

		if (const RowType* Row = Table->FindRow<RowType>(RowName, GemHunterDataContext, /*bWarnIfRowMissing=*/false))
		{
			if (Matches(*Row))
			{
				return Row;
			}
		}

		for (const TPair<FName, uint8*>& Pair : Table->GetRowMap())
		{
			const RowType* Row = reinterpret_cast<const RowType*>(Pair.Value);
			if (Row && Matches(*Row))
			{
				return Row;
			}
		}

		return nullptr;
	}
}

void UGemDatabaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadTables();
}

void UGemDatabaseSubsystem::Deinitialize()
{
	GemTable = nullptr;
	LocalityTable = nullptr;
	RockLayerTable = nullptr;
	ToolTierTable = nullptr;
	MissionTable = nullptr;
	bLoaded = false;

	Super::Deinitialize();
}

void UGemDatabaseSubsystem::ReloadTables()
{
	LoadTables();
}

bool UGemDatabaseSubsystem::ValidateTable(const UDataTable* Table, const UScriptStruct* ExpectedStruct, const TCHAR* Label) const
{
	if (!Table)
	{
		// Not an error during early bring-up: the CSVs have not been imported yet.
		UE_LOG(LogGemHunterCore, Warning,
			TEXT("Gem database: no %s DataTable is set. Import data/%s.csv and point Project Settings > Game > Gem Hunter Data at it."),
			Label, Label);
		return false;
	}

	if (Table->GetRowStruct() != ExpectedStruct)
	{
		UE_LOG(LogGemHunterCore, Error,
			TEXT("Gem database: %s DataTable '%s' uses row struct '%s', expected '%s'."),
			Label,
			*Table->GetPathName(),
			Table->GetRowStruct() ? *Table->GetRowStruct()->GetName() : TEXT("none"),
			*ExpectedStruct->GetName());
		return false;
	}

	return true;
}

void UGemDatabaseSubsystem::LoadTables()
{
	const UGemHunterDataSettings* Settings = GetDefault<UGemHunterDataSettings>();
	if (!Settings)
	{
		bLoaded = false;
		return;
	}

	// Synchronous loads: the database is small, and every dig site needs all of
	// it before the first gem can spawn, so there is nothing to gain by deferring.
	GemTable = Settings->GemTable.LoadSynchronous();
	LocalityTable = Settings->LocalityTable.LoadSynchronous();
	RockLayerTable = Settings->RockLayerTable.LoadSynchronous();
	ToolTierTable = Settings->ToolTierTable.LoadSynchronous();
	MissionTable = Settings->MissionTable.LoadSynchronous();

	bool bOk = true;
	bOk &= ValidateTable(GemTable, FGemRow::StaticStruct(), TEXT("Gems"));
	bOk &= ValidateTable(LocalityTable, FLocalityRow::StaticStruct(), TEXT("Localities"));
	bOk &= ValidateTable(RockLayerTable, FRockLayerRow::StaticStruct(), TEXT("RockLayers"));
	bOk &= ValidateTable(ToolTierTable, FToolTierRow::StaticStruct(), TEXT("ToolTiers"));
	bOk &= ValidateTable(MissionTable, FMissionRow::StaticStruct(), TEXT("Missions"));

	bLoaded = bOk;

	if (bLoaded)
	{
		UE_LOG(LogGemHunterCore, Log,
			TEXT("Gem database loaded: %d gems, %d localities, %d rock layers, %d tool tiers, %d missions."),
			GemTable->GetRowMap().Num(),
			LocalityTable->GetRowMap().Num(),
			RockLayerTable->GetRowMap().Num(),
			ToolTierTable->GetRowMap().Num(),
			MissionTable->GetRowMap().Num());
	}
}

const FGemRow* UGemDatabaseSubsystem::FindGem(FName GemId) const
{
	return FindRowByName<FGemRow>(GemTable, GemId, [GemId](const FGemRow& Row) { return Row.Id == GemId; });
}

const FLocalityRow* UGemDatabaseSubsystem::FindLocality(FName LocalityId) const
{
	return FindRowByName<FLocalityRow>(LocalityTable, LocalityId, [LocalityId](const FLocalityRow& Row) { return Row.Id == LocalityId; });
}

const FRockLayerRow* UGemDatabaseSubsystem::FindRockLayer(FName RockLayerId) const
{
	return FindRowByName<FRockLayerRow>(RockLayerTable, RockLayerId, [RockLayerId](const FRockLayerRow& Row) { return Row.Id == RockLayerId; });
}

const FMissionRow* UGemDatabaseSubsystem::FindMission(FName MissionId) const
{
	return FindRowByName<FMissionRow>(MissionTable, MissionId, [MissionId](const FMissionRow& Row) { return Row.Id == MissionId; });
}

const FToolTierRow* UGemDatabaseSubsystem::FindToolTier(EToolTier Tier) const
{
	if (!ToolTierTable)
	{
		return nullptr;
	}

	// Tool tiers are keyed by an enum rather than an Id, so there is no row-name
	// shortcut; the table has a handful of rows, so a scan is fine.
	for (const TPair<FName, uint8*>& Pair : ToolTierTable->GetRowMap())
	{
		const FToolTierRow* Row = reinterpret_cast<const FToolTierRow*>(Pair.Value);
		if (Row && Row->Tier == Tier)
		{
			return Row;
		}
	}

	return nullptr;
}

bool UGemDatabaseSubsystem::GetGem(FName GemId, FGemRow& OutGem) const
{
	if (const FGemRow* Row = FindGem(GemId))
	{
		OutGem = *Row;
		return true;
	}
	return false;
}

bool UGemDatabaseSubsystem::GetLocality(FName LocalityId, FLocalityRow& OutLocality) const
{
	if (const FLocalityRow* Row = FindLocality(LocalityId))
	{
		OutLocality = *Row;
		return true;
	}
	return false;
}

bool UGemDatabaseSubsystem::GetMission(FName MissionId, FMissionRow& OutMission) const
{
	if (const FMissionRow* Row = FindMission(MissionId))
	{
		OutMission = *Row;
		return true;
	}
	return false;
}

TArray<FName> UGemDatabaseSubsystem::GetAllGemIds() const
{
	TArray<FName> Ids;
	if (!GemTable)
	{
		return Ids;
	}

	Ids.Reserve(GemTable->GetRowMap().Num());
	for (const TPair<FName, uint8*>& Pair : GemTable->GetRowMap())
	{
		if (const FGemRow* Row = reinterpret_cast<const FGemRow*>(Pair.Value))
		{
			Ids.Add(Row->Id.IsNone() ? Pair.Key : Row->Id);
		}
	}
	return Ids;
}

TArray<FName> UGemDatabaseSubsystem::GetAllLocalityIds() const
{
	TArray<FName> Ids;
	if (!LocalityTable)
	{
		return Ids;
	}

	Ids.Reserve(LocalityTable->GetRowMap().Num());
	for (const TPair<FName, uint8*>& Pair : LocalityTable->GetRowMap())
	{
		if (const FLocalityRow* Row = reinterpret_cast<const FLocalityRow*>(Pair.Value))
		{
			Ids.Add(Row->Id.IsNone() ? Pair.Key : Row->Id);
		}
	}
	return Ids;
}

TArray<FName> UGemDatabaseSubsystem::GetMissionIdsForLocality(FName LocalityId) const
{
	TArray<FName> Ids;
	if (!MissionTable || LocalityId.IsNone())
	{
		return Ids;
	}

	for (const TPair<FName, uint8*>& Pair : MissionTable->GetRowMap())
	{
		const FMissionRow* Row = reinterpret_cast<const FMissionRow*>(Pair.Value);
		if (Row && Row->LocalityId == LocalityId)
		{
			Ids.Add(Row->Id.IsNone() ? Pair.Key : Row->Id);
		}
	}
	return Ids;
}

bool UGemDatabaseSubsystem::CanToolTierWorkLayer(EToolTier Tier, FName RockLayerId) const
{
	const FToolTierRow* ToolRow = FindToolTier(Tier);
	const FRockLayerRow* LayerRow = FindRockLayer(RockLayerId);
	if (!ToolRow || !LayerRow)
	{
		return false;
	}

	// Depth is checked against DepthMin, not DepthMax: reaching the top of a
	// band is enough to start working it.
	return ToolRow->MaxHardness >= LayerRow->Hardness && ToolRow->MaxDepth >= LayerRow->DepthMin;
}
