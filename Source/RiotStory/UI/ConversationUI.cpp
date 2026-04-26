#include "ConversationUI.h"

void UConversationUI::NotifyResponseIndexChosen(const int32 ResponseIndex)
{
	OnResponseIndexChosen.Broadcast(ResponseIndex);
}

void UConversationUI::NotifyConversationWindowClicked()
{
	OnConversationWindowClicked.Broadcast();
}
