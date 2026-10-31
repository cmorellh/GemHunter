// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#include "GemHunterPlayerController.h"

#include "DiggableTerrain.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GemCollectionSubsystem.h"
#include "GemDatabaseSubsystem.h"
#include "GemHunter.h"
#include "GemHunterGameModeBase.h"
#include "GemYieldLibrary.h"

namespace
{
	/** Phase 0 scoop size: a sieve-load, roughly 30 cm across. */
	constexpr float ScoopRadiusUu = 30.0f;
	constexpr float ScoopStrength = 1.0f;

	void ShowOnScreen(const FString& Message, const FColor Colour)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 6.0f, Colour, Message);
		}
	}

	/**
	 * Report a scoop failure to both the screen and the log.
	 *
	 * On-screen messages vanish with the PIE session, so a path that only showed
	 * one was invisible afterwards. Everything that can stop a scoop goes through
	 * here, so the log is always a complete record.
	 */
	void ReportFailure(const FString& Message, const FColor Colour)
	{
		UE_LOG(LogGemHunter, Warning, TEXT("Scoop failed: %s"), *Message);
		ShowOnScreen(Message, Colour);
	}
}

AGemHunterPlayerController::AGemHunterPlayerController()
{
	// First person: the dig face fills the screen, so there is no view target
	// blending to manage and the mouse stays captured by the game.
	bShowMouseCursor = false;
}

void AGemHunterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	Rng.Initialize(FDateTime::UtcNow().GetTicks() & MAX_int32);
}

void AGemHunterPlayerController::SetEquippedToolTier(EToolTier NewTier)
{
	if (EquippedToolTier == NewTier)
	{
		return;
	}

	EquippedToolTier = NewTier;
	UE_LOG(LogGemHunter, Verbose, TEXT("Equipped tool tier set to %d."), static_cast<int32>(NewTier));
}

UObject* AGemHunterPlayerController::FindDiggableTerrain() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Found by interface, not by class: whichever backend the level uses, this
	// works unchanged. Phase 0 has one bed per level, so the first hit is correct.
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && Actor->Implements<UDiggableTerrain>())
		{
			return Actor;
		}
	}

	return nullptr;
}

