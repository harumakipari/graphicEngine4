#pragma once

class Player;


class IInteractable
{
public:
    virtual ~IInteractable() = default;

    virtual void Interact(Player* player) = 0;

    // ƒIƒvƒVƒ‡ƒ“
    virtual bool CanInteract(Player* player) const { return true; }

    virtual std::string GetInteractText() const
    {
        return "Interact";
    }
};
