#include "pch.h"
#include "ResultBookActor.h"

void ResultBookActor::Initialize(const Transform& transform)
{
    BookBaseActor::Initialize(transform);
    SetInitPageState(BookPageState::SecondPage);

    // スコアの数字を乗せるページの親
    std::string rightName = rightPage.parentName;

    // スコアの数字モデル　
    // 親を生成する
    std::string scoreParentName = "score_number_parent";
    auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, rightName);
    scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    scoreRoot->SetRelativeLocationDirect({ -0.5f,-0.1f,-2.0f });

    totalScoreDisplay.Initialize(
        this,
        scoreParentName,
        "total_score",
        { -0.0f, -0.0f, -0.0f },
        5,
        0.7f,false);

    float subNumberSize = 0.6f;

    // コンボの数字モデル　
    // 親を生成する
    std::string comboParentName = "combo_number_parent";
    auto comboRoot = AddComponent<SceneComponent>(comboParentName, rightName);
    comboRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    comboRoot->SetRelativeLocationDirect({ -0.5f,-0.1f,-0.9f });
    comboRoot->SetRelativeScaleDirect({ subNumberSize,subNumberSize,subNumberSize });

    comboDisplay.Initialize(
        this,
        comboParentName,
        "combo",
        { -0.0f, -0.0f, -0.0f },
        2,
        0.7f, false);

    // ハートボーナス
    // 親を生成する
    std::string heartParentName = "heart_number_parent";
    auto heartRoot = AddComponent<SceneComponent>(heartParentName, rightName);
    heartRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    heartRoot->SetRelativeLocationDirect({ -0.5f,-0.1f,-0.0f });
    heartRoot->SetRelativeScaleDirect({ subNumberSize,subNumberSize,subNumberSize });
    heartDisplay.Initialize(
        this,
        heartParentName,
        "heart",
        { -0.0f, -0.0f, -0.0f },
        4,
        0.7f, false);



    // 縫い返りボーナス


    // まとめぬいボーナス


    // クリアタイム

    // ニューレコード

#if 0

    //  一の位
    numberModel = AddComponent<SkeletalMeshComponent>("numberModel", scoreParentName);
    numberModel->SetModel("./Data/TeamModels/Number/NumberModel_3.gltf", false, false);
    numberModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,180.0f });
    numberModel->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });
    numberModel->SetIsCastShadow(false);

    // スコアの数字モデル　 十の位
    auto numberTenModel = AddComponent<SkeletalMeshComponent>("numberTenModel", scoreParentName);
    numberTenModel->SetModel("./Data/TeamModels/Number/NumberModel_4.gltf", false, false);
    numberTenModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    numberTenModel->SetRelativeLocationDirect({ -0.7f,-0.0f,-0.0f });
    numberTenModel->SetIsCastShadow(false);
#endif // 0


}

void ResultBookActor::Update(float deltaTime)
{
    BookBaseActor::Update(deltaTime);
    totalScoreDisplay.SetValue(1234);
    comboDisplay.SetValue(56);
    heartDisplay.SetValue(1200);
}

void ResultBookActor::DrawImGuiDetails()
{
    BookBaseActor::DrawImGuiDetails();
}

