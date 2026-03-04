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
#include "UI/Font.h"

#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include <unordered_map>

#include "Graphics/Shadow/ShadowMap.h"
#include "UI/UIManager.h"


/// @brief 全シーン共通の基底クラス。描画・定数バッファ・ライト・ポストエフェクトを統一管理する。
class SceneBase : public Scene
{
public:
    enum class RenderPass
    {
        Sky,
        Opaque,
        Mask,
        ForwardBlend,
        Particle,
        Debug,
        UI,
    };

    using RenderHook = std::function<void(ID3D11DeviceContext*)>;

    virtual bool Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props) override;

    virtual void Update(float deltaTime) override;

    void UpdateConstantBuffer(ID3D11DeviceContext* immediateContext);

    virtual void Render(ID3D11DeviceContext* immediateContext, float delta_time) override;

    virtual bool Uninitialize(ID3D11Device* device) override;
    virtual bool OnSizeChanged(ID3D11Device* device, UINT64 width, UINT height) override;
    virtual void DrawGui() override;

    void RegisterRenderHook(const RenderPass pass, const RenderHook& hook)
    {
        renderHooks[pass].push_back(hook);
    }

    void ExecuteHooks(const RenderPass pass, ID3D11DeviceContext* immediateContext)
    {
        for (auto& hook : renderHooks[pass])
            hook(immediateContext);
    }

private:
    void ForwardRender(ID3D11DeviceContext* immediateContext);

    void DeferredRender(ID3D11DeviceContext* immediateContext, const ViewConstants& viewConstants);

    void Draw(ID3D11DeviceContext* immediateContext);

    void DrawOutliner();

    void DrawShortcutInfo();

    void DrawSceneSettingsTab();

    void DrawInspector();

    void DrawPostEffectTab();

    void DrawDockSpace();

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
        float shadowColor = 0.422f;
        float shadowDepthBias = -0.001f;
        float splitU = 0.0f;
        float	hueShift = 0.0f;;	// 色相調整

        float	saturation = 1.0f;	// 彩度調整
        float	brightness = 1.0f;	// 明度調整
        XMFLOAT2 paddings; // 16バイト境界に合わせるためのパディング

        int enableSsao = true;
        int enableCascadedShadowMaps = true;
        int enableSsr = true;
        int enableFog = false;

        int enableBloom = true;
        int enableBlur = true;
        int colorizeCascadedLayer = false;
    };

    //==============================
    // メンバー変数（描画関連）
    //==============================
    std::unique_ptr<ConstantBuffer<FrameConstants>>  sceneCBuffer;
    std::unique_ptr<ConstantBuffer<ShaderConstants>> shaderCBuffer;
    std::unique_ptr<FullScreenQuad> fullscreenQuad;

    std::unique_ptr<FrameBuffer> frameBuffer;
    std::unique_ptr<FrameBuffer> imGuiGizmoBuffer;

    std::unique_ptr<CascadedShadowMaps> cascadedShadowMaps;

    std::unique_ptr<MultipleRenderTargets> multipleRenderTargets;
    std::unique_ptr<GBuffer> gBufferRenderTarget;
    std::unique_ptr<SkyMap> skyMap;
    std::unique_ptr<LightManager> lightManager;
    std::unique_ptr<PostEffectManager> postEffectManager;
    /*static inline */std::unique_ptr<SceneEffectManager> sceneEffectManager;
    std::unique_ptr<SceneRenderer> sceneRenderer_;

    Microsoft::WRL::ComPtr<ID3D11PixelShader> finalPs;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> deferredPs;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> environmentTextures[8];

    DirectX::XMFLOAT4 lightDirection{ -0.75f, -0.581f, -0.4f, 0.0f };
    DirectX::XMFLOAT4 lightColor{ 1.0f, 1.0f, 1.0f, 20.1f };

    bool useDeferredRendering = true;
    bool enableSSAO = true;
    bool enableCascadedShadowMaps = true;
    bool enableSSR = true;
    bool enableFog = true;
    bool enableBloom = true;
    bool enableBlur = true;

    bool useDrawDebug = true;

    float criticalDepthValue = 247.0f;

    SIZE framebufferDimensions = {};

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> noise3d;

    std::shared_ptr<Actor> selectedActor_;  // 選択中のアクターを保持

    SceneRenderer sceneRender;

    //==============================
    // メンバー関数
    //==============================
    virtual void DrawSceneGui() {}///< 各シーン固有のImGui描画フック

    std::unordered_map<RenderPass, std::vector<RenderHook>> renderHooks;





};
