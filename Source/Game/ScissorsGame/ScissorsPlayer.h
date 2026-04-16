#pragma once
#include "Components/Controller/ControllerComponent.h"
#include "Core/Actor.h"
#include "Game/Actors/Base/Character.h"


class ScissorsActor;

class ScissorsPlayer :public Character
{
    enum class ScissorsIntent :uint8_t
    {
        None,
        Throw,
        Pull
    };
    struct AimData
    {
        DirectX::XMFLOAT3 dir;
        float power;
        bool isValid;
        ScissorsIntent intent = ScissorsIntent::None;
    };
    enum class State :uint8_t
    {
        Walking,
        Attacking,
        Dead
    };
public:
    explicit ScissorsPlayer(const std::string& actorName) :Character(actorName) {}

    void Initialize(const Transform& transform)override;

    void Update(float deltaTime)override;

    // ハサミの所持数を取得
    int GetScissorsCount() const { return scissorsCount; }

    // ハサミが返ってきたときの処理
    void OnScissorsReturned(ScissorsActor* scissors);

    // ダメージを受ける処理
    void TakeDamage(int damage);

private:
    // 入力から狙いの情報を取得する
    AimData GetAimData(const MoveIntent& intent, float deltaTime);

    // 狙いの情報から、ハサミを投げる距離や方向を決定して、投げる処理を試みる
    void TryAction(const AimData& aim, bool stickReleased, bool buttonReleased);

    // 狙いの情報をもとに、投げる前のプレビューを描画する
    void DrawPreview(const AimData& aim);

    // ハサミを拾う
    void PickUpNearest();

    // ハサミを引き寄せる
    void PullNearest();

    // ハサミを投げる
    void ThrowScissors(float power, DirectX::XMFLOAT3 dir);

    // 近くに落ちているハサミがあるか
    ScissorsActor* FindNearestDroppedScissors();

    // 引き寄せ終わったかどうか
    bool IsPullFinished();

    // プレイヤーの攻撃処理
    void Attack();

private:
    std::shared_ptr<SkeletalMeshComponent> skeletalMeshComponent;
    std::shared_ptr<RotationComponent> rotationComponent;
    std::shared_ptr<InputComponent> inputComponent;
    std::shared_ptr<CharacterMovementComponent> characterMovementComponent;

    int scissorsCount = 2; // ハサミの所持数
    std::vector<std::weak_ptr<ScissorsActor>> equippedScissors; // 手に持ってる
    std::vector<std::weak_ptr<ScissorsActor>> droppedScissors;  // 落ちてる

    float pickupRange = 1.0f; // ハサミを拾う範囲

    bool isCharging = false;
    float chargeTime = 0.0f;

    float maxChargeTime = 1.0f;     // 最大溜め
    float maxThrowDistance = 10.0f; // 最大距離

    float lastStickPower = 0.0f;// スティックの最終的な力　溜めの強さや投げるときの力に使用する

    State state = State::Walking;

    float damageCooldown = 0.0f; // ダメージを受けた後の無敵時間のクールダウン

};
