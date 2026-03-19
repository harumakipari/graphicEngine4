#pragma once
#include "Components/Camera/CameraComponent.h"

class MovieManager
{
public:
    void Play(const std::string& file)
    {
        camera->LoadFromJson("./Data/MovieCamera/" + file);
        camera->Start();
    }

private:
    std::shared_ptr<MovieCameraComponent> camera;
};
