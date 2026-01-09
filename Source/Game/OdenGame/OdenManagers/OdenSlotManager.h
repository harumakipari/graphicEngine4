#pragma once

class OdenSlotActor;
class OdenIngredientActor;

class OdenSlotManager
{
public:
    
    void Initialize();

    void RegisterSlot(const std::shared_ptr<OdenSlotActor>& slot);

    void RegisterIngredient(const std::shared_ptr<OdenIngredientActor>& ingredient);

    void OnBeat();

private:
    std::vector<std::shared_ptr<OdenSlotActor>> slots;
};