#include "pch.h"
#include "ControllerComponent.h"

#include "Core/Actor.h"
#include "Core/ActorManager.h"
#include "Game/Actors/Base/Character.h"
#include "Engine/Utility/Win32Utils.h"
#include "Physics/CollisionSystem.h"

void CharacterMovementComponent::Tick(float dt)
{
    // 入力方向を正規化して速度に反映
    DirectX::XMFLOAT3 wishDir = inputDir_;
    wishDir.y = 0.0f;

    float len = sqrt(wishDir.x * wishDir.x + wishDir.z * wishDir.z);
    if (len > 0.001f)
    {
        wishDir.x /= len;
        wishDir.z /= len;
    }

    velocity_.x = wishDir.x * speed_;
    velocity_.z = wishDir.z * speed_;

    // 重力加速度を適用
    if (!isGrounded_)
    {
        velocity_.y += gravity_ * dt;
    }

    if (owner_.expired())
    {
        Logger::Warning("owner is not existed so movement is nullptr!");
        _ASSERT("owner is not existed so movement is nullptr!");
    }
    auto owner = owner_.lock();

    // 位置を予測
    DirectX::XMFLOAT3 pos = owner->GetPosition();
    DirectX::XMFLOAT3 nextPos = pos;

    nextPos.x += velocity_.x * dt;
    nextPos.y += velocity_.y * dt;
    nextPos.z += velocity_.z * dt;

    // 床との衝突判定
    isGrounded_ = false;

    float fallDistance = pos.y - nextPos.y;   // ← 重要：pos - nextPos

    float rayStartY = pos.y + groundOffset_;
    float rayLength = groundOffset_ + std::max<float>(fallDistance, 0.0f) + 0.1f;

    HitResult hit;
    if (Physics::Instance().RayCast(
        { pos.x, rayStartY, pos.z },
        { 0,-1,0 },
        rayLength,
        hit,
        CollisionHelper::ToBit(CollisionLayer::WorldStatic)))
    {
        float groundY = hit.position.y;

        if (nextPos.y <= groundY)
        {
            nextPos.y = groundY;
            velocity_.y = 0;
            isGrounded_ = true;
        }
        Graphics::GetShapeRenderer()->DrawSphere(
            hit.position, 0.1f, { 1,0,0,1 });
    }

    // 壁とのレイキャスト
    DirectX::XMFLOAT3 horizMove =
    {
        nextPos.x - pos.x,
        0,
        nextPos.z - pos.z
    };

    float dist = sqrt(horizMove.x * horizMove.x + horizMove.z * horizMove.z);

    if (dist > 0.001f)
    {
        horizMove.x /= dist;
        horizMove.z /= dist;

        HitResult wallHit;
        if (Physics::Instance().RayCast(
            { pos.x, pos.y + 1.0f, pos.z },
            horizMove,
            dist + radius_,
            wallHit,
            CollisionHelper::ToBit(CollisionLayer::WorldStatic)))
        {
            nextPos.x = pos.x;
            nextPos.z = pos.z;
        }
    }

    // 位置を更新
    owner->SetPosition(nextPos);
}



MovementComponent::MovementComponent(const std::string& name, const std::shared_ptr<Actor>& owner)
    :SceneComponent(name, owner)
{
}

