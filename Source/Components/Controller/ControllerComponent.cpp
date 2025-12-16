#include "pch.h"
#include "ControllerComponent.h"

#include "Core/Actor.h"
#include "Core/ActorManager.h"
#include "Game/Actors/Base/Character.h"
#include "Engine/Utility/Win32Utils.h"
#include "Physics/CollisionSystem.h"

DirectX::XMFLOAT3 KinematicMovementComponent::MoveAndSlide(const DirectX::XMFLOAT3& desiredMove)
{
    using namespace DirectX;
    using namespace physx;

    PxScene* pxScene = Physics::Instance().GetScene();

    if (!shapeComponent_||!pxScene)
    {
        return desiredMove;
    }

    // 移動量が小さすぎる場合は無視
    XMVECTOR v = XMLoadFloat3(&desiredMove);
    if (XMVector3LengthSq(v).m128_f32[0] < 1e-6f)
        return desiredMove;
    PxTransform currentTransform;
    {
        auto owner = shapeComponent_->GetOwner();
        auto pos = owner->GetPosition();
        auto rot = owner->GetQuaternionRotation();

        currentTransform = PxTransform(
            PxVec3(pos.x, pos.y, pos.z),
            PxQuat(rot.x, rot.y, rot.z, rot.w)
        );
    }
    auto info = shapeComponent_->GetPhysicsShapeInfo();

    PxVec3 direction(
        desiredMove.x,
        desiredMove.y,
        desiredMove.z);

    float distance = direction.normalize(); // direction 正規化 + 距離取得
    if (distance <= 0.0001f)
        return desiredMove;

    PxSweepBuffer hit;
    PxQueryFilterData filterData;
    filterData.flags = PxQueryFlag::eSTATIC; // TriangleMesh は Static

    bool blocked = pxScene->sweep(
        info.geometry.any(),
        currentTransform,
        direction,
        distance,
        hit,
        PxHitFlag::eDEFAULT,
        filterData);

    if (hit.hasBlock&&hit.block.distance<=0.0001f)
    {
        return { 0.0f,0.0f,0.0f };
    }

    // ヒットなし → そのまま移動
    if (!blocked || !hit.hasBlock)
        return desiredMove;

    // めり込む直前まで移動
    float safeDist = PxMax(hit.block.distance - 0.001f, 0.0f);

    XMFLOAT3 result;
    result.x = direction.x * safeDist;
    result.y = direction.y * safeDist;
    result.z = direction.z * safeDist;

    // Slide 計算
    PxVec3 n = hit.block.normal;
    PxVec3 moveDir(
        desiredMove.x,
        desiredMove.y,
        desiredMove.z);

    // 法線方向成分を削除
    PxVec3 slide =
        moveDir - n * moveDir.dot(n);

#if 1
    // 再帰 or 1回だけスライド（まずは1回）
    result.x += slide.x * 0.5f;
    result.y += slide.y * 0.5f;
    result.z += slide.z * 0.5f;
#else
    slide.normalize();
    slide *= (distance - safeDist);

    result.x += slide.x;
    result.y += slide.y;
    result.z += slide.z;
#endif // 0

    return result;
}

MovementComponent::MovementComponent(const std::string& name, const std::shared_ptr<Actor>& owner)
    :SceneComponent(name, owner)
{
    kinematicMove_ = std::make_unique<KinematicMovementComponent>();
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

    DirectX::XMFLOAT3 dir = input->GetMoveInput();

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


    // 衝突システムから押し出しベクトルを取得
    DirectX::XMFLOAT3 pushVec = CollisionSystem::GetPushVectorForActor(owner_.lock().get());

    // 合算
    sweptMove.x += pushVec.x;
    sweptMove.y += pushVec.y;
    sweptMove.z += pushVec.z;

    //moveVec.x += pushVec.x;
    //moveVec.y += pushVec.y;
    //moveVec.z += pushVec.z;

    //R.r[0] = DirectX::XMVectorScale(R.r[0], dir.x);
    //R.r[1] = DirectX::XMVectorScale(R.r[1], dir.y);
    //R.r[2] = DirectX::XMVectorScale(R.r[2], dir.z);
    //DirectX::XMVECTOR q = DirectX::XMQuaternionRotationMatrix(R);
    //DirectX::XMFLOAT4 quaternion;
    //DirectX::XMStoreFloat4(&quaternion, q);
    //owner_->rootComponent_->SetLocalRotation(quaternion);

    owner_.lock()->rootComponent_->AddWorldOffset(sweptMove);


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


    owner_.lock()->rootComponent_->AddWorldOffset(moveVec);


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