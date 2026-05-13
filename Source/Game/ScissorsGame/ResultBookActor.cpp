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
    scoreRoot->SetRelativeLocationDirect({ -0.1f,-0.2f,-1.9f });

    const int maxDigits = 5;// 最大桁数
    scoreDigits.resize(maxDigits);

    for (int i = 0; i < maxDigits; i++)
    {
        for (int number = 0; number < 10; number++)
        {

        }
    }

    //  一の位
    numberModel = AddComponent<SkeletalMeshComponent>("numberModel", scoreParentName);
    numberModel->SetModel("./Data/TeamModels/Number/NumberModel_0.gltf", false, false);
    numberModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    numberModel->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });
    numberModel->SetIsCastShadow(false);

    // スコアの数字モデル　 十の位
    auto numberTenModel = AddComponent<SkeletalMeshComponent>("numberTenModel", scoreParentName);
    numberTenModel->SetModel("./Data/TeamModels/Number/NumberModel_0.gltf", false, false);
    numberTenModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    numberTenModel->SetRelativeLocationDirect({ -0.7f,-0.0f,-0.0f });
    numberTenModel->SetIsCastShadow(false);


}

void ResultBookActor::Update(float deltaTime)
{
    BookBaseActor::Update(deltaTime);
}

void ResultBookActor::DrawImGuiDetails()
{
    BookBaseActor::DrawImGuiDetails();
}

