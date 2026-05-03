#include "VendorCharacter.h"
#include "Conversation/ConversationGameplayTags.h"
#include "Conversation/ConversationSourceComponent.h"
#include "Interaction/InteracteeComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"

AVendorCharacter::AVendorCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	InteractionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Interaction Collision"));
	InteractionCollision->SetupAttachment(GetCapsuleComponent());
	InteractionCollision->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
	InteractionCollision->SetSphereRadius(100.0f);
	InteractionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionCollision->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionCollision->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	Interactee = CreateDefaultSubobject<UInteracteeComponent>(TEXT("Interactee"));
	Interactee->InteractionMode = EInteracteeMode::Vendor;
	Interactee->DisplayInfo.ActionText = FText::FromString(TEXT("Talk"));
	Interactee->DisplayInfo.DisplayName = FText::FromString(TEXT("Vendor"));
	Interactee->InteractionActionTag = RiotStoryConversationTags::TAG_Interaction_Verb_Talk;
	USkeletalMeshComponent* CharMesh = GetMesh();
	CharMesh->SetRenderCustomDepth(true);
	CharMesh->SetCustomDepthStencilValue(1); //Default normal highlight

	ConversationSource = CreateDefaultSubobject<UConversationSourceComponent>(TEXT("Conversation Source"));
}

UPrimitiveComponent* AVendorCharacter::GetInteractionCollisionComponent_Implementation() const
{
	return InteractionCollision.Get();
}

void AVendorCharacter::ToggleInteractableHighlight_Implementation(bool bHighlighted)
{
	if (bIsHighlighted != bHighlighted)
	{
		bIsHighlighted = bHighlighted;
		GetMesh()->SetCustomDepthStencilValue(bIsHighlighted ? 2 : 1);
	}
}

bool AVendorCharacter::CanConsumeItem_Implementation(AActor* Interactor, FName ItemId, int32 Quantity)
{
	return IsValid(Interactee.Get()) && IInventoryItemConsumerInterface::Execute_CanConsumeItem(Interactee.Get(), Interactor, ItemId, Quantity);
}

void AVendorCharacter::ConsumeItem_Implementation(AActor* Interactor, FName ItemId, int32 Quantity)
{
	if (IsValid(Interactee.Get()))
	{
		IInventoryItemConsumerInterface::Execute_ConsumeItem(Interactee.Get(), Interactor, ItemId, Quantity);
	}
}
