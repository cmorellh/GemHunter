// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GemHunterTypes.h"
#include "GemHunterPlayerController.generated.h"

/**
 * Why a scoop ended the way it did.
 *
 * The distinction that matters is Empty vs Blocked: an empty scoop is the game
 * working correctly and should be retried, while a blocked one means the level or
 * the data is misconfigured and retrying 200 times just floods the log.
 */
enum class EScoopOutcome : uint8
{
	Found,
	Empty,
	Blocked
};

/**
 * First-person dig-site controller (Decisions D1: first person only).
 *
 * Phase 0: it tracks the equipped tool tier and exposes the sieve loop as console
 * commands. Real input bindings need UInputAction and UInputMappingContext
 * assets, which are binary and have to be authored in the editor, so the exec
 * functions below let the whole yield chain be tested before those exist.
 *
 * Digging goes through IDiggableTerrain. This class must never reference a
 * concrete terrain backend.
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

	/**
	 * Take one scoop and sieve it.
	 *
	 * @param DepthMeters  how deep to dig. Zero or less means 1 m, which lands in
	 *                     the Ratnapura gravels rather than the overburden.
	 */
	UFUNCTION(Exec)
	void GHScoop(float DepthMeters);

	/** Take many scoops at one depth and print the tally. Useful for tuning drop rates. */
	UFUNCTION(Exec)
	void GHScoopMany(int32 Count, float DepthMeters);

	/** Print the vault: unique gems, total specimens, and the best of each. */
	UFUNCTION(Exec)
	void GHVault();

	/** Set the equipped tool tier by index, 0 = Hands. Lets the gates be tested. */
	UFUNCTION(Exec)
	void GHTool(int32 Tier);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gem Hunter|Tools")
	EToolTier EquippedToolTier = EToolTier::Hands;

private:
	/**
	 * Resolve the dig target through the interface, never by concrete class.
	 * Returns nullptr, and explains why, if the level has no diggable terrain.
	 */
	UObject* FindDiggableTerrain() const;

	/**
	 * Runs one scoop and reports the outcome to log and screen.
	 * On EScoopOutcome::Found, OutGemId names what was dug up.
	 */
	EScoopOutcome DoScoop(float DepthMeters, bool bVerbose, FName& OutGemId);

	/** Seeded once at BeginPlay so a session's finds vary but stay reproducible. */
	FRandomStream Rng;
};
