#include "pch.h"
#include "TutorialManager.h"
#include "Engine/Debug/Assert.h"

TutorialManager::TutorialManager()
{
    statePool.clear();
}

// 更新処理
void TutorialManager::Update(float deltaTime)
{
    // 現在のステートを実行
    if (!currentState)
        return;
    currentState->Execute(deltaTime);
}

// ステート変更
void TutorialManager::ChangeState(const std::string& name)
{
    if (currentState)
    {
        currentState->Exit();
    }

    // ステートの中から指定されたステートがあるか検索
    auto it = statePool.find(name);
    if (it != statePool.end())
    {
        // 登録されていれば指定されたステートに更新
        currentState = it->second.get();
        // また指定されたステートのEnter関数の実行
        currentState->Enter();
    }
    else
    {
        // 指定されたステートが存在しない
        _ASSERT_EXPR_A(it != statePool.end(), "Not Found State.");
    }


}

// ステート登録
void TutorialManager::RegisterState(std::unique_ptr<TutorialStep> state)
{
    std::string name = state->GetName();
    statePool[name] = std::move(state);
}


