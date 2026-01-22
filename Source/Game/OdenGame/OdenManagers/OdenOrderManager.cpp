#include "pch.h"
#include "OdenOrderManager.h"

#include "Engine/Scene/Scene.h"
#include "Game/OdenGame/OdenActors/OdenBubbleActor.h"
#include "Game/OdenGame/OdenData/OdenGameParameter.h"
#include "Utility/GameUtility.h"
#include "OdenGameManager.h"
#include "OdenSlotManager.h"
#include "Game/OdenGame/OdenGameSession.h"

// 形状からお題を探す
OrderEntry FindShapeOrder(EOdenShapeCategory shape)
{
    auto& list = OdenGameParameter::orderDB.shapeOrders;

    auto it = std::ranges::find_if(list,
        [&](const OrderEntry& e)
        {
            return e.data.requiredCategory == shape;
        });

    assert(it != list.end());
    return *it;
}

OrderEntry FindIngredientOrder(EOdenType ingredient)
{
    auto& list = OdenGameParameter::orderDB.ingredientOrders;

    auto it = std::ranges::find_if(list,
        [&](const OrderEntry& e)
        {
            return e.data.requiredIngredient == ingredient;
        });

    assert(it != list.end());
    return *it;
}

void OdenOrderManager::Initialize(const Transform& transform)
{
}

void OdenOrderManager::StartGame()
{
    slots.clear();

    BuildOrderBag();

    // お題を生成
    for (int i = 0; i < MaxOrders + arrangeOrder; i++) // 奥に見える分
    {
        slots.push_back({ {} ,i });
        SpawnOrderBubble(i);
    }
}

// 特定のお題を出現させる
void OdenOrderManager::SpawnSpecificOrderBubble(int index, const std::string& uiName)
{
    // 位置を取得する
    Transform tr(
        GetBubbleSpawnPosition(index),
        { 0,0,0,1 },
        { 1,1,1 }
    );

    std::string bubbleName = "TutorialOdenBubble_" + uiName;

    // お題アクターを生成する
    auto bubble =
        Scene::GetCurrentScene()->GetActorManager()->
        CreateAndRegisterActorWithTransform<OdenBubbleActor>(bubbleName, tr);

    // お題をランダムに生成する
    auto  specificOrder = FindOrderByUiName(uiName);

    // お題を設定する  時間制限なし
    bubble->SetOrderAndMakeUi(specificOrder->data, specificOrder->uiName, false);

    bubble->onCompleted = [this, index](OdenBubbleActor& bubble, const OdenResult score)
        {
            //　完了時の処理
            OnBubbleCompletedTutorial(index, bubble, score);
        };

    bubble->SetTargetPosition(GetBubblePosition(index));

    slots.push_back({ bubble ,index });

    // スロットマネージャーに吹き出しを登録する　これで吹き出しがビートに乗る
    auto slotManagerActor = GetOwnerScene()->GetActorManager()->GetActorByName("slotManager");
    auto slotManager = std::dynamic_pointer_cast<OdenSlotManager>(slotManagerActor);
    slotManager->RegisterBeatReactive(bubble);
}


// お客さんを出現させる
void OdenOrderManager::SpawnOrderBubble(int index)
{
    // 位置を取得する
    Transform tr(
        GetBubbleSpawnPosition(index),
        { 0,0,0,1 },
        { 1,1,1 }
    );

    // お題アクターを生成する
    auto bubble =
        Scene::GetCurrentScene()->GetActorManager()->
        CreateAndRegisterActorWithTransform<OdenBubbleActor>("OdenBubble", tr);

    // お題をランダムに生成する
    OrderEntry randomOrder = PickRandomOrder();

    // お題を設定する
    bubble->SetOrderAndMakeUi(randomOrder.data, randomOrder.uiName);

    bubble->onCompleted = [this, index](OdenBubbleActor& bubble, const OdenResult score)
        {
            //　完了時の処理
            OnBubbleCompleted(index, bubble, score);
        };

    bubble->SetTargetPosition(GetBubblePosition(index));

    slots[index].bubble = bubble;

    // スロットマネージャーに吹き出しを登録する　これで吹き出しがビートに乗る
    auto slotManagerActor = GetOwnerScene()->GetActorManager()->GetActorByName("slotManager");
    auto slotManager = std::dynamic_pointer_cast<OdenSlotManager>(slotManagerActor);
    slotManager->RegisterBeatReactive(bubble);
}

// ランダムにお題を生成する
OrderEntry OdenOrderManager::PickRandomOrder()
{
#if 0
    // あいまいな形のお題の割合
    constexpr float shapeOrderRate = 0.8f;   // 60%

    EOrderType type;
    {
        float r = MathHelper::RandomRange(0.0f, 1.0f);
        type = (r < shapeOrderRate)
            ? EOrderType::ShapeOnly
            : EOrderType::SpecificIngredient;

    }

    if (type == EOrderType::ShapeOnly)
        return GameHelper::PickRandom(OdenGameParameter::orderDB.shapeOrders);

    return GameHelper::PickRandom(OdenGameParameter::orderDB.ingredientOrders);
#else
    // Bag が空 or 使い切ったら再生成
    if (orderBag.empty() || bagIndex >= orderBag.size())
    {
        BuildOrderBag();
    }

    return orderBag[bagIndex++];
#endif // 0
}

// 順番から位置を取得する
DirectX::XMFLOAT3 OdenOrderManager::GetBubblePosition(const int index)
{
    return
    {
        basePos.x + spacing * static_cast<float>(index),
        basePos.y,
        basePos.z
    };
}

// スロットの番号から出現の位置を取得する
DirectX::XMFLOAT3 OdenOrderManager::GetBubbleSpawnPosition(const int index)
{
    auto pos = GetBubblePosition(index);
    pos.z += spawnOffsetZ;
    return pos;
}


