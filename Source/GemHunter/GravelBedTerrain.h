// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DiggableTerrain.h"
#include "GemHunterTypes.h"
#include "GravelBedTerrain.generated.h"

class UBoxComponent;

/**
 * The Phase 0 dig backend: a river-gravel bed you sieve.
 *
 * Ratnapura is a placer deposit, so the gems are already loose in the gravel and
 * nothing needs volumetric carving (see Decisions D3). This implements
 * IDiggableTerrain so the voxel backend can replace it later without gameplay
 * noticing.
 *
 * Depth here is a simple function of height below the top of the bed. A voxel
 * backend will answer the same question by sampling its own volume.
 *
 * UNITS: Unreal works in centimetres, the gem database works in metres. Every
 * conversion goes through UnrealUnitsPerMeter so the boundary sits in one place.
 */
UCLASS()
class GEMHUNTER_API AGravelBedTerrain : public AActor, public IDiggableTerrain
{
	GENERATED_BODY()

public:
	AGravelBedTerrain();

	static constexpr float UnrealUnitsPerMeter = 100.0f;

	//~ Begin IDiggableTerrain
	virtual bool DigSphere_Implementation(const FVector& WorldLocation, float Radius, float Strength) override;
	virtual float GetHardnessAtLocation_Implementation(const FVector& WorldLocation) override;
	virtual float GetDepthAtLocation_Implementation(const FVector& WorldLocation) override;
	virtual FName GetRockLayerAtLocation_Implementation(const FVector& WorldLocation) override;
	virtual bool IsTerrainReady_Implementation() override;
	//~ End IDiggableTerrain

	/** Which site this bed represents. Matches FLocalityRow::Id. */
	UFUNCTION(BlueprintPure, Category = "Gem Hunter|Terrain")
	FName GetLocalityId() const { return LocalityId; }

	/** The site's depth bands, shallowest first. Matches FRockLayerRow::Id. */
	const TArray<FName>& GetLayerIds() const { return LayerIds; }

	/** True if the point is inside the bed's bounds at all. */
	UFUNCTION(BlueprintPure, Category = "Gem Hunter|Terrain")
	bool IsWithinBed(const FVector& WorldLocation) const;

	/** How many scoops have been taken. Phase 0 stand-in for terrain deformation. */
	UFUNCTION(BlueprintPure, Category = "Gem Hunter|Terrain")
	int32 GetScoopCount() const { return ScoopCount; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem Hunter|Terrain")
	FName LocalityId;

	/**
	 * Listed explicitly rather than looked up, because FRockLayerRow has no
	 * LocalityId yet (see Known gaps in docs/ARCHITECTURE.md). Naming them here
	 * keeps one site's bands independent of another's without a schema change.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem Hunter|Terrain")
	TArray<FName> LayerIds;

	/** The diggable volume. Its top face is depth zero. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gem Hunter|Terrain")
	TObjectPtr<UBoxComponent> BedVolume;

	/** Half-extent of the bed in centimetres. Default is 20 m x 20 m, 4 m deep. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gem Hunter|Terrain")
	FVector BedHalfExtent = FVector(1000.0f, 1000.0f, 200.0f);

private:
	/** World Z of the top face of the bed, which is depth zero. */
	float GetBedTopZ() const;

	int32 ScoopCount = 0;
};
