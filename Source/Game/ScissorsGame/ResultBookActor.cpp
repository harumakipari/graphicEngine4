#include "pch.h"
#include "ResultBookActor.h"

#include "HighScoreMedalActor.h"
#include "ScoreCalculator.h"
#include "ScoreHistoryManager.h"
#include "Engine/Audio/CoreAudio.h"
#include "Engine/Scene/Scene.h"
#include "UI/Game/SceneTransitionManager.h"

void ResultBookActor::Initialize(const Transform& transform)
{
    BookBaseActor::Initialize(transform);

    CreateBookModel("./Data/TeamModels/Title/BookRight.gltf", "./Data/TeamModels/Title/BookMiddle.gltf");

    SetInitPageState(BookPageState::SecondPage);

    // UIを作成
    {
        auto uiManager = GetOwnerScene()->GetUIManager();
        // タイムをクリアしたか
        remainClearImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/time_remain_clear.png", "time_remain_clear");
        remainClearImage->SetWorldPosition(timerBonusUiPos);
        remainClearImage->SetSize(timerBonusUiSize);
        remainClearImage->SetVisible(false);
        remainClearImage->SetPivot({ 0.5f,0.5f });
        uiManager->Add(remainClearImage);


        // MM:SS の4桁
        for (int i = 0; i < 4; i++)
        {
            auto digit = std::make_shared<UIImageComponent>(
                "./Data/Textures/ScissorsUI/numberWhite.png",
                "TimerDigit"
            );

            digit->SetSize({ 45, 60 });
            digit->SetPivot({ 0.5f, 0.5f });
            digit->SetColor(XMFLOAT4{ 0.0f,0.0f,0.0f,1.0f });
            digit->zOrder = 2;

            uiManager->Add(digit);

            timerDigits.push_back(digit);
        }


        resultImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/title_patch_result.png", "title_patch_result");
        resultImage->SetWorldPosition(patchSize);
        resultImage->SetVisible(true);
        resultImage->SetPivot({ 0.5f,0.5f });
        uiManager->Add(resultImage);

        selectImage = std::make_shared<UIImageComponent>("./Data/Textures/ScissorsUI/title_patch_select.png", "title_patch_result");
        selectImage->SetWorldPosition(patchSize);
        selectImage->SetVisible(true);
        selectImage->SetPivot({ 0.5f,0.5f });
        uiManager->Add(selectImage);

    }
    // スコアの数字を乗せるページの親
    std::string rightName = rightPage.parentName;

    // ハイスコアメダル
    {
        medalSkeletalMeshComponent = AddComponent<SkeletalMeshComponent>("medalMeshComponent", rightName);
        medalSkeletalMeshComponent->SetModel("./Data/TeamModels/Title/HighScoreMedalModel.gltf", false, true);
        medalSkeletalMeshComponent->SetIsCastShadow(false);
        medalSkeletalMeshComponent->SetRelativeLocationDirect({ -0.4f,-0.2f,-1.9f });
        medalSkeletalMeshComponent->SetRelativeEulerRotationDirect({ 0.0f,20.0f,-180.0f });
        medalSkeletalMeshComponent->SetRelativeScaleDirect({ 1.0f,1.0f,1.0f });
        medalSkeletalMeshComponent->SetIsVisible(false);
    }

    // タイマーワッペン
    {
        timerPatchSkeletalMeshComponent = AddComponent<SkeletalMeshComponent>("TimeClearModel", rightName);
        timerPatchSkeletalMeshComponent->SetModel("./Data/TeamModels/Title/TimeClearModel.gltf", false, true);
        timerPatchSkeletalMeshComponent->SetIsCastShadow(false);
        timerPatchSkeletalMeshComponent->SetRelativeLocationDirect({ -3.9,-0.2f,1.9f });
        timerPatchSkeletalMeshComponent->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        timerPatchSkeletalMeshComponent->SetRelativeScaleDirect({ 1.0f,1.0f,1.0f });
        timerPatchSkeletalMeshComponent->SetIsVisible(false);
    }

    // スコアの数字モデル　
    {
        std::string scoreParentName = "score_number_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, rightName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -0.8f,-0.1f,-1.7f });
        scoreRoot->SetRelativeScaleDirect({ 0.8f,0.8f,0.8f });

        totalScoreDisplay.Initialize(
            this,
            scoreParentName,
            "total_score",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, false);
    }

    float subNumberSize = 0.4f;

    // コンボの数字モデル　
    {
        std::string comboParentName = "combo_number_parent";
        auto comboRoot = AddComponent<SceneComponent>(comboParentName, rightName);
        comboRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        comboRoot->SetRelativeLocationDirect({ -0.4f,-0.1f,-0.75f });
        comboRoot->SetRelativeScaleDirect({ subNumberSize,subNumberSize,subNumberSize });

        comboDisplay.Initialize(
            this,
            comboParentName,
            "combo",
            { -0.0f, -0.0f, -0.0f },
            2,
            0.7f, false);
    }

    // ハートボーナス
    {
        std::string heartParentName = "heart_number_parent";
        auto heartRoot = AddComponent<SceneComponent>(heartParentName, rightName);
        heartRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        heartRoot->SetRelativeLocationDirect({ -0.4f,-0.1f,-0.15f });
        heartRoot->SetRelativeScaleDirect({ subNumberSize,subNumberSize,subNumberSize });
        heartDisplay.Initialize(
            this,
            heartParentName,
            "heart",
            { -0.0f, -0.0f, -0.0f },
            4,
            0.7f, false);
    }

    // 縫い返りボーナス
    {
        std::string scoreParentName = "redirect_number_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, rightName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -0.4f,-0.1f,0.45f });
        scoreRoot->SetRelativeScaleDirect({ subNumberSize,subNumberSize,subNumberSize });
        redirectDisplay.Initialize(
            this,
            scoreParentName,
            "redirect",
            { -0.0f, -0.0f, -0.0f },
            4,
            0.7f, false);
    }


    // まとめぬいボーナス
    {
        std::string scoreParentName = "gather_number_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, rightName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -0.4f,-0.1f,1.05f });
        scoreRoot->SetRelativeScaleDirect({ subNumberSize,subNumberSize,subNumberSize });
        gatherDisplay.Initialize(
            this,
            scoreParentName,
            "gather",
            { -0.0f, -0.0f, -0.0f },
            4,
            0.7f, false);
    }

    // クリアタイム　分
    {
        std::string scoreParentName = "minute_number_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, rightName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -1.1f,-0.1f,1.6f });
        scoreRoot->SetRelativeScaleDirect({ subNumberSize,subNumberSize,subNumberSize });
        minuteDisplay.Initialize(
            this,
            scoreParentName,
            "minute",
            { -0.0f, -0.0f, -0.0f },
            2,
            0.7f, false);
    }
    // 秒
    {
        std::string scoreParentName = "second_number_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, rightName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -0.4f,-0.1f,1.6f });
        scoreRoot->SetRelativeScaleDirect({ subNumberSize,subNumberSize,subNumberSize });
        secondDisplay.Initialize(
            this,
            scoreParentName,
            "second",
            { -0.0f, -0.0f, -0.0f },
            2,
            0.7f, false);
    }

    // 裏表紙のページ
    // ランキング 1
    {
        std::string scoreParentName = "ranking_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -4.0f,0.0f,-1.5f });
        scoreRoot->SetRelativeScaleDirect({ 0.9f,0.9f,0.9f });

        ranking1Display.Initialize(
            this,
            scoreParentName,
            "ranking1",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, true);
    }
    // ランキング 2
    {
        std::string scoreParentName = "ranking2_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -4.0f,0.0f,-0.38f });
        scoreRoot->SetRelativeScaleDirect({ 0.75f,0.75f,0.75f });

        ranking2Display.Initialize(
            this,
            scoreParentName,
            "ranking2",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, true);
    }

    float downRankingScale = 0.6f;
    // ランキング 3
    {
        std::string scoreParentName = "ranking3_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -4.0f,0.0f,0.55f });
        scoreRoot->SetRelativeScaleDirect({ downRankingScale,downRankingScale,downRankingScale });

        ranking3Display.Initialize(
            this,
            scoreParentName,
            "ranking3",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, true);
    }
    // ランキング 4
    {
        std::string scoreParentName = "ranking4_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -4.0f,0.0f,1.3f });
        scoreRoot->SetRelativeScaleDirect({ downRankingScale,downRankingScale,downRankingScale });

        ranking4Display.Initialize(
            this,
            scoreParentName,
            "ranking4",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, true);
    }
    // ランキング 5
    {
        std::string scoreParentName = "ranking5_parent";
        auto scoreRoot = AddComponent<SceneComponent>(scoreParentName, backCoverName);
        scoreRoot->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
        scoreRoot->SetRelativeLocationDirect({ -4.0f,0.0f,2.05f });
        scoreRoot->SetRelativeScaleDirect({ downRankingScale,downRankingScale,downRankingScale });

        ranking5Display.Initialize(
            this,
            scoreParentName,
            "ranking5",
            { -0.0f, -0.0f, -0.0f },
            5,
            0.7f, true);
    }

    //totalScoreDisplay.SetVisible(false);    // トータルスコア
    comboDisplay.SetVisible(false); // コンボ数
    heartDisplay.SetVisible(false);    // HPボーナス
    redirectDisplay.SetVisible(false);  // 反射ボーナス
    gatherDisplay.SetVisible(false);  // まとめボーナス
    secondDisplay.SetVisible(false);  // 秒数
    minuteDisplay.SetVisible(false);  // 分数

    ranking1Display.SetVisible(false);  // ランキング
    ranking2Display.SetVisible(false);  // ランキング
    ranking3Display.SetVisible(false);  // ランキング
    ranking4Display.SetVisible(false);  // ランキング
    ranking5Display.SetVisible(false);  // ランキング


#if 0
    //  一の位
    numberModel = AddComponent<SkeletalMeshComponent>("numberModel", scoreParentName);
    numberModel->SetModel("./Data/TeamModels/Number/NumberModel_3.gltf", false, false);
    numberModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,180.0f });
    numberModel->SetRelativeLocationDirect({ 0.0f,0.0f,0.0f });
    numberModel->SetIsCastShadow(false);

    // スコアの数字モデル　 十の位
    auto numberTenModel = AddComponent<SkeletalMeshComponent>("numberTenModel", scoreParentName);
    numberTenModel->SetModel("./Data/TeamModels/Number/NumberModel_4.gltf", false, false);
    numberTenModel->SetRelativeEulerRotationDirect({ 0.0f,0.0f,0.0f });
    numberTenModel->SetRelativeLocationDirect({ -0.7f,-0.0f,-0.0f });
    numberTenModel->SetIsCastShadow(false);
