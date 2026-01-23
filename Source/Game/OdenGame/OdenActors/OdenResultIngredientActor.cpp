#include "pch.h"
#include "OdenResultIngredientActor.h"

#include "Components/Audio/CoreAudioSourceComponent.h"
#include "Components/Effect/ParticleComponent.h"


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
#if 0
    XMFLOAT3 position = GetPosition();

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