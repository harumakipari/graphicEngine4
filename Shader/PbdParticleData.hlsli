

struct PbdParticle
{
    float3 position;
    float3 expectedPosition;
    float3 velocity;
    float3 force;
    float invMass;
};

struct DistanceConstraint
{
    uint i0;
    uint i1;
    float restLength;
};

StructuredBuffer<DistanceConstraint> distanceConstraintSrv : register(t0);
RWStructuredBuffer<PbdParticle> pbdParticleUAV : register(u0);

