#include "Interaction/InteracteeComponent.h"
#include "Conversation/ConversationGameplayTags.h"
#include "Conversation/ConversationMessages.h"
#include "Conversation/ConversationSourceComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Interaction/InteractionProviderInterface.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameplayMessageSubsystem.h"

UInteracteeComponent::UInteracteeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteracteeComponent::OnRegister()
{
	Super::OnRegister();
	RefreshInteractionCollisionBinding();
}

void UInteracteeComponent::BeginPlay()
{
	Super::BeginPlay();
	RefreshInteractionCollisionBinding();
}

bool UInteracteeComponent::CanInteract_Implementation(AActor* Interactor)
{
	if (!IsValid(Interactor))
	{
		return false;
	}

	if (IsTalkingInteraction())
	{
		FConversationStartRequestMessage ConversationRequest;
		return TryBuildConversationStartRequest(Interactor, ConversationRequest);
	}

	switch (InteractionMode)
	{
	case EInteracteeMode::Loot:
		return !ItemId.IsNone() && Quantity > 0 && Interactor->FindComponentByClass<UInventoryComponent>() != nullptr;

	case EInteracteeMode::Vendor:
		if (!HasValidCostRequirement())
		{
			return true;
		}

		if (UInventoryComponent* const InteractorInventory = Interactor->FindComponentByClass<UInventoryComponent>())
		{
			if (!InteractorInventory->HasItems(RequiredItemId, RequiredQuantity))
			{
				return false;
			}

			if (bConsumeRequiredItems && !bCanConsumeItems)
			{
				return false;
			}

			return true;
		}

		return false;

	case EInteracteeMode::Custom:
	default:
		return true;
	}
}

bool UInteracteeComponent::ExecuteInteraction_Implementation(AActor* Interactor)
{
	if (!CanInteract(Interactor))
	{
		if (InteractionMode == EInteracteeMode::Vendor && !IsTalkingInteraction() && IsValid(Interactor))
		{
			BP_OnTradeFailed(Interactor);
		}

		return false;
	}

	if (IsTalkingInteraction())
	{
		FConversationStartRequestMessage ConversationRequest;
		if (!TryBuildConversationStartRequest(Interactor, ConversationRequest) || GetWorld() == nullptr)
		{
			return false;
		}

		UGameplayMessageSubsystem::Get(this).BroadcastMessage(
			RiotStoryConversationTags::TAG_Message_Conversation_StartRequest,
			ConversationRequest
		);

		return true;
	}

	switch (InteractionMode)
	{
	case EInteracteeMode::Loot:
		return HandleLootInteraction(Interactor);

	case EInteracteeMode::Vendor:
		return HandleVendorInteraction(Interactor);

	case EInteracteeMode::Custom:
	default:
		return false;
	}
}

bool UInteracteeComponent::GetInteractionDisplayInfo(AActor* Interactor, FInteractionDisplayInfo& OutDisplayInfo) const
{
	(void)Interactor;
	OutDisplayInfo = DisplayInfo;

	if (OutDisplayInfo.ActionText.IsEmpty())
	{
		switch (InteractionMode)
		{
		case EInteracteeMode::Loot:
			OutDisplayInfo.ActionText = FText::FromString(TEXT("Pick Up"));
			break;
		case EInteracteeMode::Vendor:
			OutDisplayInfo.ActionText = FText::FromString(TEXT("Talk"));
			break;
		case EInteracteeMode::Custom:
		default:
			OutDisplayInfo.ActionText = FText::FromString(TEXT("Interact"));
			break;
		}
	}

	if (OutDisplayInfo.DisplayName.IsEmpty())
	{
		if (InteractionMode == EInteracteeMode::Loot && !ItemId.IsNone())
		{
			OutDisplayInfo.DisplayName = FText::FromName(ItemId);
		}
		else if (const AActor* const OwnerActor = GetOwner())
		{
			OutDisplayInfo.DisplayName = FText::FromString(OwnerActor->GetName());
		}
		else
		{
			OutDisplayInfo.DisplayName = FText::FromString(TEXT("Interactable"));
		}
	}

	return true;
}

bool UInteracteeComponent::MatchesCollisionComponent(const UPrimitiveComponent* CollisionComponent) const
{
	return IsValid(CollisionComponent) && CollisionComponent == ResolvedInteractionCollision.Get();
}

void UInteracteeComponent::RefreshInteractionCollisionBinding()
{
	UPrimitiveComponent* CollisionComponent = nullptr;
	if (!ResolveInteractionCollisionComponent(CollisionComponent))
	{
		ResolvedInteractionCollision = nullptr;
		return;
	}

	ResolvedInteractionCollision = CollisionComponent;
	if (bAutoConfigureTraceCollision)
	{
		ConfigureCollisionForInteraction(CollisionComponent);
	}
}

bool UInteracteeComponent::IsInteractionEnabled() const
{
	return IsValid(GetOwner()) && IsValidInteractionCollisionComponent(ResolvedInteractionCollision.Get());
}

bool UInteracteeComponent::HasValidCostRequirement() const
{
	return !RequiredItemId.IsNone() && RequiredQuantity > 0;
}

bool UInteracteeComponent::IsTalkingInteraction() const
{
	return InteractionActionTag == RiotStoryConversationTags::TAG_Interaction_Verb_Talk;
}

void UInteracteeComponent::ToggleHighlight(bool bHighlighted)
{
	AActor* const OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return;
	}

	if (OwnerActor->GetClass()->ImplementsInterface(UInteractionProviderInterface::StaticClass()))
	{
		IInteractionProviderInterface::Execute_ToggleInteractableHighlight(OwnerActor, bHighlighted);
	}
}

