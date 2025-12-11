#pragma once
#include "Components/Base/SceneComponent.h"
#include "Engine/Easing/TestEasingHandler.h"

/**
 * @file
 * @brief イージングを使って任意プロパティを時間的に補間するコンポーネント。
 * @details `EasingHandler` と `PropertyAccessor` を組み合わせ、任意の float プロパティを
 *          直列シーケンスで補間できます。インスペクタ描画や非スケール時間の使用切替にも対応。
 */

template<typename T>
struct PropertyAccessor
{
	/** @brief 値の取得関数。*/
	std::function<T()> getter;      // 値を読む
	/** @brief 値の設定関数。*/
	std::function<void(T)> setter;  // 値を書き込む
};

#if 0
class EasingComponent :public SceneComponent
{
public:
	EasingComponent(const std::string& name, std::shared_ptr<Actor> owner) :SceneComponent(name, owner) {}

	virtual ~EasingComponent() = default;

protected:


};

#endif // 0
