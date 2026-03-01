#include "pch.h"
#include "Player.h"
#include <float.h>

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

#include "Graphics/Core/Graphics.h"
#include "Physics/Physics.h"
#include "Core/ActorManager.h"

#include "Components/Render/PointLightComponent.h"

// UIで追加
#include "PlayerStateDerived.h"
#include "Components/Audio/CoreAudioSourceComponent.h"
#include "Engine/Scene/Scene.h"
#include "Game/Actors/Camera/Camera.h"
#include "Game/Actors/Stage/Stage.h"
#include "Game/DarkGame/Interactable.h"

void Player::Initialize(const Transform& transform)
{
    // 描画用コンポーネントを追加
    skeletalMeshComponent = this->AddComponent<class SkeletalMeshComponent>("skeletalComponent");
    skeletalMeshComponent->SetModel("./Data/Models/Characters/Aurora_FrozenHealth/Idle.gltf");
    for (auto& material : skeletalMeshComponent->model->materials)
    {
        //material.data.alphaMode = 2;    // 全てforwardで描画したいからBLENDに変更する

        if (material.name == "M_Aurora_Hair_Blonde_FrozenHearth")
        {// 髪の毛だったら
            //material.overridePipelineName = "characterHairForward";
            //material.data.alphaMode = 2;
        }
    }

    const std::vector<std::string> animationFilenames =
    {
        "./Data/Models/Characters/Aurora_FrozenHealth/Idle_Noise_A.glb",
        "./Data/Models/Characters/Aurora_FrozenHealth/Idle_Noise_B.glb",
        "./Data/Models/Characters/Aurora_FrozenHealth/Level_Start.glb",
        "./Data/Models/Characters/Aurora_FrozenHealth/Jog_Fwd_Start.glb",
        "./Data/Models/Characters/Aurora_FrozenHealth/Jog_Fwd.glb",
        "./Data/Models/Characters/Aurora_FrozenHealth/Jog_Fwd_Stop.glb",
        "./Data/Models/Characters/Aurora_FrozenHealth/Primary_Attack_Fast_A.glb",
        "./Data/Models/Characters/Aurora_FrozenHealth/Primary_Attack_Fast_B.glb",
        "./Data/Models/Characters/Aurora_FrozenHealth/Primary_Attack_Fast_C.glb",
        "./Data/Models/Characters/Aurora_FrozenHealth/Primary_Attack_Fast_D.glb",
        "./Data/Models/Characters/Aurora_FrozenHealth/Ability_R.glb",
        //"./Data/Models/Characters/Aurora_FrozenHealth/Emote_Ice_Sculpture.glb",
        //"./Data/Models/Characters/Aurora_FrozenHealth/HitReact_Back.glb",
        //"./Data/Models/Characters/Aurora_FrozenHealth/HitReact_Front.glb",
        //"./Data/Models/Characters/Aurora_FrozenHealth/HitReact_Left.glb",
        //"./Data/Models/Characters/Aurora_FrozenHealth/HitReact_Right.glb",
        //"./Data/Models/Characters/Aurora_FrozenHealth/Death.glb",
    };

    //skeletalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
    skeletalMeshComponent->AppendAnimations(animationFilenames);
    // アニメーションコントローラーを作成
    auto controller = std::make_shared<AnimationController>(skeletalMeshComponent.get());
    controller->AddAnimation("Idle", 0);
    controller->AddAnimation("Idle_Noise_A", 1);
    controller->AddAnimation("Idle_Noise_B", 2);
    controller->AddAnimation("Level_Start", 3);
    controller->AddAnimation("Jog_Fwd_Start", 4);
    controller->AddAnimation("Jog_Fwd", 5);
    controller->AddAnimation("Jog_Fwd_Stop", 6);
    controller->AddAnimation("Primary_Attack_Fast_A", 7);
    controller->AddAnimation("Primary_Attack_Fast_B", 8);
    controller->AddAnimation("Primary_Attack_Fast_C", 9);
    controller->AddAnimation("Primary_Attack_Fast_D", 10);
    controller->AddAnimation("Ability_R", 11);
    //controller->AddAnimation("Emote_Ice_Sculpture", 12);
    //controller->AddAnimation("HitReact_Back", 13);
    //controller->AddAnimation("HitReact_Front", 14);
    //controller->AddAnimation("HitReact_Left", 15);
    //controller->AddAnimation("HitReact_Right", 16);
    //controller->AddAnimation("Death", 17);

    // アニメーションコントローラーを character に追加
    this->SetAnimationController(controller);

    // ステートマシンを作成
    stateMachine_ = std::make_shared<StateMachine>();
    stateMachine_->RegisterState(std::make_unique<PlayerIdleState>(this));
    stateMachine_->RegisterState(std::make_unique<PlayerRunningState>(this));

    // ステートマシンを character に追加
    //this->SetStateMachine(stateMachine);
    // 初期ステートを設定
    stateMachine_->ChangeState("Idle");


    // 敵からの攻撃を受ける当たり判定用のコンポーネントを追加
    std::shared_ptr<CapsuleComponent> capsuleComponent = this->AddComponent<class CapsuleComponent>("capsuleComponent", "skeletalComponent");
    DirectX::XMFLOAT3 size = skeletalMeshComponent->GetModelSize();
    height = size.y;
    radius = size.x * 0.5f;
    capsuleComponent->SetRadiusAndHeight(radius, height);
    capsuleComponent->SetMass(mass);
    capsuleComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
    capsuleComponent->SetLayer(CollisionLayer::Player);
    capsuleComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
    capsuleComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    capsuleComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
    capsuleComponent->SetCollisionOffsetY(height * 0.5f);
    capsuleComponent->SetIsVisibleDebugBox(false);
    capsuleComponent->Initialize();

#if 0
    // ポイントライトコンポーネントを追加
    auto pointLightComponent = this->AddComponent<PointLightComponent>("pointLightComponent", "skeletalComponent");
    pointLightComponent->SetRelativeLocationDirect({ 0.4f, 2.1f, 0.3f });
    pointLightComponent->SetColor({ 1.0f, 1.0f, 1.0f });
    pointLightComponent->SetRange(1.5f);
    pointLightComponent->SetIntensity(10.0f);

#endif // 0
    AddHitCallback([&](std::pair<CollisionComponent*, CollisionComponent*> hitPair)
        {
            if (auto item = std::dynamic_pointer_cast<Stage>(hitPair.second->GetActor()))
            {
                return;
            }
            //std::string a = hitPair.second->GetActor()->GetName() + "is hit player";
            //OutputDebugStringA(a.c_str());
        });


    // 入力用のコンポーネントを追加
    inputComponent = this->AddComponent<class InputComponent>("inputComponent", "skeletalComponent");

    // 移動用コンポーネントを追加
    characterMovementComponent = this->AddComponent<CharacterMovementComponent>("movementComponent", "skeletalComponent");
    //characterMovementComponent->SetUseGravity(false);

    // 回転用コンポーネントを追加
    rotationComponent = this->AddComponent<class RotationComponent>("rotationComponent", "skeletalComponent");

    particleComponent = AddComponent<ParticleComponent>("particleComponent", "skeletalComponent");
    particleComponent->Load("./Data/Effect/Files/heartTestEffect.json");

}


