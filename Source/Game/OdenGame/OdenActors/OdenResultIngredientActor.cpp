#include "pch.h"
#include "OdenResultIngredientActor.h"

#include <magic_enum.hpp>

#include "Components/Audio/CoreAudioSourceComponent.h"
#include "Components/Effect/ParticleComponent.h"
#include "Components/Controller/ControllerComponent.h"
#include "Engine/Scene/Scene.h"
#include "Game/Actors/Camera/Camera.h"


void OdenResultIngredientActor::Initialize(const Transform& transform)
{
    // 初期化処理

    // モデル登録
    std::string parentName = ingredientName + "_model";
#if 1
    ingredientModel = AddComponent<SkeletalMeshComponent>(parentName);
    std::string modelFileName = "./Data/Models/Oden_Result_Ingredient/Oden_" + ingredientName + ".gltf";
    ingredientModel->SetModel(modelFileName.c_str());
    ingredientModel->SetIsVisible(false);

#endif // 0
    // エフェクト登録
    particleComponent = this->AddComponent<class ParticleComponent>("appearEffect", parentName);
    particleComponent->Load("./Data/Effect/Files/appearEffect.json");
    //particleComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });

    // きらきらエフェクト登録
    twinkleParticleComponent = AddComponent<ParticleComponent>("twinkleComponent", parentName);
    twinkleParticleComponent->Load("./Data/Effect/Files/sparklingEffect.json");
    twinkleParticleComponent->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });
    // ループ再生設定
    ParticleComponent::AddSettings settings
    {
        .loop = true, // ループ再生
        .startDelay = 0.0f			// 再生開始遅延時間（秒）
    };
    twinkleParticleComponent->SetAddSettings(settings);

    // 音のコンポーネントを追加
    audioComponent = AddComponent<CoreAudioSourceComponent>("audioSource", parentName);
    audioComponent->SetSource(L"./Data/Sound/SE/result_ingredient_appear_high.wav");
    //audioComponent->SetSource(L"./Data/Sound/SE/result_ingredient_appear_mid.wav");
    audioComponent->SetLoop(false);

    // イージングコンポーネントを追加
    easingComponent = AddComponent<CoreEasingComponent>("easingComponent", parentName);

    // 回転コンポーネント追加
    rotationComponent = AddComponent<RotationComponent>("rotationComponent", parentName);

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

}

void OdenResultIngredientActor::Update(float deltaTime)
{
    // 更新処理
    if (isPlayEffect)
    {
        elapsedTime -= deltaTime;
        if (elapsedTime <= 0.0f)
        {
            // モデル表示
            ingredientModel->SetIsVisible(true);
            isPlayEffect = false;
        }
    }

    XMFLOAT3 position = GetPosition();

    if (auto camera = GetOwnerScene()->GetActiveCamera())
    {
        XMFLOAT3 cameraPosition = camera->GetPosition();

        XMVECTOR Position = XMLoadFloat3(&position);
        XMVECTOR CameraPos = XMLoadFloat3(&cameraPosition);

        DirectX::XMVECTOR  Direction = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(Position, CameraPos));
        XMFLOAT3 direction;
        XMStoreFloat3(&direction, Direction);
        if (rotationComponent)
            rotationComponent->SetDirection(direction);
    }

#if 0


    totalTime += deltaTime;
    // 浮遊
    constexpr float uniquePhase = 0.1f;
    float floatY = sinf(totalTime * 2.0f + uniquePhase) * 0.1f;
    position.y += floatY;
    SetPosition(position);
#endif // 0
}

// 食材が登場する
void OdenResultIngredientActor::AppearIngredient()
{
    if (particleComponent)
    {// エフェクト再生
        particleComponent->Play();
    }
    elapsedTime = modelSpawnTime;
    isPlayEffect = true;

    if (isPlayTwinkleEffect)
    {
        // キラキラエフェクト再生
        if (twinkleParticleComponent)
        {
            twinkleParticleComponent->Play();
        }
    }

    if (audioComponent)
    {
        audioComponent->Play();
    }
#if 0

    TestEasingHandler handler;
    handler.AddEasing(
        TestEaseType::OutBack,
        0.0f,
        20.0f,
        0.3f
    );

    handler.AddEasing(
        TestEaseType::InQuad,
        20.0f,
        0.0f,
        0.15f
    );

    handler.SetCompletedFunction([this]()
        {
            popupOffsetY = 0.0f;
        });
    PropertyAccessor<float> accessor;

    accessor.getter = [this]() { return popupOffsetY; };
    accessor.setter = [this](float t)
        {
            popupOffsetY = t;
        };



    easingComponent->StartHandler(handler, accessor);

#endif // 0
}

// 串内のインデックス
void OdenResultIngredientActor::SetIndexInSkewer(int index)
{
    //indexInSkewer = index;

    //float x = 1.2f + indexInSkewer * 5.0f;
    //float y = 6.723f;
    //float z = -5.506f;

    //DirectX::XMFLOAT3 size = ingredientModel->GetModelSize();
    //float spacing = y + size.y * 1.1f;

    //ingredientModel->SetRelativeLocationDirect({ x, -indexInSkewer * spacing, z });
}