EScoopOutcome AGemHunterPlayerController::DoScoop(float DepthMeters, bool bVerbose, FName& OutGemId)
{
	OutGemId = NAME_None;

	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UGemDatabaseSubsystem* Database = GameInstance
		? GameInstance->GetSubsystem<UGemDatabaseSubsystem>()
		: nullptr;

	if (!Database || !Database->IsLoaded())
	{
		ReportFailure(
			TEXT("Gem database is not loaded. Import the CSVs and set them in Project Settings > Game > Gem Hunter Data."),
			FColor::Red);
		return EScoopOutcome::Blocked;
	}

	UObject* TerrainObject = FindDiggableTerrain();
	if (!TerrainObject)
	{
		ReportFailure(TEXT("No diggable terrain in this level. Place a GravelBedTerrain actor."), FColor::Red);
		return EScoopOutcome::Blocked;
	}

	const AActor* TerrainActor = Cast<AActor>(TerrainObject);
	if (!TerrainActor || !IDiggableTerrain::Execute_IsTerrainReady(TerrainObject))
	{
		ReportFailure(TEXT("Terrain is not ready yet."), FColor::Yellow);
		return EScoopOutcome::Blocked;
	}

	// Dig straight down from the top of the dig volume. Taken from the actor's
	// bounds rather than a concrete component, so this holds for any backend.
	// A real scoop will come from the player's aim trace once input exists; the
	// geology answer is the same either way.
	FVector BoundsOrigin;
	FVector BoundsExtent;
	TerrainActor->GetActorBounds(/*bOnlyCollidingComponents=*/false, BoundsOrigin, BoundsExtent);

	// Metres to Unreal centimetres. The backend owns the same conversion; this is
	// the only other place it appears, and only to pick a probe point.
	const FVector DigPoint(
		BoundsOrigin.X,
		BoundsOrigin.Y,
		(BoundsOrigin.Z + BoundsExtent.Z) - (DepthMeters * 100.0f));

	if (!IDiggableTerrain::Execute_DigSphere(TerrainObject, DigPoint, ScoopRadiusUu, ScoopStrength))
	{
		ReportFailure(FString::Printf(TEXT("Cannot dig at %.1f m - outside the dig volume."), DepthMeters), FColor::Yellow);
		return EScoopOutcome::Blocked;
	}

	// The backend tells us the geology. Gameplay never computes it.
	const FName LayerId = IDiggableTerrain::Execute_GetRockLayerAtLocation(TerrainObject, DigPoint);

	// The locality comes from the game mode, which owns which site is loaded.
	FName LocalityId;
	if (const AGemHunterGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AGemHunterGameModeBase>())
	{
		LocalityId = GameMode->GetActiveLocalityId();
	}

	if (LocalityId.IsNone())
	{
		ReportFailure(TEXT("No active locality on the game mode."), FColor::Red);
		return EScoopOutcome::Blocked;
	}

	FFoundSpecimen Specimen;
	FText FailReason;
	if (!UGemYieldLibrary::RollScoop(Database, LocalityId, LayerId, EquippedToolTier, Rng, Specimen, FailReason))
	{
		if (bVerbose)
		{
			UE_LOG(LogGemHunter, Log, TEXT("Scoop at %.1f m: %s"), DepthMeters, *FailReason.ToString());
			ShowOnScreen(FailReason.ToString(), FColor::Silver);
		}
		return EScoopOutcome::Empty;
	}

	UGemCollectionSubsystem* Collection = GameInstance->GetSubsystem<UGemCollectionSubsystem>();
	if (Collection)
	{
		Collection->RecordFind(Specimen);
	}

	const FGemRow* Gem = Database->FindGem(Specimen.GemId);
	const FString GemName = Gem ? Gem->DisplayName.ToString() : Specimen.GemId.ToString();
	const FString Message = FString::Printf(
		TEXT("Found %s - %.2f ct, quality %.0f%%"),
		*GemName, Specimen.CaratWeight, Specimen.Quality * 100.0f);

	UE_LOG(LogGemHunter, Log, TEXT("Scoop at %.1f m (%s): %s"),
		DepthMeters, *LayerId.ToString(), *Message);

	if (bVerbose)
	{
		ShowOnScreen(Message, FColor::Green);
	}

	OutGemId = Specimen.GemId;
	return EScoopOutcome::Found;
}

void AGemHunterPlayerController::GHScoop(float DepthMeters)
{
	// Console args default to 0 when omitted, so treat that as "the gravels".
	FName GemId;
	DoScoop(DepthMeters > 0.0f ? DepthMeters : 1.0f, /*bVerbose=*/true, GemId);
}

