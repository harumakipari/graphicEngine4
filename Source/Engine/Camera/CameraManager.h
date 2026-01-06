#pragma once
#include <memory>

//#include "Engine/Scene/SceneBase.h"

class Camera;
class Scene;

class CameraManager
{
public:
    void ToggleCamera()
    {
        useDebugCamera = !useDebugCamera;
    }

    Camera* GetRenderCamera(const Scene* scene) const;

    void Clear()
    {
        //gameCamera.reset();
        debugCamera.reset();
        useDebugCamera = false;
    }

    bool IsUseDebug() const { return useDebugCamera; }

    //void SetGameCamera(const std::weak_ptr<Camera>& camera) { gameCamera = camera; }
    void SetDebugCamera(const std::shared_ptr<Camera>& camera) { debugCamera = camera; }
private:
    //std::weak_ptr<Camera> gameCamera;
    std::weak_ptr<Camera> debugCamera;

    bool useDebugCamera = false;
};
