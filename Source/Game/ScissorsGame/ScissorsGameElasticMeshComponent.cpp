#include "pch.h"
#include "ScissorsGameElasticComponent.h"
#include "Core/Actor.h"
#include "Engine/Utility/Time.h"

void ScissorsGameElasticMeshComponent::Initialize()
{
    auto actor = owner_.lock();
    DirectX::XMFLOAT3 position = actor->GetPosition();
    modelHeight = model->GetModelSize().y * actor->GetScale().y;

    // 定数バッファの作成
    elasticBuildingCBuffer = std::make_unique<ConstantBuffer<ElasticConstants>>(Graphics::GetDevice());
    elasticConstants =
    {
        /*p1*/ DirectX::XMFLOAT4(position.x, position.y, position.z, 1.0f),
        /*p2*/ DirectX::XMFLOAT4(position.x,position.y, position.z, 1.0f),
        /*p3*/ DirectX::XMFLOAT4(position.x, position.y + modelHeight, position.z, 1.0f),
        /*maxAngleDegree*/ 100.0f, // 度以上は曲がらない
        /*modelHeight*/ modelHeight,
    };
    elasticBuildingCBuffer->data = elasticConstants;

    p3Current = {
    position.x,
    position.y + modelHeight,
    position.z
    };
    p3Target = p3Current;
}
void ScissorsGameElasticMeshComponent::Tick(float deltaTime)
{
    auto actor = owner_.lock();
    DirectX::XMFLOAT3 pos = actor->GetPosition();
    float buildHeight = modelHeight;
    float midY = pos.y + buildHeight * 0.5f;
    static float momentumX = 0.0f;
    static float momentumY = 0.0f;
    static float momentumZ = 0.0f;
    elasticConstants.p1 = { pos.x,pos.y,pos.z,1.0f };
    elasticConstants.p2 = { pos.x ,midY,pos.z ,1.0f };

    switch (state)
    {
    case State::Idle:
    {
        static float timer = 0.0f;
        timer += deltaTime;
        shakePower.x = amplitude.x * sinf(timer * frequency);
        shakePower.y = amplitude.y * sinf(timer * frequency);
        shakePower.z = amplitude.z * sinf(timer * frequency);

        DirectX::XMFLOAT3 p3 = { pos.x + shakePower.x,buildHeight + shakePower.y,pos.z + shakePower.z };
        p3Current = { p3.x ,p3.y ,p3.z  };
    }
        break;
    case State::Tied:
    {
        static float totalTime = 0.0f;
        totalTime += deltaTime;
        float t = totalTime;

        float noiseX = sin(t * shakeSpeed * noiseFreqMulX + noisePhaseX) * shakeAmp;
        float noiseY = sin(t * shakeSpeed + noisePhaseY) * shakeAmp;
        float noiseZ = sin(t * shakeSpeed * noiseFreqMulZ + noisePhaseZ) * shakeAmp;

        DirectX::XMFLOAT3 p3 = {
            pos.x + shakePower.x,
            pos.y + buildHeight + shakePower.y,
            pos.z + shakePower.z
        };

        p3.x += noiseX * overshoot;
        p3.y += noiseY * overshoot * 0.3f; // ← Yは弱めが重要
        p3.z += noiseZ * overshoot;

        // ▼ クランプ
        float minY = pos.y + buildHeight /** yLimitRatio*/;
        p3.y =/* std::max<float>(p3.y, minY);*/ minY;

        p3Current = p3;
    }
    break;
    case State::None:
    break;
    }
    elasticConstants.p1 = { pos.x,pos.y,pos.z,1.0f };
    elasticConstants.p2 = { pos.x ,midY,pos.z ,1.0f };
    elasticConstants.p3 = { p3Current.x ,p3Current.y,p3Current.z ,1.0f };
}

