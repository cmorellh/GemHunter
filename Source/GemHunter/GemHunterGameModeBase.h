// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GemHunterGameModeBase.generated.h"

/**
 * Base game mode for a dig site.
 *
 * GameModeBase rather than GameMode: Earth is solo (docs/Decisions.md D1), so
 * there is no match state to run. State still lives in GameMode/PlayerState so
 * that co-op space missions stay possible later.
 *
 * Phase 0 stub: it resolves which locality and mission are active and logs them.
 */
UCLASS()
class GEMHUNTER_API AGemHunterGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGemHunterGameModeBase();

	/** Which site this level represents. Matches FLocalityRow::Id. */
	UFUNCTION(BlueprintPure, Category = "Gem Hunter")
	FName GetActiveLocalityId() const { return ActiveLocalityId; }

	/** Matches FMissionRow::Id, or NAME_None for free digging. */
	UFUNCTION(BlueprintPure, Category = "Gem Hunter")
	FName GetActiveMissionId() const { return ActiveMissionId; }

protected:
	virtual void BeginPlay() override;

	/**
	 * Set per level. Phase 0 is Ratnapura, so that is the default; a dig-site
	 * level overrides it on its own GameMode Blueprint rather than in C++.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gem Hunter")
	FName ActiveLocalityId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gem Hunter")
	FName ActiveMissionId;
};
