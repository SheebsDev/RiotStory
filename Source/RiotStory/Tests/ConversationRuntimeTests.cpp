#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Conversation/ConversationData.h"
#include "Conversation/ConversationRuntimeComponent.h"
#include "Inventory/InventoryComponent.h"
#include "NPC/VendorCharacter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConversationRuntimeStartAndChunkAdvanceTest, "RiotStory.Conversation.Runtime.StartAndChunkAdvance", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConversationRuntimeResponseBranchTest, "RiotStory.Conversation.Runtime.ResponseBranch", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConversationRuntimeInvalidNodeFailsTest, "RiotStory.Conversation.Runtime.InvalidNodeFails", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConversationRuntimeResponseCostTest, "RiotStory.Conversation.Runtime.ResponseCost", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

namespace RiotStoryConversationTests
{
	static UConversationRuntimeComponent* MakeRuntime(UObject* Outer)
	{
		return NewObject<UConversationRuntimeComponent>(Outer);
	}

	static UDataTable* MakeConversationTable(UObject* Outer)
	{
		UDataTable* const Table = NewObject<UDataTable>(Outer);
		Table->RowStruct = FConversationNodeRow::StaticStruct();
		return Table;
	}
}

bool FConversationRuntimeStartAndChunkAdvanceTest::RunTest(const FString& Parameters)
{
	AActor* const RuntimeOwner = GetMutableDefault<AActor>();
	AActor* const Interactor = GetMutableDefault<AActor>();
	AActor* const SourceActor = GetMutableDefault<AActor>();
	UConversationRuntimeComponent* const Runtime = RiotStoryConversationTests::MakeRuntime(RuntimeOwner);
	UDataTable* const ConversationTable = RiotStoryConversationTests::MakeConversationTable(RuntimeOwner);

	FConversationNodeRow StartNode;
	StartNode.Chunks = { FText::FromString(TEXT("Hello")), FText::FromString(TEXT("World")) };
	ConversationTable->AddRow(FName(TEXT("Start")), StartNode);

	FDataTableRowHandle StartHandle;
	StartHandle.DataTable = ConversationTable;
	StartHandle.RowName = FName(TEXT("Start"));

	TestTrue(TEXT("Conversation should start from a valid row handle"), Runtime->StartConversationFromHandle(Interactor, SourceActor, nullptr, StartHandle));
	TestTrue(TEXT("Conversation should be active after start"), Runtime->IsConversationActive());
	TestEqual(TEXT("Initial chunk index should be zero"), Runtime->GetCurrentChunkIndex(), 0);

	TestTrue(TEXT("Advance should move to the next chunk"), Runtime->AdvanceConversation());
	TestEqual(TEXT("Chunk index should advance"), Runtime->GetCurrentChunkIndex(), 1);

	TestTrue(TEXT("Advancing beyond the last chunk should complete conversation"), Runtime->AdvanceConversation());
	TestFalse(TEXT("Conversation should end after final chunk with no next node"), Runtime->IsConversationActive());

	return true;
}

bool FConversationRuntimeResponseBranchTest::RunTest(const FString& Parameters)
{
	AActor* const RuntimeOwner = GetMutableDefault<AActor>();
	AActor* const Interactor = GetMutableDefault<AActor>();
	AActor* const SourceActor = GetMutableDefault<AActor>();
	UConversationRuntimeComponent* const Runtime = RiotStoryConversationTests::MakeRuntime(RuntimeOwner);
	UDataTable* const ConversationTable = RiotStoryConversationTests::MakeConversationTable(RuntimeOwner);

	FConversationNodeRow StartNode;
	StartNode.Chunks = { FText::FromString(TEXT("Question")) };
	FConversationResponseEntry Response;
	Response.ResponseText = FText::FromString(TEXT("Answer"));
	Response.NextNodeId = FName(TEXT("Next"));
	StartNode.Responses = { Response };
	ConversationTable->AddRow(FName(TEXT("Start")), StartNode);

	FConversationNodeRow NextNode;
	NextNode.Chunks = { FText::FromString(TEXT("Follow-up")) };
	ConversationTable->AddRow(FName(TEXT("Next")), NextNode);

	FDataTableRowHandle StartHandle;
	StartHandle.DataTable = ConversationTable;
	StartHandle.RowName = FName(TEXT("Start"));

	TestTrue(TEXT("Conversation should start"), Runtime->StartConversationFromHandle(Interactor, SourceActor, nullptr, StartHandle));
	TestTrue(TEXT("Advance should move conversation into response-selection state"), Runtime->AdvanceConversation());
	TestTrue(TEXT("Runtime should be awaiting a response"), Runtime->IsAwaitingResponse());

	TestTrue(TEXT("Selecting response should branch to target node"), Runtime->SelectConversationResponse(0));
	TestEqual(TEXT("Runtime should now be on the branched node"), Runtime->GetActiveNodeId(), FName(TEXT("Next")));
	TestEqual(TEXT("Branched node should start at chunk zero"), Runtime->GetCurrentChunkIndex(), 0);

	return true;
}

