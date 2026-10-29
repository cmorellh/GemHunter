// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#include "DiggableTerrain.h"

// Inert defaults. A backend that implements the interface overrides these;
// until one exists, gameplay code can call them safely and get "nothing here".

bool IDiggableTerrain::DigSphere_Implementation(const FVector& WorldLocation, float Radius, float Strength)
{
	return false;
}

float IDiggableTerrain::GetHardnessAtLocation_Implementation(const FVector& WorldLocation)
{
	return 0.0f;
}

float IDiggableTerrain::GetDepthAtLocation_Implementation(const FVector& WorldLocation)
{
	return 0.0f;
}

FName IDiggableTerrain::GetRockLayerAtLocation_Implementation(const FVector& WorldLocation)
{
	return NAME_None;
}

bool IDiggableTerrain::IsTerrainReady_Implementation()
{
	return false;
}
