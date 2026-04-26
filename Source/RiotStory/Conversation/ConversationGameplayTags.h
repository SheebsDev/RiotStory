#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace RiotStoryConversationTags
{
	RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Interaction_Verb_Talk);
	RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Conversation_StartRequest);
	RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Conversation_StateChanged);
	RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Conversation_Ended);
	RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Conversation_ResponseEvent);

	//~Begin Bartender Events
	RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Interaction_Bartender_ChooseDrink);
	//~End Bartender Events

	//~Begin Greeter Events
	RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Interaction_Greeter_StartCardGame);
	RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Interaction_Greeter_AwardTickets);
	//~End Greeter Events
}

