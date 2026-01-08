#pragma once
#include "Engine/Audio/CoreAudio.h"
#include "Components/Base/Component.h"

enum class EOdenType :uint8_t
{
    Daikon,
    Egg,
    Tsukune,
    Chikuwa,
    Konnyaku,
};

enum class EOdenOrientation :uint8_t
{
    Deg0,
    Deg90,
    Deg180,
    Deg270
};

enum class EOdenOrderShape :uint8_t
{
    TriangleLike,
    SquareLike,
    LongLike,
    RoundLike
};

struct OdenData
{
    EOdenType type;
    EOdenOrientation orientation;
    EOdenOrderShape shapeTag;
    int score;
};


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

    void UpdateVisiualRotation();

private:

};