#pragma once
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"

class ScissorsPlayer;

// ハサミのアクター
class ScissorsActor :public Actor
{
public:
    enum class State :uint8_t
    {
        Equipped,   // 手に持ってる
        Dropped,    // 地面にある
        Pulling     // 引き寄せ中
    };

public:
    explicit ScissorsActor(const std::string& actorName) :Actor(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    // ハサミをプレイヤーに持たせる
    void SetOwnerPlayer(ScissorsPlayer* player)
    {
        owner = player;
    }

    // ハサミを地面に落とす
    void Drop(const DirectX::XMFLOAT3& pos);

    // ハサミを引き寄せる
    void StartPull(const DirectX::XMFLOAT3& target);

    // ハサミを拾う
    void PickUp();

    // 現在の状態を取得
    State GetState() const { return state; }

    
private:
    State state = State::Equipped;
    ScissorsPlayer* owner;
    std::shared_ptr<SkeletalMeshComponent> meshComponent;
    std::shared_ptr<SphereComponent> sphereComponent; // 当たり判定用
};