#endif // 0

    // 矢印ボタンのUIを作成する
    CreateButtonArrow();

    phaseTimer = 0.0f;

    // スコアカウントアップ音のオーディオコンポーネント
    scoreCountUpAudioComponent = AddComponent<CoreAudioSourceComponent>("chargeAudioComponent", parentName);
    scoreCountUpAudioComponent->SetSource(L"./Data/Sound/SE1/result_score_count_up.wav");
    scoreCountUpAudioComponent->SetVolume(1.0f);
    scoreCountUpAudioComponent->SetLoop(true);

    // メダル用初期化
    easingRunner = std::make_unique<EasingRunner>();
    medalValue = 0.0f;

    // タイマーワッペン用
    easingTimeRunner = std::make_unique<EasingRunner>();
    timerValue = 0.0f;

    // 新記録かどうか
    isNewRecord = false;

    // 二枚目の矢印UIは最初は表示しない
    showSecondPageButtonArrow = false;

    ResultData stats = ScoreSystem::GetResultStats();
    if (stats.remainHp > 0)
    {// HPが0以上だったら
        // ステージクリア
        isCleared = true;
    }
    else
    {
        isCleared = false;
    }


    isPlayTimerPatch = false;
}

void ResultBookActor::Update(float deltaTime)
{
    BookBaseActor::Update(deltaTime);
    // スコアを表示する
    const ResultData& stats = ScoreSystem::GetResultStats();
    totalScoreDisplay.SetValue(displayedTotalScore);
    phaseTimer += deltaTime;

    comboDisplay.Update(deltaTime); // コンボ数
    heartDisplay.Update(deltaTime);    // HPボーナス
    redirectDisplay.Update(deltaTime);  // 反射ボーナス
    gatherDisplay.Update(deltaTime);  // まとめボーナス
    secondDisplay.Update(deltaTime);  // 秒数
    minuteDisplay.Update(deltaTime);  // 分数
    ranking1Display.Update(deltaTime);
    ranking2Display.Update(deltaTime);
    ranking3Display.Update(deltaTime);
    ranking4Display.Update(deltaTime);
    ranking5Display.Update(deltaTime);


    int ranking = ScoreHistoryManager::GetRanking(stats.stageName, currentTotalScore);
    std::array<NumberDisplay*, 5> rankingDisplays =
    {
        &ranking1Display,
        &ranking2Display,
        &ranking3Display,
        &ranking4Display,
        &ranking5Display
    };

    for (auto* display : rankingDisplays)
    {
        display->SetColor(numberColor);
    }

    if (ranking >= 0 && ranking < rankingDisplays.size())
    {
        rankingDisplays[ranking]->SetColor(playerColor);
    }


    totalScoreDisplay.SetColor(numberColor);
    comboDisplay.SetColor(numberColor);
    heartDisplay.SetColor(numberColor);
    gatherDisplay.SetColor(numberColor);
    redirectDisplay.SetColor(numberColor);

    //ranking1Display.SetColor(numberColor);
    //ranking2Display.SetColor(numberColor);
    //ranking3Display.SetColor(numberColor);
    //ranking4Display.SetColor(numberColor);
    //ranking5Display.SetColor(numberColor);

    minuteDisplay.SetColor(numberColor);
    secondDisplay.SetColor(numberColor);

    // リザルトイメージ
    resultImage->SetWorldPosition(patchPos);
    resultImage->SetSize(patchSize);
    resultImage->SetWorldAngleDegree(patchAngle);

    // セレクトイメージ
    selectImage->SetWorldPosition(patchPos);
    selectImage->SetSize(patchSize);
    selectImage->SetWorldAngleDegree(patchAngle);

    // リザルトイメージ
    if (bookState == BookPageState::SecondPage)
    {
        resultImage->SetVisible(true);
    }
    else
    {
        resultImage->SetVisible(false);
    }

    // セレクトイメージ
    if (bookState == BookPageState::FirstPage)
    {
        selectImage->SetVisible(true);
    }
    else
    {
        selectImage->SetVisible(false);
    }



    // メダルのスケールを更新
    easingRunner->Tick(deltaTime);
    currentScale = std::lerp(startScale, endScale, medalValue);
    medalSkeletalMeshComponent->SetRelativeScaleDirect({ currentScale,currentScale,currentScale });

    // タイマーワッペンのスケールを更新
    easingTimeRunner->Tick(deltaTime);
    currentTimerScale = std::lerp(startScale, endTimerScale, timerValue);
    timerPatchSkeletalMeshComponent->SetRelativeScaleDirect({ currentTimerScale,currentTimerScale,currentTimerScale });

    // タイムをクリアしたか
    remainClearImage->SetWorldPosition(timerBonusUiPos);
    remainClearImage->SetSize(timerBonusUiSize);


    float remain = ScoreSystem::GetRemainTimeToClear();
    //if (resultPhase >= ResultPhase::AddTimeBonus && !ScoreSystem::IsTimeClear() && bookState == BookPageState::SecondPage && isCleared)
    {// 時間ボーナス加算のフェーズで、目標タイムをクリアできていないとき、かつ二枚目のページが開いているとき、かつステージをクリアしているとき
        remainClearImage->SetVisible(true);

#ifdef _DEBUG
        remain = 9.0f;
#endif // _DEBUG
        // 秒に変換
        int totalSeconds = static_cast<int>(remain);
        UpdateTimerDigits(totalSeconds);

        // MM:SS
        // [0][1] : [2][3]
        timerDigits[0]->SetWorldPosition({ timerBonusUiPos.x - spacing * 2.0f + minuteSpacing.x, timerBonusUiPos.y + minuteSpacing.y });
        timerDigits[1]->SetWorldPosition({ timerBonusUiPos.x - spacing * 1.0f + minuteSpacing.x, timerBonusUiPos.y + minuteSpacing.y });

        timerDigits[2]->SetWorldPosition({ timerBonusUiPos.x + spacing * 1.0f + secondSpacing.x, timerBonusUiPos.y + secondSpacing.y });
        timerDigits[3]->SetWorldPosition({ timerBonusUiPos.x + spacing * 2.0f + secondSpacing.x, timerBonusUiPos.y + secondSpacing.y });

        for (int i = 0; i < 4; i++)
        {
            timerDigits[i]->SetSize(numberSize);
            //timerDigits[i]->SetColor(DirectX::XMFLOAT4{ 0.471f,0.455f,0.498f,1.0f });
        }
    }
    //else
    //{
    //    remainClearImage->SetVisible(false);
    //    for (int i = 0; i < 4; i++)
    //    {
    //        timerDigits[i]->SetVisible(false);
    //    }
    //}


#if 1
    if (isCleared && !isPlayTimerPatch)
    {// クリアしていたら
        if (resultPhase >= ResultPhase::AddTimeBonus && ScoreSystem::IsTimeClear() && bookState == BookPageState::SecondPage)
        {
            isPlayTimerPatch = true;
            TimerPatchPlay();
        }

    }
#endif // 0


#if 0
    totalScoreDisplay.SetValue(97777);
    comboDisplay.SetValue(20);
    heartDisplay.SetValue(100);
    gatherDisplay.SetValue(150);
    redirectDisplay.SetValue(150);

    ranking1Display.SetValue(900);
    ranking2Display.SetValue(9176);
    ranking3Display.SetValue(1176);
    ranking4Display.SetValue(51176);
    ranking5Display.SetValue(51176);


    minuteDisplay.SetValue(1);
    secondDisplay.SetValue(5, 2);

#else
#if 0
    int score = stats.totalScore;
    totalScoreDisplay.SetValue(score);

    // 最大コンボ数を表示する
    comboDisplay.SetValue(stats.maxCombo);

    // 残りハートボーナスを表示する
    heartDisplay.SetValue(stats.remainHp * 150);

    // 反射ボーナスを表示する
    redirectDisplay.SetValue(stats.reflectionBonusScore);

    // まとめぬいボーナスを追加する
    gatherDisplay.SetValue(stats.dashBonusScore);

    // 時間を表示する
    int totalSeconds = static_cast<int>(stats.gameTimer);
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    minuteDisplay.SetValue(minutes);
    secondDisplay.SetValue(seconds, 2);


    // ハイスコアだったら、メダルを表示する
    //ScoreHistoryManager::IsNewRecord()

    // ランキングを取得する
    std::vector<ScoreHistoryManager::Entry> ranking = ScoreHistoryManager::GetTop5(stats.stageName);
    // 5件分必ず表示（足りないところは0）
    int scores[5] = { 0, 0, 0, 0, 0 };

    for (size_t i = 0; i < ranking.size() && i < 5; i++)
    {
        scores[i] = ranking[i].score;
    }

    ranking1Display.SetValue(scores[0]);
    ranking2Display.SetValue(scores[1]);
    ranking3Display.SetValue(scores[2]);
    ranking4Display.SetValue(scores[3]);
    ranking5Display.SetValue(scores[4]);

#endif // 0
#endif


    switch (resultPhase)
    {
    case ResultPhase::Wait:

        displayedTotalScore = stats.enemyScore;
        currentTotalScore = stats.enemyScore;
        Logger::Log(U8("enemyScore:") + std::to_string(stats.enemyScore));
#ifdef _DEBUG
        //displayedTotalScore = 1500;
        //currentTotalScore = 1500;
#endif // _DEBUG
        phaseTimer = 0.0f;
        break;

    case ResultPhase::ShowEnemyScore:
        if (phaseTimer >= preShowScoreInterval)
        {
            phaseTimer = 0.0f;
            resultPhase = ResultPhase::ShowCombo;
        }
        break;
    case ResultPhase::ShowCombo:
        if (!phaseInitialized)
        {
            phaseInitialized = true;
            int comboCount = stats.maxCombo;
            comboDisplay.SetValue(comboCount);
            comboDisplay.SetVisible(true); // コンボ数
            CoreAudio::PlayOneShot(L"./Data/Sound/SE1/result_show_score.wav", 1.0f);
        }
        if (phaseTimer >= preShowScoreInterval)
        {
            phaseInitialized = false;
            phaseTimer = 0.0f;
            resultPhase = ResultPhase::ShowHeart;
        }
        break;
    case ResultPhase::ShowHeart:
        if (!phaseInitialized)
        {
            phaseInitialized = true;

            int heartBonus = stats.remainHp * 150;
#ifdef _DEBUG
            //heartBonus = 1500;
#endif // _DEBUG


            heartDisplay.SetValue(heartBonus);
            heartDisplay.SetVisible(true);
            // SE
            CoreAudio::PlayOneShot(L"./Data/Sound/SE1/result_show_score.wav", 1.0f);

            addTargetScore = currentTotalScore + heartBonus;
            currentTotalScore = addTargetScore;
        }

        if (phaseTimer >= preShowScoreInterval)
        {
            phaseInitialized = false;
            phaseTimer = 0.0f;
            resultPhase = ResultPhase::ShowRedirect;
        }
        break;
    case ResultPhase::ShowRedirect:
        if (!phaseInitialized)
        {
            phaseInitialized = true;

            int redirectScore = stats.reflectionBonusScore;

#ifdef _DEBUG
            //redirectScore = 2500;
#endif // _DEBUG

            redirectDisplay.SetValue(redirectScore);
            redirectDisplay.SetVisible(true);
            // SE
            CoreAudio::PlayOneShot(L"./Data/Sound/SE1/result_show_score.wav", 1.0f);

            addTargetScore = currentTotalScore + redirectScore;
            currentTotalScore = addTargetScore;
        }
        if (phaseTimer >= preShowScoreInterval)
        {
            phaseInitialized = false;
            phaseTimer = 0.0f;
            resultPhase = ResultPhase::ShowGather;
        }
        break;
    case ResultPhase::ShowGather:
        if (!phaseInitialized)
        {
            phaseInitialized = true;

            int gatherBonus = stats.dashBonusScore;

#ifdef _DEBUG
            //gatherBonus = 550;
#endif // _DEBUG


            gatherDisplay.SetValue(gatherBonus);
            gatherDisplay.SetVisible(true);
            // SE
            CoreAudio::PlayOneShot(L"./Data/Sound/SE1/result_show_score.wav", 1.0f);

            addTargetScore = currentTotalScore + gatherBonus;
            currentTotalScore = addTargetScore;
        }
        if (phaseTimer >= preShowScoreInterval)
        {
            phaseInitialized = false;
            phaseTimer = 0.0f;
            resultPhase = ResultPhase::ShowTimeBonus;
        }
        break;
    case ResultPhase::ShowTimeBonus:
        if (!phaseInitialized)
        {
            phaseInitialized = true;

            // 時間を表示する
            int totalSeconds = static_cast<int>(stats.gameTimer);
            int minutes = totalSeconds / 60;
            int seconds = totalSeconds % 60;


            minuteDisplay.SetVisible(true);
            minuteDisplay.SetValue(minutes);
            secondDisplay.SetVisible(true);
            secondDisplay.SetValue(seconds, 2);

            int timerBonus = ScoreSystem::CalculateTimeClearBonus();
            Logger::Log(U8("タイムボーナス：") + std::to_string(timerBonus));

            //timerBonus = 1000;

            if (isCleared)
            {//　ステージをクリアしていたら
                addTargetScore = currentTotalScore + timerBonus;
                currentTotalScore = addTargetScore;
            }

            if (ScoreSystem::IsTimeClear())
            {// 目標タイムをクリアした
                Logger::Log(U8("目標タイムをクリアした"));
                //timeClearImage->SetVisible(true);   // 「CLEAR!」
                //TimerPatchPlay();
            }
            else
            {
                //remainClearImage->SetVisible(true);
                Logger::Log(U8("あと ") + std::to_string(remain) + U8(" 秒でクリア"));
            }

            // SE
            CoreAudio::PlayOneShot(L"./Data/Sound/SE1/result_show_score.wav", 1.0f);
        }
        if (phaseTimer >= preShowScoreInterval)
        {
            phaseInitialized = false;
            phaseTimer = 0.0f;
            resultPhase = ResultPhase::AddTotalScore;
            addTimer = 0.0f;
            // スコアカウントアップのSEを再生する
            scoreCountUpAudioComponent->Play();
            startScore = displayedTotalScore;
        }
        break;
    case ResultPhase::AddTotalScore:
    {
        const float duration = 1.5f;

        float rate = std::clamp(phaseTimer / duration, 0.0f, 1.0f);

        int rawScore = static_cast<int>(
            std::lerp(startScore, addTargetScore, rate)
            );

        // 100刻みで表示
        displayedTotalScore = (rawScore / 100) * 100;

        if (rate >= 1.0f)
        {
            displayedTotalScore = addTargetScore;
            currentTotalScore = addTargetScore;

            phaseInitialized = false;
            phaseTimer = 0.0f;

            resultPhase = ResultPhase::PreShowRanking;

            scoreCountUpAudioComponent->Stop();
        }

        break;
    }
    case ResultPhase::PreShowRanking:
        if (phaseTimer >= preShowScoreInterval)
        {
            phaseInitialized = false;
            phaseTimer = 0.0f;
            resultPhase = ResultPhase::ShowRanking;

            Logger::Log(U8("総スコア") + std::to_string(currentTotalScore));
            Logger::Log(U8("最大コンボ数") + std::to_string(stats.maxCombo));
            Logger::Log(U8("反射ボーナス点") + std::to_string(stats.reflectionBonusScore));
            Logger::Log(U8("複数ボーナス") + std::to_string(stats.dashBonusScore));
            Logger::Log(U8("残りHP") + std::to_string(stats.remainHp));
            Logger::Log(U8("所要時間") + std::to_string(stats.gameTimer));
            // スコアを記録する
            ScoreHistoryManager::Submit(stats.stageName, currentTotalScore);

        }
        break;

    case ResultPhase::ShowRanking:
        // 二ページ目の矢印UIを表示する
        showSecondPageButtonArrow = true;

        if (!phaseInitialized)
        {
            phaseInitialized = true;

            // ランキングを取得する
            std::vector<ScoreHistoryManager::Entry> ranking = ScoreHistoryManager::GetTop5(stats.stageName);
            // 5件分必ず表示（足りないところは0）
            int scores[5] = { 0, 0, 0, 0, 0 };

            for (size_t i = 0; i < ranking.size() && i < 5; i++)
            {
                scores[i] = ranking[i].score;
            }

            ranking1Display.SetValue(scores[0]);
            ranking1Display.SetVisible(true);

            ranking2Display.SetValue(scores[1]);
            ranking2Display.SetVisible(true);

            ranking3Display.SetValue(scores[2]);
            ranking3Display.SetVisible(true);

            ranking4Display.SetValue(scores[3]);
            ranking4Display.SetVisible(true);

            ranking5Display.SetValue(scores[4]);
            ranking5Display.SetVisible(true);

            // SE
            CoreAudio::PlayOneShot(L"./Data/Sound/SE1/result_show_score.wav", 1.0f);

            resultPhase = ResultPhase::PreHighScore;
            phaseTimer = 0.0f;
        }
        break;
    case ResultPhase::PreHighScore:
        if (phaseTimer >= preShowScoreInterval)
        {
            phaseInitialized = false;
            phaseTimer = 0.0f;
            resultPhase = ResultPhase::HighScore;

            // 新記録かどうか
            isNewRecord = ScoreHistoryManager::IsNewRecord(stats.stageName, currentTotalScore);

        }
        break;
    case ResultPhase::HighScore:
        if (isNewRecord)
        {
#if 0
            if (auto medal = GetOwnerScene()->GetActorManager()->GetActorOfType<HighScoreMedalActor>())
            {
                medal->Play();
            }
#else
            MedalPlay();
#endif // 0

        }
        resultPhase = ResultPhase::Complete;
        break;
    case ResultPhase::Complete:
        break;
    }
}


