#include "Inventory/LootPickup.h"
#include "Interaction/InteracteeComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

ALootPickup::ALootPickup()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	InteractionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Interaction Collision"));
	InteractionCollision->SetupAttachment(Root);
	InteractionCollision->SetSphereRadius(60.0f);
	InteractionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionCollision->SetCollisionObjectType(ECC_WorldDynamic);
	InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionCollision->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(InteractionCollision);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Interactee = CreateDefaultSubobject<UInteracteeComponent>(TEXT("Interactee"));
	Interactee->InteractionMode = EInteracteeMode::Loot;
	Interactee->DisplayInfo.ActionText = FText::FromString(TEXT("Pick Up"));
	Interactee->DisplayInfo.DisplayName = FText::FromString(TEXT("Loot"));
}

UPrimitiveComponent* ALootPickup::GetInteractionCollisionComponent_Implementation() const
{
	return InteractionCollision.Get();
}

void ALootPickup::ToggleInteractableHighlight_Implementation(bool bHighlighted)
{
	if (bIsHighlighted != bHighlighted)
	{
		bIsHighlighted = bHighlighted;
		//TODO: Update material
	}
}

