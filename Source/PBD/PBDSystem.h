#pragma once

#include <vector>

#include "PBDConstaraintData.h"
#include "PBDParticleData.h"

namespace PBD
{
    class System
    {
    public:
        // ����Vertex��3.1(2)initialize
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
            c.Initialize(particles); // �����p�x���v�Z����
            bendingConstraints.push_back(c);
        }

        void Update(float deltaTime)
        {
            AddForceToVelocity(deltaTime);

            DampVelocities(deltaTime, damping);

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
        std::vector<Particle>& GetParticles()  { return particles; }


        void DebugRender(ID3D11DeviceContext* immediateContext)
        {
            const auto& p = GetParticles();

            // ���q��`��
            for (const auto& particle : p)
                ShapeRenderer::DrawPoint(immediateContext, particle.position, { 1,0,1,1 });

            // �S����`��
            for (const auto& c : distanceConstraints)
            {
                ShapeRenderer::DrawLineSegment(immediateContext,p[c.i0].position, p[c.i1].position, { 1,1,0,1 });
            }

            for (const auto& b : bendingConstraints)
            {
                ShapeRenderer::DrawLineSegment(immediateContext, p[b.p1].position, p[b.p2].position, { 0,1,1,1 });
                ShapeRenderer::DrawLineSegment(immediateContext, p[b.p1].position, p[b.p3].position, { 0,1,1,1 });
                ShapeRenderer::DrawLineSegment(immediateContext, p[b.p1].position, p[b.p4].position, { 0,1,1,1 });
                ShapeRenderer::DrawLineSegment(immediateContext, p[b.p2].position, p[b.p3].position, { 0,1,1,1 });
            }
        }
    private:

        // ���x�ɊO�͂�������
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

        //�ʒu�\������O�ɑ��x������������ 3.5Damping
        void DampVelocities(float deltaTime, float damping)
        {
            using namespace DirectX;

            if (particles.empty()) return;

            float totalMass = 0.0f;
            XMVECTOR xcm = XMVectorZero();
            XMVECTOR vcm = XMVectorZero();

            // (1) �d�S�ʒu�����߂�
            // (2) �d�S���x�̌v�Z
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

            // (3)�@L�@�S�̂̊p�^���ʁ@�@L = cross(r,p) �ʒu�x�N�g���A�^���x�N�g��
            XMVECTOR L = XMVectorZero();

            // (4) �����e���\�������߂�@x,y,z�ǂ̎��ɑ΂��Ă���]�̂��ɂ�����\�������Ƃ�
            XMFLOAT3X3 I = {}; // �����e���\��
            float I_[3][3] = { 0 };

            for (auto& p : particles)
            {
                if (p.invMass == 0.0f) continue;

                float m = 1.0f / p.invMass;
                XMVECTOR xi = XMLoadFloat3(&p.position);
                XMVECTOR vi = XMLoadFloat3(&p.velocity);
                XMVECTOR ri = XMVectorSubtract(xi, xcm);//ri �_���ǂꂭ�炢�d�S�Ɨ���Ă��邩�A

                L = L + XMVector3Cross(ri, XMVectorScale(vi, m)); // �S�̂̊p�^����

                // �����e���\��
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

            // (5) �p���x�@L=Iw���@w=I^-1*L
            XMMATRIX I_mat = XMLoadFloat3x3(reinterpret_cast<XMFLOAT3X3*>(I_));
            XMVECTOR det;
            XMMATRIX I_inv = XMMatrixInverse(&det, I_mat); // I^-1 �t�s��
            if (fabsf(XMVectorGetX(det)) < 1e-6f) return; // �s���ȍs����

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
                XMVECTOR ri = XMVectorSubtract(xi, xcm);//ri �_���ǂꂭ�炢�d�S�Ɨ���Ă��邩

                XMVECTOR deltaVi = vcm + XMVector3Cross(w, ri) - vi; //(7)
                vi = XMVectorAdd(vi, XMVectorScale(deltaVi, damping));
                XMStoreFloat3(&p.velocity, vi); // (8)
            }
        }



        // �����I�I�C���[�ϕ��ŐV�����ʒu�����ς���
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

        // �ՓˍS���֐��𐶐�
        void GenerateCollisionConstraints()
        {
            SolveFloorConstraint();
        }

        // �S���𔽉f����֐��@�E�E�����solverIteration����
        void ProjectConstraints()
        {
            for (auto& c : distanceConstraints)
            {
                c.Solve(particles, solveIterationCount);
            }

            for (auto& c : bendingConstraints)
            {
                c.Solve(particles, solveIterationCount);
            }
        }

        // ���x���v�Z������
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

        // friction restitution �Ƃ��ɉ����ĕύX
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
        //float damping = 0.05f;
        float damping = 0.0f;
    };

}