void Player::Update(float elapsedTime)
{
    using namespace DirectX;

    // これは絶対入れる　アニメーションの更新をしているから
    Character::Update(elapsedTime);


    if (InputSystem::GetInputState("RB", InputStateMask::Trigger))
    {
        Logger::Log("RBが押された");
    }
    if (InputSystem::GetInputState("LockOn", InputStateMask::Trigger))
    {
        Logger::Log("LockOnが押された");
    }
    if (InputSystem::GetInputState("RT", InputStateMask::Trigger))
    {
        Logger::Log("RTが押された");
    }

#if 1
    auto intent = inputComponent->GetIntent();
    //characterMovementComponent->SetMoveDirection({ 1,0,0 });
    DirectX::XMFLOAT3 moveDir = { 0,0,0 };

    if (auto camera = dynamic_cast<MainCamera*>(GetOwnerScene()->GetActiveCamera()))
    {
        auto camForward = camera->CameraForwardXZ();
        auto camRight = camera->CameraRightXZ();

        // 左スティック入力
        float stickX = intent.leftMove.x;
        float stickZ = intent.leftMove.z;

        // カメラ基準の移動方向
        moveDir.x = camForward.x * stickZ + camRight.x * stickX;
        moveDir.z = camForward.z * stickZ + camRight.z * stickX;

        camera->AddYaw(intent.rightMove.x * elapsedTime * 3.0f);
        camera->AddPitch(intent.rightMove.y * elapsedTime * 3.0f);
    }


    characterMovementComponent->SetMoveDirection(moveDir);
    rotationComponent->SetDirection(moveDir);

#endif // 0

    //characterMovementComponent->SetMoveDirection(moveDir);
    //rotationComponent->SetDirection(moveDir);

    //particleComponent->Play();
    return;

    //if (GameManager::GetGameTimerStart() && !onceFrag)
    //{// ゲームが開始されたら
    //    state = State::Idle;
    //    onceFrag = true;
    //}


    // ステージ境界
    //DirectX::XMFLOAT3 pos = GetPosition();
    //pos.x = std::clamp(pos.x, -21.0f, 21.0f);
    //pos.z = std::clamp(pos.z, -16.0f, 16.0f);
    //pos.y = 0.8f;
    //SetPosition(pos);

    //DirectX::XMFLOAT3 leftPos = boxLeftHitComponent->GetRelativeLocation();
    //leftPos.y = leftFirstPos.y;
    //boxLeftHitComponent->SetRelativeLocationDirect(leftPos);

    //DirectX::XMFLOAT3 rightPos = boxRightHitComponent->GetRelativeLocation();
    //rightPos.y = rightFirstPos.y;
    //boxRightHitComponent->SetRelativeLocationDirect(rightPos);



    {// 無敵時間の更新
        if (invisibleTime > 0.0f)
        {
            invisibleTime -= elapsedTime;
        }
        if (bossInvisibleTime > 0.0f)
        {
            bossInvisibleTime -= elapsedTime;
        }
    }





    // プレイヤーの被弾時に色を変える処理
    if (isHitBlinking)
    {
        hitBlinkElapsed += elapsedTime;
        isRed = true;
        constexpr float blinkInterval = 0.1f; // 点滅間隔（秒）
        int blinkCount = static_cast<int>(hitBlinkElapsed / blinkInterval);
        isRed = (blinkCount % 2 == 0);

        //// 終了
        if (hitBlinkElapsed >= hitBlinkTotalTime)
        {
            isHitBlinking = false;
            isRed = false;
        }
    }
    if (isRed)
    {
        color.x = 3.0f;
        color.y = 0.0f;
        color.z = 0.0f;
    }
    else
    {
        color.x = 1.0f;
        color.y = 1.0f;
        color.z = 1.0f;
    }

#if USE_IMGUI
    ImGui::Begin("Player");
    ImGui::ColorEdit3("playerDamage", &color.x);
    ImGui::DragFloat3("playerDamageColor", &color.x);
    ImGui::End();
#endif
}

