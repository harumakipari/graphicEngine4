#pragma once

#include <d3d11.h>
#include <vector>
#include <memory>

class PostEffectBase
{
public:
    virtual ~PostEffectBase() = default;

    virtual void Initialize(ID3D11Device* device, uint32_t width, uint32_t height) = 0;

    //virtual void 
};

class PostProcessSystem
{
public:
    void Initialize(ID3D11Device* device);

private:
    std::vector<std::unique_ptr<PostEffectBase>> effects;
};