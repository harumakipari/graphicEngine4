#include "pch.h"
#include "OdenIngredientActor.h"

#include <magic_enum.hpp>

#include "OdenTrashActor.h"
#include "Engine/Input/InputSystem.h"
#include "Engine/Scene/Scene.h"
#include "Physics/CollisionFunction.h"

#include "Game/OdenGame/OdenActors/OdenBubbleActor.h"  
#include "Game/OdenGame/OdenActors/OdenSlotActor.h"    
#include "Game/OdenGame/OdenActors/OdenTrashActor.h"    
#include "Game/OdenGame/OdenData/OdenGameParameter.h"
#include "Game/OdenGame/OdenManagers/OdenGameManager.h"
#include "Game/OdenGame/OdenTutorial/TutorialActor.h"
#include "Game/OdenGame/OdenTutorial/TutorialManager.h"


void OdenIngredientActor::Initialize(const Transform& transform)
{
    // イージングコンポーネントを追加
    easingRunner = std::make_unique<EasingRunner>();

    InitParam(ingredientName);
}

void OdenIngredientActor::Update(float deltaTime)
{
    // 位置を取得
    DirectX::XMFLOAT3 position = GetPosition();



#if 0
    // 浮いてくる処理
    if (position.y <= 0.95f)
    {
        constexpr float targetY = 1.0f;
        constexpr float speed = 3.0f; // 大きいほど速く浮く

        float t = std::clamp(deltaTime * speed, 0.0f, 1.0f);
        //Logger::Log("position y:" + std::to_string(position.y));
        position.y = std::lerp(position.y, targetY, t);
        SetPosition(position);
    }
#else
    constexpr float targetY = 1.0f;
    constexpr float speed = 3.0f;

    float t = std::clamp(deltaTime * speed, 0.0f, 1.0f);

    position.y = std::lerp(position.y, targetY, t);

    // 行き過ぎ防止（保険）
    if (position.y > targetY)
        position.y = targetY;

    SetPosition(position);

#endif // 0
    // イージングコンポーネントの更新
    easingRunner->Tick(deltaTime);

    // ドットモデルの表示非表示切り替え
    UpdateDotLineVisibility();

    // ポーズ中はゲーム入力を一切受け付けない
    if (Scene::GetCurrentScene()->IsPaused())
        return;

    // UIがマウスを使っているならゲーム操作しない
    if (Scene::GetCurrentScene()->GetUIManager()->IsMouseCaptured())
        return;

    DirectX::XMFLOAT2 cursor;
    // ビューポート外だったら、入力しない
    if (!InputSystem::GetMousePositionUI(cursor))
        return;

    // スタートとエンドの演出で入力を禁止していたら
    if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
    {
        if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
        {
            if (!gameManager->IsGameInputEnabled())
            {
                return;
            }
        }
    }


    // ① 押した瞬間：選択判定
    if (dragState == EOdenDragState::InSlot &&
        InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger))
    {
        TryBeginDrag(cursor);
    }

    // ② ドラッグ中：移動
    if (dragState == EOdenDragState::Dragging)
    {
        UpdateDragging(cursor);

        if (InputSystem::GetInputState("MouseLeft", InputStateMask::Release))
        {
            EndDrag(cursor);
        }
    }
}

void OdenIngredientActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    ImGui::Text("Drag State: %s", magic_enum::enum_name(dragState).data());
    ImGui::Text("HoverTarget: %s", magic_enum::enum_name(currentHoverTarget).data());

    ImGui::Text(U8("Topが提出の形: %s"), magic_enum::enum_name(odenOrientation.top).data());
    const OdenShapeData& shape = faceShapeTable.faceShapes.at(odenOrientation.top);
    ImGui::Text(U8("TopのShapeCategory: %s"), magic_enum::enum_name(shape.category).data());

    if (ImGui::Button(U8("横回転")))
    {
        RotateHorizontal();
    }

    if (ImGui::Button(U8("縦回転")))
    {
        RotateVertical();
    }

#endif
}

// 具材が横回転するときに呼ぶ関数
void OdenIngredientActor::RotateHorizontal()
{
    if (dragState == EOdenDragState::Dragging) // ドラック中は
        return;

    // 内部向き更新
    RotateHorizontalOrientation(odenOrientation);

    // 見た目の回転
    StartRotationAnim(ERotateType::Horizontal);

#if 0
    TestEasingHandler handler;
    handler.AddEasing(
        TestEaseType::OutCubic,  // くるっと感が出る
        startAngle,
        targetAngle,
        0.25f                   // 回転時間（秒）
    );

    handler.SetCompletedFunction([this, targetAngle]()
    {
        // 最終値を保証
        odenIngredientAngleDegree.z = targetAngle;

    });

    PropertyAccessor<float> accessor;
    accessor.getter = [this]() { return odenIngredientAngleDegree.z; };
    accessor.setter = [this](float v)
    {
        odenIngredientAngleDegree.z = v;
    };

    easingRunner->StartHandler(handler, accessor);
#endif // 0

}

