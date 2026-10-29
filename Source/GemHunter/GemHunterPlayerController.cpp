// Copyright Gem Hunter contributors. Licensed under Apache-2.0.

#include "GemHunterPlayerController.h"

#include "GemHunter.h"

AGemHunterPlayerController::AGemHunterPlayerController()
{
	// First person: the dig face fills the screen, so there is no view target
	// blending to manage and the mouse stays captured by the game.
	bShowMouseCursor = false;
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
