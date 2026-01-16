#include "pch.h"
#include "OdenNextViewActor.h"

#include <ranges>

#include "Engine/Scene/Scene.h"
#include "OdenManagers/OdenSlotManager.h"


void OdenNextViewActor::Initialize(const Transform& transform)
{
    struct IngredientInfo
    {
        std::string name;
        std::string texturePath;
    };

    const std::vector<IngredientInfo> list =
    {
        { "Daikon", "./Data/Textures/UI/NextViewIngredients/Daikon.png" },
        { "Egg", "./Data/Textures/UI/NextViewIngredients/Egg.png" },
        { "Tsukune", "./Data/Textures/UI/NextViewIngredients/Tsukune.png" },
        { "Chikuwa", "./Data/Textures/UI/NextViewIngredients/Chikuwa.png" },
        { "Konnyaku", "./Data/Textures/UI/NextViewIngredients/Konnyaku.png" },
        { "Cake", "./Data/Textures/UI/NextViewIngredients/Cake.png" },
        { "Donut", "./Data/Textures/UI/NextViewIngredients/Donut.png" },
        { "Goboten", "./Data/Textures/UI/NextViewIngredients/Goboten.png" },
        { "Hanpen", "./Data/Textures/UI/NextViewIngredients/Hanpen.png" },
        { "Kobumusubi", "./Data/Textures/UI/NextViewIngredients/Kobumusubi.png" },
        { "Shirataki", "./Data/Textures/UI/NextViewIngredients/Shirataki.png" },
    };

    for (const auto& info : list)
    {
        ingredientTextures[info.name] =
            std::make_shared<Sprite>(
                Graphics::GetDevice(),
                std::wstring(info.texturePath.begin(), info.texturePath.end()).c_str()
            );
    }

    for (int i = 0; i < 3; ++i)
    {
        auto ui = std::make_shared<UIImageComponent>("nextViewUi");

        ui->SetVisible(false);
        ui->SetPivot({ 0.5f, 0.5f });
        ui->SetSize({ 128.0f, 128.0f });

        // 位置は固定でOK
        ui->SetWorldPosition({
            1800.0f,
            200.0f + i * 80.0f
            });

        GetOwnerScene()->GetUIManager()->Add(ui);

        nextSlots[i] = ui;
    }
}

void OdenNextViewActor::Update(float elapsedTime)
{
    auto slotManagerActor = GetOwnerScene()->GetActorManager()->GetActorByName("slotManager");

    if (!slotManagerActor)
        return;

    auto slotManager = std::dynamic_pointer_cast<OdenSlotManager>(slotManagerActor);

    // 全スロットOFF
    for (auto& slot : nextSlots)
    {
        slot->SetVisible(false);
    }

    // 次の3つを表示
    for (int i = 0; i < 3; ++i)
    {
        std::string ingredientName =
            slotManager->GetPreviewIngredient(i);

        if (ingredientName.empty())
            continue;

        auto& slot = nextSlots[i];

        slot->SetTexture(
            ingredientTextures[ingredientName]
        );

        slot->SetVisible(true);
    }
}
