// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#include "GravelBedTerrain.h"

#include "Components/BoxComponent.h"
#include "Engine/GameInstance.h"
#include "GemDatabaseSubsystem.h"
#include "GemYieldLibrary.h"
#include "GemHunter.h"

AGravelBedTerrain::AGravelBedTerrain()
	: LocalityId(TEXT("ratnapura"))
{
	PrimaryActorTick.bCanEverTick = false;

	BedVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("BedVolume"));
	SetRootComponent(BedVolume);
	BedVolume->SetBoxExtent(BedHalfExtent);
	// Query-only: the volume answers "is this point in the bed". It is not
	// something the player collides with.
	BedVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BedVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	BedVolume->SetHiddenInGame(true);

	// Phase 0 defaults match the rows in data/RockLayers.csv: overburden on top,
	// then the gem-bearing gravels that Sri Lankan miners call illam.
	LayerIds = { FName(TEXT("topsoil")), FName(TEXT("river_gravel")) };
}

void AGravelBedTerrain::BeginPlay()
{
	Super::BeginPlay();

	BedVolume->SetBoxExtent(BedHalfExtent);

	// Validate against the database once, loudly, rather than failing silently on
	// every scoop. A typo in LayerIds is otherwise very hard to spot.
	const UGemDatabaseSubsystem* Database = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UGemDatabaseSubsystem>()
		: nullptr;

	if (!Database || !Database->IsLoaded())
	{
		UE_LOG(LogGemHunter, Warning,
			TEXT("GravelBed '%s': gem database not loaded, so this bed will yield nothing."),
			*GetName());
		return;
	}

	if (!Database->FindLocality(LocalityId))
	{
		UE_LOG(LogGemHunter, Error,
			TEXT("GravelBed '%s': LocalityId '%s' has no row in the locality table."),
			*GetName(), *LocalityId.ToString());
	}

	for (const FName& LayerId : LayerIds)
	{
		if (!Database->FindRockLayer(LayerId))
		{
			UE_LOG(LogGemHunter, Error,
				TEXT("GravelBed '%s': LayerId '%s' has no row in the rock layer table."),
				*GetName(), *LayerId.ToString());
		}
	}

	UE_LOG(LogGemHunter, Log,
		TEXT("GravelBed '%s' ready: locality '%s', %d layers, bed %.0f x %.0f m, %.1f m deep."),
		*GetName(), *LocalityId.ToString(), LayerIds.Num(),
		(BedHalfExtent.X * 2.0f) / UnrealUnitsPerMeter,
		(BedHalfExtent.Y * 2.0f) / UnrealUnitsPerMeter,
		(BedHalfExtent.Z * 2.0f) / UnrealUnitsPerMeter);
}

float AGravelBedTerrain::GetBedTopZ() const
{
	return GetActorLocation().Z + BedHalfExtent.Z;
}

bool AGravelBedTerrain::IsWithinBed(const FVector& WorldLocation) const
{
	const FVector Local = GetActorTransform().InverseTransformPosition(WorldLocation);
	return FMath::Abs(Local.X) <= BedHalfExtent.X
		&& FMath::Abs(Local.Y) <= BedHalfExtent.Y
		&& FMath::Abs(Local.Z) <= BedHalfExtent.Z;
}

float AGravelBedTerrain::GetDepthAtLocation_Implementation(const FVector& WorldLocation)
{
	// Positive going down, in metres, measured from the top face of the bed.
	const float DepthUu = GetBedTopZ() - WorldLocation.Z;
	return FMath::Max(0.0f, DepthUu) / UnrealUnitsPerMeter;
}

FName AGravelBedTerrain::GetRockLayerAtLocation_Implementation(const FVector& WorldLocation)
{
	if (!IsWithinBed(WorldLocation))
	{
		return NAME_None;
	}

	const UGemDatabaseSubsystem* Database = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UGemDatabaseSubsystem>()
		: nullptr;
	if (!Database)
	{
		return NAME_None;
	}

	const float DepthMeters = GetDepthAtLocation_Implementation(WorldLocation);
	return UGemYieldLibrary::FindLayerAtDepth(Database, LayerIds, DepthMeters);
}

float AGravelBedTerrain::GetHardnessAtLocation_Implementation(const FVector& WorldLocation)
{
	const UGemDatabaseSubsystem* Database = GetGameInstance()
		? GetGameInstance()->GetSubsystem<UGemDatabaseSubsystem>()
		: nullptr;
	if (!Database)
	{
		return 0.0f;
	}

	const FName LayerId = GetRockLayerAtLocation_Implementation(WorldLocation);
	const FRockLayerRow* Layer = Database->FindRockLayer(LayerId);
	return Layer ? Layer->Hardness : 0.0f;
}

bool AGravelBedTerrain::DigSphere_Implementation(const FVector& WorldLocation, float Radius, float Strength)
{
	if (!IsWithinBed(WorldLocation))
	{
		return false;
	}

	// Phase 0 deforms nothing: a placer deposit is worked by scooping and sieving
	// rather than carving, so the bed only needs to count scoops and report where
	// they came from. The voxel backend is what will actually remove material.
	++ScoopCount;

	UE_LOG(LogGemHunter, Verbose,
		TEXT("GravelBed '%s': scoop %d at depth %.2f m (radius %.0f, strength %.2f)."),
		*GetName(), ScoopCount, GetDepthAtLocation_Implementation(WorldLocation), Radius, Strength);

	return true;
}

bool AGravelBedTerrain::IsTerrainReady_Implementation()
{
	// Nothing streams in Phase 0, so the bed is ready as soon as it exists.
	return true;
}