// 具材が縦回転するときに呼ぶ関数
void OdenIngredientActor::RotateVertical()
{
    if (dragState == EOdenDragState::Dragging) // ドラック中は
        return;

    // 内部的に回転する　奥に
    RotateVerticalOrientation(odenOrientation);

    // 見た目の回転
    StartRotationAnim(ERotateType::Vertical);

#if 0
    float startAngle = odenIngredientAngleDegree.x;
    float targetAngle = startAngle + 90.0f;

    MathHelper::ClampEulerAngle(targetAngle);


    TestEasingHandler handler;
    handler.AddEasing(
        TestEaseType::OutCubic,  // くるっと感が出る
        startAngle,
        targetAngle,
        0.25f                   // 回転時間（秒）
    );

    handler.SetCompletedFunction([this, targetAngle]()
    {
        // 最終値を保証
        odenIngredientAngleDegree.x = targetAngle;

    });

    PropertyAccessor<float> accessor;
    accessor.getter = [this]() { return odenIngredientAngleDegree.x; };
    accessor.setter = [this](float v)
    {
        odenIngredientAngleDegree.x = v;
    };

    easingRunner->StartHandler(handler, accessor);

#endif // 0
}


// 食材がお題の上にあるか
bool OdenIngredientActor::IsHoveringOrder()const
{
    return hoverTarget == EHoverTarget::OrderBubble;
}

void OdenIngredientActor::InitParam(const std::string& ingredientName)
{
    //GetRootComponent()->SetRelativeEulerRotationDirect({ -25.0f,0.0f,0.0f });

    // モデル登録
    std::string parentName = ingredientName + "_model";
    ingredientModel = AddComponent<SkeletalMeshComponent>(parentName);
    std::string modelFileName = "./Data/Models/Oden_Ingredient/Oden_" + ingredientName + ".gltf";
    ingredientModel->SetModel(modelFileName.c_str());
    ingredientModel->SetValue(1);

    // 当たり判定を登録
    boxComponent = AddComponent<BoxComponent>("boxComponent", parentName);
    DirectX::XMFLOAT3 size = ingredientModel->GetModelSize();
    //boxComponent->SetBoxExtent({ size.x * 1.2f,size.y * 1.2f,size.z * 1.2f });
    boxComponent->SetBoxExtent({ 2.5f, 1.5f, 2.5f });
    boxComponent->SetMass(40.0f);
    boxComponent->SetLayer(CollisionLayer::Oden);
    boxComponent->Initialize();

    // 食材の種類を登録
    auto maybeEnum = magic_enum::enum_cast<EOdenType>(ingredientName);
    if (maybeEnum.has_value())
    {
        ingredientType = maybeEnum.value();
    }
    else
    {
        Logger::Error(U8("おでんの具材の名前のEOdenTypeが登録されていません！"));
        ingredientType = EOdenType::None; // たとえばデフォルト
    }

    // 食材の面に対応する形を登録
    auto it = OdenGameParameter::odenTypeShapes.find(ingredientName);
    if (it != OdenGameParameter::odenTypeShapes.end())
    {
        faceShapeTable = it->second;
    }
    else
    {
        Logger::Error(U8("OdenFaceShapeTable が見つかりません: ") + ingredientName);
        faceShapeTable = {}; // 空で初期化
    }

    // 値段を設定する
    // price = ingredientPriceTable[ingredientType];
    price = 100.0f; // 全て 100 点にする
#if 1

    float halfX = size.x + 0.001f;
    float halfY = size.y + 0.001f;
    float halfZ = size.z + 0.001f;

#if 1
    // 点線のモデルを置く場所と角度を設定する
    std::unordered_map<EOdenFace, FaceTransform> faceTransformTable =
    {
        { EOdenFace::Front,  {{ 0, 0, -halfZ }, { -90,   0, 0 }} },
        { EOdenFace::Back,   {{ 0, 0,  halfZ }, { 90, 0, 0 }} },

        { EOdenFace::Left,   {{ -halfX, 0, 0 }, { 0, 0, 90 }} },
        { EOdenFace::Right,  {{  halfX, 0, 0 }, { 0,  0, -90 }} },

        { EOdenFace::Top,    {{ 0,  0.05f, 0 }, { 0, 0, 0 }} },
        { EOdenFace::Bottom, {{ 0, -0.05f, 0 }, {  0, 0, 180 }} },
    };

#endif // 0

    for (auto& [face, shapeData] : faceShapeTable.faceShapes)
    {
        //if (face != EOdenFace::Top && face != EOdenFace::Bottom && face != EOdenFace::Left && face != EOdenFace::Right)
        //{
        //    continue;
        //}

        std::string shapeName = GetDotLineModelPath(ingredientType, face, shapeData);
        std::string modelDotLineFileName = "./Data/Models/Oden_DotLine/" + shapeName;

        if (shapeName == "")
        {
            continue;
        }
        std::string dotLineName = "dot_line" + std::string(magic_enum::enum_name(face));
        auto dot = AddComponent<DotLineMeshComponent>(dotLineName, parentName);
        dot->overrideDeferredPipelineName = "OdenDotLineMesh";
        dot->overrideForwardPipelineName = "OdenDotLineMesh";

        FaceTransform ft = ResolveFaceTransform(ingredientType, face, faceTransformTable);

        // const FaceTransform& ft = faceTransformTable[face];
        //dot->SetRelativeLocationDirect({ 0.0f,-0.03f,0.0f });
        //dot->SetRelativeLocationDirect({ 0.0f,0.03f,0.0f });
        dot->SetRelativeLocationDirect(ft.localOffset);
        dot->SetRelativeEulerRotationDirect(ft.localEuler);
        dot->SetRelativeScaleDirect(ft.localScale);

        dot->SetValue(1);

        dot->SetModel(modelDotLineFileName);
        dot->SetIsCastShadow(false);

        // ここでリスト追加
        dotLineByFace[face] = dot;
    }

#endif // 0

    // パーティクルコンポーネントを追加
    particleComponent = AddComponent<ParticleComponent>("twinkleComponent", parentName);
    particleComponent->Load("./Data/Effect/Files/sparklingEffect.json");
    particleComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });
    // ループ再生設定
    ParticleComponent::AddSettings settings
    {
        .loop = true, // ループ再生
    };
    particleComponent->SetAddSettings(settings);


}