void ScissorsGameElasticMeshComponent::UpdateConstantBuffer(ID3D11DeviceContext* immediateContext) const
{
    elasticBuildingCBuffer->data = elasticConstants;
    elasticBuildingCBuffer->Activate(immediateContext, 6);
}


void ScissorsGameElasticMeshComponent::SetElasticEnabled(const bool enabled)
{
    elasticEnabled = enabled;

    if (!enabled)
    {
        // OFFにした瞬間に余計な運動量を消す
        elasticParameters.momentumX = 0.0f;
        elasticParameters.momentumY = 0.0f;
        elasticParameters.momentumZ = 0.0f;
    }
}



void ScissorsGameElasticMeshComponent::DrawImGuiInspector()
{
#ifdef USE_IMGUI
    SceneComponent::DrawImGuiInspector();
    if (ImGui::TreeNode((name_ + "  model").c_str()))
    {
        ImGui::Checkbox("isVisible", &isVisible_);
        ImGui::TreePop();
    }
    const char* stateNames[] = { "Idle", "Tied", "None" };
    int current = static_cast<int>(state);

    if (ImGui::Combo("State", &current, stateNames, IM_ARRAYSIZE(stateNames)))
    {
        state = static_cast<State>(current);
    }
    ImGui::DragFloat3("p2", &elasticConstants.p2.x, 0.1f);
    ImGui::DragFloat3("p3", &elasticConstants.p3.x, 0.1f);
    ImGui::SliderFloat("Stiffness", &elasticParameters.stiffness, 0.1f, 10.0f);
    ImGui::SliderFloat("Damping", &elasticParameters.damping, 0.1f, 0.99f);
    //ImGui::SliderFloat("Mass", &elasticParameters.mass, 0.1f, 5.0f);
    //ImGui::SliderFloat("momentumX", &elasticParameters.momentumX, -10.0f, 10.0f);
    //ImGui::SliderFloat("momentumY", &elasticParameters.momentumY, -10.0f, 10.0f);
    //ImGui::SliderFloat("momentumZ", &elasticParameters.momentumZ, -10.0f, 10.0f);
    //ImGui::SliderFloat(U8("サクランボの伸び"), &elasticParameters.maxDist, -5.0f, 10.0f);
    //ImGui::SliderFloat(" maxAngleDegrees", &elasticParameters.maxAngleDegrees, 0.0f, 360.0f);
    ImGui::DragFloat("shakeAmp", &shakeAmp, 0.1f);
    ImGui::DragFloat("shakeSpeed", &shakeSpeed, 0.1f);
    ImGui::DragFloat("overshoot", &overshoot, 0.1f);
    ImGui::DragFloat3("shake power", &shakePower.x, 0.1f);
    ImGui::DragFloat3("amplitude", &amplitude.x, 0.1f);
    ImGui::DragFloat("frequency", &frequency, 0.1f);
    ImGui::Separator();
    ImGui::Text("=== Tied Shake ===");

    ImGui::SliderFloat("ShakeAmp", &shakeAmp, 0.0f, 3.0f);
    ImGui::SliderFloat("ShakeSpeed", &shakeSpeed, 0.1f, 20.0f);
    ImGui::SliderFloat("Overshoot", &overshoot, 0.0f, 5.0f);

    ImGui::SliderFloat("Y Limit Ratio", &yLimitRatio, 0.3f, 1.0f);

    ImGui::Separator();
    ImGui::Text("=== Noise Phase ===");

    ImGui::DragFloat("Phase X", &noisePhaseX, 0.1f);
    ImGui::DragFloat("Phase Y", &noisePhaseY, 0.1f);
    ImGui::DragFloat("Phase Z", &noisePhaseZ, 0.1f);

    ImGui::Separator();
    ImGui::Text("=== Noise Frequency ===");

    ImGui::DragFloat("Freq X", &noiseFreqMulX, 0.1f);
    ImGui::DragFloat("Freq Z", &noiseFreqMulZ, 0.1f);

#endif
}