// コントローラー対応の本が開く処理
void ResultBookActor::HandlePadInput()
{
    bool pushA = InputSystem::GetInputState("GamePadA", InputStateMask::Trigger);

    bool pushR = InputSystem::GetInputState("BookRight", InputStateMask::Trigger);
    bool pushL = InputSystem::GetInputState("BookLeft", InputStateMask::Trigger);

    switch (bookState)
    {
    case BookPageState::FirstPage:
        if (pushL)
        {
            CoreAudio::PlayOneShot(L"./Data/Sound/SE1/push_button.wav");
            // タイトルへシーン遷移する
            SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "TitleScene"), std::make_pair("fromScene","ResultScene") });
        }
        if (pushR)
        {
            // 二ページ目を開く
            OpenSecondPage(2.0f);
        }
        break;
    case BookPageState::SecondPage:
        if (showSecondPageButtonArrow)
        {// 矢印が表示されている時だけ、入力を受け付ける
            if (pushL)
            {
                // 一ページ目に戻る
                CloseSecondPage(2.0f);
            }
            if (pushR)
            {
                CoreAudio::PlayOneShot(L"./Data/Sound/SE1/push_button.wav");
                // タイトルへシーン遷移する
                SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "TitleScene"), std::make_pair("fromScene","ResultScene") });
            }
        }
        break;
    }
}

