#pragma once
#include "OdenDataStruct.h"

struct OdenFaceShapeTable
{
    std::unordered_map<EOdenFace, OdenShapeData> faceShapes;
};

// 仮のダイコンデータ
static OdenFaceShapeTable DaikonShapeTable =
{
    {
        { EOdenFace::Front,  {EOdenShapeCategory::SquareLike, {0.2f, 1.0f} } },
        { EOdenFace::Back,   {EOdenShapeCategory::SquareLike, {0.2f, 1.0f} } },
        { EOdenFace::Left,   {EOdenShapeCategory::LongLike,   {0.1f, 2.5f} } },
        { EOdenFace::Right,  {EOdenShapeCategory::LongLike,   {0.1f, 2.5f} } },
        { EOdenFace::Top,    {EOdenShapeCategory::RoundLike,  {0.4f, 1.2f} } },
        { EOdenFace::Bottom, {EOdenShapeCategory::RoundLike,  {0.4f, 1.2f} } },
    }
};

static inline std::unordered_map<std::string, OdenFaceShapeTable> odenTypeShapes =
{
    {"Daikon",DaikonShapeTable}
};


struct OdenOrderDataTable
{
    std::unordered_map<std::string, OrderData> odenOrders;
};

static OdenOrderDataTable gameOdenOrderData =
{
    {
        {"UI_Order_RoundLike",{EOrderType::ShapeOnly,EOdenShapeCategory::RoundLike,{0.4f, 1.2f},EOdenType::None,3.0f,0.0f}},    // 〇
    }
};