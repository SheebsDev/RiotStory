#include "CardGameUI.h"

#include "Game/RiotStoryGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"

void UCardGameUI::NotifyContinueClicked()
{
    FGameCommandCardThrowGameMessage EndMessage;
    EndMessage.Type = EGameCommandCardThrowGame::End;

    UGameplayMessageSubsystem::Get(this).BroadcastMessage(
        RiotStoryGameplayTags::TAG_GameCommand_CardThrowGame,
        EndMessage
    );
}
