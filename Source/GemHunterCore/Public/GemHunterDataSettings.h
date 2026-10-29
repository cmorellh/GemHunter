// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/DeveloperSettings.h"
#include "GemHunterDataSettings.generated.h"

/**
 * Where the gem database finds its DataTables. Edited in
 * Project Settings > Game > Gem Hunter Data, stored in Config/DefaultGame.ini.
 *
 * These are soft pointers so the tables are picked by a designer, not hardcoded
 * by path in C++. Point them at the assets created by importing the CSVs in data/.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Gem Hunter Data"))
class GEMHUNTERCORE_API UGemHunterDataSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGemHunterDataSettings();

	virtual FName GetCategoryName() const override { return FName(TEXT("Game")); }

	/** Row struct: FGemRow */
	UPROPERTY(Config, EditAnywhere, Category = "DataTables", meta = (AllowedClasses = "/Script/Engine.DataTable"))
	TSoftObjectPtr<UDataTable> GemTable;

	/** Row struct: FLocalityRow */
	UPROPERTY(Config, EditAnywhere, Category = "DataTables", meta = (AllowedClasses = "/Script/Engine.DataTable"))
	TSoftObjectPtr<UDataTable> LocalityTable;

	/** Row struct: FRockLayerRow */
	UPROPERTY(Config, EditAnywhere, Category = "DataTables", meta = (AllowedClasses = "/Script/Engine.DataTable"))
	TSoftObjectPtr<UDataTable> RockLayerTable;

	/** Row struct: FToolTierRow */
	UPROPERTY(Config, EditAnywhere, Category = "DataTables", meta = (AllowedClasses = "/Script/Engine.DataTable"))
	TSoftObjectPtr<UDataTable> ToolTierTable;

	/** Row struct: FMissionRow */
	UPROPERTY(Config, EditAnywhere, Category = "DataTables", meta = (AllowedClasses = "/Script/Engine.DataTable"))
	TSoftObjectPtr<UDataTable> MissionTable;

	/** Slot name used by UGemCollectionSubsystem. */
	UPROPERTY(Config, EditAnywhere, Category = "Save")
	FString SaveSlotName;
};
