#pragma once

#include "NativeGameplayTags.h"

namespace GaiaGameplayTags
{
	GAIAGAME_API	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

	GAIAGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	GAIAGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Mouse);
	GAIAGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Stick);
	GAIAGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Crouch);
	GAIAGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_AutoRun);


	// These are mappings from MovementMode enums to GameplayTags associated with those enums (below)
	GAIAGAME_API	extern const TMap<uint8, FGameplayTag> MovementModeTagMap;
	GAIAGAME_API	extern const TMap<uint8, FGameplayTag> CustomMovementModeTagMap;

	GAIAGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Walking);
	GAIAGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_NavWalking);
	GAIAGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Falling);
	GAIAGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Swimming);
	GAIAGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Flying);

	GAIAGAME_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Movement_Mode_Custom);
};
