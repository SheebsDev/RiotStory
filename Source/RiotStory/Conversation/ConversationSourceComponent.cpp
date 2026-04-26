#include "Conversation/ConversationSourceComponent.h"

UConversationSourceComponent::UConversationSourceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UConversationSourceComponent::HasValidStartNode() const
{
	return StartNode.DataTable != nullptr && StartNode.RowName != NAME_None;
}

