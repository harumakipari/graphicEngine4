#include "pch.h"
#include "ResultBookActor.h"

void ResultBookActor::Initialize(const Transform& transform)
{
    BookBaseActor::Initialize(transform);
    SetInitPageState(BookPageState::SecondPage);

    // スコアの数字を乗せるページの親
    std::string rightName = rightPage.parentName;

    // スコアの数字モデル　 一の位
    std::string scoreParentName = "score_number_0";
    numberModel = AddComponent<SkeletalMeshComponent>(scoreParentName, rightName);
    numberModel->SetModel("./Data/TeamModels/Number/NumberModel_0.gltf", false, false);
    numberModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    numberModel->SetRelativeLocationDirect({ -0.1f,-0.2f,-1.9f });
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

