#include "pch.h"
#include "BookmarkCamera.h"

#include "Components/Camera/CameraComponent.h"


void CameraBookmarkManager::Save(CameraComponent* camera)
{
    CameraBookmark b;
    b.state = camera->GetState();
    bookmarks.push_back(b);
}

void CameraBookmarkManager::Load(CameraComponent* camera, int index)
{
    camera->SetState(bookmarks[index].state);
}