void AGemHunterPlayerController::GHScoopMany(int32 Count, float DepthMeters)
{
	const int32 Scoops = Count > 0 ? Count : 100;
	const float Depth = DepthMeters > 0.0f ? DepthMeters : 1.0f;

	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UGemDatabaseSubsystem* Database = GameInstance
		? GameInstance->GetSubsystem<UGemDatabaseSubsystem>()
		: nullptr;

	int32 Finds = 0;
	int32 Taken = 0;
	TMap<FName, int32> Tally;
	for (int32 Index = 0; Index < Scoops; ++Index)
	{
		// Quiet: one line per scoop would flood the log at these counts.
		FName GemId;
		const EScoopOutcome Outcome = DoScoop(Depth, /*bVerbose=*/false, GemId);

		if (Outcome == EScoopOutcome::Blocked)
		{
			// Something is misconfigured, so the remaining scoops would fail
			// identically. ReportFailure has already logged the reason once.
			UE_LOG(LogGemHunter, Warning,
				TEXT("Stopped after %d of %d scoops: the dig is blocked, see the reason above."),
				Taken, Scoops);
			break;
		}

		++Taken;
		if (Outcome == EScoopOutcome::Found)
		{
			++Finds;
			// Tally this run's finds only. Reading the vault instead would fold in
			// every earlier session and report a hit rate that never happened.
			Tally.FindOrAdd(GemId)++;
		}
	}

	if (Taken == 0)
	{
		return;
	}

	UE_LOG(LogGemHunter, Log, TEXT("--- %d scoops at %.1f m: %d finds (%.1f%% hit rate) ---"),
		Taken, Depth, Finds, 100.0f * Finds / Taken);

	Tally.ValueSort([](int32 A, int32 B) { return A > B; });
	for (const TPair<FName, int32>& Pair : Tally)
	{
		const FGemRow* Gem = Database ? Database->FindGem(Pair.Key) : nullptr;
		UE_LOG(LogGemHunter, Log, TEXT("  %-18s %4d  (%s)"),
			*Pair.Key.ToString(),
			Pair.Value,
			Gem ? *UEnum::GetDisplayValueAsText(Gem->Rarity).ToString() : TEXT("?"));
	}

	ShowOnScreen(FString::Printf(TEXT("%d scoops: %d finds. See the log for the tally."), Taken, Finds),
		FColor::Cyan);
}

void AGemHunterPlayerController::GHVault()
{
	UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	const UGemCollectionSubsystem* Collection = GameInstance
		? GameInstance->GetSubsystem<UGemCollectionSubsystem>()
		: nullptr;
	const UGemDatabaseSubsystem* Database = GameInstance
		? GameInstance->GetSubsystem<UGemDatabaseSubsystem>()
		: nullptr;

	if (!Collection || !Database)
	{
		return;
	}

	const TArray<FFoundSpecimen> All = Collection->GetAllSpecimens();
	UE_LOG(LogGemHunter, Log, TEXT("--- Vault: %d specimens, %d unique gems ---"),
		All.Num(), Collection->GetUniqueGemCount());

	// One line per gem in the database, so the gaps are as visible as the finds.
	for (const FName& GemId : Database->GetAllGemIds())
	{
		const int32 Count = Collection->GetFoundCount(GemId);
		FFoundSpecimen Best;
		if (Count > 0 && Collection->GetBestSpecimen(GemId, Best))
		{
			UE_LOG(LogGemHunter, Log, TEXT("  %-18s x%-4d best %.2f ct, quality %.0f%%"),
				*GemId.ToString(), Count, Best.CaratWeight, Best.Quality * 100.0f);
		}
		else
		{
			UE_LOG(LogGemHunter, Log, TEXT("  %-18s -"), *GemId.ToString());
		}
	}

	ShowOnScreen(FString::Printf(TEXT("Vault: %d specimens, %d unique. See the log."),
		All.Num(), Collection->GetUniqueGemCount()), FColor::Cyan);
}

void AGemHunterPlayerController::GHTool(int32 Tier)
{
	const UEnum* ToolEnum = StaticEnum<EToolTier>();
	const int32 MaxIndex = ToolEnum ? ToolEnum->NumEnums() - 2 : 0; // -1 for _MAX, -1 for zero-based

	if (Tier < 0 || Tier > MaxIndex)
	{
		ShowOnScreen(FString::Printf(TEXT("Tool tier must be 0..%d."), MaxIndex), FColor::Yellow);
		return;
	}

	SetEquippedToolTier(static_cast<EToolTier>(Tier));

	const FString Name = ToolEnum
		? UEnum::GetDisplayValueAsText(EquippedToolTier).ToString()
		: FString::FromInt(Tier);
	ShowOnScreen(FString::Printf(TEXT("Equipped: %s"), *Name), FColor::Cyan);
	UE_LOG(LogGemHunter, Log, TEXT("Equipped tool tier %d (%s)."), Tier, *Name);
}
