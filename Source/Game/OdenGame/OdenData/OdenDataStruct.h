#pragma once

enum class EOdenType :uint8_t
{
    Daikon,
    Egg,
    Tsukune,
    Chikuwa,
    Konnyaku,
};

enum class EOdenOrientation :uint8_t
{
    Deg0,
    Deg90,
    Deg180,
    Deg270
};

// おでんの形のカテゴリー分け
enum class EOdenShapeCategory :uint8_t
{
    TriangleLike,// △
    SquareLike, // 四角
    LongLike,   // 長い
    RoundLike   // 〇
};

// 形の近さを判定する用
struct ShapeProperty
{
    float roundness;   // 丸さ (0.0 ~ 1.0)
    float aspectRatio; // 縦横比（1.0 = 正方形）
};

// おでんの形に関するデータ
struct OdenShapeData
{
    EOdenShapeCategory category;
    ShapeProperty property;
};


struct OdenData
{
    EOdenType type;
    EOdenOrientation orientation;
    OdenShapeData shapeTag;
    int score;
};

// おでんの面の向き　
enum class EOdenFace :uint8_t
{
    Front,   // -Z
    Back,    // +Z
    Left,    // -X
    Right,   // +X
    Top,     // +Y
    Bottom   // -Y
};