bool FConversationRuntimeInvalidNodeFailsTest::RunTest(const FString& Parameters)
{
	AActor* const RuntimeOwner = GetMutableDefault<AActor>();
	AActor* const Interactor = GetMutableDefault<AActor>();
	UConversationRuntimeComponent* const Runtime = RiotStoryConversationTests::MakeRuntime(RuntimeOwner);
	UDataTable* const ConversationTable = RiotStoryConversationTests::MakeConversationTable(RuntimeOwner);

	FDataTableRowHandle InvalidHandle;
	InvalidHandle.DataTable = ConversationTable;
	InvalidHandle.RowName = FName(TEXT("MissingNode"));

	TestFalse(TEXT("Conversation should fail to start when node does not exist"), Runtime->StartConversationFromHandle(Interactor, nullptr, nullptr, InvalidHandle));
	TestFalse(TEXT("Conversation should remain inactive after failed start"), Runtime->IsConversationActive());

	return true;
}

bool FConversationRuntimeResponseCostTest::RunTest(const FString& Parameters)
{
	AVendorCharacter* const Interactor = GetMutableDefault<AVendorCharacter>();
	UInventoryComponent* const Inventory = NewObject<UInventoryComponent>(Interactor);
	AActor* const RuntimeOwner = GetMutableDefault<AActor>();
	UConversationRuntimeComponent* const Runtime = RiotStoryConversationTests::MakeRuntime(RuntimeOwner);
	UDataTable* const ConversationTable = RiotStoryConversationTests::MakeConversationTable(RuntimeOwner);

	TestNotNull(TEXT("Interactor should be valid"), Interactor);
	TestNotNull(TEXT("Inventory should be valid"), Inventory);
	Interactor->AddOwnedComponent(Inventory);

	FConversationNodeRow StartNode;
	StartNode.Chunks = { FText::FromString(TEXT("Buy item?")) };
	FConversationResponseEntry BuyResponse;
	BuyResponse.ResponseText = FText::FromString(TEXT("Buy"));
	BuyResponse.NextNodeId = FName(TEXT("Done"));
	BuyResponse.CostItemId = FName(TEXT("Credits"));
	BuyResponse.CostQuantity = 2;
	StartNode.Responses = { BuyResponse };
	ConversationTable->AddRow(FName(TEXT("Start")), StartNode);

	FConversationNodeRow DoneNode;
	DoneNode.Chunks = { FText::FromString(TEXT("Purchased")) };
	ConversationTable->AddRow(FName(TEXT("Done")), DoneNode);

	FDataTableRowHandle StartHandle;
	StartHandle.DataTable = ConversationTable;
	StartHandle.RowName = FName(TEXT("Start"));

	Inventory->AddItems(FName(TEXT("Credits")), 1);

	TestTrue(TEXT("Conversation should start"), Runtime->StartConversationFromHandle(Interactor, nullptr, nullptr, StartHandle));
	TestTrue(TEXT("Advance should reach response choice state"), Runtime->AdvanceConversation());
	TestFalse(TEXT("Selecting costed response should fail with insufficient currency"), Runtime->SelectConversationResponse(0));
	TestTrue(TEXT("Runtime should still await response after failed spend"), Runtime->IsAwaitingResponse());
	TestEqual(TEXT("Currency should remain unchanged on failed spend"), Inventory->GetItemCount(FName(TEXT("Credits"))), 1);

	Inventory->AddItems(FName(TEXT("Credits")), 1);
	TestTrue(TEXT("Selecting costed response should succeed with sufficient currency"), Runtime->SelectConversationResponse(0));
	TestEqual(TEXT("Currency should be deducted after successful spend"), Inventory->GetItemCount(FName(TEXT("Credits"))), 0);
	TestEqual(TEXT("Conversation should advance to the response target node"), Runtime->GetActiveNodeId(), FName(TEXT("Done")));

	return true;
}

#endif
