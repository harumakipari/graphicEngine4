#include "pch.h"
#include "TransitionEffect.h"

#include "Components/Audio/AudioSourceComponent.h"
#include "Engine/Scene/Scene.h"


void ScaleTransitionEffect::Initialize(const Transform& transform)
{
    // 画像が大きくなるアニメーション
    //Audio::PlayOneShot(L"./Data/Sounds/SE/gameStart.wav", 1.5f);
    easingComponent = AddComponent<CoreEasingComponent>("easingComponent");

    float width = 1920.0f;
    float height = 1080.0f;

    TestEasingHandler floatHandler;
    floatHandler.AddWait(0.5f);
    floatHandler.AddEasing(TestEaseType::OutSine, 1.0f, 220.0f, 2.5f);
    floatHandler.SetCompletedFunction([&]()
        {
            isFinishTransitionPerform = true;
        });
    PropertyAccessor<float> floatAccessor;
    floatAccessor.getter = [&]()->float { return 1.0f; };
    floatAccessor.setter = [&](float value) {
        float sizeX = width * value;
        float sizeY = height * value;
        spriteScale = value;
        };
    easingComponent->StartHandler(floatHandler, floatAccessor);

    const auto scene = GetOwnerScene();

    sprite = std::make_shared<UIImageComponent>("./Data/Textures/UI/scene_change_blue.png", "sceneChange");
    sprite->SetWorldPosition({ width * 0.5f, height * 0.5f });
    sprite->SetPivot({ 0.5f,0.5f });
    sprite->SetScale({ 1.0f,1.0f });
    sprite->SetSize({ width, height });
    scene->GetUIManager()->Add(sprite);
}


void ScaleTransitionEffect::Update(float dt)
{
    sprite->SetScale({ spriteScale,spriteScale });

}