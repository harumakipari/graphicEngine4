#include "pch.h"
#include "OdenGameParameter.h"
#include <magic_enum.hpp>

#include "OdenShapeDataTable.h"


void OdenGameParameter::LoadOdenFaceShapeTableFromCSV(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::string msg = "Failed to open CSV file: " + filePath;
        Logger::Error(msg.c_str());
        
        return;
    }

    std::string line;
    std::getline(file, line); // ヘッダ行をスキップ

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string name, faceStr, categoryStr, roundStr, aspectStr, holeStr;

        std::getline(ss, name, ',');
        std::getline(ss, faceStr, ',');
        std::getline(ss, categoryStr, ',');
        std::getline(ss, roundStr, ',');
        std::getline(ss, aspectStr, ',');
        std::getline(ss, holeStr, ',');

        auto maybeFace = magic_enum::enum_cast<EOdenFace>(faceStr);
        auto maybeCategory = magic_enum::enum_cast<EOdenShapeCategory>(categoryStr);

        if (!maybeFace.has_value() || !maybeCategory.has_value())
            continue; // 無効な行はスキップ

        OdenShapeData data;
        data.category = maybeCategory.value();
        data.property.roundness = std::stof(roundStr);
        data.property.aspectRatio = std::stof(aspectStr);
        data.property.holeNess = std::stof(holeStr);

        odenTypeShapes[name].faceShapes[maybeFace.value()] = data;
    }

    file.close();
}

// オ－ダーのデータ
void OdenGameParameter::LoadOrderDBFromCSV(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) 
    {
        std::string msg = "Failed to open CSV file: " + filePath;
        Logger::Error(msg.c_str());

        return;
    }

    std::string line;
    getline(file, line); // ヘッダスキップ

    while (getline(file, line))
    {
        std::stringstream ss(line);
        std::string uiName, typeStr, categoryStr, roundStr, aspectStr, holeStr, ingredientStr;

        getline(ss, uiName, ',');
        std::getline(ss, typeStr, ',');
        std::getline(ss, categoryStr, ',');
        std::getline(ss, roundStr, ',');
        std::getline(ss, aspectStr, ',');
        std::getline(ss, holeStr, ',');
        std::getline(ss, ingredientStr, ',');

        OrderEntry entry;
        entry.uiName = uiName;

        // EOrderType
        auto maybeType = magic_enum::enum_cast<EOrderType>(typeStr);
        entry.data.type = maybeType.value_or(EOrderType::ShapeOnly);

        // EOdenShapeCategory
        auto maybeCat = magic_enum::enum_cast<EOdenShapeCategory>(categoryStr);
        entry.data.requiredCategory = maybeCat.value_or(EOdenShapeCategory::None);

        // ShapeProperty
        entry.data.targetProperty.roundness = stof(roundStr);
        entry.data.targetProperty.aspectRatio = stof(aspectStr);
        entry.data.targetProperty.holeNess = stof(holeStr);

        // EOdenType
        auto maybeIngredient = magic_enum::enum_cast<EOdenType>(ingredientStr);
        entry.data.requiredIngredient = maybeIngredient.value_or(EOdenType::None);

        // 仕分け
        if (entry.data.type == EOrderType::ShapeOnly)
            orderDB.shapeOrders.push_back(entry);
        else
            orderDB.ingredientOrders.push_back(entry);
    }

    file.close();
}