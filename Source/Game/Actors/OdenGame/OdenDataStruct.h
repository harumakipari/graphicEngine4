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

enum class EOdenOrderShape :uint8_t
{
    TriangleLike,
    SquareLike,
    LongLike,
    RoundLike
};

struct OdenData
{
    EOdenType type;
    EOdenOrientation orientation;
    EOdenOrderShape shapeTag;
    int score;
};

