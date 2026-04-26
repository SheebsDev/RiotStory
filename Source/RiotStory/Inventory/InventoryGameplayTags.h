#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace RiotStoryInventoryTags
{
	RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Inventory_ItemChanged);

	RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Change_Added);
	RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Change_Removed);
	RIOTSTORY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Inventory_Change_Consumed);
}