void MovementComponent::Tick(float deltaTime)
{
    if (!owner_.lock())
    {
        _ASSERT("owner is not existed so movement is nullptr!");
        return;
    }

    //auto input = owner_->GetComponentByName("inputComponent");
    auto input = owner_.lock()->GetComponent<InputComponent>();
    if (!input)
    {
        _ASSERT("input is not existed so movement is nullptr!");
        return;
    }


    // character が動けるかどうか
    if (auto character = std::dynamic_pointer_cast<Character>(owner_.lock()))
    {
        if (!character->CanMove())
        {
            return;
        }
    }

    DirectX::XMFLOAT3 dir = input->GetIntent().move;

    if (std::abs(dir.x - 0.0f) <= FLT_EPSILON && std::abs(dir.y - 0.0f) <= FLT_EPSILON && std::abs(dir.z - 0.0f) <= FLT_EPSILON)
    {
        return;
    }

    DirectX::XMFLOAT4 rotation = owner_.lock()->GetQuaternionRotation();
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation));

    // 正規化
    DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&dir);
    v = DirectX::XMVector3Normalize(v);
    v = DirectX::XMVectorScale(v, speed_ * deltaTime);
    DirectX::XMFLOAT3 moveVec;
    DirectX::XMStoreFloat3(&moveVec, v);

    // TriangleMeshにたいするSweep
    //DirectX::XMFLOAT3 sweptMove = kinematicMove_->MoveAndSlide(moveVec);
    DirectX::XMFLOAT3 sweptMove = moveVec;


    // 水平方向のみ使う
    DirectX::XMFLOAT3 horizontalMove = sweptMove;
    horizontalMove.y = 0.0f;

    // 長さチェック
    float moveLen = std::sqrt(
        horizontalMove.x * horizontalMove.x +
        horizontalMove.z * horizontalMove.z
    );

    if (moveLen > 0.0001f)
    {
        // 正規化
        horizontalMove.x /= moveLen;
        horizontalMove.z /= moveLen;

        DirectX::XMFLOAT3 pos = owner_.lock()->GetPosition();

        HitResult hit;
        bool hitWall = Physics::Instance().SphereCast(
            { pos.x, pos.y + wallRayHeight_, pos.z },
            horizontalMove,
            moveLen + wallRadius_,
            0.1f, hit,
            CollisionHelper::ToBit(CollisionLayer::WorldStatic)
        );

        if (hitWall)
        {
            // 壁まで進める最大距離（少し余裕を持たせる）
            float allowedDist = hit.distance - 0.01f;

            allowedDist = std::max<float>(allowedDist, 0.0f);

            // 水平方向の移動量を制限
            sweptMove.x = horizontalMove.x * allowedDist;
            sweptMove.z = horizontalMove.z * allowedDist;

            // さらに「滑り」成分を追加したい場合
            DirectX::XMFLOAT3 n = hit.normal;

            float dot =
                moveVec.x * n.x +
                moveVec.z * n.z;

            if (dot < 0.0f)
            {
                sweptMove.x -= n.x * dot;
                sweptMove.z -= n.z * dot;
            }
        }
    }

    /* ============================
       ★ ここまで壁当たり判定 ★
       ============================ */

    owner_.lock()->GetRootComponent()->AddWorldOffset(sweptMove);


    float yaw = DirectX::XMConvertToDegrees(std::atan2f(dir.x, dir.z));
    //owner_->rootComponent_->SetLerpQuaternion(DirectX::XMFLOAT3(0.0f, yaw, 0.0f));

    //owner_->rootComponent_->LerpQuaternion(deltaTime);
}

void MovementComponentOutInput::Tick(float deltaTime)
{
    if (!owner_.lock())
    {
        _ASSERT("owner is not existed so movement is nullptr!");
        return;
    }

    //auto input = owner_->GetComponentByName("inputComponent");
    //auto input = owner_.lock()->GetComponent<InputComponent>();
    //if (!input)
    //{
    //    _ASSERT("input is not existed so movement is nullptr!");
    //    return;
    //}

    DirectX::XMFLOAT3 dir = velocity_;

    if (std::abs(dir.x - 0.0f) <= FLT_EPSILON && std::abs(dir.y - 0.0f) <= FLT_EPSILON && std::abs(dir.z - 0.0f) <= FLT_EPSILON)
    {
        return;
    }

    DirectX::XMFLOAT4 rotation = owner_.lock()->GetQuaternionRotation();
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation));

    // 正規化
    //DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&dir);
    //v = DirectX::XMVector3Normalize(v);
    //v = DirectX::XMVectorScale(v, speed_ * deltaTime);
    DirectX::XMFLOAT3 moveVec = velocity_;
    //DirectX::XMStoreFloat3(&moveVec, v);

    // 衝突システムから押し出しベクトルを取得
    DirectX::XMFLOAT3 pushVec = CollisionSystem::GetPushVectorForActor(owner_.lock().get());

    // 合算
    moveVec.x += pushVec.x;
    moveVec.y += pushVec.y;
    moveVec.z += pushVec.z;


    owner_.lock()->GetRootComponent()->AddWorldOffset(moveVec);


    float yaw = DirectX::XMConvertToDegrees(std::atan2f(dir.x, dir.z));
    //owner_->rootComponent_->SetLerpQuaternion(DirectX::XMFLOAT3(0.0f, yaw, 0.0f));

    //owner_->rootComponent_->LerpQuaternion(deltaTime);

}

void RotationComponent::SetDirection(const DirectX::XMFLOAT3& dir)
{
    // 方向に変化がなければ何もしない
    if (IsSameDirection(dir, previousDirection_))
        return;

    previousDirection_ = dir;
    direction_ = dir;

    // 補間の初期化
    lerpTime_ = 0.0f;
    startAngle_ = owner_.lock()->GetEulerRotation();
    startRotation_ = owner_.lock()->GetQuaternionRotation();
    float targetYaw = std::atan2f(dir.x, dir.z);

    DirectX::XMStoreFloat4(&targetRotation_, DirectX::XMQuaternionRotationRollPitchYaw(startAngle_.x, targetYaw, startAngle_.z));
}

void RotationComponent::Tick(float deltaTime)
{
    if (lerpTime_ >= rotateTime_)
        return; // 補間完了

    lerpTime_ += deltaTime;
    float t = lerpTime_ / rotateTime_;
    if (t > 1.0f) t = 1.0f;

    auto q = SlerpQuaternion(startRotation_, targetRotation_, t);
    owner_.lock()->SetQuaternionRotation(q);
}