void Player::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    Character::DrawImGuiDetails();
#endif

}

// 遅延更新処理
void Player::LateUpdate(float elapsedTime)
{

}

void Player::Turn(float elapsedTime)
{
    using namespace DirectX;
#if 0
    //pad.Acquire();

//入力情報を取得
    float thumbStateLx = pad.ThumbStateLx();
    float thumbStateLy = pad.ThumbStateLy();


    DirectX::XMVECTOR PAD = XMVectorSet(thumbStateLx, 0.0f, thumbStateLy, 0.0f);
    PAD = DirectX::XMVector3Normalize(PAD);
    // 入力がほぼゼロなら回転しない
    if (std::abs(thumbStateLx) < 0.1f && std::abs(thumbStateLy) < 0.1f)
    {
        return;
    }

    //プレイヤーが向いている前方向ベクトルを求める
    front = GetForward();
    DirectX::XMVECTOR FRONT = DirectX::XMLoadFloat3(&front);
    FRONT = DirectX::XMVector3Normalize(FRONT);

    //プレイヤーの３軸を求める
    DirectX::XMVECTOR PX, PY, PZ;
    PY = DirectX::XMVectorSet(0, 1, 0, 0);
    PZ = FRONT;
    PX = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(PY, PZ));

    ////カメラの注視点と位置を取得する
    //Camera& camera = Camera::Instance();
    //DirectX::XMFLOAT4 cameraPosition = camera.GetCameraPosition();
    //DirectX::XMFLOAT4 cameraFocus = camera.GetFocus();
    //XMVECTOR CP = XMLoadFloat4(&cameraPosition);
    //XMVECTOR CF = XMLoadFloat4(&cameraFocus);
    ////カメラの座標軸を求める
    //XMVECTOR CX, CY, CZ;
    //CY = XMVectorSet(0, 1, 0, 0);
    //CZ = XMVector3Normalize(CF - CP);
    //CX = XMVector3Normalize(XMVector3Cross(CY, CZ));
    //CY = XMVector3Normalize(XMVector3Cross(CZ, CX));

    //const float inputDeadZone = 0.0001f;

    ////スティックの入力をカメラの座標軸に変換して進行方向のベクトルを求める
    ////カメラ基準の移動方向
    //DirectX::XMVECTOR CameraBasedMoveDirection = DirectX::XMVector3Normalize(thumbStateLx * CX + thumbStateLy * CZ);

    ////進行方向ベクトルとplayerの今の向いている方向の X 軸右ベクトルで内積する
    //float turningSpeedFactor = DirectX::XMVectorGetX(DirectX::XMVector3Dot(CameraBasedMoveDirection, PX));

    //if (std::fabsf(turningSpeedFactor) > inputDeadZone)
    //{
    //    turningSpeed = maxTurningSpeed * turningSpeedFactor;
    //}
    // 


    //float cross = DirectX::XMVectorGetX(DirectX::XMVector3Cross(CameraBasedMoveDirection, PZ));
    //if (cross > 0.0f)
    //{//左に回転する
    //    rotation.y -= rot;
    //}
    //else
    //{//右に回転する
    //    rotation.y += rot;
    //}

