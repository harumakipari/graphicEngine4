#pragma once

#include "Engine/Scene/Scene.h"

#include <d3d11.h>
#include <wrl.h>
#include <memory>

#include "Core/ActorManager.h"
#include "Engine/Scene/SceneBase.h"


#include "Graphics/Renderer/SceneRenderer.h"

#include "Game/Actors/Camera/LoadingCamera.h"
#include "Game/Actors/Player/Player.h"
#include "Game/ScissorsGame/DecalData.h"
#include "Game/ScissorsGame/NeedleEnemyActor.h"
#include "Game/ScissorsGame/ScoreCalculator.h"
#include "Game/ScissorsGame/StageData.h"
#include "Game/ScissorsGame/YarnEnemyActor.h"




class GameScene : public SceneBase
{
public:
    //　敵の調整
    struct EnemyTuning
    {
        // 吹っ飛び系
        float knockbackDistanceDash = 8.0f;
        float knockbackDistanceNormal = 5.0f;

        float knockbackHeightDash = 5.0f;
        float knockbackHeightNormal = 8.0f;

        float knockbackDurationDash = 0.7f;
        float knockbackDurationNormal = 0.9f;

        // 発光系
        float flashDuration = 0.7f;
        float emissivePower = 20.0f;
        float flashSharpness = 7.0f;
    };

    // コインの調整　
    struct CoinTuning
    {
        float duration = 2.0f; // 演出に掛ける時間
        float height = 1.5f;

        // ===== 上昇トレイル =====
        float trailSpawnInterval = 0.15f;
        float trailSize = 15.0f;

        // ===== バースト =====
        int burstCount = 10;
        float burstSize = 40.0f;
        float burstShrinkSpeed = 500.0f;
        std::string modelPath = "./Data/TeamModels/Item/NormalButtonCoin.glb";
    };

public:
    bool Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props) override;

    void Start() override;

    void Update(float deltaTime) override;

    // 定数バッファの更新処理をシーンごとにカスタマイズできるようにするための仮想関数
    void UpdateConstants(ID3D11DeviceContext* immediateContext, float deltaTime)override;

    void Render(ID3D11DeviceContext* immediateContext, float deltaTime) override;

    bool Uninitialize(ID3D11Device* device) override;

    void DrawGui() override;

    void SetUpActors()override;

    //シーンの自動登録
    static inline Scene::Autoenrollment<GameScene> _autoenrollment;

private:
    // ゲーム開始処理
    void OnGameStart();

    // ステージをロードする
    void LoadStage(STAGE_NAME stageId);

    // ステージごとのギミック生成
    void SpawnStageGimmicks(STAGE_NAME stageId);

    // ステージごとのBGMを設定する
    void SetupBGM(STAGE_NAME stageId);

    // STAGE_NAMEを変換する関数
    STAGE_NAME StringToStageName(const std::string& name);

    // デカールパス
    void GBufferDecalPass(ID3D11DeviceContext* immediateContext);

    void SpawnEnemy(
        const XMFLOAT3& pos,
        YarnEnemyType type, bool isBig,
        float speed = 2.0f, const XMFLOAT3& dir = { 1,0,0 });


public:
    EnemyTuning enemyTuning = {}; // 敵の調整用
    CoinTuning normalCoin = {}; // コインの調整用
    CoinTuning  bonusCoin = {}; // ボーナスコインの調整用


private:
    TPSCameraComponent* mainCameraComponent = nullptr;
    std::shared_ptr<ScissorsPlayer1> player;
    std::shared_ptr<NeedleEnemyActor> needleEnemyActor;

    std::shared_ptr<UIImageComponent> mouseCursorPar;   // マウスパー
    std::shared_ptr<UIImageComponent> mouseCursorGrab;  // マウス掴み
    std::shared_ptr<UIImageComponent> mouseCursorPause; // マウス　ポーズ

    std::shared_ptr<CoreAudioSourceComponent> audioBgmComponent; // BGMコンポーネント

    // デカール用
    std::unique_ptr<GeometricCube> decal_cube;// デバック用のキューブ
    // デカール用テクスチャ
    struct decal_texture
    {
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> color_shader_resource_view;
    };
    std::vector<decal_texture> decal_textures;

    std::vector<decal_data> decal_datas;
#if 0
    // デカール用定数バッファ
    struct gbuffer_decal_constants
    {
        DirectX::XMFLOAT4X4 decal_inverse_transform;
        DirectX::XMFLOAT4 decal_direction;
    };
    std::shared_ptr<ConstantBuffer<gbuffer_decal_constants>> decalCBuffer;
#else
    // 
    struct GameSceneConstants
    {
        DirectX::XMFLOAT2 playerScreenPosition = { 0.0f,0.0f };  //プレイヤーの場所　死亡演出に必要な定数バッファ
        DirectX::XMFLOAT2 screenSize = { 1280.0f,720.0f };  //スクリーンサイズ
        float radius = 0.0f;
        DirectX::XMFLOAT3 gameOverColor = { 0.965f,0.588f,0.475f };
    };
    std::shared_ptr<ConstantBuffer<GameSceneConstants>> gameSceneCBuffer;
#endif // 0

    // デカール用ピクセルシェーダー
    Microsoft::WRL::ComPtr<ID3D11PixelShader> gbuffer_decal_pixel_shader;

    // ジオメトリックシェーダー
    Microsoft::WRL::ComPtr<ID3D11VertexShader> geometric_primitive_vertex_shader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> geometric_primitive_input_layout;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> geometric_primitive_pixel_shader;

    float elapsedTime = 0.0f;
    const float decalRayCastInterval = 1.0f;   //

    std::unique_ptr<FrameBuffer> sceneFrameBuffer; // UIも含めたフレームバッファ
    Microsoft::WRL::ComPtr<ID3D11PixelShader> gameOverPs;

    float gameOverRadius = 0.0f;

};
