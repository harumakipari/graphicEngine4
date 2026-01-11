#pragma once
#include "Engine/Utility/JsonFileHandler.h"
#include "OdenShapeDataTable.h"



// ゲームパラメータ
class OdenGameParameter
{
public:
    // 具材のそれぞれの面のデータ
    static void LoadOdenFaceShapeTableFromCSV(const std::string& filePath);

    // オ－ダーのデータ
    static void LoadOrderDBFromCSV(const std::string& filePath);

    inline static std::unordered_map<std::string, OdenFaceShapeTable> odenTypeShapes;

    inline static OdenOrderDatabase orderDB;
};