#if 0
    XMVECTOR Q = XMVectorSet(rotation.x, rotation.y, rotation.z, 0.0f);
    XMMATRIX R = XMMatrixRotationQuaternion(Q); //クォータニオンから回転行列を作成
    XMVECTOR Y = R.r[1];//モデルのローカルなY軸を取り出す
    Q = XMQuaternionNormalize(XMQuaternionMultiply(Q, XMQuaternionRotationAxis(Y, +DirectX::XMConvertToRadians(turningSpeed) * elapsedTime)));
    XMStoreFloat3(&rotation, Q);
#else
    //rotation.y += DirectX::XMConvertToRadians(turningSpeed) * elapsedTime;
#endif

#endif // 0
    return;

    float vx = inputComponent->GetThumbStateRx();
    float vz = inputComponent->GetThumbStateRy();
    float speed = DirectX::XMConvertToRadians(currentTurnSpeed) * elapsedTime;

    // 進行ベクトルがゼロベクトルの場合は処理する必要なし
    float length = sqrtf(vx * vx + vz * vz);
    if (length <= 0.001f)
    {
        return;
    }
    // 進行ベクトルを単位ベクトル化
    vx /= length;
    vz /= length;

    // 自身の回転値から前方向を求める
    float frontX = sinf(angle.y);
    float frontZ = cosf(angle.y);

    // 回転角を求めるため、2つの単位ベクトルの内積を計算する
    float dot = frontX * vx + frontZ * vz;

    // 内積値は-1.0～1.0で表現されており、2つの単位ベクトルの角度が
    // 小さいほど1.0に近づくという性質を利用して回転速度を調整する
    float rot = 1.0f - dot;

    if (rot > speed)
    {
        rot = speed;
    }

    // 左右判定を行うために２つの単位ベクトルの外積を計算する
    float cross = (frontX * vz) - (frontZ * vx);

    // 2Dの外積値が生の場合か負の場合かによって左右判定が行える
    // 左右判定を行うことによって左右回転を選択する
    if (cross < 0.0f)
    {
        angle.y += rot;
    }
    else
    {
        angle.y -= rot;
    }

    DirectX::XMFLOAT4 quaternion;
    DirectX::XMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(angle.x, angle.y, angle.z);
    DirectX::XMStoreFloat4(&quaternion, q);

    SetQuaternionRotation(quaternion);



