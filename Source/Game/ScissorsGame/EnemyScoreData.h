#pragma once

// 敵のスコアデータ
struct EnemyScoreData
{
    int baseScore;          // 敵を倒したときのスコア
    int killBonus;
};

// 敵の種類
enum class YarnEnemyType :uint8_t
{
    Static,   // その場でじっとしている
    MoveHorizontal, // 横に直線移動する
    MoveVertical,   // 縦に直線移動する
    MoveLinear, // 直線移動する
    MoveToCenter,   // 中心に向かって移動する
    WaveHorizontal, // 横に波打ちながら移動する
    WaveVertical,  // 縦に波打ちながら移動する
    WaveMoveBehavior, // 波うちながら移動する
    ChasePlayer,          // プレイヤーを追いかける
    RescueEnemy, // 敵を助ける
    LongRangeAttack, // 遠距離攻撃
};

// ボスに与えるダメージ
struct BossDamageContext
{
    float baseDamage = 10.0f;

    int killedEnemyBeforeHitCount = 0;
    bool isBossStunned = false;
};

// 敵が死亡した原因
enum class KillType:uint8_t
{
    Dash,
    Reflected,
    Bobbin,
};