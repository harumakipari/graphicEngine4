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

    //for (int i = 0; i < maxDigits; i++)
    //{
    //    DirectX::XMFLOAT3 offset = { -0.7f * i, 0.0f, 0.0f };

    //    scoreDigits[i].SetParent(
    //        scoreParentName,
    //        this,
    //        "digit_" + std::to_string(i),
    //        offset
    //    );
    //}

#if 1
    //  一の位
    numberModel = AddComponent<SkeletalMeshComponent>("numberModel", scoreParentName);
    numberModel->SetModel("./Data/TeamModels/Number/NumberModel_0.gltf", false, false);
    numberModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    numberModel->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });
    numberModel->SetIsCastShadow(false);

    // スコアの数字モデル　 十の位
    auto numberTenModel = AddComponent<SkeletalMeshComponent>("numberTenModel", scoreParentName);
    numberTenModel->SetModel("./Data/TeamModels/Number/NumberModel_1.gltf", false, false);
    numberTenModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    numberTenModel->SetRelativeLocationDirect({ -0.7f,-0.0f,-0.0f });
    numberTenModel->SetIsCastShadow(false);
#endif // 0


}

void ResultBookActor::Update(float deltaTime)
{
    BookBaseActor::Update(deltaTime);

    //SetScore(5000);
}

void ResultBookActor::DrawImGuiDetails()
{
    BookBaseActor::DrawImGuiDetails();
}

// スコアを設定する
void ResultBookActor::SetScore(int score)
{
    for (int i = 0; i < scoreDigits.size(); i++)
    {
        int digit = score % 10;
        scoreDigits[i].SetDigit(digit);
        score /= 10;
    }
}