// ドラック開始処理
void OdenIngredientActor::TryBeginDrag(const DirectX::XMFLOAT2& cursor)
{
#if 1
    if (auto tutorialActor = GetOwnerScene()->GetActorManager()->GetActorByName("OdenTutorialActor"))
    {// チュートリアルだったら
        if (auto tutorial = std::dynamic_pointer_cast<TutorialActor>(tutorialActor))
        {
            if (auto currentStep = tutorial->GetTutorialManager()->GetCurrentState())
            {
                if (!currentStep->IsIngredientGrabEnabled())
                {
                    return; // ← Raycast すらしない
                }
            }
        }
    }

    HitResultWithActor result;
    if (!CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::Oden)))
        return;

    if (result.actor != this)
        return;

    if (auto tutorialActor = GetOwnerScene()->GetActorManager()->GetActorByName("OdenTutorialActor"))
    {// チュートリアルだったら
        if (auto tutorial = std::dynamic_pointer_cast<TutorialActor>(tutorialActor))
        {
            if (auto currentStep = tutorial->GetTutorialManager()->GetCurrentState())
            {
                auto res = currentStep->CanGrabIngredient(ingredientType, GetCurrentShape().category);
                if (res != ETutorialIngredientResult::Allow)
                {
                    currentStep->OnDeniedGrab(shared_from_this());
                    return;
                }
                currentStep->OnAllowGrab(shared_from_this());
            }
        }
    }

    dragState = EOdenDragState::Dragging;
    grabbedFromSlot = currentSlot;
    CoreAudio::PlayOneShot(L"./Data/Sound/SE/grab_ingredient.wav", 1.0f);
    Logger::Log(U8("おでんを掴んだ！"));
    // チュートリアルが今掴まれている具材を知る溜めの関数
    NotifyGrabbed();
#else
    HitResultWithActor result;
    if (CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::Oden)))
    {
        if (result.actor == this)
        {
            dragState = EOdenDragState::Dragging;
            grabbedFromSlot = currentSlot;

            // ★ 重要：スロットから外す
            if (auto slot = currentSlot.lock())
            {
                slot->RemoveIngredient();
            }
            currentSlot.reset();

            CoreAudio::PlayOneShot(L"./Data/Sound/SE/grab_ingredient.wav", 1.0f);
            Logger::Log(U8("おでんを掴んだ！"));
        }
    }

#endif // 0
}

// ドラック中の処理
void OdenIngredientActor::UpdateDragging(const DirectX::XMFLOAT2& cursor) 
{
    HitResultWithActor result;
    if (CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::WorldStatic)))
    {
        XMFLOAT3 pos = result.hitPoint;
        pos.y = 1.0f;
        SetPosition(pos);

        //DebugDrawManager::DrawSphere(pos, 0.5f, { 1,1,0,1 });
    }
    UpdateHoverTarget(cursor);
}

void OdenIngredientActor::EndDrag(const DirectX::XMFLOAT2& cursor)
{
    dragState = EOdenDragState::InSlot;
    // おでんを離す音　SE再生
    CoreAudio::PlayOneShot(L"./Data/Sound/SE/release_ingredient.wav", 1.0f);
    switch (DetectHoverTarget(cursor))
    {
    case EHoverTarget::OdenSlot:
        {
            if (auto tutorialActor = GetOwnerScene()->GetActorManager()->GetActorByName("OdenTutorialActor"))
            {// チュートリアルだったら
                if (auto tutorial = std::dynamic_pointer_cast<TutorialActor>(tutorialActor))
                {
                    if (auto currentStep = tutorial->GetTutorialManager()->GetCurrentState())
                    {
                        if (!currentStep->CanSwapIngredient())
                        {
                            ReturnToSlot();
                            return;
                        }
                    }
                }
            }
            auto actor = GetHoverSlot(cursor);
            if (auto slot = std::static_pointer_cast<OdenSlotActor>(actor.lock()))
                SwapWithSlot(slot);
            else
                ReturnToSlot();
            Logger::Log(U8("枠の上でマウスクリックを離した！"));
            break;
        }
    case EHoverTarget::OrderBubble:
        TrashSelf();
        //DebugDrawManager::DrawSphere({ 0, 0, 0 }, 13.5f, { 1,0,1,1 });
        Logger::Log(U8("お題の上でマウスクリックを離した！"));
        break;
    case EHoverTarget::TrashBin:
        // 食材を破棄した時に呼ぶ関数
        //DebugDrawManager::DrawSphere({ 0, 0, 0 }, 13.5f, { 1,0,1,1 });
        TrashSelf();
        Logger::Log(U8("ゴミ箱の上でマウスクリックを離した！"));
        //HandleDropOnTrash();
        break;
    default:
        ReturnToSlot();
        break;
    }

    Logger::Log(U8("おでんを離した！"));
}

