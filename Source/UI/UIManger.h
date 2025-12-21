#pragma once
#include "UI/Widgets/Widget.h"

class UIManager
{
public:
    void Update(float deltaTime)
    {
        for (auto ui : rootComponents)
        {
            {
                ui->Update(deltaTime);
            }
        }
    }

    void Draw()
    {
        for (auto ui : rootComponents)
        {
            {
                ui->Draw();
            }
        }
    }

    void Add(std::shared_ptr<UICoreComponent> ui)
    {
        rootComponents.push_back(ui);
    }

private:
    std::vector<std::shared_ptr<UICoreComponent>> rootComponents;
};
