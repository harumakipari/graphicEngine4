#pragma once
#include "Core/Actor.h"
#include "Game/OdenGame/OdenActors/BeatReactive.h"


// 　タイトル店
// 　モデル
//
class OdenTitleStageActor :public Actor
{
private:
    enum class Difficulty :uint8_t
    {
        Tutorial = -1,
        Easy,
        Normal,
        Hard
    };

    struct DifficultySelect
    {
        Difficulty difficulty;
        std::shared_ptr<SkeletalMeshComponent> model;
        std::shared_ptr<BoxComponent> collider;
    };
public:
    OdenTitleStageActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float elapsedTime)override;

private:
    // 難易度によっての遷移シーン選択
    void RequestChangeScene(Difficulty diff);

private:
    std::shared_ptr<StaticMeshComponent> storeModelComponent;     // 店のモデル
    std::shared_ptr<SkeletalMeshComponent> selectModelComponent;   // 難易度選択のモデル
    std::shared_ptr<BoxComponent> selectModelBoxComponent;   // 難易度選択の当たり判定

    bool isHovering = false;
    DirectX::XMFLOAT3 defaultScale = { 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT3 hoverScale = { 1.15f, 1.15f, 1.15f };

    std::vector<DifficultySelect> difficultySelects;

};