// マウスを離した時のターゲットを返す
OdenIngredientActor::EHoverTarget OdenIngredientActor::DetectHoverTarget(const DirectX::XMFLOAT2& cursor) 
{
    HitResultWithActor result;
    if (CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::OdenHoverTarget)))
    {
        // マウスカーソルが
        if (auto odenSlot = dynamic_cast<OdenSlotActor*>(result.actor))
        {// おでんの枠モデルに当たっていたら、
            hoverTarget= EHoverTarget::OdenSlot;
            return EHoverTarget::OdenSlot;
        }
        if (auto odenBubble = dynamic_cast<OdenBubbleActor*>(result.actor))
        {// お題の上だったら
            if (odenBubble->CanAcceptIngredient())
            {// オーダー可能な時
                odenBubble->OnIngredientDropped(*this); // これちょっと怖い。。
                hoverTarget = EHoverTarget::OrderBubble;
                return EHoverTarget::OrderBubble;
            }
            hoverTarget = EHoverTarget::None;
            return EHoverTarget::None;
        }
        if (auto odenTrash = dynamic_cast<OdenTrashActor*>(result.actor))
        {// ゴミ箱の上だったら
            hoverTarget = EHoverTarget::TrashBin;
            return EHoverTarget::TrashBin;
        }

        //DebugDrawManager::DrawSphere({ 0, 0, 0 }, 13.5f, { 1,0,1,1 });

    }
    //DebugDrawManager::DrawSphere({ 0, 0, 0 }, 13.5f, { 1,0,1,1 });
    Logger::Log(U8("何もないところでマウスクリックを離した！"));
    hoverTarget = EHoverTarget::None;
    return EHoverTarget::None;
}

// 離したときのターゲットのアクターを返す
std::weak_ptr<Actor> OdenIngredientActor::GetHoverSlot(const DirectX::XMFLOAT2& cursor)
{
    HitResultWithActor result;
    if (CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::OdenHoverTarget)))
    {// マウスカーソルが
        return result.actor->shared_from_this();
    }
    Logger::Warning(U8("離したときのターゲットのアクターが nullptr です！"));

    return std::weak_ptr<Actor>(); // ← 空の weak_ptr
}

// スロットで入れ替える（Drag中は呼び出さない)
void OdenIngredientActor::SwapWithSlot(const std::weak_ptr<OdenSlotActor>& targetSlot)
{
    auto from = grabbedFromSlot.lock();
    auto to = targetSlot.lock();

    if (!from || !to)
        return;

    if (from == to)
    {
        // 同じスロットなら戻すだけ
        ReturnToSlot();
        return;
    }

    auto other = to->RemoveIngredient();

    // 自分を target に
    to->SetIngredient(std::static_pointer_cast<OdenIngredientActor>(shared_from_this()));
    SetCurrentSlot(to);
    SetPosition(to->GetPosition());

    // 相手を元スロットに
    from->SetIngredient(other);
    if (other)
    {
        other->SetCurrentSlot(from);
        other->SetPosition(from->GetPosition());
    }


    if (auto tutorialActor = GetOwnerScene()->GetActorManager()->GetActorByName("OdenTutorialActor"))
    {// チュートリアルだったら
        if (auto tutorial = std::dynamic_pointer_cast<TutorialActor>(tutorialActor))
        {// スワップを通知する
            tutorial->GetTutorialManager()->NotifyIngredientSwapped();
        }
    }

}

// 元のスロットに戻す
void OdenIngredientActor::ReturnToSlot()
{
    auto slot = grabbedFromSlot.lock();
    if (!slot)
        return;

    // スロットに戻す
    slot->SetIngredient(std::static_pointer_cast<OdenIngredientActor>(shared_from_this()));
    SetCurrentSlot(slot);
    SetPosition(slot->GetPosition());
}

// 横回転時の面の向き状態を更新
void OdenIngredientActor::RotateHorizontalOrientation(OdenOrientation& o)
{// 右に回転する
    OdenOrientation old = o;
    o.top = old.left;
    o.left = old.bottom;
    o.bottom = old.right;
    o.right = old.top;
}

// 縦回転時の面の向き状態を更新
void OdenIngredientActor::RotateVerticalOrientation(OdenOrientation& o)
{// 奥に向かって回転する
    OdenOrientation old = o;
    o.top = old.front;
    o.front = old.bottom;
    o.back = old.top;
    o.bottom = old.back;
}

// 食材を破棄した時に呼ぶ関数
void OdenIngredientActor::TrashSelf()
{
    currentSlot.lock()->RemoveIngredient();
    MarkPendingKill();
}

// 食材の向きからクォータニオンを求める
XMVECTOR OdenIngredientActor::FaceToQuat(const EOdenFace face) const
{
    using namespace DirectX;

    switch (face)
    {
    case EOdenFace::Top:    return XMVectorSet(0, 1, 0, 0);
    case EOdenFace::Bottom: return XMVectorSet(0, -1, 0, 0);
    case EOdenFace::Right:  return XMVectorSet(1, 0, 0, 0);
    case EOdenFace::Left:   return XMVectorSet(-1, 0, 0, 0);
    case EOdenFace::Front:  return XMVectorSet(0, 0, -1, 0);
    case EOdenFace::Back:   return XMVectorSet(0, 0, 1, 0);
    }

    return XMVectorZero();
}

