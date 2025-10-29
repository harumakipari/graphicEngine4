#pragma once
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h"

using namespace DirectX;

// ============================
// 質点（MassPoint）
// ============================
class MassPoint
{
public:
    XMFLOAT3 position;
    XMFLOAT3 prevPosition;
    XMFLOAT3 velocity;
    float invMass = 1.0f;
    bool isKinematic = false;

    MassPoint() = default;
    MassPoint(float invMass, XMFLOAT3 pos, bool isKinematic = false)
    {
        Init(invMass, pos, isKinematic);
    }

    void Init(float invMass, XMFLOAT3 pos, bool isKinematic)
    {
        this->invMass = invMass;
        this->position = pos;
        this->prevPosition = pos;
        this->velocity = { 0,0,0 };
        this->isKinematic = isKinematic;
    }

    // Verlet積分（位置予測）
    void UpdatePosition_Verlet(float dt, const XMFLOAT3& accel)
    {
        if (isKinematic) return;

        XMVECTOR pos = XMLoadFloat3(&position);
        XMVECTOR prev = XMLoadFloat3(&prevPosition);
        XMVECTOR a = XMLoadFloat3(&accel);

        // Verlet
        XMVECTOR newPos = XMVectorAdd(pos, XMVectorSubtract(pos, prev));
        newPos = XMVectorAdd(newPos, XMVectorScale(a, dt * dt));

        XMStoreFloat3(&prevPosition, pos);
        XMStoreFloat3(&position, newPos);
    }

    // 速度再計算
    void RecomputeVelocity(float dt)
    {
        if (isKinematic) return;
        velocity.x = (position.x - prevPosition.x) / dt;
        velocity.y = (position.y - prevPosition.y) / dt;
        velocity.z = (position.z - prevPosition.z) / dt;
    }
};

// ============================
// 距離拘束（DistConstraint）
// ============================
class DistConstraints
{
public:
    MassPoint* a = nullptr;
    MassPoint* b = nullptr;
    float restLength = 1.0f;
    float stiffness = 1.0f;

    DistConstraints() = default;
    DistConstraints(MassPoint* a, MassPoint* b, float len, float stiff)
    {
        Init(a, b, len, stiff);
    }

    void Init(MassPoint* a, MassPoint* b, float len, float stiff)
    {
        this->a = a;
        this->b = b;
        this->restLength = len;
        this->stiffness = stiff;
    }

    // 拘束解決（PBDの位置修正）
    void Solve()
    {
        if (!a || !b) return;
        XMVECTOR posA = XMLoadFloat3(&a->position);
        XMVECTOR posB = XMLoadFloat3(&b->position);
        XMVECTOR delta = XMVectorSubtract(posB, posA);

        float dist = XMVectorGetX(XMVector3Length(delta));
        if (dist < 1e-6f) return;

        XMVECTOR dir = XMVectorScale(delta, 1.0f / dist);
        float diff = (dist - restLength) * stiffness;

        float w1 = a->isKinematic ? 0.0f : a->invMass;
        float w2 = b->isKinematic ? 0.0f : b->invMass;
        float wSum = w1 + w2;
        if (wSum <= 0.0f) return;

        XMVECTOR corr = XMVectorScale(dir, diff / wSum);

        if (!a->isKinematic)
            posA = XMVectorAdd(posA, XMVectorScale(corr, w1));
        if (!b->isKinematic)
            posB = XMVectorSubtract(posB, XMVectorScale(corr, w2));

        XMStoreFloat3(&a->position, posA);
        XMStoreFloat3(&b->position, posB);
    }
};

// ============================
// テストActor
// ============================
class TestPBD : public Actor
{
public:
    TestPBD(std::string name) : Actor(name) {}
    virtual ~TestPBD() = default;

    void Initialize(const Transform& transform) override
    {
        const int POINT_NUM = 6;
        const float REST_LEN = 3.0f;
        const float STIFFNESS = 0.1f;

        points_.clear();
        constraints_.clear();

        XMFLOAT3 start = transform.GetLocation();

        for (int i = 0; i < POINT_NUM; i++)
        {
            bool isKinematic = (i == 0); // 一番上を固定
            float invMass = isKinematic ? 0.0f : 1.0f;

            XMFLOAT3 pos = { start.x, start.y - i * REST_LEN, start.z };
            points_.emplace_back(invMass, pos, isKinematic);

            auto mesh = this->NewSceneComponent<SkeletalMeshComponent>("point_" + std::to_string(i));
            mesh->SetModel("./Data/Debug/Primitives/sphere.glb", false);
            mesh->SetWorldLocationDirect(pos);
            meshes_.push_back(mesh.get());
        }

        for (int i = 0; i < POINT_NUM - 1; i++)
        {
            constraints_.emplace_back(&points_[i], &points_[i + 1], REST_LEN, STIFFNESS);
        }
    }

    void Update(float dt) override
    {
        Simulate(dt);

        for (size_t i = 0; i < points_.size(); i++)
        {
            meshes_[i]->SetWorldLocationDirect(points_[i].position);
        }
    }

private:
    void Simulate(float dt)
    {
        const int ITER = 10;
        const XMFLOAT3 GRAV = { 0.0f, -0.98f, 0.0f };

        // 1. Verlet予測
        for (auto& p : points_)
            p.UpdatePosition_Verlet(dt, GRAV);

        // 2. 拘束解決
        for (int i = 0; i < ITER; i++)
        {
            for (auto& c : constraints_)
                c.Solve();
        }

        // 3. 速度再計算
        for (auto& p : points_)
            p.RecomputeVelocity(dt);
    }

private:
    std::vector<MassPoint> points_;
    std::vector<DistConstraints> constraints_;
    std::vector<SkeletalMeshComponent*> meshes_;
};


struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 velocity;
    float mass;     // invMass でも。。
    bool fixed;
};

struct Constraint
{
    float stiffness = 0.5f;// 0.0f~1.0f
    int i, j = 0;

    void ProjectDistanceConstraint(int i, int j, float restLength, float stiffness)
    {

    }
};


