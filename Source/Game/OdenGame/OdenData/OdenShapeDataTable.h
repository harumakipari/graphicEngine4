#pragma once
#include "OdenDataStruct.h"

struct OdenFaceShapeTable
{
    std::unordered_map<EOdenFace, OdenShapeData> faceShapes;
};

// aspectRatio ‚É‚Â‚¢‚Ä
/*
    Š®‘S‚É³•ûŒ`	1.0
    ‚Ü‚ ³•ûŒ`	0.8
    ‚¿‚å‚¢’·‚¢	0.5
    –¾‚ç‚©‚É’·‚¢	0.0
 */

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



