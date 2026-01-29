#pragma once
#include "UI/Widgets/Widget.h"

class UIManager
{
public:
    // çXêVèàóù
    void Update(float deltaTime);

    void Draw(ID3D11DeviceContext* immediateContext) const;

    void DrawFont(ID3D11DeviceContext* immediateContext) const;

    void DrawSceneChangeSprite(ID3D11DeviceContext* immediateContext) const;

    void Add(const std::shared_ptr<UICoreComponent>& ui)
    {
        if (!ui) return;
        pendingAdd.push_back(ui);
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

    void SetAllUIActive(bool visible, bool enabled);

    bool IsMouseCaptured() const { return mouseCaptured; }
    void SetMouseCaptured(const bool v) { mouseCaptured = v; }

private:
    std::vector<std::shared_ptr<UICoreComponent>> rootComponents;
    std::vector<std::shared_ptr<UICoreComponent>> pendingAdd; // Åöí«â¡ë“Çø

    bool visible = true;
    bool enabled = true;
    bool mouseCaptured = false;
};
