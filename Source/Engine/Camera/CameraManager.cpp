#include "pch.h"
#include "CameraManager.h"

#include "Engine/Scene/Scene.h"
#include "Game/Actors/Camera/Camera.h"

Camera* CameraManager::GetRenderCamera(const Scene* scene) const
{
    if (useDebugCamera)
    {
        if (auto dbg = debugCamera.lock())
            return dbg.get();
    } 

    if (auto cam = scene->GetActiveCamera())
        return cam;

    Logger::Error(Logger::LogCategory::System, U8("ƒJƒƒ‰‚ªnullptr‚ð•Ô‚µ‚Ä‚¢‚Ü‚·I"));
    return nullptr;
}

void CameraManager::ToggleCamera(const Scene* scene)
{
    useDebugCamera = !useDebugCamera;
    if (useDebugCamera)
    {
        if (const auto dbg = debugCamera.lock())
        {
            if (auto cam = scene->GetActiveCamera())
            {
                dbg->SetPosition(cam->GetPosition());
                dbg->SetQuaternionRotation(cam->GetQuaternionRotation());
                auto cameraCom= cam->GetCameraComponent();
                float fov = cameraCom->GetFov();
                float pitch = cameraCom->GetPitch();
                float yaw = cameraCom->GetYaw();

                if (auto dbgComp=dynamic_cast <DebugCameraComponent*>(dbg->GetCameraComponent()))
                {
                    dbgComp->SetIsUseDebug(useDebugCamera);
                    dbgComp->SetFov(fov);
                    dbgComp->SetYawAndPitch(yaw, pitch);
                    
                }
                //dbg->SetQuaternionRotation({0.0f,0.0f,0.0f,1.0f});
                //CameraState caneraState=
                //{
                //   cam->GetPosition(),
                //    {0.0f,0.0f,0.0f},
                //     DirectX::XMConvertToRadians(30.0f)
                //};
                //dbg->GetCameraComponent()->SetState(caneraState);
            }
        }
    }
}
