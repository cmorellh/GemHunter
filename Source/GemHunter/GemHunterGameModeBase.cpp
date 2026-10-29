// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#include "GemHunterGameModeBase.h"

#include "Engine/GameInstance.h"
#include "GemCollectionSubsystem.h"
#include "GemDatabaseSubsystem.h"
#include "GemHunter.h"
#include "GemHunterPlayerController.h"

AGemHunterGameModeBase::AGemHunterGameModeBase()
	: ActiveLocalityId(TEXT("ratnapura"))
	, ActiveMissionId(TEXT("ratnapura_01_first_sieve"))
{
	PlayerControllerClass = AGemHunterPlayerController::StaticClass();
}

void AGemHunterGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// Phase 0 stub: confirm the data layer resolved before any gameplay is built
	// on top of it. A missing row here means a CSV was never imported.
	const UGemDatabaseSubsystem* Database = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UGemDatabaseSubsystem>()
		: nullptr;

	if (!Database || !Database->IsLoaded())
	{
		UE_LOG(LogGemHunter, Warning,
			TEXT("Gem database is not loaded. Import the CSVs in data/ as DataTables and set them in Project Settings > Game > Gem Hunter Data."));
		return;
	}

	const FLocalityRow* Locality = Database->FindLocality(ActiveLocalityId);
	if (!Locality)
	{
		UE_LOG(LogGemHunter, Warning, TEXT("No locality row for '%s'."), *ActiveLocalityId.ToString());
		return;
	}

	const UGemCollectionSubsystem* Collection = GetGameInstance()->GetSubsystem<UGemCollectionSubsystem>();
	UE_LOG(LogGemHunter, Log,
		TEXT("Dig site ready: %s (%s, %s) - %s, %d gems in table, vault holds %d unique."),
		*Locality->Name.ToString(),
		*Locality->Region,
		*Locality->Country,
		*Locality->DepositType,
		Locality->FoundGems.Num(),
		Collection ? Collection->GetUniqueGemCount() : 0);
}
