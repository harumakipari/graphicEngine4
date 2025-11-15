#pragma once

#include "Engine/Scene/Scene.h"

#include <d3d11.h>
#include <wrl.h>

#include "Graphics/PostProcess/FullScreenQuad.h"
#include "Graphics/Core/ConstantBuffer.h"
#include "Graphics/Sprite/Sprite.h"
#include "Graphics/Core/Shader.h"
#include "Graphics/Core/RenderState.h"
#include "Graphics/Resource/ShaderToy.h"

#include "Graphics/PostProcess/Bloom.h"
#include "Graphics/Shadow/ShadowMap.h"
#include "Graphics/Environment/SkyMap.h"
#include "Graphics/Shadow/CascadeShadowMap.h"
#include "Graphics/PostProcess/MultipleRenderTargets.h"
#include "Graphics/Effect/EffectSystem.h"

#include "Core/Actor.h"
#include "Core/ActorManager.h"
#include "Engine/Scene/SceneBase.h"

#include "Game/Actors/Camera/TitleCamera.h"
#include "Game/Actors/Enemy/EmptyEnemy.h"

class LoadingScene : public SceneBase
{
    std::unique_ptr<Sprite> hit_space_key;

    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shaders[8];
    std::unique_ptr<FullScreenQuad> bit_block_transfer;
    std::string preload_scene;


    std::unique_ptr<Sprite> splash;


    void SetUpActors() override;


    // shaderToy
    //std::unique_ptr<ShaderToy> shaderToy;

    //std::unique_ptr<RenderState> renderingState;

    struct constants
    {
        float time = 0;
        float width;
        float height;
        //NOISE
        float seedScale = 5.0f;
    };

    std::unique_ptr<ConstantBuffer<constants>> cbuffer;
    //前のシーン
    std::unique_ptr<ID3D11ShaderResourceView> preSceneTexture;

    // ShaderToy で追加
    std::unique_ptr<FullScreenQuad> shaderToyTransfer; // ShadowToy用の
    std::unique_ptr<FrameBuffer> shaderToyFrameBuffer; // ShadowToy用の
    Microsoft::WRL::ComPtr<ID3D11PixelShader> shaderToyPS;

    struct ShaderToyCB
    {
        DirectX::XMFLOAT4 iResolution;
        DirectX::XMFLOAT4 iMouse;
        DirectX::XMFLOAT4 iChannelResolution[4];
        float iTime;
        float iFrame;
        float iPad0;
        float iPad1;
    };
    ShaderToyCB shaderToy;
    Microsoft::WRL::ComPtr<ID3D11Buffer> shaderToyConstantBuffer;

public:
    size_t type = 1;
    bool Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props) override;

    void Update(float deltaTime) override;

    float time = 0;
    void Start() override;

    void Render(ID3D11DeviceContext* immediate_context, float deltaTime) override;

    bool Uninitialize(ID3D11Device* device) override;

    void DrawGui() override;

    //シーンの自動登録
    static inline Scene::Autoenrollment<LoadingScene> _autoenrollment;

private:
    std::shared_ptr<EmptyEnemy> enemy = nullptr;

    std::shared_ptr<TitleCamera> mainCameraActor = nullptr;

    // ImGuiで使用する
    std::shared_ptr<Actor> selectedActor_;  // 選択中のアクターを保持

    DirectX::XMFLOAT3 cameraTarget = { 0.0f,0.0f,0.0f };


    SceneRenderer sceneRender;

};