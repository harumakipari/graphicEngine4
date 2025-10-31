#pragma once

#include "Core/Actor.h"
#include "Graphics/Core/ConstantBuffer.h"
#include "Components/CollisionShape/ShapeComponent.h"

class ElasticBuilding : public Actor
{
public:
    ElasticBuilding(const std::string& modelName) :Actor(modelName)
    {
    }
    std::shared_ptr<SkeletalMeshComponent> skeltalMeshComponent;
    std::shared_ptr<SkeletalMeshComponent> pointComponent;
    std::shared_ptr<SkeletalMeshComponent> bezierComponent;
    std::shared_ptr<SkeletalMeshComponent> buildComponent;
    std::shared_ptr<BoxComponent> boxComponent;
    DirectX::XMFLOAT3 p1;
    DirectX::XMFLOAT3 p2;
    DirectX::XMFLOAT3 p3;
    float t = 0.0f;

    struct ElasticBuildingConstants
    {
        DirectX::XMFLOAT4 p1; // �n�_
        DirectX::XMFLOAT4 p2; // ����_
        DirectX::XMFLOAT4 p3; // �I�_
        float buildProgress; // 0.0 ~ 1.0  t
        float buildHeight; // �r���̍���
    };

    std::unique_ptr<ConstantBuffer<ElasticBuildingConstants>> elasticBuildingCBuffer;

    void Initialize(const Transform& transform)override
    {
        // �`��p�R���|�[�l���g��ǉ�
        skeltalMeshComponent = this->NewSceneComponent<class SkeletalMeshComponent>("skeltalComponent");
        skeltalMeshComponent->SetModel("./Data/Debug/Primitives/cube.glb");
        skeltalMeshComponent->SetIsVisible(false);
        pointComponent = this->NewSceneComponent<class SkeletalMeshComponent>("pointComponent");
        pointComponent->SetModel("./Data/Debug/Primitives/sphere.glb");
        pointComponent->SetWorldLocationDirect({ 0.0f,5.0f,0.0f });
        pointComponent->SetIsVisible(false);
        bezierComponent = this->NewSceneComponent<class SkeletalMeshComponent>("bezierComponent");
        bezierComponent->SetModel("./Data/Debug/Primitives/platQube.glb");
        bezierComponent->SetWorldScaleDirect({ 0.2f,0.2f,0.2f });
        bezierComponent->SetIsVisible(false);
        buildComponent = this->NewSceneComponent<class SkeletalMeshComponent>("buildComponent");
        //buildComponent->SetModel("./Data/Models/Building/bomb_bill.gltf");
        buildComponent->SetModel("./Data/Models/Characters/GirlSoldier/Idle.gltf");
        //buildComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::RH_Y_UP;
        //buildComponent->model->isModelInMeters = true;
        //buildComponent->SetWorldLocationDirect({ 4.0f,0.0f,0.0f });
        buildComponent->overridePipelineName = "elasticBuilding";
        buildComponent->overrideCascadeShadowPipelineName = "CascadeShadowMapElasticBuilding";
        buildComponent->SetIsVisible(false);
        elasticBuildingCBuffer = std::make_unique<ConstantBuffer<ElasticBuildingConstants>>(Graphics::GetDevice());
        AABB box = buildComponent->model->GetAABB();
        DirectX::XMFLOAT3 size = { box.max.x - box.min.x,box.max.y - box.min.y,box.max.z - box.min.z };
        boxComponent = this->NewSceneComponent<class BoxComponent>("boxComponent", "skeltalComponent");
        boxComponent->SetModelHeight(size.y);
        //boxComponent->SetLayer(CollisionLayer::WorldStatic);
        //boxComponent->SetResponseToLayer(CollisionLayer::Player, CollisionComponent::CollisionResponse::Block);
        boxComponent->SetHalfBoxExtent({ size.x * 0.5f,size.y * 0.5f,size.z * 0.5f });
        boxComponent->Initialize();


        p1 = transform.GetLocation();
        p3 = { GetPosition().x,GetPosition().y + buildHeight,GetPosition().z };

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());
    }

    void Update(float deltaTime)override
    {
        using namespace DirectX;
        // angle �ɂ���āA�ʒu��ݒ肷��
        SetPosByAngle(DirectX::XMConvertToRadians(angle));
        // �x�W�F�Ȑ��̌v�Z
        if (pointComponent)
        {
            DirectX::XMFLOAT3 bezierPos = GetBezierPoint(p1, { p1.x, p1.y + buildHeight,p1.z }, pointComponent->GetComponentLocation(), t);
            if (bezierComponent)
            {
                bezierComponent->SetWorldLocationDirect(bezierPos);
            }
        }
        if (pointComponent)
        {
            // �x�W�F�Ȑ��̐ڐ�
            DirectX::XMFLOAT3 tangent = GetBezierTangent(p1, { p1.x, p1.y + buildHeight,p1.z }, pointComponent->GetComponentLocation(), t);
            XMVECTOR Tangent = DirectX::XMLoadFloat3(&tangent);
            DirectX::XMVECTOR Quat = FromToRotation(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), Tangent);
            DirectX::XMFLOAT4 rot;
            DirectX::XMStoreFloat4(&rot, Quat);
            if (bezierComponent)
            {
                bezierComponent->SetRelativeRotationDirect(rot);
            }
        }
        elasticBuildingCBuffer->data.buildProgress = t;
        elasticBuildingCBuffer->data.p1 = { p1.x,p1.y,p1.z,1.0f };