#if 0
    float thumbStateRx = inputComponent->GetThumbStateRx();
    float thumbStateRy = inputComponent->GetThumbStateRy();

    DirectX::XMVECTOR PAD = XMVectorSet(thumbStateRx, 0.0f, thumbStateRy, 0.0f);
    PAD = DirectX::XMVector3Normalize(PAD);
    // 入力がほぼゼロなら回転しない
    if (std::abs(thumbStateRx) < 0.1f && std::abs(thumbStateRy) < 0.1f)
    {
        return;
    }

    //プレイヤーが向いている前方向ベクトルを求める
    front = GetForward();
    DirectX::XMVECTOR FRONT = DirectX::XMLoadFloat3(&front);
    FRONT = DirectX::XMVector3Normalize(FRONT);

    //プレイヤーの３軸を求める
    DirectX::XMVECTOR PX, PY, PZ;
    PY = DirectX::XMVectorSet(0, 1, 0, 0);
    PZ = FRONT;
    PX = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(PY, PZ));

    ////カメラの注視点と位置を取得する
    //Camera& camera = Camera::Instance();
    //DirectX::XMFLOAT4 cameraPosition = camera.GetCameraPosition();
    //DirectX::XMFLOAT4 cameraFocus = camera.GetFocus();
    //XMVECTOR CP = XMLoadFloat4(&cameraPosition);
    //XMVECTOR CF = XMLoadFloat4(&cameraFocus);
    ////カメラの座標軸を求める
    //XMVECTOR CX, CY, CZ;
    //CY = XMVectorSet(0, 1, 0, 0);
    //CZ = XMVector3Normalize(CF - CP);
    //CX = XMVector3Normalize(XMVector3Cross(CY, CZ));
    //CY = XMVector3Normalize(XMVector3Cross(CZ, CX));

    const float inputDeadZone = 0.0001f;

    ////スティックの入力をカメラの座標軸に変換して進行方向のベクトルを求める
    ////カメラ基準の移動方向
    //DirectX::XMVECTOR CameraBasedMoveDirection = DirectX::XMVector3Normalize(thumbStateLx * CX + thumbStateLy * CZ);

    DirectX::XMFLOAT3 moveDir = inputComponent->GetMoveInput();
    DirectX::XMVECTOR MoveDirection = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&moveDir));
    //float lenSq = moveDir.x * moveDir.x + moveDir.y * moveDir.y + moveDir.z * moveDir.z;
    //if (lenSq > 0.0001f)
    //{
    //    rotationComponent->SetDirection(moveDir);
    //}

    ////進行方向ベクトルとplayerの今の向いている方向の X 軸右ベクトルで内積する
    float turningSpeedFactor = DirectX::XMVectorGetX(DirectX::XMVector3Dot(MoveDirection, PX));

    if (std::fabsf(turningSpeedFactor) > inputDeadZone)
    {
        turningSpeed = maxTurningSpeed * turningSpeedFactor;
    }



    float cross = DirectX::XMVectorGetX(DirectX::XMVector3Cross(MoveDirection, PZ));
    if (cross > 0.0f)
    {//左に回転する
        angle.y -= rot;
    }
    else
    {//右に回転する
        angle.y += rot;
    }