// 回転を始める
void OdenIngredientActor::StartRotationAnim(const ERotateType rotateType)
{
    using namespace DirectX;

    startOrientation = XMLoadFloat4(&visualOrientation);

    switch (rotateType)
    {
    case ERotateType::Vertical:
        verticalAngle = 90.0f;
        MathHelper::ClampEulerAngle(verticalAngle);
        targetRotation = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), XMConvertToRadians(verticalAngle));
        break;
    case ERotateType::Horizontal:
        horizontalAngle = -90.0f;
        MathHelper::ClampEulerAngle(horizontalAngle);
        targetRotation = XMQuaternionRotationAxis(XMVectorSet(0, 0, 1, 0), XMConvertToRadians(horizontalAngle));
        break;
    }
    XMVECTOR target = OrientationToQuat(odenOrientation);

    //XMVECTOR q = XMQuaternionMultiply(startOrientation, targetRotation);
    //q = XMQuaternionNormalize(q);
    //XMStoreFloat4(&visualOrientation, q);

    //ingredientModel->SetRelativeRotationDirect(visualOrientation);


    TestEasingHandler handler;
    handler.AddEasing(TestEaseType::OutCubic, 0.f, 1.0f, 0.25f);

    handler.SetCompletedFunction([this, target]()
    {
        XMStoreFloat4(&visualOrientation, target);
        ingredientModel->SetRelativeRotationDirect(visualOrientation);
        //ingredientModel->SetWorldRotationDirect(visualOrientation);
    });

    PropertyAccessor<float> accessor;
    accessor.getter = [this]() { return 1.0f; };
    accessor.setter = [this](float t)
    {
        XMVECTOR q = XMQuaternionSlerp(startOrientation, XMQuaternionMultiply(startOrientation, targetRotation), t);
        //Logger::Log(U8("t の値") + std::to_string(t));
        q = XMQuaternionNormalize(q);
        XMStoreFloat4(&visualOrientation, q);

        ingredientModel->SetRelativeRotationDirect(visualOrientation);
        //ingredientModel->SetWorldRotationDirect(visualOrientation);
    };

    easingRunner->StartHandler(handler, accessor);
}

// お題出現時の動き
void OdenIngredientActor::AppearIngredient()
{

}

// 点線のモデルパスを取得する関数
std::string OdenIngredientActor::GetDotLineModelPath(EOdenType ingredientType,
                                                     EOdenFace face,
                                                     const OdenShapeData& shapeData)
{
    struct DotLineKey
    {
        EOdenType ingredient;
        EOdenFace face;

        bool operator==(const DotLineKey& rhs) const
        {
            return ingredient == rhs.ingredient && face == rhs.face;
        }
    };

    struct DotLineKeyHash
    {
        size_t operator()(const DotLineKey& k) const
        {
            return (size_t)k.ingredient ^ ((size_t)k.face << 8);
        }
    };

    static const std::unordered_map<DotLineKey, std::string, DotLineKeyHash>
        DotLineByIngredientFace =
        {
            // --- ちくわ ---
            {{ EOdenType::Chikuwa, EOdenFace::Front   }, "Oden_DotLine_LongRect.gltf" },
            {{ EOdenType::Chikuwa, EOdenFace::Back   }, "Oden_DotLine_LongRect.gltf" },
            {{ EOdenType::Chikuwa, EOdenFace::Top   }, "Oden_DotLine_LongRect.gltf" },
            {{ EOdenType::Chikuwa, EOdenFace::Bottom   }, "Oden_DotLine_LongRect.gltf" },
            {{ EOdenType::Chikuwa, EOdenFace::Left   }, "Oden_DotLine_Small_Donut.gltf" },
            {{ EOdenType::Chikuwa, EOdenFace::Right   }, "Oden_DotLine_Small_Donut.gltf" },

            // --- 大根 ---
            {{ EOdenType::Daikon, EOdenFace::Left   }, "Oden_DotLine_Rect.gltf" },
            {{ EOdenType::Daikon, EOdenFace::Right   }, "Oden_DotLine_Rect.gltf" },

            // --- ごぼてん ---
            {{ EOdenType::Goboten, EOdenFace::Front   }, "Oden_DotLine_LongRect.gltf" },
            {{ EOdenType::Goboten, EOdenFace::Back   }, "Oden_DotLine_LongRect.gltf" },
            {{ EOdenType::Goboten, EOdenFace::Top   }, "Oden_DotLine_LongRect.gltf" },
            {{ EOdenType::Goboten, EOdenFace::Bottom   }, "Oden_DotLine_LongRect.gltf" },
            {{ EOdenType::Goboten, EOdenFace::Left   }, "Oden_DotLine_Small_Circle.gltf" },
            {{ EOdenType::Goboten, EOdenFace::Right   }, "Oden_DotLine_Small_Circle.gltf" },

            // --- ドーナツ ---
            {{ EOdenType::Donut, EOdenFace::Top   }, "Oden_DotLine_Donut.gltf" },
            {{ EOdenType::Donut, EOdenFace::Bottom   }, "Oden_DotLine_Donut.gltf" },

            // --- ケーキ ---
            {{ EOdenType::Cake, EOdenFace::Top   }, "Oden_DotLine_Cake.gltf" },
            {{ EOdenType::Cake, EOdenFace::Bottom   }, "Oden_DotLine_Cake.gltf" },
            {{ EOdenType::Cake, EOdenFace::Back   }, "Oden_DotLine_Cake_Back.gltf" },

            // --- たまご ---
            {{ EOdenType::Egg, EOdenFace::Front   }, "Oden_DotLine_Egg.gltf" },
            {{ EOdenType::Egg, EOdenFace::Back   }, "Oden_DotLine_Egg.gltf" },
            {{ EOdenType::Egg, EOdenFace::Left   }, "Oden_DotLine_Egg.gltf" },
            {{ EOdenType::Egg, EOdenFace::Right   }, "Oden_DotLine_Egg.gltf" },

            // --- しらたき ---
            {{ EOdenType::Shirataki, EOdenFace::Left   }, "Oden_DotLine_Small_Circle.gltf" },
            {{ EOdenType::Shirataki, EOdenFace::Right   }, "Oden_DotLine_Small_Circle.gltf" },

            // --- こぶむすび ---
            {{ EOdenType::Kobumusubi, EOdenFace::Left   }, "Oden_DotLine_Small_Circle.gltf" },
            {{ EOdenType::Kobumusubi, EOdenFace::Right   }, "Oden_DotLine_Small_Circle.gltf" },

        };

    // ① 食材 × 面 専用があれば最優先
    auto it = DotLineByIngredientFace.find({ ingredientType, face });
    if (it != DotLineByIngredientFace.end())
    {
        return it->second;
    }


    std::string modelPath;
    switch (shapeData.category)
    {
    case EOdenShapeCategory::TriangleLike:
        modelPath = "Oden_DotLine_Triangle.gltf";
        break;

    case EOdenShapeCategory::RoundLike:
        modelPath = "Oden_DotLine_Circle.gltf";
        break;

    case EOdenShapeCategory::SquareLike:
        modelPath = "Oden_DotLine_Rect.gltf";
        break;
    case EOdenShapeCategory::RibbonLike:
        modelPath = "Oden_DotLine_Ribbon.gltf";
        break;
    default:
        modelPath = "";
        break;
    }

    return modelPath;
}

