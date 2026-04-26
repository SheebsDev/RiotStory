#include "Inventory/InventoryComponent.h"
#include "Inventory/InventoryGameplayTags.h"
#include "Interaction/InventoryItemConsumerInterface.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameplayMessageSubsystem.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::AddItems(FName ItemId, int32 Quantity)
{
	if (!IsValidItemRequest(ItemId, Quantity))
	{
		return;
	}

	const int32 OldCount = GetItemCount(ItemId);
	const int32 NewCount = OldCount + Quantity;
	SetItemCount(ItemId, NewCount);
	BroadcastItemChangedEvent(ItemId, OldCount, NewCount, nullptr, RiotStoryInventoryTags::TAG_Inventory_Change_Added);
}

bool UInventoryComponent::RemoveItems(FName ItemId, int32 Quantity)
{
	if (!HasItems(ItemId, Quantity))
	{
		return false;
	}

	const int32 OldCount = GetItemCount(ItemId);
	const int32 NewCount = OldCount - Quantity;
	SetItemCount(ItemId, NewCount);
	BroadcastItemChangedEvent(ItemId, OldCount, NewCount, nullptr, RiotStoryInventoryTags::TAG_Inventory_Change_Removed);
	return true;
}

bool UInventoryComponent::HasItems(FName ItemId, int32 Quantity) const
{
	if (!IsValidItemRequest(ItemId, Quantity))
	{
		return false;
	}

	const int32 CurrentCount = GetItemCount(ItemId);
	return CurrentCount >= Quantity;
}

int32 UInventoryComponent::GetItemCount(FName ItemId) const
{
	if (ItemId.IsNone())
	{
		return 0;
	}

	if (const int32* const ExistingCount = ItemStacks.Find(ItemId))
	{
		return *ExistingCount;
	}

	return 0;
}

bool UInventoryComponent::TryUseItem(FName ItemId, int32 Quantity, AActor* PreferredConsumer)
{
	return TryUseItemWithConsumerObject(ItemId, Quantity, PreferredConsumer);
}

bool UInventoryComponent::TryUseItemWithConsumerObject(FName ItemId, int32 Quantity, UObject* PreferredConsumer)
{
	if (!HasItems(ItemId, Quantity))
	{
		return false;
	}

	UObject* ConsumerObject = nullptr;
	if (!ResolveConsumerObject(PreferredConsumer, ConsumerObject))
	{
		return false;
	}

	AActor* const Interactor = GetOwner();
	if (!IInventoryItemConsumerInterface::Execute_CanConsumeItem(ConsumerObject, Interactor, ItemId, Quantity))
	{
		return false;
	}

	const int32 OldCount = GetItemCount(ItemId);
	const int32 NewCount = OldCount - Quantity;
	if (NewCount < 0)
	{
		return false;
	}

	AActor* const ConsumerActor = ResolveConsumerActorForEvent(ConsumerObject);
	SetItemCount(ItemId, NewCount);
	BroadcastItemChangedEvent(ItemId, OldCount, NewCount, ConsumerActor, RiotStoryInventoryTags::TAG_Inventory_Change_Consumed);

	IInventoryItemConsumerInterface::Execute_ConsumeItem(ConsumerObject, Interactor, ItemId, Quantity);
	OnItemUsed.Broadcast(ItemId, Quantity, ConsumerActor);
	return true;
}

bool UInventoryComponent::ResolveConsumerObject(UObject* PreferredConsumer, UObject*& OutConsumer) const
{
	OutConsumer = nullptr;

	if (IsConsumerObject(PreferredConsumer))
	{
		OutConsumer = PreferredConsumer;
		return true;
	}

	AActor* const OwnerActor = GetOwner();
	if (IsConsumerObject(OwnerActor))
	{
		OutConsumer = OwnerActor;
		return true;
	}

	if (IsValid(OwnerActor))
	{
		TInlineComponentArray<UActorComponent*> OwnerComponents;
		OwnerActor->GetComponents(OwnerComponents);
		for (UActorComponent* const OwnerComponent : OwnerComponents)
		{
			if (IsConsumerObject(OwnerComponent))
			{
				OutConsumer = OwnerComponent;
				return true;
			}
		}
	}

	return false;
}

bool UInventoryComponent::IsConsumerObject(UObject* CandidateObject)
{
	return IsValid(CandidateObject) && CandidateObject->GetClass()->ImplementsInterface(UInventoryItemConsumerInterface::StaticClass());
}

AActor* UInventoryComponent::ResolveConsumerActorForEvent(UObject* ConsumerObject) const
{
	if (AActor* const ConsumerActor = Cast<AActor>(ConsumerObject))
	{
		return ConsumerActor;
	}

	if (const UActorComponent* const ConsumerComponent = Cast<UActorComponent>(ConsumerObject))
	{
		return ConsumerComponent->GetOwner();
	}

	return nullptr;
}

void UInventoryComponent::BroadcastItemChangedEvent(
	FName ItemId,
	int32 OldCount,
	int32 NewCount,
	AActor* RelatedActor,
	FGameplayTag ChangeReasonTag)
{
	if (ItemId.IsNone() || OldCount == NewCount)
	{
		return;
	}

	FInventoryItemChangedEvent ChangeEvent;
	ChangeEvent.OwnerActor = GetOwner();
	ChangeEvent.ItemId = ItemId;
	ChangeEvent.OldCount = OldCount;
	ChangeEvent.NewCount = NewCount;
	ChangeEvent.Delta = NewCount - OldCount;
	ChangeEvent.RelatedActor = RelatedActor;
	ChangeEvent.ChangeReasonTag = ChangeReasonTag;

	OnItemChanged.Broadcast(ChangeEvent);
	OnItemCountChanged.Broadcast(ItemId, NewCount);

	if (GetWorld() != nullptr)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(
			RiotStoryInventoryTags::TAG_Message_Inventory_ItemChanged,
			ChangeEvent
		);
	}
}

void UInventoryComponent::SetItemCount(FName ItemId, int32 NewCount)
{
	if (NewCount > 0)
	{
		ItemStacks.Add(ItemId, NewCount);
	}
	else
	{
		ItemStacks.Remove(ItemId);
	}
}

bool UInventoryComponent::IsValidItemRequest(FName ItemId, int32 Quantity)
{
	return !ItemId.IsNone() && Quantity > 0;
}
