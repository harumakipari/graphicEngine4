#ifndef CONTROLLER_COMPONENT_H
#define CONTROLLER_COMPONENT_H

// C++ 標準ライブラリ
#include <memory>
#include <unordered_map>
#include <string>

// 他ライブラリ
#include <DirectXMath.h>

// プロジェクトの他のヘッダ
#include "Components/Base/Component.h"
#include "Components/Base/SceneComponent.h"
#include "Engine/Input/GamePad.h"
#include "Engine/Input/InputSystem.h"

class Actor;

struct MoveIntent
{
    DirectX::XMFLOAT3 move;
    bool jump = false;
};

class InputComponent :public Component
{
public:
    InputComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :Component(name, owner) {}

    const MoveIntent& GetIntent() const { return intent_; }

    void Tick(float) override
    {
        intent_.move = { 0,0,0 };
        if (!CameraManager::IsUseDebug())
        {
            if (InputSystem::GetInputState("W"))
            {
                intent_.move.z += 1.0f;
            }
            if (InputSystem::GetInputState("S"))
            {
                intent_.move.z -= 1.0f;
            }
            if (InputSystem::GetInputState("D"))
            {
                intent_.move.x += 1.0f;
            }
            if (InputSystem::GetInputState("A"))
            {
                intent_.move.x -= 1.0f;
            }
        }
    }

    const DirectX::XMFLOAT3& GetMoveInput() const { return intent_.move; }

    float GetTumbStateLx()
    {
        return pad.ThumbStateLx();
    }
    float GetTumbStateLy()
    {
        return pad.ThumbStateLy();
    }
    // [a]:-1   [d]:+1
    float GetThumbStateRx()
    {
        return pad.ThumbStateRx();
    }
    // [w]:+1  [s]:-1
    float GetThumbStateRy()
    {
        return pad.ThumbStateRy();
    }

private:
    MoveIntent intent_;
    GamePad pad;
};

class CharacterMovementComponent : public SceneComponent
{
public:
    CharacterMovementComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :SceneComponent(name, owner) {}

    void Tick(float dt) override;

    void SetMoveDirection(const DirectX::XMFLOAT3& dir)
    {
        inputDir_ = dir;
    }

    void ApplyIntent(const MoveIntent& intent)
    {
        inputDir_ = intent.move;
    }

private:
    // 状態
    DirectX::XMFLOAT3 velocity_{ 0,0,0 };
    bool isGrounded_ = false;

    // 設定値
    float speed_ = 5.0f;
    float gravity_ = -4.9f;
    float groundOffset_ = 1.0f;
    float radius_ = 0.4f;

    DirectX::XMFLOAT3 inputDir_{ 0,0,0 };
};



class RotationComponent :public SceneComponent
{
public:
    RotationComponent(const std::string& name, const std::shared_ptr<Actor>& owner) :SceneComponent(name, owner) {}

    void SetDirection(const DirectX::XMFLOAT3& dir);

    void Tick(float deltaTime)override;

    void SetRotateTime(float t) { this->rotateTime_ = t; }
private:
    // t: 補間率（0.0?1.0）
    DirectX::XMFLOAT4 SlerpQuaternion(const DirectX::XMFLOAT4& current, const DirectX::XMFLOAT4& target, float t)
    {
        using namespace DirectX;

        XMVECTOR q1 = XMLoadFloat4(&current);
        XMVECTOR q2 = XMLoadFloat4(&target);
        XMVECTOR result = XMQuaternionSlerp(q1, q2, t);

        XMFLOAT4 out;
        XMStoreFloat4(&out, result);
        return out;
    }

    bool IsSameDirection(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b)
    {
        constexpr float epsilon = 0.001f;
        return std::abs(a.x - b.x) < epsilon &&
            std::abs(a.y - b.y) < epsilon &&
            std::abs(a.z - b.z) < epsilon;
    }

    DirectX::XMFLOAT3 direction_ = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 previousDirection_ = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 startAngle_ = { 0.0f,0.0f,0.0f};  // degree
    DirectX::XMFLOAT4 targetRotation_ = { 0.0f,0.0f,0.0f,1.0f };
    DirectX::XMFLOAT4 startRotation_ = { 0.0f,0.0f,0.0f,1.0f };
    float lerpTime_ = 0.0f;
    float rotateTime_ = 0.3f;    // 3秒で rotation する
    float rotateSpeed_ = 10.0f; // ( degree / second )
};


#endif //CONTROLLER_COMPONENT_H