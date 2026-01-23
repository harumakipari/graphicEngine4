#pragma once

enum class EOdenType :uint8_t
{
    None,
    Daikon,
    Egg,
    Tsukune,
    Chikuwa,
    Konnyaku,
    Hanpen,
    Goboten,
    Cake,
    Donut,
    Shirataki,
    Kobumusubi,
    Count,
};

// おでんの形のカテゴリー分け
enum class EOdenShapeCategory :uint8_t
{
    None,
    TriangleLike,// △
    SquareLike, // 四角
    RoundLike,   // 〇
    RibbonLike,   // リボン型
    DonutLike,  // 穴あき
};

// 形の近さを判定する用
struct ShapeProperty
{
    float roundness;   // 丸さ (0.0 ~ 1.0)
    float aspectRatio; // 縦横比（1.0 = 正方形）
    float holeNess; // 穴が空いているか (0.0 = なし , 1.0 = ドーナツ型)
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
    int score;
};

// 具材の価格データ
static  std::unordered_map<EOdenType, float> ingredientPriceTable =
{
    { EOdenType::Daikon, 100.0f },
    { EOdenType::Egg, 100.0f },
    { EOdenType::Chikuwa, 80.0f },
    { EOdenType::Konnyaku, 90.0f },
    { EOdenType::Tsukune, 90.0f },
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


//-------- スコアに関係------

// オーダーのタイプ
enum class EOrderType
{
    ShapeOnly,
    SpecificIngredient
};


struct OrderData
{
    EOrderType type;

    // Shape系
    EOdenShapeCategory requiredCategory;
    ShapeProperty targetProperty;

    // 具材指定系
    EOdenType requiredIngredient;
};


struct OdenSubmitLog
{
    EOdenType type; // 具材の種類
    int count = 1;             // 基本1だが拡張用
    float score = 0.0f;
    bool wasFever = false;  // 提出時にフィーバー中だったかどうか
    // 後々Great　Goodとか
};


enum class GameDifficulty :uint8_t
{
    Easy,
    Normal,
    Hard
};