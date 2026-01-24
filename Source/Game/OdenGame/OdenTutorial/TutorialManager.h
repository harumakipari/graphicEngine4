#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "TutorialStep.h"

class TutorialManager
{
public:
    TutorialManager();
    virtual ~TutorialManager() = default;

    // コピー禁止（オブジェクトの重複を防ぐ）
    TutorialManager(const TutorialManager&) = delete;
    TutorialManager& operator =(const TutorialManager&) = delete;
    TutorialManager(TutorialManager&&) noexcept = delete;
    TutorialManager& operator =(TutorialManager&&) noexcept = delete;

    // 更新処理
    void Update(float deltaTime);

    // ステート変更
    void ChangeState(const std::string& name);

    // ステート登録
    void RegisterState(std::unique_ptr<TutorialStep> state);

    // ステート名取得
    const char* GetStateName() const { return currentState == nullptr ? "" : currentState->GetName(); }

    // 現在のステートを取得
     TutorialStep* GetCurrentState()  { return currentState; }

private:
    // 現在のステート
    TutorialStep* currentState = nullptr;

    // 各ステートを保持する配列
    std::unordered_map<std::string, std::unique_ptr<TutorialStep>> statePool;
};