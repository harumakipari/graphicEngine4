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
        float stiffness;    // 0.0f~1.0f 1.0fのほうが拘束が固い

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

            // 正規化方向
            XMFLOAT3 n = { dir.x / dist,dir.y / dist,dir.z / dist };

            // stiffnessを補正する　論文に書いてた、、
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
        }

    };
}
