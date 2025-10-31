#pragma once
#include <DirectXMath.h>
#include <vector>
#include "PBDParticleData.h"

namespace PBD
{
    struct DistanceConstraint
    {
        int p1;
        int p2;

        float restLength;
        float stiffness;    // 0.0f~1.0f 1.0f�̂ق����S�����ł�

        DistanceConstraint(int i1, int i2, float length, float k = 1.0f) : p1(i1), p2(i2), restLength(length), stiffness(k) {}

        void Solve(std::vector<Particle>& particles, int iterationCount) const
        {
            auto& pA = particles[p1];
            auto& pB = particles[p2];

            if (pA.IsStatic() && pB.IsStatic()) return;

            // p4 Figure2
            XMFLOAT3 dir =
            {
                pA.expectedPosition.x - pB.expectedPosition.x,
                pA.expectedPosition.y - pB.expectedPosition.y,
                pA.expectedPosition.z - pB.expectedPosition.z,
            };

            float dist = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
            if (dist < 1e-6f)return;

            // Constraint
            float C = dist - restLength;

            float w1 = pA.invMass;
            float w2 = pB.invMass;

            float invSum = w1 + w2;

            // ���K������
            XMFLOAT3 n = { dir.x / dist,dir.y / dist,dir.z / dist };

            // stiffness��␳����@�_���ɏ����Ă��A�A
            //float kPrime = 1.0f - powf(1.0f - stiffness, 1.0f / iterationCount);
            float kPrime =stiffness;


            XMFLOAT3 deltaPA =
            {
                 (-w1 / invSum) * C * n.x,
                 (-w1 / invSum) * C * n.y,
                 (-w1 / invSum) * C * n.z
            };

            XMFLOAT3 deltaPB =
            {
                 (w2 / invSum) * C * n.x,
                 (w2 / invSum) * C * n.y,
                 (w2 / invSum) * C * n.z
            };

            pA.expectedPosition.x += deltaPA.x * kPrime;
            pA.expectedPosition.y += deltaPA.y * kPrime;
            pA.expectedPosition.z += deltaPA.z * kPrime;

            pB.expectedPosition.x += deltaPB.x * kPrime;
            pB.expectedPosition.y += deltaPB.y * kPrime;
            pB.expectedPosition.z += deltaPB.z * kPrime;
            char buf[256];
            float deltaLen = sqrtf(deltaPA.x * deltaPA.x + deltaPA.y * deltaPA.y + deltaPA.z * deltaPA.z);
            sprintf_s(buf, "C=%.6f k'=%f deltaLen=%.6f invSum=%f\n", C, kPrime, deltaLen, invSum);
            OutputDebugStringA(buf);
        }
    };


    struct BendingConstraint
    {
        int p1, p2, p3, p4;
        float restAngle;
        float stiffness;

        // p1 p2 �������ӂ����L����O�p�`
        BendingConstraint(int _p1, int _p2, int _p3, int _p4, float _stiffness = 0.5f): p1(_p1), p2(_p2), p3(_p3), p4(_p4), stiffness(_stiffness)
        {
            restAngle = 0.0f;
        }

        void Initialize(const std::vector<Particle>& particles)
        {
            using namespace DirectX;

            // �����p�x ��0 �����߂�
            auto& pa = particles[p1];
            auto& pb = particles[p2];
            auto& pc = particles[p3];
            auto& pd = particles[p4];

            XMFLOAT3 n1 = MathHelper::ComputeTriangleNormal(pa.position, pb.position, pc.position);
            XMFLOAT3 n2 = MathHelper::ComputeTriangleNormal(pa.position, pb.position, pd.position);

            float dot = n1.x * n2.x + n1.y * n2.y + n1.z * n2.z;
            dot = std::clamp(dot, -1.0f, 1.0f);

            restAngle = acosf(dot); 
        }

        void Solve(std::vector<Particle>& particles) const 
        {
            using namespace DirectX;

            auto& pa = particles[p1];
            auto& pb = particles[p2];
            auto& pc = particles[p3];
            auto& pd = particles[p4];

            // �e�ʒu
            XMFLOAT3 p1_ = pa.expectedPosition;
            XMFLOAT3 p2_ = pb.expectedPosition;
            XMFLOAT3 p3_ = pc.expectedPosition;
            XMFLOAT3 p4_ = pd.expectedPosition;

            // ����position�ɂ��e�@��  
            XMFLOAT3 n1 = MathHelper::ComputeTriangleNormal(p1_, p2_, p3_);
            XMFLOAT3 n2 = MathHelper::ComputeTriangleNormal(p1_, p2_, p4_);

            float dot = n1.x * n2.x + n1.y * n2.y + n1.z * n2.z;
            dot = std::clamp(dot, -1.0f, 1.0f);
            float phi = acosf(dot);

            // �S���֐�
            float C = phi - restAngle;
            if (fabsf(C) < 1e-6f)return;

        }
    };
}