bool UInteracteeComponent::CanConsumeItem_Implementation(AActor* Interactor, FName ItemIdToConsume, int32 QuantityToConsume)
{
	if (InteractionMode != EInteracteeMode::Vendor)
	{
		return false;
	}

	if (!bCanConsumeItems || !IsValid(Interactor) || ItemIdToConsume.IsNone() || QuantityToConsume <= 0)
	{
		return false;
	}

	if (!HasValidCostRequirement())
	{
		return true;
	}

	if (ItemIdToConsume != RequiredItemId)
	{
		return false;
	}

	return QuantityToConsume >= RequiredQuantity;
}

void UInteracteeComponent::ConsumeItem_Implementation(AActor* Interactor, FName ItemIdToConsume, int32 QuantityToConsume)
{
	BP_OnItemConsumed(Interactor, ItemIdToConsume, QuantityToConsume);
}

bool UInteracteeComponent::ResolveInteractionCollisionComponent(UPrimitiveComponent*& OutCollisionComponent) const
{
	OutCollisionComponent = nullptr;

	AActor* const OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return false;
	}

	if (OwnerActor->GetClass()->ImplementsInterface(UInteractionProviderInterface::StaticClass()))
	{
		UPrimitiveComponent* const ProviderCollision = IInteractionProviderInterface::Execute_GetInteractionCollisionComponent(OwnerActor);
		if (IsValidInteractionCollisionComponent(ProviderCollision))
		{
			OutCollisionComponent = ProviderCollision;
			return true;
		}
	}

	if (IsValidInteractionCollisionComponent(ExplicitInteractionCollision.Get()))
	{
		OutCollisionComponent = ExplicitInteractionCollision.Get();
		return true;
	}

	TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents;
	OwnerActor->GetComponents(PrimitiveComponents);
	for (UPrimitiveComponent* const PrimitiveComponent : PrimitiveComponents)
	{
		if (IsValidInteractionCollisionComponent(PrimitiveComponent))
		{
			OutCollisionComponent = PrimitiveComponent;
			return true;
		}
	}

	return false;
}

bool UInteracteeComponent::IsValidInteractionCollisionComponent(const UPrimitiveComponent* CollisionComponent) const
{
	if (!IsValid(CollisionComponent))
	{
		return false;
	}

	const ECollisionEnabled::Type CollisionEnabled = CollisionComponent->GetCollisionEnabled();
	return CollisionEnabled == ECollisionEnabled::QueryOnly || CollisionEnabled == ECollisionEnabled::QueryAndPhysics;
}

void UInteracteeComponent::ConfigureCollisionForInteraction(UPrimitiveComponent* CollisionComponent) const
{
	if (!IsValid(CollisionComponent))
	{
		return;
	}

	CollisionComponent->SetCollisionResponseToChannel(InteractionTraceChannel, ECR_Block);
}

bool UInteracteeComponent::TryBuildConversationStartRequest(AActor* Interactor, FConversationStartRequestMessage& OutRequestMessage) const
{
	AActor* const OwnerActor = GetOwner();
	if (!IsValid(Interactor) || !IsValid(OwnerActor))
	{
		return false;
	}

	const UConversationSourceComponent* const ConversationSource = OwnerActor->FindComponentByClass<UConversationSourceComponent>();
	if (!IsValid(ConversationSource) || !ConversationSource->HasValidStartNode())
	{
		return false;
	}

	OutRequestMessage.InteractorActor = Interactor;
	OutRequestMessage.SourceActor = OwnerActor;
	OutRequestMessage.SourceInteractee = const_cast<UInteracteeComponent*>(this);
	OutRequestMessage.StartNode = ConversationSource->StartNode;
	return true;
}

bool UInteracteeComponent::HandleLootInteraction(AActor* Interactor)
{
	UInventoryComponent* const Inventory = Interactor ? Interactor->FindComponentByClass<UInventoryComponent>() : nullptr;
	if (!IsValid(Inventory) || ItemId.IsNone() || Quantity <= 0)
	{
		return false;
	}

	Inventory->AddItems(ItemId, Quantity);
	BP_OnLootCollected(Interactor, ItemId, Quantity);

	AActor* const OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return true;
	}

	if (bDestroyOwnerAfterLoot)
	{
		OwnerActor->Destroy();
		return true;
	}

	if (bDisableOwnerAfterLoot)
	{
		OwnerActor->SetActorHiddenInGame(true);
		OwnerActor->SetActorEnableCollision(false);
		OwnerActor->SetActorTickEnabled(false);
	}

	return true;
}

bool UInteracteeComponent::HandleVendorInteraction(AActor* Interactor)
{
	if (!IsValid(Interactor))
	{
		return false;
	}

	if (!HasValidCostRequirement())
	{
		BP_OnTradeSucceeded(Interactor);
		return true;
	}

	UInventoryComponent* const InteractorInventory = Interactor->FindComponentByClass<UInventoryComponent>();
	if (!IsValid(InteractorInventory))
	{
		BP_OnTradeFailed(Interactor);
		return false;
	}

	bool bWasSuccessful = false;
	if (bConsumeRequiredItems)
	{
		bWasSuccessful = InteractorInventory->TryUseItemWithConsumerObject(RequiredItemId, RequiredQuantity, this);
	}
	else
	{
		bWasSuccessful = InteractorInventory->HasItems(RequiredItemId, RequiredQuantity);
	}

	if (bWasSuccessful)
	{
		BP_OnTradeSucceeded(Interactor);
	}
	else
	{
		BP_OnTradeFailed(Interactor);
	}

	return bWasSuccessful;
}
