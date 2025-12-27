#pragma once
#include "Cherry.h"
#include "Core/Actor.h"

#include "Components/Effect/ParticleComponent.h"
#include "Components/Elastic/ElasticComponent.h"
#include "Engine/Audio/CoreAudio.h"
#include "Engine/Input/InputSystem.h"
#include "Physics/CollisionFunction.h"

class Pudding : public Actor
{
public:
    Pudding(const std::string& modelName) :Actor(modelName)
    {
    }
    std::shared_ptr<ElasticMeshComponent> elasticBuilding;
    std::shared_ptr<SkeletalMeshComponent> cherry;
    std::shared_ptr<SkeletalMeshComponent> whip;
    std::shared_ptr<ParticleComponent> particleComponent;
    std::shared_ptr<CoreAudioSourceComponent> audioSourceComponent;

    void Initialize(const Transform& transform)override
    {
        // 描画用コンポーネントを追加
        elasticBuilding = this->AddComponent<ElasticMeshComponent>("elasticBuilding");
        //elasticBuilding->SetModel("./Data/Models/Building/bomb_bill.gltf");
        //elasticBuilding->SetModel("./Data/Models/pink_pudding/scene.gltf");
        //elasticBuilding->SetModel("./Data/Models/cherry_pudding/scene.gltf");
        elasticBuilding->SetModel("./Data/Models/cherry_pudding/pudding.glb");

        std::shared_ptr<BoxComponent> boxComponent = this->AddComponent<class BoxComponent>("boxComponent", "elasticBuilding");
        DirectX::XMFLOAT3 size = elasticBuilding->GetModelSize();
        boxComponent->SetBoxExtent({ size.x * 0.5f,size.y * 0.5f,size.z * 0.5f });
        boxComponent->SetMass(40.0f);
        boxComponent->SetLayer(CollisionLayer::Enemy);
        //boxComponent->SetCollisionOffsetY(size.y * 0.5f);
        boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
        boxComponent->Initialize();

        whip = this->AddComponent<SkeletalMeshComponent>("whip", "elasticBuilding");
        whip->SetModel("./Data/Models/cherry_pudding/whip.glb");
        whip->SetRelativeLocationDirect({ 0.0f,size.y,0.0f });

        cherry = this->AddComponent<SkeletalMeshComponent>("cherry", "whip");
        cherry->SetModel("./Data/Models/cherry_pudding/cherry.glb");


        particleComponent = this->AddComponent<class ParticleComponent>("particleComponent", "elasticBuilding");
        particleComponent->Load("./Data/Effect/Files/testEffect.json");

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        elasticBuilding->Initialize();

#if 0
        arrowGauge = std::make_shared<UIGaugeComponent>("./Data/Textures/UI/arrow.png", "arrowGauge");
        arrowGauge->position = { 50, 300 };
        arrowGauge->pivot = { 0.5f,0.5f };
        arrowGauge->size = { 350, 200 };
        arrowGauge->value = 1.0f;
#else
        arrow = std::make_shared<UIImageComponent>("./Data/Textures/UI/triangle.png", "arrowGauge");
        arrow->SetWorldPosition({ 50, 300 });
        arrow->SetPivot({ 0.5f,0.5f });
        arrow->SetSize({ 50, 50 });
#endif // 0

        GetOwnerScene()->GetUIManager()->Add(arrow);

        audioSourceComponent = AddComponent<CoreAudioSourceComponent>("audioComponent", "elasticBuilding");
        audioSourceComponent->SetSource(L"./Data/Sound/SE/stretch_long.wav");

    }
    void Update(float deltaTime)override
    {

        const auto& pull = elasticBuilding->GetPullInfo();

        if (InputSystem::GetInputState("MouseLeft"))
        {
            audioSourceComponent->Play();
            if (audioSourceComponent && !audioSourceComponent->IsPlaying())
            {
                audioSourceComponent->Play();
            }

        }

        if (InputSystem::GetInputState("MouseLeft", InputStateMask::Release))
        {
            if (particleComponent)
            {
                particleComponent->Play();
                CoreAudio::PlayOneShot(L"./Data/Sound/SE/pudding.wav");

            }
        }
        XMFLOAT3 surfacePos, tangent;
        elasticBuilding->GetSurfacePositionTangent(surfacePos, tangent);
        whip->SetWorldLocationDirect(surfacePos);

        // ===== 回転 =====
        XMVECTOR yAxis = XMVector3Normalize(XMLoadFloat3(&tangent));

        // fallback（真上を向いてるとき用）
        XMVECTOR worldForward = XMVectorSet(0, 0, 1, 0);
        if (fabsf(XMVectorGetX(XMVector3Dot(yAxis, worldForward))) > 0.99f)
        {
            worldForward = XMVectorSet(0, 0, 1, 0);
        }

        // X軸 = forward × Y
        XMVECTOR xAxis = XMVector3Normalize(XMVector3Cross(yAxis, worldForward));

        // Z軸 = Y × X
        XMVECTOR zAxis = XMVector3Cross(xAxis, yAxis);

        // 回転行列
        XMMATRIX rotMatrix =
        {
            xAxis,
            yAxis,
            zAxis,
            XMVectorSet(0, 0, 0, 1)
        };

        XMVECTOR rotQuat = XMQuaternionRotationMatrix(rotMatrix);
        DirectX::XMFLOAT4 rot;
        XMStoreFloat4(&rot, rotQuat);
        whip->SetRelativeRotationDirect(rot);

        // 逆方向
        XMVECTOR dir = -XMVector3Normalize(XMLoadFloat3(&tangent));

        // 上方向を少し足す（放物線っぽさ）
        XMVECTOR launchDir =
            XMVector3Normalize(dir + XMVectorSet(0, 0.5f, 0, 0));

        // ===== 引っ張り量 =====
        float pullAmount = pull.amount;
        Logger::Log((std::string("サクランボの引っ張った量") + std::to_string(pullAmount)).c_str());


        // ちょい溜めると一気に強くなる
        float power = std::lerp(minPower, maxPower, pullAmount);

        XMVECTOR velocity = launchDir * power;

        XMFLOAT3 vel;
        XMStoreFloat3(&vel, velocity);

        if (InputSystem::GetInputState("MouseLeft", InputStateMask::Release))
        {
            cherry->SetIsVisible(false);
            XMFLOAT3 pos = cherry->GetComponentLocation();
            Transform cherryTr{ pos,{0.0f,0.0f,0.0f},{1.0f,1.0f,1.0f} };
            auto flyingCherry = GetOwnerScene()->GetActorManager()->CreateAndRegisterActorWithTransform<Cherry>("cherry", cherryTr);
            if (flyingCherry.get())
            {
                flyingCherry->Launch(surfacePos, vel);
            }
        }

        XMFLOAT3 launch;
        XMStoreFloat3(&launch, launchDir);

        // XZ 平面に落とす
        XMFLOAT2 dir2D = { launch.x, launch.z };

        // normalize
        float len = sqrtf(dir2D.x * dir2D.x + dir2D.y * dir2D.y);
        if (len > 0.0001f)
        {
            dir2D.x /= len;
            dir2D.y /= len;
        }

        // UI角度（右=0°, 下=+）
        float angleRad = atan2f(-dir2D.y, dir2D.x);
        float angleDeg = DirectX::XMConvertToDegrees(angleRad);


        if (pull.active)
        {
            arrow->SetVisible(true);

            // === ワールド → スクリーン ===
            XMFLOAT3 worldTop = surfacePos;

            XMFLOAT2 screenPos =
                CollisionFunction::GetScreenPositionFromWorldPosition(worldTop);

            arrow->SetWorldPosition(
                {
                    screenPos.x,
                    screenPos.y
                });

            arrow->SetWorldAngleDegree(angleDeg);
            //arrowGauge->value = pull.amount;
        }
        else
        {
            //arrow->visible = false;
        }

    }
    void DrawImGuiDetails()
    {
#ifdef USE_IMGUI
        ImGui::DragFloat(U8("サクランボの最小速度"), &minPower, 0.02f);
        ImGui::DragFloat(U8("サクランボの最大速度"), &maxPower, 0.02f);
#endif
    };

private:
    float minPower = 2.0f;
    float maxPower = 10.0f;

    std::shared_ptr<UIGaugeComponent> arrowGauge;
    std::shared_ptr<UIImageComponent> arrow;
};