#if 0
    XMVECTOR Q = XMVectorSet(rotation.x, rotation.y, rotation.z, 0.0f);
    XMMATRIX R = XMMatrixRotationQuaternion(Q); //クォータニオンから回転行列を作成
    XMVECTOR Y = R.r[1];//モデルのローカルなY軸を取り出す
    Q = XMQuaternionNormalize(XMQuaternionMultiply(Q, XMQuaternionRotationAxis(Y, +DirectX::XMConvertToRadians(turningSpeed) * elapsedTime)));
    XMStoreFloat3(&rotation, Q);
#else
    angle.y += DirectX::XMConvertToRadians(turningSpeed) * elapsedTime;
#endif
    DirectX::XMFLOAT4 quaternion;
    DirectX::XMVECTOR Quat = DirectX::XMQuaternionRotationRollPitchYaw(angle.x, angle.y, angle.z);
    DirectX::XMStoreFloat4(&quaternion, Quat);
    SetQuaternionRotation(quaternion);

#endif // 0
}

void Player::Move(float elapsedTime)
{
    //pad.Acquire();

    DirectX::XMFLOAT3 pos = GetPosition();
    //進行方向のベクトル取得
    DirectX::XMFLOAT3 moveVec = GetMoveVec();
    velocity.x = moveVec.x;
    velocity.z = moveVec.z;
    float moveSpeed = 5.0f * elapsedTime;
    pos.x += moveVec.x * moveSpeed;
    pos.z += moveVec.z * moveSpeed;
    SetPosition(pos);
    //position.x += velocity.x * moveSpeed;
    //position.z += velocity.z * moveSpeed;
}

//当たった時の処理
void Player::Hit()
{
    hp -= 1;
}


// インタラクト対象検索
IInteractable* Player::FindInteractable() const
{
    float bestDist = 2.0f;
    IInteractable* best = nullptr;

    for (auto& actor : GetOwnerScene()->GetActorManager()->GetAllActors())
    {
        auto interactable = dynamic_cast<IInteractable*>(actor.get());

        if (!interactable) continue;

        float dist = MathHelper::Distance(GetPosition(), actor->GetPosition());

        if (dist < bestDist)
        {
            bestDist = dist;
            best = interactable;
        }
    }

    return best;
}



//スティックの入力値から移動ベクトルを取得
DirectX::XMFLOAT3 Player::GetMoveVec()
{
    //pad.Acquire();
#if 0
    //入力情報を取得
    float ax = pad.ThumbStateLx();
    float ay = pad.ThumbStateLy();

    //カメラ方向とスティックの入力値によって進行方向を計算する
    Camera& camera = Camera::Instance();
    const DirectX::XMFLOAT3& cameraRight = camera.GetRight();
    const DirectX::XMFLOAT3& cameraFront = camera.GetFront();

    //カメラ右方向ベクトル[X軸]をXZ平面での単位ベクトルに変換
    float cameraRightX = cameraRight.x;
    float cameraRightZ = cameraRight.z;
    float cameraRightLength = std::sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
    if (cameraRightLength > 0.0f)
    {
        //単位ベクトル化
        cameraRightX /= cameraRightLength;
        cameraRightZ /= cameraRightLength;
    }

    //カメラの前方向のベクトル[Z軸]をXZ単位ベクトルに変換
    float cameraFrontX = cameraFront.x;
    float cameraFrontZ = cameraFront.z;
    float cameraFrontLength = std::sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
    if (cameraFrontLength > 0.0f)
    {
        //単位ベクトル化
        cameraFrontX /= cameraFrontLength;
        cameraFrontZ /= cameraFrontLength;
    }

    //進行ベクトルを計算する
    DirectX::XMFLOAT3 vec;
    vec.x = (cameraRightX * ax) + (cameraFrontX * ay);
    vec.z = (cameraRightZ * ax) + (cameraFrontZ * ay);
    //Y軸方向には移動しない
    vec.y = 0.0f;

    return vec;
#endif
    return DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
}

