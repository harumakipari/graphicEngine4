#pragma once
#include "OdenDataStruct.h"

struct OdenFaceShapeTable
{
    std::unordered_map<EOdenFace, OdenShapeData> faceShapes;
};

// aspectRatio について
/*
    完全に正方形	1.0
    まあ正方形	0.8
    ちょい長い	0.5
    明らかに長い	0.0
 */

 // ダイコンの形と面のデータ
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

// それぞれの食材の名前と形と面のデータ
static inline std::unordered_map<std::string, OdenFaceShapeTable> odenTypeShapes =
{
    {"Daikon",DaikonShapeTable}
};

struct OrderEntry
{
    std::string uiName;
    OrderData data;
};

struct OdenOrderDatabase
{
    std::vector<OrderEntry> shapeOrders;
    std::vector<OrderEntry> ingredientOrders;
};

static OdenOrderDatabase OrderDB =
{
    // ShapeOnly
    {
        {
            "UI_Order_CircleLike",
            {EOrderType::ShapeOnly, EOdenShapeCategory::RoundLike,{1,1,0}, EOdenType::None,3,0}
        },
        {
            "UI_Order_SquareLike",
            {EOrderType::ShapeOnly, EOdenShapeCategory::SquareLike,{0.3f,0.8f,0}, EOdenType::None,3,0}
        },
        {
            "UI_Order_TriangleLike",
            {EOrderType::ShapeOnly, EOdenShapeCategory::TriangleLike,{0,0,0}, EOdenType::None,3,0}
        },
    },

    // Ingredient
    {
        {
            "UI_Order_Daikon",
            {EOrderType::SpecificIngredient, EOdenShapeCategory::None,{0,0,0}, EOdenType::Daikon,3,0}
        },
        {
            "UI_Order_Egg",
            {EOrderType::SpecificIngredient, EOdenShapeCategory::None,{0,0,0}, EOdenType::Egg,3,0}
        },
    }
};

