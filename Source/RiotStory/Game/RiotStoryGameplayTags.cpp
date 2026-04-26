#include "RiotStoryGameplayTags.h"

namespace RiotStoryGameplayTags
{
    UE_DEFINE_GAMEPLAY_TAG(TAG_GameEvent_State, "GameEvent.State");

    UE_DEFINE_GAMEPLAY_TAG(TAG_GameCommand_StartModeTransition, "GameCommand.StartModeTransition");

    UE_DEFINE_GAMEPLAY_TAG(TAG_GameEvent_ModeTransitionStarted, "GameEvent.ModeTransition.Started");
    UE_DEFINE_GAMEPLAY_TAG(TAG_GameEvent_ModeTransitionAtSwap, "GameEvent.ModeTransition.AtSwap");
    UE_DEFINE_GAMEPLAY_TAG(TAG_GameEvent_ModeTransitionEnded, "GameEvent.ModeTransition.Ended");

    UE_DEFINE_GAMEPLAY_TAG(TAG_GameCommand_CardThrowGame, "GameCommand.CardThrowGame");

    UE_DEFINE_GAMEPLAY_TAG(TAG_GameEvent_CardThrowGame_StateChanged, "GameEvent.CardThrowGame.StateChanged");
    UE_DEFINE_GAMEPLAY_TAG(TAG_GameEvent_CardThrowGame_CardThrown, "GameEvent.CardThrowGame.CardThrown");
    UE_DEFINE_GAMEPLAY_TAG(TAG_GameEvent_CardThrowGame_CardScored, "GameEvent.CardThrowGame.CardScored");

    UE_DEFINE_GAMEPLAY_TAG(TAG_GameEvent_Drinks_DrankBeverage, "GameEvent.Drinks.DrankBeverage");
    UE_DEFINE_GAMEPLAY_TAG(TAG_GameEvent_Drinks_ChoseDrink, "GameEvent.Drinks.ChoseDrink");

    UE_DEFINE_GAMEPLAY_TAG(TAG_GameEvent_Story_Watched, "GameEvent.Story.Watched");
}
