#pragma once

#include <vector>

#include "PBDConstaraintData.h"
#include "PBDParticleData.h"

namespace PBD
{
    // どこかにグローバルで置く
    struct PBDParams
    {
        int iterationCount = 5;
        float distanceStiffness = 1.0f;
        float bendingStiffness = 0.5f;
        float damping = 0.98f;
        XMFLOAT3 externalForce = { 0.0f, -9.8f, 0.0f }; // 重力
    };


    class System
    {
    public:
        // これVertexの3.1(2)initialize
        void AddParticle(const DirectX::XMFLOAT3& pos, float mass)
        {
            Particle p;
            p.position = pos;
            p.expectedPosition = pos;
            p.invMass = (mass > 0.0f) ? 1.0f / mass : 0.0f;
            particles.push_back(p);
        }

        void AddBendingConstraint(int i1, int i2, int i3, int i4, float stiffness)
        {
            BendingConstraint c(i1, i2, i3, i4, stiffness);
            c.Initialize(particles); // 初期角度を計算する
            bendingConstraints.push_back(c);
        }

        void Update(float deltaTime)
        {
            AddForceToVelocity(deltaTime);

            DampVelocities(deltaTime, gPbdParams.damping);

            ExpectedPosition(deltaTime);

            GenerateCollisionConstraints();

            for (int i = 0; i < gPbdParams.iterationCount; ++i)
            {
                ProjectConstraints();
            }

            CalculateVelocities(deltaTime);

            UpdateVelocity(deltaTime);
        }

        void AddDistanceConstraints(int i1, int i2, float stiffness)
        {
            XMFLOAT3 diff =
            {
                particles[i1].position.x - particles[i2].position.x,
                particles[i1].position.y - particles[i2].position.y,
                particles[i1].position.z - particles[i2].position.z,
            };

            float restLength = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
            distanceConstraints.emplace_back(i1, i2, restLength, stiffness);
        }

        const std::vector<Particle>& GetParticles() const { return particles; }
        std::vector<Particle>& GetParticles() { return particles; }


        void DebugRender(ID3D11DeviceContext* immediateContext)
        {
            const auto& p = GetParticles();

            // 粒子を描く
            for (const auto& particle : p)
                ShapeRenderer::DrawPoint(immediateContext, particle.position, { 1,0,1,1 });

            // 拘束を描く
            for (const auto& c : distanceConstraints)
            {
                ShapeRenderer::DrawLineSegment(immediateContext, p[c.i0].position, p[c.i1].position, { 1,1,0,1 });
            }

            for (const auto& b : bendingConstraints)
            {
                ShapeRenderer::DrawLineSegment(immediateContext, p[b.p1].position, p[b.p2].position, { 0,1,1,1 });
                ShapeRenderer::DrawLineSegment(immediateContext, p[b.p1].position, p[b.p3].position, { 0,1,1,1 });
                ShapeRenderer::DrawLineSegment(immediateContext, p[b.p1].position, p[b.p4].position, { 0,1,1,1 });
                ShapeRenderer::DrawLineSegment(immediateContext, p[b.p2].position, p[b.p3].position, { 0,1,1,1 });
            }
        }

        void DrawGui()
        {
#ifdef USE_IMGUI
            ImGui::Begin("pbd parameter");
            ImGui::SliderInt("Iterations", &gPbdParams.iterationCount, 1, 20);
            ImGui::SliderFloat("Distance Stiffness", &gPbdParams.distanceStiffness, 0.0f, 1.0f);
            ImGui::SliderFloat("Bending Stiffness", &gPbdParams.bendingStiffness, 0.0f, 1.0f);
            ImGui::SliderFloat("Damping", &gPbdParams.damping, 0.0f, 1.0f);
            ImGui::DragFloat3("External Force", &gPbdParams.externalForce.x, 0.001f);
            ImGui::End();
#endif
        }
    private:

        // 速度に外力を加える
        void AddForceToVelocity(float deltaTime)
        {
            XMFLOAT3 force = gPbdParams.externalForce;
            for (auto& p : particles)
            {
                if (p.IsStatic()) continue;

                p.AddForce(force);
                p.velocity.x += deltaTime * p.force.x * p.invMass;
                p.velocity.y += deltaTime * p.force.y * p.invMass;
                p.velocity.z += deltaTime * p.force.z * p.invMass;
            }
        }

