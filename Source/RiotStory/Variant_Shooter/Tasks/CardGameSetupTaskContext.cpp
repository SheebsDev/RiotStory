#include "Variant_Shooter/Tasks/CardGameSetupTaskContext.h"

bool FCardGameSetupTaskContext::IsValidForTransition() const
{
    return ShooterController.IsValid() && ShooterPawn.IsValid() && CardGameStartPoint.IsValid() && ModeTransitionUI.IsValid();
}

bool FCardGameSetupTaskContext::IsValidForSwap() const
{
    return ShooterController.IsValid() && ShooterPawn.IsValid() && CardGameStartPoint.IsValid();
}

bool FCardGameSetupTaskContext::IsValidForFinalize() const
{
    return bSwapApplied && ShooterPawn.IsValid() && ShooterController.IsValid();
}
