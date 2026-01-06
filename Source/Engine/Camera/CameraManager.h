#pragma once
#include <memory>

class Camera;

class CameraManager
{
public:
    static void ToggleCamera()
    {
        useDebugCamera = !useDebugCamera;
    }

    static Camera* GetCurrentCamera()
    {
        return useDebugCamera ? debugCamera.lock().get() : gameCamera.lock().get();
    }

    static void Clear()
    {
        gameCamera.reset();
        debugCamera.reset();
        useDebugCamera = false;
    }

    static bool IsUseDebug() { return useDebugCamera; }

    static void SetGameCamera(const std::weak_ptr<Camera>& camera) { gameCamera = camera; }
    static void SetDebugCamera(const std::shared_ptr<Camera>& camera) { debugCamera = camera; }
private:
    static inline std::weak_ptr<Camera> gameCamera;
    static inline std::weak_ptr<Camera> debugCamera;

    static inline bool useDebugCamera = false;
};
