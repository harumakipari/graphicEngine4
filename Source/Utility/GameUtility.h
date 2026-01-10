#pragma once
#include "Math/MathHelper.h"

// ゲームで使用するユーティリティ関数
namespace GameHelper
{
    // ベクターからランダムで一つ選択するユーティリティ関数
    template<typename T>
    inline const T& PickRandom(const std::vector<T>& list)
    {
        assert(!list.empty());
        int index = MathHelper::RandomRange(0, static_cast<int>(list.size()) - 1);
        return list[index];
    }
}