void ResultBookActor::DrawImGuiDetails()
{
#ifdef USE_IMGUI

    BookBaseActor::DrawImGuiDetails();
    if (ImGui::Button(U8("ステートを最初に戻す")))
    {
        resultPhase = ResultPhase::ShowEnemyScore;
        phaseTimer = 0.0f;
        displayedTotalScore = 0;
        addTargetScore = 0;
        currentTotalScore = 0;

        comboDisplay.SetVisible(false);
        heartDisplay.SetVisible(false);
        redirectDisplay.SetVisible(false);
        gatherDisplay.SetVisible(false);

        ranking1Display.SetVisible(false);
        ranking2Display.SetVisible(false);
        ranking3Display.SetVisible(false);
        ranking4Display.SetVisible(false);
        ranking5Display.SetVisible(false);

    }
    ImGui::DragFloat2(U8("patchSize"), &patchSize.x);
    ImGui::DragFloat2(U8("patchPos"), &patchPos.x);
    ImGui::DragFloat(U8("patchAngle"), &patchAngle);


    ImGui::DragFloat(U8("本を開く前の時間"), &preShowScoreInterval, 0.1f, 0.0f, 5.0f);
    ImGui::DragInt(U8("加算スピード"), &addStep, 10, 0, 1000);
    ImGui::DragFloat2(U8("タイマーボーナスUIの位置"), &timerBonusUiPos.x, 10.0f);
    ImGui::DragFloat2(U8("タイマーボーナスUIのサイズ"), &timerBonusUiSize.x, 5.0f);
    ImGui::DragFloat(U8("数字の間"), &spacing);
    ImGui::DragFloat2(U8("数字の幅"), &numberSize.x);
    ImGui::DragFloat2(U8("分の間"), &minuteSpacing.x);
    ImGui::DragFloat2(U8("秒の間"), &secondSpacing.x);
    ImGui::ColorEdit4("numberColor", &numberColor.x);
    ImGui::ColorEdit4("playerColor", &playerColor.x);
#endif
}

