#pragma once
#include "Engine/Audio/CoreAudio.h"
#include "Components/Base/Component.h"

#include "Game/Actors/OdenGame/OdenDataStruct.h"


class OdenDataComponent : public Component
{
public:
    OdenDataComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :Component(name, owner) {}
    ~OdenDataComponent() override;

    void Initialize() override
    {
        
    }

    void Tick(float deltaTime) override;

    void DrawImGuiInspector() override;

    void UpdateShapeTag();

    void UpdateVisualRotation();

    EOdenOrderShape GetShapeTag()const
    {
        return odenData.shapeTag;
    }

private:
    OdenData odenData;

};