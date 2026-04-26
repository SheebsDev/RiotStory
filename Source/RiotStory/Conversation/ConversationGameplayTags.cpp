#include "Conversation/ConversationGameplayTags.h"

namespace RiotStoryConversationTags
{
	UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Verb_Talk, "Interaction.Verb.Talk");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Conversation_StartRequest, "Message.Conversation.StartRequest");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Conversation_StateChanged, "Message.Conversation.StateChanged");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Conversation_Ended, "Message.Conversation.Ended");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Conversation_ResponseEvent, "Message.Conversation.ResponseEvent");

	UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Bartender_ChooseDrink, "Interaction.Bartender.ChooseDrink");

	UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Greeter_StartCardGame, "Interaction.Greeter.StartCardGame");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Interaction_Greeter_AwardTickets, "Interaction.Greeter.AwardTickets");
}

