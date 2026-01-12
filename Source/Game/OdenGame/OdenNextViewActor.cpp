#include "pch.h"
#include "OdenNextViewActor.h"

#include <ranges>

#include "Engine/Scene/Scene.h"
#include "OdenManagers/OdenSlotManager.h"


void OdenNextViewActor::Initialize(const Transform& transform)
{
    struct IngredientUIInfo
    {
        std::string name;
        std::string texturePath;
    };

    const std::vector<IngredientUIInfo> ingredientList =
    {
        { "Daikon",    "./Data/Textures/UI/Ingredients/Daikon.png" },
        { "Egg",       "./Data/Textures/UI/Ingredients/Egg.png" },
        { "Tsukune",   "./Data/Textures/UI/Ingredients/Tsukune.png" },
        { "Chikuwa",   "./Data/Textures/UI/Ingredients/Chikuwa.png" },
        { "Konnyaku",  "./Data/Textures/UI/Ingredients/Konnyaku.png" },
    };

    for (const auto& info : ingredientList)
    {
        auto ui = std::make_shared<UIImageComponent>(info.texturePath, "nextViewUi");

        ui->SetVisible(false);
        ui->SetPivot({ 0.5f, 0.5f });
        ui->SetSize({ 128.0f, 128.0f });

        GetOwnerScene()->GetUIManager()->Add(ui);

        ingredients.emplace(info.name, ui);
    }
}

void OdenNextViewActor::Update(float elapsedTime)
{
    auto slotManagerActor = GetOwnerScene()->GetActorManager()->GetActorByName("slotManager");

    if (!slotManagerActor)
        return;

    auto slotManager = std::dynamic_pointer_cast<OdenSlotManager>(slotManagerActor);

    for (auto& ui : ingredients | std::views::values)
    {
        ui->SetVisible(false);
    }

    // ŽŸ‚Ì3‚Â‚ðON
    for (int i = 0; i < 3; ++i)
    {
        std::string ingredientName =
            slotManager->GetPreviewIngredient(i);

        if (ingredientName.empty())
            continue;

        auto it = ingredients.find(ingredientName);
        if (it == ingredients.end())
            continue;

        auto& ui = it->second;

        ui->SetVisible(true);

        // c‚É•À‚×‚éi‰Eãj
        ui->SetWorldPosition({ 1600.0f,200.0f + i * 80.0f });
    }
}
