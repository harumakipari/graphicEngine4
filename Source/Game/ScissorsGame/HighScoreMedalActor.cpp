#include "pch.h"
#include "HighScoreMedalActor.h"

#include "Engine/Audio/CoreAudio.h"
#include "Engine/Easing/TestEasingHandler.h"


void HighScoreMedalActor::Initialize(const Transform& transform)
{
    std::string parentName = "HighScoreMedalActor";
    skeletalMeshComponent = AddComponent<SkeletalMeshComponent>(parentName);
    skeletalMeshComponent->SetModel("./Data/TeamModels/Title/HighScoreMedalModel.gltf", false, true);
    skeletalMeshComponent->SetIsCastShadow(false);
    skeletalMeshComponent->SetIsVisible(false);
    easingRunner = std::make_unique<EasingRunner>();

    medalValue = 0.0f;


}

void HighScoreMedalActor::Update(float elapsedTime)
{
    easingRunner->Tick(elapsedTime);
    currentScale = std::lerp(startScale, endScale, medalValue);
    SetScale({ currentScale,currentScale,currentScale });
}

void HighScoreMedalActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button(U8("ハイスコアメダル演出")))
    {
        Play();
    }
    ImGui::DragFloat(U8("メダルが移動する時間"), &interval, 0.1f, 0.1f, 3.0f);
    ImGui::DragFloat(U8("メダルの最初のスケール"), &startScale, 1.0f, 0.1f, 30.0f);
#endif
}

// 終了時の処理
void HighScoreMedalActor::Finalize()
{

}


void HighScoreMedalActor::Play()
{
    skeletalMeshComponent->SetIsVisible(true);
    //  メダルのSEを再生
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/result_high_score_budge.wav", 1.5f);

    TestEasingHandler handler;

    handler.AddWait(0.0f);

    handler.AddEasing(
        TestEaseType::OutExp,
        0.0f,
        1.0f,
        interval
    );

    handler.SetCompletedFunction([this]()
        {
            medalValue = 1.0f;
        });

    PropertyAccessor<float> accessor;

    accessor.getter =
        [this]()
        {
            return medalValue;
        };

    accessor.setter =
        [this](float t)
        {
            medalValue = t;
        };

    easingRunner->StartHandler(handler, accessor);

}
