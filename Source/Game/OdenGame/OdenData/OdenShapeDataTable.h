#pragma once
#include "OdenDataStruct.h"

struct OdenFaceShapeTable
{
    std::unordered_map<EOdenFace, OdenShapeData> faceShapes;
};


/*
    完全に正方形	1.0
    まあ正方形	0.8
    ちょい長い	0.5
    明らかに長い	0.0
 */

 // 仮のダイコンデータ
static OdenFaceShapeTable DaikonShapeTable =
{
    // ダイコン
    {
        { EOdenFace::Front,  {EOdenShapeCategory::SquareLike, {0.3f, 0.5f,0.0f} } },
        { EOdenFace::Back,   {EOdenShapeCategory::SquareLike, {0.3f, 0.5f,0.0f} } },
        { EOdenFace::Left,   {EOdenShapeCategory::LongLike,   {0.3f, 0.5f,0.0f} } },
        { EOdenFace::Right,  {EOdenShapeCategory::LongLike,   {0.3f, 0.5f,0.0f} } },
        { EOdenFace::Top,    {EOdenShapeCategory::RoundLike,  {1.0f, 1.0f,0.0f} } },
        { EOdenFace::Bottom, {EOdenShapeCategory::RoundLike,  {1.0f, 1.0f,0.0f} } },
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
        {"UI_Order_CircleLike",{EOrderType::ShapeOnly,EOdenShapeCategory::RoundLike,{1.0f, 1.0f,0.0f},EOdenType::None,3.0f,0.0f}},    // 〇
        //{"UI_Order_CircleLike",{EOrderType::ShapeOnly,EOdenShapeCategory::RoundLike,{1.0f, 1.0f,0.0f},EOdenType::None,3.0f,0.0f}},    // 〇


        // 食材指定　判定時は EOrderType と EOdenType のみ使用
        {"UI_Order_Daikon",{EOrderType::SpecificIngredient,EOdenShapeCategory::RoundLike,{1.0f, 1.2f,0.0f},EOdenType::Daikon,3.0f,0.0f}},    // ダイコン
    }
};