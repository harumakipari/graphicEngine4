#include "pch.h"
#include "CameraManager.h"

#include "Engine/Scene/Scene.h"

Camera* CameraManager::GetRenderCamera(const Scene* scene) const
{
    if (useDebugCamera)
    {
        if (auto dbg = debugCamera.lock())
            return dbg.get();
    }

    if (auto cam = scene->GetActiveCamera())
        return cam;

    Logger::Error(Logger::LogCategory::System, U8("ƒJƒƒ‰‚ªnullptr‚ğ•Ô‚µ‚Ä‚¢‚Ü‚·I"));
    return nullptr;
}
