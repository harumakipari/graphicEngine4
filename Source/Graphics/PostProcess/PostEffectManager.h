#pragma once

#include "PostEffectBase.h"


class PostEffectManager
{
public:
    void Initialize(ID3D11Device* device)
    {
        
    }

    void ApplyAll(ID3D11DeviceContext* immediateContext,ID3D11ShaderResourceView* colorSrv)
    {
        
    }



private:
    std::vector<std::unique_ptr<PostEffectBase>> effects;
};