#include "pch.h"
#include "TransitionEffect.h"

#include "Engine/Scene/Scene.h"


void ScaleTransitionEffect::Initialize()
{
    // 画像が小さくなるアニメーション
    //Audio::PlayOneShot(L"./Data/Sounds/SE/gameStart.wav", 1.5f);
    easingRunner = std::make_unique<EasingRunner>();

    float width = 1920.0f;
    float height = 1080.0f;

    float startScale = 220.0f;

#if 0
    TestEasingHandler floatHandler;
    floatHandler.AddWait(0.5f);
    floatHandler.AddEasing(TestEaseType::OutSine, startScale, 1.0f, 2.5f);
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
#endif // 0
    const auto scene = Scene::GetCurrentScene();

    sprite = std::make_shared<UISceneChangeComponent>("./Data/Textures/UI/scene_change_blue.png", "sceneChange");
    sprite->SetWorldPosition({ width * 0.5f, height * 0.5f });
    sprite->SetPivot({ 0.5f,0.5f });
    sprite->SetScale({ startScale,startScale });
    sprite->SetSize({ width, height });
    sprite->zOrder = 1000;
    //scene->GetUIManager()->Add(sprite);
}

void ScaleTransitionEffect::OnSceneChanged() const
{
    const auto scene = Scene::GetCurrentScene();
    scene->GetUIManager()->Add(sprite);
}

void ScaleTransitionEffect::Start(TransitionDirection dir)
{
    isFinishTransitionPerform = false;

    TestEasingHandler handler;
    float startScale = 120.0f;

    if (dir == TransitionDirection::Close)
    {
        spriteScale = startScale;
        handler.AddEasing(TestEaseType::OutSine, startScale, 1.0f, 2.5f);
    }
    else // Open
    {
        startScale = 1.0f;
        handler.AddEasing(TestEaseType::InSine, 1.0f, 100.0f, 1.0f);
    }

    handler.SetCompletedFunction([this]()
        {
            isFinishTransitionPerform = true;
        });

    PropertyAccessor<float> accessor;
    accessor.getter = [this]() { return spriteScale; };
    accessor.setter = [this](float v) { spriteScale = v; };

    easingRunner->StartHandler(handler, accessor);
}

void ScaleTransitionEffect::Update(float deltaTime)
{
    easingRunner->Tick(deltaTime);
    sprite->SetScale({ spriteScale,spriteScale });

}