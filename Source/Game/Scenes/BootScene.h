#pragma once

#include "Engine/Scene/Scene.h"

#include <d3d11.h>
#include <wrl.h>
#include <memory>

#include "Graphics/Sprite/Sprite.h"
#include "Graphics/PostProcess/FullScreenQuad.h"
#include "Graphics/PostProcess/GBuffer.h"
#include "Core/ActorManager.h"
#include "Engine/Scene/SceneBase.h"

#include "Graphics/Environment/SkyMap.h"
#include "Graphics/Shadow/CascadeShadowMap.h"
#include "Graphics/PostProcess/MultipleRenderTargets.h"
#include "Graphics/Effect/EffectSystem.h"

#include "Graphics/Core/LightManager.h"
#include "Graphics/PostProcess/PostEffectManager.h"

#include "Graphics/Renderer/SceneRenderer.h"
#include "Graphics/Core/ConstantBuffer.h"

#include "Game/Actors/Player/TitlePlayer.h"
#include "Game/Actors/Camera/TitleCamera.h"
#include "Graphics/PostProcess/SceneEffectManager.h"

#include "UI/Widgets/Widget.h"
#include "Physics/CollisionMesh.h"

#include "PBD/PBDSystem.h"

class BootScene : public SceneBase
{
#if 0
    struct FrameConstants
    {
        float elapsedTime = 0.0f;
        float deltaTime = 0.0f;
        float gravity = -9.8f;
    };

    struct ShaderConstants
    {
        int  enableSsao;
        int  enableCascadedShadowMaps;
        int  enableSsr;
        int  enableFog;

        int  enableBloom;
        int  enableBlur;
        int  directionalLightEnable = true; // 平行光源の on / off
        int  colorizeCascadedLayer = false;

        float shadowColor = 0.2f;
        float shadowDepthBias = 0.0005f;
    };

    std::unique_ptr<ConstantBuffer<FrameConstants>>     sceneCBuffer;
    std::unique_ptr <ConstantBuffer<ShaderConstants>>   shaderCBuffer;



    DirectX::XMFLOAT4 lightDirection{ -0.75f, -0.581f, -0.4f, 0.0f };
    DirectX::XMFLOAT4 lightColor{ 1.0f,1.0f,1.0f,4.1f };
    float iblIntensity = 2.0f;  //Image Basesd Lightingの強度

    Microsoft::WRL::ComPtr<ID3D11PixelShader> finalPs;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> deferredPs;

    // フルスクリーンクアッドを使ったレンダーターゲットのブレンド・コピー処理
    std::unique_ptr<FullScreenQuad> fullscreenQuadTransfer;

    // CASCADED_SHADOW_MAPS
    std::unique_ptr<CascadedShadowMaps> cascadedShadowMaps;
    float criticalDepthValue = 62.0f; 

    // MULTIPLE_RENDER_TARGETS
    std::unique_ptr<MultipleRenderTargets> multipleRenderTargets;

    // GBUFFER
    std::unique_ptr<GBuffer> gBufferRenderTarget;
    bool useDeferredRendering = false;

    //スカイマップ
    std::unique_ptr<SkyMap> skyMap;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> environmentTextures[8];

    std::unique_ptr<FullScreenQuad> frameBufferBlit;


    SIZE framebufferDimensions = {};
#endif // 0
    ActorColliderManager actorColliderManager;

    UIRoot uiRoot;

public:
    bool Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props) override;

    void Start() override;

    void Update(float deltaTime) override;

    void Render(ID3D11DeviceContext* immediateContext, float deltaTime) override;

    bool Uninitialize(ID3D11Device* device) override;

    //bool OnSizeChanged(ID3D11Device* device, UINT64 width, UINT height) override;

    void DrawGui() override;

    void SetUpActors()override;

    //シーンの自動登録
    static inline Scene::Autoenrollment<BootScene> _autoenrollment;

private:
#if 0
    std::unique_ptr<LightManager> lightManager;
    std::unique_ptr<PostEffectManager> postEffectManager;
    std::unique_ptr<SceneEffectManager> sceneEffectManager;
    // SCREEN_SPACE_AMBIENT_OCCLUSION
    bool enableSSAO = true;
    bool enableCascadedShadowMaps = true;
    bool enableSSR = true;
    bool enableFog = false;
    bool enableBloom = false;

    // ImGuiで使用する
    std::shared_ptr<Actor> selectedActor_;  // 選択中のアクターを保持

#endif // 0
    std::shared_ptr<TitlePlayer> titlePlayer;
    std::shared_ptr<Stage>  title;
    std::shared_ptr<CollisionMesh> stageCollisionMesh;
    std::shared_ptr<TitleCamera> mainCameraActor;

    SceneRenderer sceneRender;

    std::unique_ptr<PBD::System> pbd;
};