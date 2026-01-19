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


void OdenIngredientActor::Initialize(const Transform& transform)
{
    // イージングコンポーネントを追加
    easingRunner = std::make_unique<EasingRunner>();

    InitParam(ingredientName);
}

void OdenIngredientActor::Update(float deltaTime)
{
    // イージングコンポーネントの更新
    easingRunner->Tick(deltaTime);

    // 位置を取得
    DirectX::XMFLOAT3 position = GetPosition();

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

    //// 見た目の回転
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


void OdenIngredientActor::InitParam(const std::string& ingredientName)
{
    //GetRootComponent()->SetRelativeEulerRotationDirect({ -25.0f,0.0f,0.0f });

    // モデル登録
    std::string parentName = ingredientName + "_model";
    ingredientModel = AddComponent<SkeletalMeshComponent>(parentName);
    std::string modelFileName = "./Data/Models/Oden_Ingredient/Oden_" + ingredientName + ".gltf";
    ingredientModel->SetModel(modelFileName.c_str());

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
    struct FaceTransform
    {
        DirectX::XMFLOAT3 localOffset;
        DirectX::XMFLOAT3 localEuler; // 面の法線方向を向く回転
    };

    float halfX = size.x + 0.001f;
    float halfY = size.y + 0.001f;
    float halfZ = size.z + 0.001f;

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

    for (auto& [face, shapeData] : faceShapeTable.faceShapes)
    {
        if (face != EOdenFace::Top && face != EOdenFace::Bottom)
        {
            continue;
        }

        std::string shapeName = GetDotLineModelPath(ingredientType, face, shapeData);
        std::string modelDotLineFileName = "./Data/Models/Oden_DotLine/" + shapeName;

        if (shapeName == "")
        {
            continue;
        }
        std::string dotLineName = "dot_line" + std::string(magic_enum::enum_name(odenOrientation.bottom));
        auto dot = AddComponent<DotLineMeshComponent>(dotLineName, parentName);

        const FaceTransform& ft = faceTransformTable[face];
        //dot->SetRelativeLocationDirect({ 0.0f,-0.03f,0.0f });
        //dot->SetRelativeLocationDirect({ 0.0f,0.03f,0.0f });
        dot->SetRelativeLocationDirect(ft.localOffset);
        dot->SetRelativeEulerRotationDirect(ft.localEuler);
        dot->SetModel(modelDotLineFileName);
        dot->SetIsCastShadow(false);
    }

#endif // 0

    // 点線のモデルを追加 食材の子供とする。
    //dotLineModel = AddComponent<DotLineMeshComponent>("dotLineModel", parentName);
    //dotLineModel->SetRelativeLocationDirect({ 0.0f,0.001f,0.0f });
    //std::string modelDotLineFileName = "./Data/Models/Oden_DotLine/Oden_DotLine_Triangle.gltf";
    //dotLineModel->SetModel(modelDotLineFileName.c_str());
    //dotLineModel->overrideDeferredPipelineName = "OdenDotLineMesh";
    //dotLineModel->overrideForwardPipelineName = "OdenDotLineMesh";
}

// ドラック開始処理
void OdenIngredientActor::TryBeginDrag(const DirectX::XMFLOAT2& cursor)
{
    HitResultWithActor result;
    if (CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::Oden)))
    {
        if (result.actor == this)
        {
            dragState = EOdenDragState::Dragging;
            grabbedFromSlot = currentSlot;
            // おでんをつかむ音　SE再生
            CoreAudio::PlayOneShot(L"./Data/Sound/SE/grab_ingredient.wav", 1.0f);
            Logger::Log(U8("おでんを掴んだ！"));
        }
    }
}

// ドラック中の処理
void OdenIngredientActor::UpdateDragging(const DirectX::XMFLOAT2& cursor) const
{
    HitResultWithActor result;
    if (CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::WorldStatic)))
    {
        XMFLOAT3 pos = result.hitPoint;
        pos.y = 1.0f;
        SetPosition(pos);

        //DebugDrawManager::DrawSphere(pos, 0.5f, { 1,1,0,1 });
    }
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
        DebugDrawManager::DrawSphere({ 0, 0, 0 }, 13.5f, { 1,0,1,1 });
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
OdenIngredientActor::EHoverTarget OdenIngredientActor::DetectHoverTarget(const DirectX::XMFLOAT2& cursor) const
{
    HitResultWithActor result;
    if (CollisionFunction::RaycastFromMouse(cursor, result, CollisionHelper::ToBit(CollisionLayer::OdenHoverTarget)))
    {
        // マウスカーソルが
        if (auto odenSlot = dynamic_cast<OdenSlotActor*>(result.actor))
        {// おでんの枠モデルに当たっていたら、
            return EHoverTarget::OdenSlot;
        }
        if (auto odenBubble = dynamic_cast<OdenBubbleActor*>(result.actor))
        {// お題の上だったら
            if (odenBubble->CanAcceptIngredient())
            {// オーダー可能な時
                odenBubble->OnIngredientDropped(*this); // これちょっと怖い。。
                return EHoverTarget::OrderBubble;
            }
            return EHoverTarget::None;
        }
        if (auto odenTrash = dynamic_cast<OdenTrashActor*>(result.actor))
        {// ゴミ箱の上だったら
            return EHoverTarget::TrashBin;
        }

        //DebugDrawManager::DrawSphere({ 0, 0, 0 }, 13.5f, { 1,0,1,1 });

    }
    //DebugDrawManager::DrawSphere({ 0, 0, 0 }, 13.5f, { 1,0,1,1 });
    Logger::Log(U8("何もないところでマウスクリックを離した！"));
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
        {{ EOdenType::Chikuwa, EOdenFace::Top   }, "Oden_DotLine_LongRect.gltf" },
        {{ EOdenType::Chikuwa, EOdenFace::Bottom   }, "Oden_DotLine_LongRect.gltf" },

        // --- 大根 ---
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
    default:
        modelPath = "";
        break;
    }

    return modelPath;
}