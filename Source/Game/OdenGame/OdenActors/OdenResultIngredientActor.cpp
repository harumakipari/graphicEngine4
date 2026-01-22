#include "pch.h"
#include "OdenResultIngredientActor.h"

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
        .startDelay = 0.2f			// 再生開始遅延時間（秒）
    };
    twinkleParticleComponent->SetAddSettings(settings);

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

}