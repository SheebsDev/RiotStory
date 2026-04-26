#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * Events that are specific to the Riot Story game
 * will be list here until further split scope is need to expand
 */
namespace RiotStoryGameplayTags
{
    //~Begin GameState Events
    RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameEvent_State);
    //~End GameState Events

    //~Begin ModeTransition Events/Commands
    RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameCommand_StartModeTransition);

    RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameEvent_ModeTransitionStarted);
    RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameEvent_ModeTransitionAtSwap);
    RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameEvent_ModeTransitionEnded);
    //~End ModeTransition Events/Commands

    //~Begin Card Gameplay Events/Commands
    RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameCommand_CardThrowGame);

	RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameEvent_CardThrowGame_StateChanged);
    RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameEvent_CardThrowGame_CardThrown);
    RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameEvent_CardThrowGame_CardScored);
	//~End Card Gameplay Events

    //~Begin Bartender Gameplay Events
    RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameEvent_Drinks_DrankBeverage);
    RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameEvent_Drinks_ChoseDrink);
    //~End Bartender Gameplay Events

    //~Begin Watched Events
    RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_GameEvent_Story_Watched);
    //~End Watched Events
}