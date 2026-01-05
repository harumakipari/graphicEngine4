#pragma once
#include "UI/Widgets/Widget.h"

class UIManager
{
public:
    void Update(float deltaTime)
    {
        if (!enabled) return;
        mouseCaptured = false;
        for (auto ui : rootComponents)
        {
            if (ui->IsEnabled())
            {
                ui->UpdateTransform();
                ui->Update(deltaTime);
            }
        }
    }

    void Draw() const
    {
        if (!visible) return;
        for (auto ui : rootComponents)
        {
            if (ui->IsVisible() /*&& ui->IsEnabled()*/)
            {
                ui->Draw();
            }
        }
    }

    void Add(std::shared_ptr<UICoreComponent> ui)
    {
        rootComponents.push_back(ui);
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
