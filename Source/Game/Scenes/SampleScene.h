#pragma once

#include "Engine/Scene/Scene.h"

#include <d3d11.h>
#include <wrl.h>
#include <memory>

#include "Core/ActorManager.h"
#include "Engine/Scene/SceneBase.h"


#include "Graphics/Renderer/SceneRenderer.h"

#include "Game/Actors/Camera/LoadingCamera.h"
#include "Game/Actors/Player/Player.h"
#include "Game/Actors/Stage/ClothSimulate.h"
#include "Game/Actors/Stage/Stage.h"
#include "Game/DarkGame/DarkActors/DarkStageAsset.h"

#include "UI/Widgets/Widget.h"

#include "PBD/PBDSystem.h"

class SampleScene : public SceneBase
{
public:
    bool Initialize(ID3D11Device* device, UINT64 width, UINT height, const std::unordered_map<std::string, std::string>& props) override;

    void Start() override;

    void Update(float deltaTime) override;

    bool Uninitialize(ID3D11Device* device) override;

    void DrawGui() override;

    void SetUpActors()override;

    //ÉVÅ[ÉìÇÃé©ìÆìoò^
    static inline Scene::Autoenrollment<SampleScene> _autoenrollment;

private:
    std::shared_ptr<StageAsset> stageAsset= std::make_shared<StageAsset>();
    std::shared_ptr<StageAsset> stageCandelabraAsset = std::make_shared<StageAsset>();
    std::shared_ptr<StageAsset> stageBrazierAsset = std::make_shared<StageAsset>();
    std::shared_ptr<StageAsset> stageGroundBrazierAsset = std::make_shared<StageAsset>();
    std::shared_ptr<StageAsset> stageMeltedWaxAsset = std::make_shared<StageAsset>();
    std::shared_ptr<StageAsset> stageStandingBrazierAsset = std::make_shared<StageAsset>();

    std::thread loadStageThread;
    std::thread loadStageAssetsThread;

    std::unique_ptr<PBD::System> pbd;

    std::unique_ptr<ClothSimulate> clothSimulate;

    std::shared_ptr<InterleavedGltfModel> model;

    std::shared_ptr<Player> player;
    TPSCameraComponent* mainCameraComponent = nullptr;


};