// シーン遷移が完了したらスコア表示開始
void ResultBookActor::StartShowEnemyScore()
{
    resultPhase = ResultPhase::ShowEnemyScore;
}

// 矢印ボタンのUIを作成する
void ResultBookActor::CreateButtonArrow()
{
    DirectX::XMFLOAT2 uiLeftPos = { 12, 875 };
    DirectX::XMFLOAT2 uiRightPos = { 1650, 875 };
    DirectX::XMFLOAT2 uiArrowSize = { 250, 150 };


    auto uiManager = GetOwnerScene()->GetUIManager();
    // 一ページ左
    firstButtons.left = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/title_arrow.png", "title_arrow");
    firstButtons.left->SetWorldPosition(uiLeftPos);
    firstButtons.left->SetSize(uiArrowSize);
    uiManager->Add(firstButtons.left);

    firstButtons.left->onClick = [this]()
        {
            CoreAudio::PlayOneShot(L"./Data/Sound/SE1/push_button.wav");
            // タイトルへシーン遷移する
            SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "TitleScene"), std::make_pair("fromScene","ResultScene") });
        };

    // ゲームパッドの画像を設定する
    firstButtons.gamePadLeft = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/title_arrow_control.png");
    // キーボードの画像を設定する
    firstButtons.keyboardLeft = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/title_arrow.png");


    // 一ページ右
    firstButtons.right = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/result_arrow.png", "result_arrow");
    firstButtons.right->SetWorldPosition(uiRightPos);
    firstButtons.right->SetSize(uiArrowSize);
    uiManager->Add(firstButtons.right);

    firstButtons.right->onClick = [this]()
        {
            OpenSecondPage(2.0f);
        };

    // ゲームパッドの画像を設定する
    firstButtons.gamePadRight = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/result_arrow_control.png");
    // キーボードの画像を設定する
    firstButtons.keyboardRight = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/result_arrow.png");


    // 二ページ目左
    secondButtons.left = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/stage_select_arrow.png", "stage_select_arrow");
    secondButtons.left->SetWorldPosition(uiLeftPos);
    secondButtons.left->SetSize(uiArrowSize);
    uiManager->Add(secondButtons.left);

    secondButtons.left->onClick = [this]()
        {
            // 一ページ目に戻る
            CloseSecondPage(2.0f);
        };

    // ゲームパッドの画像を設定する
    secondButtons.gamePadLeft = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/stage_select_arrow_control.png");
    // キーボードの画像を設定する
    secondButtons.keyboardLeft = std::make_shared<Sprite>(Graphics::GetDevice(), L"./Data/Textures/ScissorsUI/stage_select_arrow.png");


    // 二ページ目右
    secondButtons.right = std::make_shared<UIButtonComponent>("./Data/Textures/ScissorsUI/title_arrow_right.png", "title_arrow_right");
    secondButtons.right->SetWorldPosition(uiRightPos);
    secondButtons.right->SetSize(uiArrowSize);
    uiManager->Add(secondButtons.right);

    secondButtons.right->onClick = [this]()
        {
            CoreAudio::PlayOneShot(L"./Data/Sound/SE1/push_button.wav");
            // タイトルへシーン遷移する
            SceneTransitionManager::Instance().RequestTransition("LoadingScene", { std::make_pair("preload", "TitleScene"), std::make_pair("fromScene","ResultScene") });
        };


}

