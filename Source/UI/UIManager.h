#pragma once
#include "UI/Widgets/Widget.h"

class UIManager
{
public:
    void Update(float deltaTime)
    {
        if (!enabled) return;
        for (auto ui : rootComponents)
        {
            if (ui->enabled)
            {
                ui->Update(deltaTime);
            }
        }
    }

    void Draw()
    {
        if (!visible) return;
        for (auto ui : rootComponents)
        {
            if (ui->visible)
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
private:
    std::vector<std::shared_ptr<UICoreComponent>> rootComponents;
    bool visible = true;
    bool enabled = true;
};
