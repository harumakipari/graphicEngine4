#include "pch.h"
#include "ParticleComponent.h"
#include "Core/Vector.h"
#include "Widgets/Utils/Dialog.h"
#include "Core/Actor.h"

//REGISTER_COMPONENT(ParticleComponent, "Effects");

void ParticleComponent::Load(const std::string& filePath)
{
    effectHandle = EffectManager::LoadEffectData(filePath);
}

void ParticleComponent::Play()
{
    // エフェクト再生
    if (effectHandle != -1)
    {
        // エフェクト発生前コールバック実行
        if (settings.onPreEmit)
        {
            settings.onPreEmit();
        }

        // ゲームオブジェクトの位置と回転を取得
        XMFLOAT3 position = owner_.lock()->GetPosition();
        XMFLOAT3 rotation = owner_.lock()->GetEulerRotation();

        // 線上にエフェクトを再生する場合
        if (settings.lineData.useLine)
        {
            // 線上に分割してエフェクトを再生
            for (int i = 0; i <= settings.lineData.segments.size(); ++i)
            {
                LineData::Segment& segment = settings.lineData.segments[i % settings.lineData.segments.size()];

                XMFLOAT3 startPos = segment.start ? segment.start->GetTranslation() : XMFLOAT3();
                XMFLOAT3 endPos = segment.end ? segment.end->GetTranslation() : XMFLOAT3();
                int segmentCount = segment.segmentCount > 0 ? segment.segmentCount : 1;

                if (segment.start && segment.end)
                {
                    XMFLOAT4 startRotation = segment.start->GetRotation();
                    XMFLOAT4 endRotation = segment.end->GetRotation();
                    // 始点から終点へ向く回転を設定
                    DirectX::XMStoreFloat3(&rotation, MathHelper::QuaternionLookAt(XMLoadFloat4(&startRotation), XMLoadFloat4(&endRotation)));
                }

                for (int j = 0; j < segmentCount; ++j)
                {
                    float t = static_cast<float>(j) / static_cast<float>(segmentCount);
                    XMFLOAT3 pos{};
                    pos.x = startPos.x + (endPos.x - startPos.x) * t;
                    pos.y = startPos.y + (endPos.y - startPos.y) * t;
                    pos.z = startPos.z + (endPos.z - startPos.z) * t;
                    EffectManager::Play(effectHandle, pos, rotation);
                }
            }
        }
        else
        {
            // 通常の位置でエフェクト再生
            EffectManager::Play(effectHandle, position, rotation);
        }
        elapsedTimeSincePlay = 0.0f;
        duration = CalculateDuration();
        isPlaying = true;
        // 再生開始遅延経過時間リセット
        elapsedDelayTime = 0.0f;

    }
}

void ParticleComponent::Stop()
{
    // エフェクト停止
    isPlaying = false;
}

void ParticleComponent::Tick(float deltaTime)
{
    // ループ再生でない場合は何もしない
    if (effectHandle == -1 || !IsPlaying() /*|| !settings.loop*/)
    {
        return;
    }


    // 再生開始遅延時間の処理
    if (elapsedDelayTime < settings.startDelay)
    {
        elapsedDelayTime += deltaTime;
        if (elapsedDelayTime < settings.startDelay)
        {
            // まだ遅延時間内なので再生しない
            return;
        }
    }

    // エフェクト発生前コールバック実行
    if (settings.onPreEmit)
    {
        settings.onPreEmit();
    }
    XMFLOAT3 position = owner_.lock()->GetPosition();
    XMFLOAT3 rotation = owner_.lock()->GetEulerRotation();

    // 再生してからの経過時間更新
    elapsedTimeSincePlay += deltaTime;

    if (!settings.loop && elapsedTimeSincePlay >= duration)
    {
        isPlaying = false;
    }

    if (settings.loop && elapsedTimeSincePlay >= duration)
    {
        elapsedTimeSincePlay = 0.0f;
        // エフェクト再生
        EffectManager::Play(effectHandle, position, rotation);
    }

}

float ParticleComponent::CalculateDuration() const
{
    float maxDuration = 0.0f;

    auto& effect = EffectManager::GetEffectData(effectHandle);

    for (auto& emitter : effect.emitters)
    {
        float d =
            emitter.emitData.initialDelay.max +
            emitter.emitData.emitInterval.max * (emitter.emitData.emitCount.max - 1) +
            emitter.motionData.lifeTime.max;

        maxDuration = std::max<float>(maxDuration, d);
    }

    return maxDuration;
}

void ParticleComponent::DrawImGuiInspector()
{
#ifdef USE_IMGUI
    if (ImGui::Button("Load Effect"))
    {
        // ダイアログを開いてエフェクトデータを読み込む
        effectHandle = EffectManager::LoadEffectDataWithDialog();
    }
    ImGui::SameLine();
    if (ImGui::Button("Play Effect"))
    {
        Play();
    }

    if (effectHandle != -1)
    {
        ImGui::Text("Effect Handle: %d", effectHandle);
    }
    else
    {
        ImGui::Text("No Effect Loaded");
    }

#endif // USE_IMGUI

}