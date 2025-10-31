#pragma once

#include <vector>

#include "PBDConstaraintData.h"
#include "PBDParticleData.h"

namespace PBD
{
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
            OutputDebugStringA("");

            AddForceToVelocity(deltaTime);

            DampVelocities(deltaTime);

            ExpectedPosition(deltaTime);

            GenerateCollisionConstraints();

            for (int i = 0; i < solveIterationCount; ++i)
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

    private:

        // 速度に外力を加える
        void AddForceToVelocity(float deltaTime)
        {
            XMFLOAT3 force = gravity;
            for (auto& p : particles)
            {
                if (p.IsStatic()) continue;

                p.AddForce(force);
                p.velocity.x += deltaTime * p.force.x * p.invMass;
                p.velocity.y += deltaTime * p.force.y * p.invMass;
                p.velocity.z += deltaTime * p.force.z * p.invMass;
            }
        }

        void DampVelocities(float deltaTime)
        {
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
                c.Solve(particles, solveIterationCount);
            }

            for (auto& c:bendingConstraints)
            {
                c.Solve(particles, solveIterationCount);
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

        XMFLOAT3 gravity = { 0.0f,-9.8f,0.0f };
        int solveIterationCount = 10; // 3 ~ 20
    };

}
