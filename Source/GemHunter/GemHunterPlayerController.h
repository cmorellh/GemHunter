// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GemHunterTypes.h"
#include "GemHunterPlayerController.generated.h"

/**
 * First-person dig-site controller (docs/Decisions.md D1: first person only).
 *
 * Phase 0 stub: it tracks the equipped tool tier and nothing else. Input
 * bindings, the dig trace and the gem-extraction flow come with the voxel
 * backend, and must go through IDiggableTerrain rather than any voxel library.
 */
UCLASS()
class GEMHUNTER_API AGemHunterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGemHunterPlayerController();

	/** Phase 0 starts the player on the sieve, per the roadmap. */
	UFUNCTION(BlueprintPure, Category = "Gem Hunter|Tools")
	EToolTier GetEquippedToolTier() const { return EquippedToolTier; }

	UFUNCTION(BlueprintCallable, Category = "Gem Hunter|Tools")
	void SetEquippedToolTier(EToolTier NewTier);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gem Hunter|Tools")
	EToolTier EquippedToolTier = EToolTier::Hands;
};
