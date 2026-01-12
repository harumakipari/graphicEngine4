#pragma once
#include "UI/Widgets/Widget.h"

class UIManager
{
public:
    // 更新処理
    void Update(float deltaTime);

    
    void Draw(ID3D11DeviceContext* immediateContext) const
    {
        if (!visible) return;

        // 描画順にソート zOrderが大きいほど後に描画される
        std::vector<std::shared_ptr<UICoreComponent>> sortedComponents = rootComponents;
        std::sort(sortedComponents.begin(), sortedComponents.end(),
            [](const std::shared_ptr<UICoreComponent>& a, const std::shared_ptr<UICoreComponent>& b)
            {
                return a->zOrder < b->zOrder;
            });

        for (auto& ui : sortedComponents)
        {
            if (ui->IsVisible())
            {
                ui->Draw(immediateContext);
            }
        }
    }


    void Add(const std::shared_ptr<UICoreComponent>& ui)
    {
        rootComponents.push_back(ui);
    }

    void Cleanup()
    {
        rootComponents.erase(
            std::remove_if(
                rootComponents.begin(),
                rootComponents.end(),
                [](const std::shared_ptr<UICoreComponent>& ui)
                {
                    return ui->IsPendingKill();
                }
            ),
            rootComponents.end()
        );
    }

    void Clear()
    {
        rootComponents.clear();
    }

    void DrawImGUi();

    bool IsMouseCaptured() const { return mouseCaptured; }
    void SetMouseCaptured(const bool v) { mouseCaptured = v; }

private:
    std::vector<std::shared_ptr<UICoreComponent>> rootComponents;
    bool visible = true;
    bool enabled = true;
    bool mouseCaptured = false;
};
