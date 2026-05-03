#pragma once

#include "CoreMinimal.h"
#include "RiotStoryRenderStencil.generated.h"

UENUM(BlueprintType)
enum class ERiotStoryCustomDepthStencil : uint8
{
	None = 0,
	InteractableHighlight = 1,
	InteractableFocused = 2
};
