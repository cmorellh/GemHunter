// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DiggableTerrain.generated.h"

/**
 * The seam between gameplay and whichever voxel backend we land on.
 *
 * The voxel backend is NOT chosen yet: the open-source UE5VoxelTerrainDemo
 * was tested on UE 5.3 and we build against 5.8, and Voxel Plugin 2 is a
 * paid dependency that contributors could not redistribute. Nothing in the
 * game module may talk to a voxel library directly - it talks to this
 * interface, and the backend is an actor that implements it.
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UDiggableTerrain : public UInterface
{
	GENERATED_BODY()
};

class GEMHUNTERCORE_API IDiggableTerrain
{
	GENERATED_BODY()

public:
	/**
	 * Remove terrain inside a sphere.
	 * @param Strength  0..1 fraction of material removed; lets soft tools chip rather than carve.
	 * @return true if the backend actually modified anything.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Gem Hunter|Terrain")
	bool DigSphere(const FVector& WorldLocation, float Radius, float Strength);
	virtual bool DigSphere_Implementation(const FVector& WorldLocation, float Radius, float Strength);

	/** Mohs hardness of the material at a point, for comparison against a tool's MaxHardness. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gem Hunter|Terrain")
	float GetHardnessAtLocation(const FVector& WorldLocation);
	virtual float GetHardnessAtLocation_Implementation(const FVector& WorldLocation);

	/** Metres below the dig site's surface datum. Positive going down. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gem Hunter|Terrain")
	float GetDepthAtLocation(const FVector& WorldLocation);
	virtual float GetDepthAtLocation_Implementation(const FVector& WorldLocation);

	/** Id of the FRockLayerRow covering a point, or NAME_None if outside every band. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gem Hunter|Terrain")
	FName GetRockLayerAtLocation(const FVector& WorldLocation);
	virtual FName GetRockLayerAtLocation_Implementation(const FVector& WorldLocation);

	/** False while chunks are still streaming, so gameplay can hold off spawning gems. */
	UFUNCTION(BlueprintNativeEvent, Category = "Gem Hunter|Terrain")
	bool IsTerrainReady();
	virtual bool IsTerrainReady_Implementation();
};