// 点線モデルの角度とoffsetを取得する関数
OdenIngredientActor::FaceTransform OdenIngredientActor::ResolveFaceTransform(
    EOdenType ingredient,
    EOdenFace face,
    const std::unordered_map<EOdenFace, FaceTransform>& baseTable
)
{
    using FaceKey = std::pair<EOdenType, EOdenFace>;

    struct FaceKeyHash
    {
        size_t operator()(const FaceKey& k) const
        {
            return (size_t)k.first ^ ((size_t)k.second << 8);
        }
    };

    static const std::unordered_map<FaceKey, FaceTransform, FaceKeyHash>
        IngredientFaceOverride =
        {
            // --- ちくわ ---
            {
                { EOdenType::Chikuwa, EOdenFace::Top },
                { { 0, 0.08f, 0 }, { 0, 0, 0 } }
            },
            {
                { EOdenType::Chikuwa, EOdenFace::Bottom },
                { { 0, -0.08f, 0 }, { 0, 0, 180 } }
            },
            {
                { EOdenType::Chikuwa, EOdenFace::Front },
                { { 0, 0.0f, 0 }, { -90, 0, 0 } }
            },
            {
                { EOdenType::Chikuwa, EOdenFace::Back },
                { { 0, 0.0f, 0 }, { 90, 0, 0 } }
            },
            {
                { EOdenType::Chikuwa, EOdenFace::Left },
                { { -3.181f, 0.0f, 0 }, { -0, 0, 0 } }
            },
            {
                { EOdenType::Chikuwa, EOdenFace::Right },
                { { 0.08f, 0.0f, 0 }, { 0, 0, 0 } }
            },

            // --- ごぼてん ---
            {
                { EOdenType::Goboten, EOdenFace::Top },
                { { 0, 0.08f, 0 }, { 0, 0, 0 } }
            },
            {
                { EOdenType::Goboten, EOdenFace::Bottom },
                { { 0, -0.08f, 0 }, { 0, 0, 180 } }
            },
            {
                { EOdenType::Goboten, EOdenFace::Front },
                { { 0, 0.0f, 0 }, { -90, 0, 0 } }
            },
            {
                { EOdenType::Goboten, EOdenFace::Back },
                { { 0, 0.0f, 0 }, { 90, 0, 0 } }
            },
            {
                { EOdenType::Goboten, EOdenFace::Left },
                { { -3.3f, 0.0f, 0 }, { -0, 0, 0 } ,{1.0f,0.78f,0.78f}}
            },
            {
                { EOdenType::Goboten, EOdenFace::Right },
                { { 0.08f, 0.0f, 0 }, { 0, 0, 0 },{1.0f,0.78f,0.78f}}
            },


            // --- 大根 ---
            {
                { EOdenType::Daikon, EOdenFace::Left },
                { { -0.75f, 0.0f, 0 }, { 90, 0, 90 } }
            },
            {
                { EOdenType::Daikon, EOdenFace::Top },
                { { 0, 0.03f, 0 }, { 0, 0, 0 } }
            },
            {
                { EOdenType::Daikon, EOdenFace::Bottom },
                { { 0, -0.03f, 0 }, { 0, 0, 180 } }

            },
            {
                { EOdenType::Daikon, EOdenFace::Right },
                { { 1.4f, 0.0f, 0.f }, { 90, 0, 90 } }
            },
            {
                { EOdenType::Daikon, EOdenFace::Back },
                { { 0.f, 0.0f, 0.8f }, { 90, 0, 0 } }
            },
            {
                { EOdenType::Daikon, EOdenFace::Front },
                { { 0.0f, 0.0f, -1.3f }, { 90, 0, 0 } }
            },

            // --- こんにゃく ---
            {
                { EOdenType::Konnyaku, EOdenFace::Top },
                { { 0, 0.03f, 0 }, { 0, 0, 0 } }
            },
            {
                { EOdenType::Konnyaku, EOdenFace::Bottom },
                { { 0, -0.03f, 0 }, { 0, 0, 180 } }

            },
            {
                { EOdenType::Konnyaku, EOdenFace::Left },
                { { -0.3f, 0.0f, -0.f }, { 90, -37, 90 } }
            },
            {
                { EOdenType::Konnyaku, EOdenFace::Right },
                { { 0.9f, 0.0f, -0.4f }, { 90, 37, 90 } }
            },
            {
                { EOdenType::Konnyaku, EOdenFace::Back },
                { { 0.f, 0.0f, 0.3f }, { 90, 0, 0 } }
            },
            {
                { EOdenType::Konnyaku, EOdenFace::Front },
                { { -0.1f, 0.0f, -0.9f }, { -90, 0, 0 } }
            },

            // --- はんぺん ---
            {
                { EOdenType::Hanpen, EOdenFace::Top },
                { { 0, 0.03f, 0 }, { 0, 0, 0 } }
            },
            {
                { EOdenType::Hanpen, EOdenFace::Bottom },
                { { 0, -0.03f, 0 }, { 0, 0, 180 } }
            },
            {
                { EOdenType::Hanpen, EOdenFace::Left },
                { { -0.3f, 0.0f, -0.f }, { 90, -37, 90 } }
            },
            {
                { EOdenType::Hanpen, EOdenFace::Right },
                { { 0.9f, 0.0f, -0.4f }, { 90, 37, 90 } }
            },
            {
                { EOdenType::Hanpen, EOdenFace::Back },
                { { 0.f, 0.0f, 0.3f }, { 90, 0, 0 } }
            },
            {
                { EOdenType::Hanpen, EOdenFace::Front },
                { { -0.1f, 0.0f, -0.9f }, { -90, 0, 0 } }
            },

            // --- ケーキ ---
            {
                { EOdenType::Cake, EOdenFace::Top },
                { { 0, 0.03f, 0 }, { 0, 0, 0 } }
            },
            {
                { EOdenType::Cake, EOdenFace::Bottom },
                { { 0, -0.03f, 0 }, { 0, -180, 180 } }

            },
            {
                { EOdenType::Cake, EOdenFace::Left },
                { { -0.93f, 0.1f, -0.1f }, { -90, 90, 3 },{0.82f,1.0f,1.15f} }
            },
            {
                { EOdenType::Cake, EOdenFace::Right },
                { { 1.6f, 0.05f, -0.05f }, { 90, 0, 90 } ,{0.79f,1.44f,1.53f}}
            },
            {
                { EOdenType::Cake, EOdenFace::Back },
                { { 0.f, 0.f, 0.f }, { 0, -0, 0 } ,{1.0f,1.0f,1.0f}}
            },
            {
                { EOdenType::Cake, EOdenFace::Front },
                { { -0.1f, 0.05f, -0.85f }, { 90, 21, 0 },{1.03f,1.0f,1.5f} }
            },


            // --- ドーナツ ---
            {
                { EOdenType::Donut, EOdenFace::Left },
                { { -0.8f, 0.f, -0.f }, { -90, 90, 0 },{1.0f,1.0f,1.0f} }
            },
            {
                { EOdenType::Donut, EOdenFace::Right },
                { { 0.8f, 0.0f, -0.0f }, { 90, 90, 0 } ,{1.0f,1.0f,1.0f} }
            },
            {
                { EOdenType::Donut, EOdenFace::Back },
                { { 0.f, 0.f, 0.7f }, { 90, 0, 0 } ,{1.0f,1.0f,1.0f} }
            },
            {
                { EOdenType::Donut, EOdenFace::Front },
                { { -0.f, 0.0f, -0.8f }, { -90, 0, 0 },{1.0f,1.0f,1.0f} }
            },


            // --- たまご ---
            {
                { EOdenType::Egg, EOdenFace::Top },
                { { 0, 0.05f, 0 }, { 0, 0, 0 } ,{0.78f,0.78f,0.78f}}
            },
            {
                { EOdenType::Egg, EOdenFace::Bottom },
                { { 0, 0.05f, 0 }, { -180, 0, 0 } ,{0.78f,0.78f,0.78f}}

            },
            {
                { EOdenType::Egg, EOdenFace::Left },
                { { 0.4f, 0.0f, -0.f }, { 0, -90, 0 }  ,{1.0f,1.0f,1.0f}}
            },
            {
                { EOdenType::Egg, EOdenFace::Right },
                { { -0.4f, 0.0f, 0.0f }, { 0, 90, 0 } ,{1.0f,1.0f,1.0f}}
            },
            {
                { EOdenType::Egg, EOdenFace::Back },
                { { 0.f, 0.0f, -0.5f }, { 0, 0, 0 }  ,{1.0f,1.0f,1.0f}}
            },
            {
                { EOdenType::Egg, EOdenFace::Front },
                { { 0.0f, 0.0f, -1.0f }, { 0, 0, 0 }  ,{1.0f,1.0f,1.0f}}
            },


            // --- つくね ---
            {
                { EOdenType::Tsukune, EOdenFace::Top },
                { { 0, 0.05f, 0 }, { 0, 0, -15 } ,{0.78f,0.78f,0.78f}}
            },
            {
                { EOdenType::Tsukune, EOdenFace::Bottom },
                { { 0, -0.25f, 0 }, { -180, 0, 0 } ,{0.78f,0.78f,0.78f}}

            },
            {
                { EOdenType::Tsukune, EOdenFace::Left },
                { { -0.0f, 0.0f, -0.f }, { 0, -0, 90 }  ,{0.78f,0.78f,0.78f}}
            },
            {
                { EOdenType::Tsukune, EOdenFace::Right },
                { { 0.8f, 0.02f, 0.05f }, { 0, 0, 90 } ,{0.78f,0.78f,0.78f}}
            },
            {
                { EOdenType::Tsukune, EOdenFace::Back },
                { { 0.f, 0.0f, 0.3f }, { 90, 0, 0 }  ,{0.78f,0.78f,0.78f}}
            },
            {
                { EOdenType::Tsukune, EOdenFace::Front },
                { { 0.07f, 0.0f, -0.8f }, { 90, 0, 0 }  ,{0.78f,0.78f,0.78f}}
            },

            // --- しらたき ---
            {
                { EOdenType::Shirataki, EOdenFace::Left },
                { { 0.f, 0.0f, -0.f }, { 0, 0, 180 } }
            },
            {
                { EOdenType::Shirataki, EOdenFace::Right },
                { { 0.f, 0.0f, -0.f }, { 0, 0, 0 } }
            },
            {
                { EOdenType::Shirataki, EOdenFace::Back },
                { { 0.f, 0.0f, -0.23f }, { 90, 0, 0 } }
            },
            {
                { EOdenType::Shirataki, EOdenFace::Front },
                { { 0.0f, 0.0f, 0.33f }, { -90, 0, 0 } }
            },
            {
                { EOdenType::Shirataki, EOdenFace::Top },
                { { 0.f, -0.25f, -0.0f }, {0, 0, 0 } }
            },
            {
                { EOdenType::Shirataki, EOdenFace::Bottom },
                { { 0.0f, 0.35f, 0.f }, { 0, 0, -180 } }
            },


            // --- こぶむすび ---
            {
                { EOdenType::Kobumusubi, EOdenFace::Left },
                { { 0.f, 0.0f, -0.f }, { 0, 0, 180 } }
            },
            {
                { EOdenType::Kobumusubi, EOdenFace::Right },
                { { 0.f, 0.0f, -0.f }, { 0, 0, 0 } }
            },
            {
                { EOdenType::Kobumusubi, EOdenFace::Back },
                { { 0.f, 0.0f, -0.23f }, { 90, 0, 0 } }
            },
            {
                { EOdenType::Kobumusubi, EOdenFace::Front },
                { { 0.0f, 0.0f, 0.33f }, { -90, 0, 0 } }
            },
            {
                { EOdenType::Kobumusubi, EOdenFace::Top },
                { { 0.f, -0.25f, -0.0f }, {0, 0, 0 } }
            },
            {
                { EOdenType::Kobumusubi, EOdenFace::Bottom },
                { { 0.0f, 0.35f, 0.f }, { 0, 0, -180 } }
            },

        };

    auto base = baseTable.at(face);

    auto it = IngredientFaceOverride.find({ ingredient, face });
    if (it != IngredientFaceOverride.end())
    {
        return it->second; // override
    }

    return base;
}

