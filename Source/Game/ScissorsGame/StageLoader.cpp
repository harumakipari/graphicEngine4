#include "pch.h"
#include "StageLoader.h"

StageData StageLoader::Load(int stageId)
{
    StageData data;

    switch (stageId)
    {
    case 1:
        data.waves = {
            {
                {
                    { {5,0,5}, YarnEnemyType::Static, 0.0f },
                    { {8,0,5}, YarnEnemyType::LongRangeAttack, 0.0f, true }
                },
                false,
                1
            }
        };
        break;

    case 2:
        data.waves = {
            {
                {
                    { {18,0,5}, YarnEnemyType::Static, 0.0f },
                    { {15,0,7}, YarnEnemyType::Static, 0.3f }
                },
                false
            }
        };
        break;
    }

    return data;
}