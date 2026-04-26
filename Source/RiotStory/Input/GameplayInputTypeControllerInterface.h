#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayInputTypeControllerInterface.generated.h"

UENUM(BlueprintType, meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor="true"))
enum class EGameplayInputType : uint8
{
	None = 0 UMETA(Hidden),
	Move = 1 << 0,
	Look = 1 << 1,
	Combat = 1 << 2,
	Interact = 1 << 3,
	All = 0x0F
};
ENUM_CLASS_FLAGS(EGameplayInputType);

UINTERFACE(MinimalAPI)
class UGameplayInputTypeControllerInterface : public UInterface
{
	GENERATED_BODY()
};

class RIOTSTORY_API IGameplayInputTypeControllerInterface
{
	GENERATED_BODY()

public:
	virtual void SetGameplayInputTypeEnabled(EGameplayInputType InputType, bool bEnabled) = 0;
	virtual void SetGameplayInputTypesEnabled(EGameplayInputType InputTypes, bool bEnabled) = 0;
	virtual void SetAllGameplayInputTypesEnabled(bool bEnabled) = 0;
	virtual bool IsGameplayInputTypeEnabled(EGameplayInputType InputType) const = 0;
};

