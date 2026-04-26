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
    MoveToCenter,   // 中心に向かって移動する
    WaveHorizontal, // 横に波打ちながら移動する
    WaveVertical,  // 縦に波打ちながら移動する
    ChasePlayer,          // プレイヤーを追いかける
    RescueEnemy, // 敵を助ける
};