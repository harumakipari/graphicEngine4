#pragma once
#include <ranges>
#include <unordered_map>

#include "SceneEffectBase.h"

class SceneEffectManager
{
public:
    void Initialize(ID3D11Device* device, uint32_t width, uint32_t height)
    {
        for (auto& effect : effects | std::views::values)
        {
            effect->Initialize(device, width, height);
        }
    }

    void ApplyAll(ID3D11DeviceContext* immediateContext, ID3D11ShaderResourceView* gbufferColor, ID3D11ShaderResourceView* gbufferNormal,
        ID3D11ShaderResourceView* gbufferDepth, ID3D11ShaderResourceView* shadowMap)
    {
        for (auto& effect : effects | std::views::values)
        {
            effect->Apply(immediateContext, gbufferColor, gbufferNormal, gbufferDepth, shadowMap);
        }
    }

    void AddEffect(std::unique_ptr<SceneEffectBase> effect)
    {
        const std::string name = effect->GetName();
        effects[name] = std::move(effect);
    }

    ID3D11ShaderResourceView* GetOutput(const std::string& name) const
    {
        auto it = effects.find(name);
        if (it != effects.end())
            return it->second->GetOutputSRV();
        return nullptr;
    }

    ID3D11ShaderResourceView* GetFinalOutput() const { return lastOutput; }

private:
    std::unordered_map<std::string, std::unique_ptr<SceneEffectBase>> effects;
    ID3D11ShaderResourceView* lastOutput = nullptr;

};