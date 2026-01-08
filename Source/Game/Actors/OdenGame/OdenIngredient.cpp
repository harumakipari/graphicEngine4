#include "pch.h"
#include "OdenIngredient.h"


// ‹ïŞ‚ª‰¡‰ñ“]‚·‚é‚Æ‚«‚ÉŒÄ‚ÔŠÖ”
void OdenIngredientActor::RotateHorizontal()
{
    odeIngredientAngleDegree.x += 90.0f;
    MathHelper::ClampEulerAngle(odeIngredientAngleDegree.x);
}

// ‹ïŞ‚ªc‰ñ“]‚·‚é‚Æ‚«‚ÉŒÄ‚ÔŠÖ”
void OdenIngredientActor::RotateVertical()
{
    odeIngredientAngleDegree.z += 90.0f;
    MathHelper::ClampEulerAngle(odeIngredientAngleDegree.z);
}

void OdenIngredientActor::Update(float elapsedTime)
{
    ingredientModel->SetRelativeEulerRotationDirect(odeIngredientAngleDegree);
}



void OdenDaikonActor::Initialize(const Transform& transform)
{
    // ƒ‚ƒfƒ‹“o˜^
    std::string parentName = "Daikon_model";
    ingredientModel = AddComponent<SkeletalMeshComponent>(parentName);
    ingredientModel->SetModel("./Data/Models/Oden_Ingredient/Oden_Daikon.gltf");

    // ‰Šú‚ÌŠp“x’²®
    SetAngleOffset({ -70.0f,0.0f,0.0f });
}

void OdenDaikonActor::Update(float elapsedTime)
{
    OdenIngredientActor::Update(elapsedTime);
}

void OdenDaikonActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI
    if (ImGui::Button(U8("‰¡‰ñ“]")))
    {
        RotateHorizontal();
    }

    if (ImGui::Button(U8("c‰ñ“]")))
    {
        RotateVertical();
    }

#endif
}
