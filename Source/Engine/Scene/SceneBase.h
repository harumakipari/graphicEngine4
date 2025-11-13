#pragma once
#include "Engine/Scene/Scene.h"
#include "Graphics/Core/ConstantBuffer.h"
#include "Graphics/PostProcess/FullScreenQuad.h"
#include "Graphics/Core/LightManager.h"
#include "Graphics/PostProcess/PostEffectManager.h"
#include "Graphics/PostProcess/SceneEffectManager.h"
#include "Graphics/Renderer/SceneRenderer.h"
#include "Graphics/Environment/SkyMap.h"
#include "Graphics/PostProcess/GBuffer.h"
#include "Graphics/PostProcess/MultipleRenderTargets.h"
#include "Graphics/Shadow/CascadeShadowMap.h"
#include "UI/Widgets/Widget.h"

#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include <unordered_map>

/// @brief 全シーン共通の基底クラス。描画・定数バッファ・ライト・ポストエフェクトを統一管理する。
class SceneBase : public Scene
{
public:
    virtual bool Initialize(ID3D11Device* device, UINT64 width, UINT height,
        const std::unordered_map<std::string, std::string>& props) override;

    virtual void Update(float deltaTime) override;

    void UpdateConstantBuffer(ID3D11DeviceContext* immediateContext);

    virtual bool Uninitialize(ID3D11Device* device) override{}
    virtual bool OnSizeChanged(ID3D11Device* device, UINT64 width, UINT height) override;
    virtual void DrawGui() override;

private:
    void DrawOutliner();

    void DrawShortcutInfo();

    void DrawSceneSettingsTab();

    void DrawInspector();

    void DrawPostEffectTab();

    void SetupImGuiStyle();

    void DrawGizmo();

protected:
    //==============================
    // 定数バッファ構造体
    //==============================
    struct FrameConstants
    {
        float elapsedTime = 0.0f;
        float deltaTime = 0.0f;
        float gravity = -9.8f;
    };

    struct ShaderConstants
    {
        int enableSsao = true;
        int enableCascadedShadowMaps = true;
        int enableSsr = true;
        int enableFog = false;

        int enableBloom = false;
        int enableBlur = false;
        int directionalLightEnable = true;
        int colorizeCascadedLayer = false;

        float shadowColor = 0.2f;
        float shadowDepthBias = 0.0005f;
    };

    //==============================
    // メンバー変数（描画関連）
    //==============================
    std::unique_ptr<ConstantBuffer<FrameConstants>>  sceneCBuffer;
    std::unique_ptr<ConstantBuffer<ShaderConstants>> shaderCBuffer;
    std::unique_ptr<FullScreenQuad> fullscreenQuad;

    std::unique_ptr<CascadedShadowMaps> cascadedShadowMaps;
    std::unique_ptr<MultipleRenderTargets> multipleRenderTargets;
    std::unique_ptr<GBuffer> gBufferRenderTarget;
    std::unique_ptr<SkyMap> skyMap;
    std::unique_ptr<LightManager> lightManager;
    std::unique_ptr<PostEffectManager> postEffectManager;
    std::unique_ptr<SceneEffectManager> sceneEffectManager;
    std::unique_ptr<SceneRenderer> sceneRenderer_;

    Microsoft::WRL::ComPtr<ID3D11PixelShader> finalPs;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> deferredPs;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> environmentTextures[8];

    DirectX::XMFLOAT4 lightDirection{ -0.75f, -0.581f, -0.4f, 0.0f };
    DirectX::XMFLOAT4 lightColor{ 1.0f, 1.0f, 1.0f, 4.1f };
    float iblIntensity_ = 2.0f;

    bool useDeferredRendering = false;
    bool enableSSAO = true;
    bool enableCascadedShadowMaps = true;
    bool enableSSR = true;
    bool enableFog = false;
    bool enableBloom = false;

    float criticalDepthValue = 62.0f;

    SIZE framebufferDimensions = {};

    // UI
    UIRoot uiRoot_;

    std::shared_ptr<Actor> selectedActor_;  // 選択中のアクターを保持


    //==============================
    // メンバー関数
    //==============================
    virtual void DrawSceneGui();  ///< 各シーン固有のImGui描画フック

};
