#pragma once

#include "Engine/Scene/Scene.h"

#include <d3d11.h>
#include <wrl.h>
#include <memory>

#include "Graphics/Sprite/Sprite.h"
#include "Graphics/PostProcess/FullScreenQuad.h"
#include "Graphics/PostProcess/GBuffer.h"
#include "Core/ActorManager.h"

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

class BootScene : public Scene
{
    struct SceneConstants
    {
        bool enableSsao;
        float reflectionIntensity;
        float time = 0.0f;
        // shader のフラグ
        int enableCascadedShadowMaps;
        int enableSsr;
        int enableFog;
        int enableBloom;
        float deltaTime = 0.0f;

        float gravity = 9.8f;
        int enableBlur = 0;
        float pads[2] = {};
    };

    bool directionalLightEnable = true; // 平行光源の on / off

    struct ShaderConstants
    {
        float shadowColor = 0.2f;
        float shadowDepthBias = 0.0005f;
        bool colorizeCascadeLayer = false;
        float maxDistance = 15.0f;// SCREEN_SPACE_REFLECTION

        float resolution = 0.3f;// SCREEN_SPACE_REFLECTION
        int steps = 10;// SCREEN_SPACE_REFLECTION
        float thickness = 0.5f;// SCREEN_SPACE_REFLECTION
        float pad;
    };
    //ShaderConstants shaderConstants;

    // FOG
    struct FogConstants
    {
        float fogColor[4] = { 1.000f,1.000f, 1.000f, 1.000f }; // w: fog intensuty

        float fogDensity = 0.02f;
        float fogHeightFalloff = 0.05f;
        float groundLevel = 0.0f;
        float fogCutoffDistance = 500.0f;

        float mieScatteringFactor = 0.55f;

        int enableDither = 1;
        int enableBlur = 1;

        float timeScale = 0.35f;
        float noiseScale = 0.2f;
        float pads[3];
    };
    //FogConstants fogConstants;

    //Glitch
    struct SpriteConstants
    {
        float elapsedTime = 0.0f;
        UINT enableGlitch = true;
        float pads[2];
    };
    //SpriteConstants spriteConstants;

    // ConstantBuffer クラスで管理
    std::unique_ptr<ConstantBuffer<SceneConstants>>     sceneCBuffer;
    std::unique_ptr <ConstantBuffer<ShaderConstants>>   shaderCBuffer;
    std::unique_ptr <ConstantBuffer<FogConstants>>      fogCBuffer;
    std::unique_ptr <ConstantBuffer<SpriteConstants>>   spriteCBuffer;

    ActorColliderManager actorColliderManager;


    DirectX::XMFLOAT4 lightDirection{ -0.75f, -0.581f, -0.4f, 0.0f };
    DirectX::XMFLOAT4 lightColor{ 1.0f,1.0f,1.0f,4.1f };
    float iblIntensity = 2.0f;  //Image Basesd Lightingの強度

    std::unique_ptr<Sprite> splash;

    //Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShaders[8];

    Microsoft::WRL::ComPtr<ID3D11PixelShader> finalPs;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> deferredPs;

    // フルスクリーンクアッドを使ったレンダーターゲットのブレンド・コピー処理
    std::unique_ptr<FullScreenQuad> fullscreenQuadTransfer;

    // CASCADED_SHADOW_MAPS
    std::unique_ptr<CascadedShadowMaps> cascadedShadowMaps;
    //float criticalDepthValue = 0.0f; // If this value is 0, the camera's far panel distance is used.
    float criticalDepthValue = 62.0f; // If this value is 0, the camera's far panel distance is used.

    // MULTIPLE_RENDER_TARGETS
    std::unique_ptr<MultipleRenderTargets> multipleRenderTargets;

    // GBUFFER
    std::unique_ptr<GBuffer> gBufferRenderTarget;
    bool useDeferredRendering = false;

    //スカイマップ
    std::unique_ptr<SkyMap> skyMap;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceViews[8];

    std::unique_ptr<FullScreenQuad> frameBufferBlit;

    void SetUpActors();

    SIZE framebufferDimensions = {};

    UIRoot uiRoot;

public:
    bool Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props) override;

    void Start() override;

    void Update(ID3D11DeviceContext* immediateContext, float deltaTime) override;

    void Render(ID3D11DeviceContext* immediateContext, float deltaTime) override;

    bool Uninitialize(ID3D11Device* device) override;

    bool OnSizeChanged(ID3D11Device* device, UINT64 width, UINT height) override;

    void DrawGui() override;

    //シーンの自動登録
    static inline Scene::Autoenrollment<BootScene> _autoenrollment;

private:
    std::unique_ptr<LightManager> lightManager;
    std::unique_ptr<PostEffectManager> postEffectManager;
    std::unique_ptr<SceneEffectManager> sceneEffectManager;
    std::shared_ptr<TitlePlayer> titlePlayer;
    std::shared_ptr<Stage>  title;
    std::shared_ptr<CollisionMesh> stageCollisionMesh;
    std::shared_ptr<TitleCamera> mainCameraActor;
    // ImGuiで使用する
    std::shared_ptr<Actor> selectedActor_;  // 選択中のアクターを保持

    DirectX::XMFLOAT3 cameraTarget = { 0.0f,0.0f,0.0f };

    Renderer actorRender;
    SceneRenderer sceneRender;

    // SCREEN_SPACE_AMBIENT_OCCLUSION
    bool enableSSAO = true;
    bool enableCascadedShadowMaps = true;
    bool enableSSR = true;
    bool enableFog = false;
    bool enableBloom = false;

    // SCREEN_SPACE_REFLECTION
    float reflectionIntensity = 0.1f;
    float maxDistance = 15.0f;
    float resolution = 0.3f;
    int steps = 10;
    float thickness = 0.5f;

    // BLOOM 
    float bloomIntensity = 0.266f;
    float bloomThreshold = 3.525f;

    std::unique_ptr<PBD::System> pbd;
};