#if 1
        elasticBuildingCBuffer->data.p2 = { p2.x,p2.y,p2.z,1.0f };
        elasticBuildingCBuffer->data.p3 = { p3.x,p3.y,p3.z,1.0f };
#else
        elasticBuildingCBuffer->data.p2 = { p1.x,p1.y + buildHeight,p1.z,1.0f };
        elasticBuildingCBuffer->data.p3 = { pointComponent->GetComponentLocation().x,pointComponent->GetComponentLocation().y,pointComponent->GetComponentLocation().z,1.0f };

#endif // 0
        elasticBuildingCBuffer->data.buildHeight = buildHeight;
        // ���ŁE�E
        elasticBuildingCBuffer->Activate(Graphics::GetDeviceContext(), 6);


        AABB box = buildComponent->model->GetAABB();
        DirectX::XMFLOAT3 size = { box.max.x - box.min.x,box.max.y - box.min.y,box.max.z - box.min.z };
        //ShapeRenderer::Instance().DrawBox(Graphics::GetDeviceContext(), buildComponent->GetComponentWorldTransform().ToWorldTransform(), size, {1.0f,0.0f,1.0f,1.0f});
        int a = 0;
    }



    void DrawImGuiDetails()
    {
#ifdef USE_IMGUI
        ImGui::DragFloat("buildHeight", &buildHeight, 0.1f, 1.0f, 20.0f);
        ImGui::DragFloat("angle", &angle, 1.0f, -90.0f, 90.0f);
        ImGui::DragFloat("t", &t, 0.01f, 0.0f, 1.0f);
#endif
    };

    // �񎟃x�W�F�Ȑ�
    DirectX::XMFLOAT3 GetBezierPoint(DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, DirectX::XMFLOAT3 p3, float t)
    {
        float r = 1.0f - t;
        DirectX::XMFLOAT3 p;
        p = { r * r * p1.x + 2 * r * t * p2.x + t * t * p3.x,
            r * r * p1.y + 2 * r * t * p2.y + t * t * p3.y,
            r * r * p1.z + 2 * r * t * p2.z + t * t * p3.z };
        return p;
    }

    // �x�W�F�Ȑ��̐ڐ�
    DirectX::XMFLOAT3 GetBezierTangent(DirectX::XMFLOAT3 p1, DirectX::XMFLOAT3 p2, DirectX::XMFLOAT3 p3, float t)
    {
        DirectX::XMFLOAT3 tangent;
        tangent =
        { 2 * (1 - t) * (p2.x - p1.x) + 2 * t * (p3.x - p2.x),
            2 * (1 - t) * (p2.y - p1.y) + 2 * t * (p3.y - p2.y),
            2 * (1 - t) * (p2.z - p1.z) + 2 * t * (p3.z - p2.z) };
        return tangent;
    }

    // �r���̍���
    float buildHeight = 5.0f;
    // P1 �� posistion 
    // P2 �� P1 ���� buildHeight ��ɏオ�����ʒu

    // ����𔼌a�Ƃ����������쐬����
    float angle = 0.0f;

    void SetPosByAngle(float angle)
    {
        float x = sinf(angle) * buildHeight;
        float y = cosf(angle) * buildHeight;
        //float z = sinf(angle) * buildHeight;
        float z = 0.0f;
        if (pointComponent)
        {
            pointComponent->SetRelativeLocationDirect({ x,y,z });
        }
    }

    DirectX::XMVECTOR FromToRotation(DirectX::XMVECTOR from, DirectX::XMVECTOR to)
    {
        using namespace DirectX;
        // ���K��
        DirectX::XMVECTOR f = DirectX::XMVector3Normalize(from);
        DirectX::XMVECTOR t = DirectX::XMVector3Normalize(to);

        float dot = DirectX::XMVectorGetX(XMVector3Dot(f, t));
        if (dot > 0.99999f)
        {
            return XMQuaternionIdentity();
        }

        // �^�t�̕����������灨�C�ӂ̎���180�x��]
        if (dot < -0.99999f)
        {
            // f �ɐ����ȃx�N�g����T��
            XMVECTOR axis = XMVector3Cross(f, XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f));
            if (XMVector3LengthSq(axis).m128_f32[0] < 0.0001f)
            {
                axis = XMVector3Cross(f, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
            }
            axis = XMVector3Normalize(axis);
            return XMQuaternionRotationAxis(axis, XM_PI);   // 180 �x
        }
        // ��ʓI�ȃP�[�X
        XMVECTOR axis = XMVector3Cross(f, t);
        float s = sqrtf((1.0f + dot) * 2.0f);
        float invs = 1.0f / s;

        XMVECTOR quat = XMVectorSet(XMVectorGetX(axis) * invs, XMVectorGetY(axis) * invs,
            XMVectorGetZ(axis) * invs, s * 0.5f);

        return XMQuaternionNormalize(quat);
    }

};