// 注文が完了した時に呼ばれる関数
void OdenOrderManager::OnBubbleCompleted(int slotIndex, OdenBubbleActor& bubble, const OdenResult score)
{
    Logger::Log(U8("オーダー完了時のスコア = ") + std::to_string(score.price));

    // 総合スコアを加算する
    if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
    {
        if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
        {
#if 0
            // スコアを加算する
            gameManager->AddScore(score.price);
            OdenGameSession::Instance().totalScore += score.price;

            // 満足度を加算する
            gameManager->AddSatisfaction(score.satisfaction);
            // リザルト画面のために実際に提出された食材の種類とスコアを記録する

            // OdenGameSession::Instance().submitLogs.emplace_back(bubble.GetIngredientType(), 1, 0.0f);
            //gameManager->AddSubmitLog(bubble.GetIngredientType(), score.price);
#else
            // コンボを加算する フィーバーゲージが溜まる
            gameManager->OnSubmitSuccess();
            //gameManager->AddSubmitLog(bubble.GetIngredientType(), score.price);
          //  bool wasFever = gameManager->IsFeverMode();
         //   OdenGameSession::Instance().submitLogs.emplace_back(bubble.GetIngredientType(), 1, 0.0f, wasFever);
#endif
        }
    }

    bubble.SetLeaving();
    slots[slotIndex].bubble.reset();

    // 少し遅らせて新しい客を出すのもアリ
    SpawnOrderBubble(slotIndex);
}

// 注文が完了した時に呼ばれる関数 チュートリアル用
void OdenOrderManager::OnBubbleCompletedTutorial(int slotIndex, OdenBubbleActor& bubble, OdenResult score)
{
    Logger::Log(U8("チュートリアル用オーダー完了時のスコア = ") + std::to_string(score.price));

    // 総合スコアを加算する
    if (auto actor = GetOwnerScene()->GetActorManager()->GetActorByName("odenGameManager"))
    {
        if (auto gameManager = std::dynamic_pointer_cast<OdenGameManager>(actor))
        {
#if 0
            // スコアを加算する
            gameManager->AddScore(score.price);
            OdenGameSession::Instance().totalScore += score.price;

            // 満足度を加算する
            gameManager->AddSatisfaction(score.satisfaction);
            // リザルト画面のために実際に提出された食材の種類とスコアを記録する

            // OdenGameSession::Instance().submitLogs.emplace_back(bubble.GetIngredientType(), 1, 0.0f);
            //gameManager->AddSubmitLog(bubble.GetIngredientType(), score.price);
#else
            // コンボを加算する フィーバーゲージが溜まる
            gameManager->OnSubmitSuccess();
            //gameManager->AddSubmitLog(bubble.GetIngredientType(), score.price);
          //  bool wasFever = gameManager->IsFeverMode();
          //  OdenGameSession::Instance().submitLogs.emplace_back(bubble.GetIngredientType(), 1, 0.0f, wasFever);

#endif // 0
        }
    }

    bubble.SetLeaving();
    slots[slotIndex].bubble.reset();
}


// お題のバッグを生成する
void OdenOrderManager::BuildOrderBag()
{
    struct OrderBagEntry
    {
        OrderEntry order;
        int count;
    };


    std::vector<OrderBagEntry> entries;

    difficulty = OdenGameSession::GetDifficulty();

    if (difficulty == GameDifficulty::Easy)
    {
        entries =
        {
            //{ FindShapeOrder(EOdenShapeCategory::TriangleLike), 1 },
            { FindShapeOrder(EOdenShapeCategory::SquareLike),   2 },
            //{ FindShapeOrder(EOdenShapeCategory::RoundLike),   1 },
            { FindIngredientOrder(EOdenType::Daikon), 1 },
        };
    }
    else if (difficulty == GameDifficulty::Hard)
    {
        entries =
        {
            { FindShapeOrder(EOdenShapeCategory::TriangleLike), 2 },
            { FindShapeOrder(EOdenShapeCategory::SquareLike),   4 },
            { FindShapeOrder(EOdenShapeCategory::RibbonLike),   2 },
            { FindShapeOrder(EOdenShapeCategory::RoundLike),   4 },
            { FindIngredientOrder(EOdenType::Daikon), 1 },
            { FindIngredientOrder(EOdenType::Egg),    1 },
            { FindIngredientOrder(EOdenType::Tsukune),1 },
            { FindIngredientOrder(EOdenType::Chikuwa),  1 },
            { FindIngredientOrder(EOdenType::Konnyaku), 1 },
            { FindIngredientOrder(EOdenType::Hanpen),    1 },
            { FindIngredientOrder(EOdenType::Goboten),    1 },
            { FindIngredientOrder(EOdenType::Cake),    1 },
            { FindIngredientOrder(EOdenType::Donut),  1 },
            { FindIngredientOrder(EOdenType::Shirataki),1 },
            { FindIngredientOrder(EOdenType::Kobumusubi),1 },
        };
    }

    for (auto& e : entries)
        for (int i = 0; i < e.count; ++i)
            orderBag.push_back(e.order);

    GameHelper::Shuffle(orderBag);
    bagIndex = 0;
}

// UI名からお題を探す
const OrderEntry* OdenOrderManager::FindOrderByUiName(const std::string& uiName)
{
    for (const auto& o : OdenGameParameter::orderDB.shapeOrders)
    {
        if (o.uiName == uiName)
            return &o;
    }
    for (const auto& o : OdenGameParameter::orderDB.ingredientOrders)
    {
        if (o.uiName == uiName)
            return &o;
    }
    return nullptr;
}