        //位置予測する前に速度を減衰させる 3.5Damping
        void DampVelocities(float deltaTime, float damping)
        {
            using namespace DirectX;

            if (particles.empty()) return;

            float totalMass = 0.0f;
            XMVECTOR xcm = XMVectorZero();
            XMVECTOR vcm = XMVectorZero();

            // (1) 重心位置を求める
            // (2) 重心速度の計算
            for (auto& p : particles)
            {
                if (p.invMass == 0.0f) continue;

                float m = 1.0f / p.invMass;
                totalMass += m;
                XMVECTOR pos = XMLoadFloat3(&p.position);
                XMVECTOR vel = XMLoadFloat3(&p.velocity);
                xcm = XMVectorAdd(xcm, XMVectorScale(pos, m));
                vcm = XMVectorAdd(vcm, XMVectorScale(vel, m));
            }

            xcm = XMVectorScale(xcm, 1.0f / totalMass);
            vcm = XMVectorScale(vcm, 1.0f / totalMass);

            // (3)　L　全体の角運動量　　L = cross(r,p) 位置ベクトル、運動ベクトル
            XMVECTOR L = XMVectorZero();

            // (4) 慣性テンソルを求める　x,y,zどの軸に対しても回転のしにくさを表したいとき
            XMFLOAT3X3 I = {}; // 慣性テンソル
            float I_[3][3] = { 0 };

            for (auto& p : particles)
            {
                if (p.invMass == 0.0f) continue;

                float m = 1.0f / p.invMass;
                XMVECTOR xi = XMLoadFloat3(&p.position);
                XMVECTOR vi = XMLoadFloat3(&p.velocity);
                XMVECTOR ri = XMVectorSubtract(xi, xcm);//ri 点がどれくらい重心と離れているか、

                L = L + XMVector3Cross(ri, XMVectorScale(vi, m)); // 全体の角運動量

                // 慣性テンソル
                XMFLOAT3 r;
                XMStoreFloat3(&r, ri);
                float r2 = r.x * r.x + r.y * r.y + r.z * r.z;

                I_[0][0] += m * (r2 - r.x * r.x);
                I_[1][1] += m * (r2 - r.y * r.y);
                I_[2][2] += m * (r2 - r.z * r.z);
                I_[0][1] -= m * (r.x * r.y);
                I_[0][2] -= m * (r.x * r.z);
                I_[1][0] -= m * (r.y * r.x);
                I_[1][2] -= m * (r.y * r.z);
                I_[2][0] -= m * (r.z * r.x);
                I_[2][1] -= m * (r.z * r.y);

            }

            // (5) 角速度　L=Iwより　w=I^-1*L
            XMMATRIX I_mat = XMLoadFloat3x3(reinterpret_cast<XMFLOAT3X3*>(I_));
            XMVECTOR det;
            XMMATRIX I_inv = XMMatrixInverse(&det, I_mat); // I^-1 逆行列
            if (fabsf(XMVectorGetX(det)) < 1e-6f) return; // 不正な行列回避

            XMVECTOR w = XMVector3TransformNormal(L, I_inv);


            // (6) ~ (9)
            for (auto& p : particles)
            {
                if (p.IsStatic())
                {
                    continue;
                }

                XMVECTOR xi = XMLoadFloat3(&p.position);
                XMVECTOR vi = XMLoadFloat3(&p.velocity);
                XMVECTOR ri = XMVectorSubtract(xi, xcm);//ri 点がどれくらい重心と離れているか

                XMVECTOR deltaVi = vcm + XMVector3Cross(w, ri) - vi; //(7)
                vi = XMVectorAdd(vi, XMVectorScale(deltaVi, damping));
                XMStoreFloat3(&p.velocity, vi); // (8)
            }
        }



        // 明示的オイラー積分で新しい位置を見積もる
        void ExpectedPosition(float deltaTime)
        {
            for (auto& p : particles)
            {
                p.expectedPosition.x = p.position.x + deltaTime * p.velocity.x;
                p.expectedPosition.y = p.position.y + deltaTime * p.velocity.y;
                p.expectedPosition.z = p.position.z + deltaTime * p.velocity.z;

                p.ClearForce();
            }
        }

        // 衝突拘束関数を生成
        void GenerateCollisionConstraints()
        {
            SolveFloorConstraint();
        }

        // 拘束を反映する関数　・・これをsolverIteration分回す
        void ProjectConstraints()
        {
            for (auto& c : distanceConstraints)
            {
                c.Solve(particles, gPbdParams.iterationCount, gPbdParams.distanceStiffness);
            }

            for (auto& c : bendingConstraints)
            {
                c.Solve(particles, gPbdParams.iterationCount, gPbdParams.bendingStiffness);
            }
        }

        // 速度を計算し直す
        void CalculateVelocities(float deltaTime)
        {
            for (auto& p : particles)
            {
                p.velocity.x = (p.expectedPosition.x - p.position.x) / deltaTime;
                p.velocity.y = (p.expectedPosition.y - p.position.y) / deltaTime;
                p.velocity.z = (p.expectedPosition.z - p.position.z) / deltaTime;

                p.position = p.expectedPosition;
            }
        }

        // friction restitution とかに応じて変更
        void UpdateVelocity(float deltaTime)
        {

        }

    private:
        void SolveFloorConstraint()
        {
            float floorY = 0.0f;
            for (auto& p : particles)
            {
                if (p.expectedPosition.y < floorY)
                {
                    p.expectedPosition.y = floorY;
                }
            }
        }

        std::vector<Particle> particles;
        std::vector<DistanceConstraint> distanceConstraints;
        std::vector<BendingConstraint> bendingConstraints;

        PBDParams gPbdParams;
        //XMFLOAT3 gravity = { 0.0f,-9.8f,0.0f };
        //int solveIterationCount = 3; // 3 ~ 20
        //float damping = 0.05f;
    };

}