class SpringyBall : public Actor
{
public:
    SpringyBall(const std::string& modelName) :Actor(modelName)
    {
    }
    std::shared_ptr<SkeletalMeshComponent> pointComponent;

    float momentum = -1.0f;  // �����o�b�t�@
    float speed = 4.0f;     // �{�[���̍d��
    float ballPos = 0.0f; // ���݂̍���
    float damping = 0.95f;   // ������

    void Initialize(const Transform& transform)override
    {
        // �`��p�R���|�[�l���g��ǉ�
        pointComponent = this->NewSceneComponent<class SkeletalMeshComponent>("pointComponent");
        pointComponent->SetModel("./Data/Debug/Primitives/sphere.glb");

        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        ballPos = transform.GetLocation().y;
    }

    void Update(float deltaTime)override
    {
        using namespace DirectX;
        XMFLOAT3 pos = GetPosition();
        Setp(deltaTime);
        pos.y = ballPos;
        SetPosition(pos);
    }

    void Setp(float deltaTime)
    {
        float targetHeight = flag ? 3.0f : 7.0f;
        float grad = ballPos - targetHeight;// x - a
        momentum = damping * momentum + grad/*parmator*/;
        ballPos -= deltaTime * speed * momentum;
    }

    void DrawImGuiDetails()
    {
#ifdef USE_IMGUI
        ImGui::Checkbox("frag", &flag);
#endif
    };

    bool flag = false;
};