// 点線の表示・非表示を更新する関数
void OdenIngredientActor::UpdateDotLineVisibility()
{
    for (auto& [face, weakDot] : dotLineByFace)
    {
        if (auto dot = weakDot.lock())
        {
            bool isTopFace = (face == odenOrientation.top);
            dot->SetIsVisible(isTopFace);
        }
    }
}

// チュートリアルが今掴まれている食材を知る
void OdenIngredientActor::NotifyGrabbed()
{
    if (auto tutorialActor = GetOwnerScene()->GetActorManager()->GetActorByName("OdenTutorialActor"))
    {// チュートリアルだったら
        if (auto tutorial = std::dynamic_pointer_cast<TutorialActor>(tutorialActor))
        {
            tutorial->OnIngredientGrabbed(shared_from_this());
        }
    }
}

// ドラック中にどこに食材があるかを判別する
void OdenIngredientActor::UpdateHoverTarget(const DirectX::XMFLOAT2& cursor)
{
    HitResultWithActor result;
    if (CollisionFunction::RaycastFromMouse(cursor, result,
        CollisionHelper::ToBit(CollisionLayer::OdenHoverTarget)))
    {
        if (dynamic_cast<OdenBubbleActor*>(result.actor))
        {
            hoverTarget = EHoverTarget::OrderBubble;
            return;
        }
        if (dynamic_cast<OdenSlotActor*>(result.actor))
        {
            hoverTarget = EHoverTarget::OdenSlot;
            return;
        }
        if (dynamic_cast<OdenTrashActor*>(result.actor))
        {
            hoverTarget = EHoverTarget::TrashBin;
            return;
        }
    }
    hoverTarget = EHoverTarget::None;
}
