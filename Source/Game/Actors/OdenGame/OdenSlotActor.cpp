#include "pch.h"
#include "OdenSlotActor.h"

#include "OdenIngredient.h"

void OdenSlotActor::OnBeat() const
{
    auto odenActor = odenIngredientActor.lock();
    if (!odenActor)// ‚¨‚Å‚ñ‚ÌHÞ‚ª“ü‚Á‚Ä‚¢‚½‚çA
        return;

    switch (rotationType)
    {
    case ERotationType::Horizontal:
        odenActor->RotateHorizontal();
        break;
    case ERotationType::Vertical:
        odenActor->RotateVertical();
        break;
    }
}
