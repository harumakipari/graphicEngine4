#pragma once

#include "PostEffectBase.h"


class PostEffectManager
{
public:
    void Initialize(ID3D11Device* device, uint32_t width, uint32_t height)
    {
        for (auto& effect : effects)
        {
            effect->Initialize(device, width, height);
        }
    }

    void ApplyAll(ID3D11DeviceContext* immediateContext, ID3D11ShaderResourceView* colorSRV)
    {
        ID3D11ShaderResourceView* current = colorSRV;
        for (auto& effect : effects)
        {
            effect->Apply(immediateContext, current);
            current = effect->GetOutputSRV();
        }
        lastOutput = current;
    }

    void AddEffect(std::unique_ptr<PostEffectBase> effect)
    {
        effects.push_back(std::move(effect));
    }

    ID3D11ShaderResourceView* GetFinalOutput() const { return lastOutput; }

private:
    std::vector<std::unique_ptr<PostEffectBase>> effects;
    ID3D11ShaderResourceView* lastOutput = nullptr;
};