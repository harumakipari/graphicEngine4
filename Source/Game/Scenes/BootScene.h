#pragma once

#include "Engine/Scene/Scene.h"

#include <d3d11.h>
#include <wrl.h>
#include <memory>

#include "Core/ActorManager.h"
#include "Engine/Scene/SceneBase.h"

#include "Graphics/Effect/EffectSystem.h"

#include "Graphics/Renderer/SceneRenderer.h"

#include "Game/Actors/Player/TitlePlayer.h"
#include "Game/Actors/Camera/TitleCamera.h"

#include "UI/Widgets/Widget.h"
#include "Physics/CollisionMesh.h"

#include "PBD/PBDSystem.h"

class BootScene : public SceneBase
{
    ActorColliderManager actorColliderManager;

    UIRoot uiRoot;

public:
    bool Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props) override;

    void Start() override;

    void Update(float deltaTime) override;

    void Render(ID3D11DeviceContext* immediateContext, float deltaTime) override;

    bool Uninitialize(ID3D11Device* device) override;

    void DrawGui() override;

    void SetUpActors()override;

    //ÉVÅ[ÉìÇÃé©ìÆìoò^
    static inline Scene::Autoenrollment<BootScene> _autoenrollment;

private:
    std::shared_ptr<TitlePlayer> titlePlayer;
    std::shared_ptr<Stage>  title;
    std::shared_ptr<CollisionMesh> stageCollisionMesh;
    std::shared_ptr<TitleCamera> mainCameraActor;

    SceneRenderer sceneRender;

    std::unique_ptr<PBD::System> pbd;
};