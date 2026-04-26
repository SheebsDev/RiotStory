#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "GameFramework/Actor.h"
#include "Inventory/InventoryComponent.h"
#include "NPC/VendorCharacter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryAddRemoveHasCountTest, "RiotStory.Inventory.AddRemoveHasCount", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryTryUseNoDeductOnFailureTest, "RiotStory.Inventory.TryUseItem.NoDeductOnFailure", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryTryUsePreferredConsumerTest, "RiotStory.Inventory.TryUseItem.PreferredConsumer", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryTryUseOwnerFallbackTest, "RiotStory.Inventory.TryUseItem.OwnerFallback", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInventoryTryUsePreferredConsumerObjectTest, "RiotStory.Inventory.TryUseItem.PreferredConsumerObject", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace RiotStoryInventoryTests
{
	static UInventoryComponent* MakeInventory(AActor* OwnerActor)
	{
		return NewObject<UInventoryComponent>(OwnerActor);
	}
}

bool FInventoryAddRemoveHasCountTest::RunTest(const FString& Parameters)
{
	AActor* const OwnerActor = GetMutableDefault<AActor>();
	UInventoryComponent* const Inventory = RiotStoryInventoryTests::MakeInventory(OwnerActor);
	TestNotNull(TEXT("Inventory component should be created"), Inventory);

	const FName ItemId(TEXT("Credits"));
	Inventory->AddItems(ItemId, 5);

	TestEqual(TEXT("GetItemCount should return inserted quantity"), Inventory->GetItemCount(ItemId), 5);
	TestTrue(TEXT("HasItems should pass for valid quantity"), Inventory->HasItems(ItemId, 3));
	TestFalse(TEXT("HasItems should fail when requesting too many"), Inventory->HasItems(ItemId, 10));
	TestTrue(TEXT("RemoveItems should succeed when enough quantity exists"), Inventory->RemoveItems(ItemId, 2));
	TestEqual(TEXT("Count should update after remove"), Inventory->GetItemCount(ItemId), 3);

	return true;
}

bool FInventoryTryUseNoDeductOnFailureTest::RunTest(const FString& Parameters)
{
	AActor* const OwnerActor = GetMutableDefault<AActor>();
	UInventoryComponent* const Inventory = RiotStoryInventoryTests::MakeInventory(OwnerActor);
	TestNotNull(TEXT("Inventory component should be created"), Inventory);

	const FName ItemId(TEXT("Credits"));
	Inventory->AddItems(ItemId, 4);

	const bool bUsed = Inventory->TryUseItem(ItemId, 2, nullptr);
	TestFalse(TEXT("TryUseItem should fail when neither preferred nor owner is a consumer"), bUsed);
	TestEqual(TEXT("Count should not change after failed use"), Inventory->GetItemCount(ItemId), 4);

	return true;
}

bool FInventoryTryUsePreferredConsumerTest::RunTest(const FString& Parameters)
{
	AActor* const OwnerActor = GetMutableDefault<AActor>();
	UInventoryComponent* const Inventory = RiotStoryInventoryTests::MakeInventory(OwnerActor);
	AVendorCharacter* const PreferredConsumerActor = GetMutableDefault<AVendorCharacter>();

	TestNotNull(TEXT("Inventory component should be created"), Inventory);
	TestNotNull(TEXT("Preferred consumer should be created"), PreferredConsumerActor);

	const FName ItemId(TEXT("Credits"));
	Inventory->AddItems(ItemId, 5);

	const bool bUsed = Inventory->TryUseItem(ItemId, 2, PreferredConsumerActor);
	TestTrue(TEXT("TryUseItem should succeed with valid preferred consumer"), bUsed);
	TestEqual(TEXT("Count should decrease after successful use"), Inventory->GetItemCount(ItemId), 3);

	return true;
}

bool FInventoryTryUseOwnerFallbackTest::RunTest(const FString& Parameters)
{
	AVendorCharacter* const OwnerActor = GetMutableDefault<AVendorCharacter>();
	UInventoryComponent* const Inventory = RiotStoryInventoryTests::MakeInventory(OwnerActor);

	TestNotNull(TEXT("Owner actor should be created"), OwnerActor);
	TestNotNull(TEXT("Inventory component should be created"), Inventory);

	const FName ItemId(TEXT("Credits"));
	Inventory->AddItems(ItemId, 3);

	const bool bUsed = Inventory->TryUseItem(ItemId, 1, nullptr);
	TestTrue(TEXT("TryUseItem should succeed by falling back to owner consumer"), bUsed);
	TestEqual(TEXT("Count should decrease after successful owner fallback use"), Inventory->GetItemCount(ItemId), 2);

	return true;
}

bool FInventoryTryUsePreferredConsumerObjectTest::RunTest(const FString& Parameters)
{
	AActor* const OwnerActor = GetMutableDefault<AActor>();
	UInventoryComponent* const Inventory = RiotStoryInventoryTests::MakeInventory(OwnerActor);
	AVendorCharacter* const PreferredConsumerActor = GetMutableDefault<AVendorCharacter>();

	TestNotNull(TEXT("Inventory component should be created"), Inventory);
	TestNotNull(TEXT("Preferred consumer object should be created"), PreferredConsumerActor);

	const FName ItemId(TEXT("Credits"));
	Inventory->AddItems(ItemId, 5);

	const bool bUsed = Inventory->TryUseItemWithConsumerObject(ItemId, 2, PreferredConsumerActor);
	TestTrue(TEXT("TryUseItemWithConsumerObject should succeed with component consumer"), bUsed);
	TestEqual(TEXT("Count should decrease after successful use"), Inventory->GetItemCount(ItemId), 3);

	return true;
}

#endif