// 目標タイムまでのタイマー表示を更新
void ResultBookActor::UpdateTimerDigits(int totalSeconds)
{
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    int minuteTens = minutes / 10;
    int minuteOnes = minutes % 10;

    int secondTens = seconds / 10;
    int secondOnes = seconds % 10;

    int numbers[4] =
    {
        minuteTens,
        minuteOnes,
        secondTens,
        secondOnes
    };

    for (int i = 0; i < 4; i++)
    {
        int digit = numbers[i];

        timerDigits[i]->SetUV({
            150.0f * digit,
            0.0f,
            150.0f,
            200.0f
            });

        bool visible = true;

        // 分の十の位
        if (i == 0 && digit == 0)
        {
            visible = false;
        }

        // 秒の十の位
        if (i == 2 && digit == 0)
        {
            visible = false;
        }

        timerDigits[i]->SetVisible(visible);
    }
}

// 矢印ボタンUIを表示する
void ResultBookActor::ShowButtonArrow()
{

}

// 演出開始
void ResultBookActor::MedalPlay()
{
    medalSkeletalMeshComponent->SetIsVisible(true);
    //  メダルのSEを再生
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/result_high_score_budge.wav", 1.5f);

    TestEasingHandler handler;

    handler.AddWait(0.0f);

    handler.AddEasing(
        TestEaseType::OutExp,
        0.0f,
        1.0f,
        interval
    );

    handler.SetCompletedFunction([this]()
        {
            medalValue = 1.0f;
        });

    PropertyAccessor<float> accessor;

    accessor.getter =
        [this]()
        {
            return medalValue;
        };

    accessor.setter =
        [this](float t)
        {
            medalValue = t;
        };

    easingRunner->StartHandler(handler, accessor);
}

// タイマーのワッペンの演出開始
void ResultBookActor::TimerPatchPlay()
{
    timerPatchSkeletalMeshComponent->SetIsVisible(true);
    //  メダルのSEを再生
    CoreAudio::PlayOneShot(L"./Data/Sound/SE1/timer_patch.wav", 1.5f);

    TestEasingHandler handler;

    handler.AddWait(0.0f);

    handler.AddEasing(
        TestEaseType::OutExp,
        0.0f,
        1.0f,
        interval
    );

    handler.SetCompletedFunction([this]()
        {
            timerValue = 1.0f;
        });

    PropertyAccessor<float> accessor;

    accessor.getter =
        [this]()
        {
            return timerValue;
        };

    accessor.setter =
        [this](float t)
        {
            timerValue = t;
        };

    easingTimeRunner->StartHandler(handler, accessor);
}