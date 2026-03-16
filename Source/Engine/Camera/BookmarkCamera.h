#pragma once

class CameraComponent;

struct CameraState
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 rotation;
    float fov;
};

struct CameraBookmark
{
    std::string name;
    CameraState state;
};

class CameraBookmarkManager
{
public:

    std::vector<CameraBookmark> bookmarks;

    void Save(CameraComponent* camera);

    void Load(CameraComponent* camera, int index);

};