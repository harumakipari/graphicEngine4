#pragma once

#include "Engine/Scene/Scene.h"

#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include <vector>

#include "Graphics/Sprite/Sprite.h"
#include "Graphics/Sprite/SpriteBatch.h"
#include "Graphics/PostProcess/FrameBuffer.h"
#include "Graphics/PostProcess/FullScreenQuad.h"

#include "Graphics/Resource/GltfModelBase.h"
#include "Graphics/Renderer/ShapeRenderer.h"


#include "Graphics/PostProcess/Bloom.h"
#include "Graphics/Environment/SkyMap.h"
#include "Graphics/Shadow/CascadeShadowMap.h"
#include "Graphics/PostProcess/MultipleRenderTargets.h"

#include "Core/Actor.h"
#include "Game/Actors/Player/Player.h"
#include "Game/Actors/Stage/Stage.h"
#include "Game/Actors/Enemy/RiderEnemy.h"

#include "Physics/CollisionMesh.h"
#include "Graphics/PostProcess/GBuffer.h"

#include "Graphics/Effect/EffectSystem.h"

#include "Core/ActorManager.h"
#include "Engine/Scene/SceneBase.h"

#include "Utils/EasingHandler.h"

class MainScene : public SceneBase
{
    std::unique_ptr<Sprite> sprites[8];
    std::unique_ptr<SpriteBatch> sprite_batches[8];

    // ScreenSpaceProjectionMapping
    static constexpr int MAX_PROJECTION_MAPPING = 32;
    struct ProjectionMappingConstants
    {
        DirectX::XMFLOAT4X4 projectionMappingTransforms[MAX_PROJECTION_MAPPING]{};
        DirectX::XMUINT4 enableMapping[MAX_PROJECTION_MAPPING / 4]{};
        DirectX::XMUINT4 textureId[MAX_PROJECTION_MAPPING / 4]{};
    };
    ProjectionMappingConstants projectionMappingConstants;

    DirectX::XMFLOAT3 eyes[MAX_PROJECTION_MAPPING];
    DirectX::XMFLOAT3 targets[MAX_PROJECTION_MAPPING];
    float distance = 20.0f;
    float rotations[MAX_PROJECTION_MAPPING];
    float fovY = 10.0f;

    Microsoft::WRL::ComPtr<ID3D11PixelShader> screenSpaceProjectionMappingPixelShader;

    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffers[8];

    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShaders[8];

    //当たり判定のメッシュ
    std::unique_ptr<CollisionMesh> collisionMesh;


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
    std::unique_ptr<EffectSystem> effectSystem;
public:
    bool Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props) override;
    void Start() override;
    void Update(float deltaTime) override;
    void Render(ID3D11DeviceContext* immediateContext, float deltaTime) override;
    bool Uninitialize(ID3D11Device* device) override;
    void DrawGui() override;
private:
    // ImGuiで使用する
    std::shared_ptr<Actor> selectedActor_;  // 選択中のアクターを保持

    //std::unique_ptr<World> gameWorld_;

    //画面内に描画されるもの
    std::vector<std::shared_ptr<Actor>> actors;

    //シーンで使うモデル
    std::map<std::string, std::shared_ptr<GltfModelBase>> models;

    //モデルをロード
    void LoadModel();

    //Actorをセット
    void SetUpActors() override;

    //パーティクルシステムセットする
    void SetParticleSystem();

    //プレイヤー
    std::shared_ptr<Player> player;

    //敵
    std::shared_ptr<RiderEnemy> enemies[1];

    // カメラシェイク
    float power = 0.01f;
    float timer = 0.17f;
public:
    DirectX::XMFLOAT4 imGuiNDC = { 0.0f,0.0f,0.0f,0.0f };



    //ステージ
    std::shared_ptr<Stage> stage;

    //TODO:02IMGUI用
    DirectX::XMFLOAT3 spherePosition{ 2.0f,0.0f,0.0f };
    DirectX::XMFLOAT4 playerDebugColor{ 1.0f,1.0f,1.0f,1.0f };
    DirectX::XMFLOAT4 enemyDebugColor{ 1.0f,1.0f,1.0f,1.0f };

    //前フレームの武器の座標
    DirectX::XMFLOAT3 preSpherePosition{ 0.0f,0.0f,0.0f };

    //武器と敵が当たったフラグ
    bool isHitWeapon = false;

    //プレイヤーと敵があたったフラグ
    bool isHitBody = false;

    //撃破イベント起動フラグ
    bool isBossDeath = false;
    bool isPlayerDeath = false;
    //フェード
    EasingHandler fadeHandler;
    float fadeValue = 0.0f;
    EasingHandler waitHandler;

    //パーティクル
    //bool integrateParticles{ true };
    //std::unique_ptr<ParticleSystem> particles;
    //Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleTexture;
    //Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> colorTemperChart;
    //int blendMode = 1; // 0:ALPHA, 1:ADD

    DirectX::XMFLOAT3 p1{ 0.0f,0.5f,0.0f };
    DirectX::XMFLOAT3 p2{ 1.0f,0.5f,0.0f };

    // シーンの自動登録
    static inline Scene::Autoenrollment<MainScene> _autoenrollment;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> noise2d;

    // VOLUMETRIC_CLOUDSCAPES
    DirectX::XMFLOAT4 cameraFocus{ 0.0f, 1.0f, 0.0f, 1.0f };

    SceneRenderer actorRender;

    Renderer render;

    ActorColliderManager actorColliderManager;

    // ボスが画面外かどうか
    bool IsFrameOutEnemy()
    {
        return isFrameOutEnemy;
    }
    // カーソルの位置取得
    DirectX::XMFLOAT2 GetCursorIntersectPos()
    {
        return intersection;
    }
    // カーソルの度数
    float GetAngleCursorDegree()
    {
        return angleDegree;
    }
private:
    // 敵のスクリーン座標
    DirectX::XMFLOAT2 enemyScreenPosition = { 0.0f,0.0f };
    // プレイヤーのスクリーン座標
    DirectX::XMFLOAT2 playerScreenPosition = { 0.0f,0.0f };
    // カーソル描画の交点
    DirectX::XMFLOAT2 intersection = { 0.0f,0.0f };
    // カーソルの描画角度
    float angleDegree = 0.0f;
    // ボスが画面外かどうか
    bool isFrameOutEnemy = false;
};
