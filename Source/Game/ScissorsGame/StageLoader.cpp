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
                    { {5,0,5}, YarnEnemyType::RescueEnemy, 0.0f },
                    { {8,0,5}, YarnEnemyType::Static, 0.0f, true },
                    { {2,0,5}, YarnEnemyType::Static, 0.0f }
                },
                false,
                1
            }
        };
        break;

    case 2:
        data.waves = {
       {
                // Wave 1
                {
                    { { 5,0,5 },  YarnEnemyType::Static, 0.0f },
                    { { 8,0,5 },  YarnEnemyType::Static, 0.0f,true },
    #if 0
                    { { 10,0,5 }, YarnEnemyType::Static, 0.0f ,true},
                    { { 12,0,5 }, YarnEnemyType::Static, 0.0f },
                    { { 15,0,5 }, YarnEnemyType::WaveHorizontal, 0.0f },
                    { { 18,0,5 }, YarnEnemyType::WaveVertical, 0.0f },

                    { {  5,0,8 }, YarnEnemyType::LongRangeAttack, 0.0f },
                    { { 5,0,10 }, YarnEnemyType::ChasePlayer, 0.0f },
                    { { 5,0,12 }, YarnEnemyType::WaveHorizontal, 0.0f },
                    { { 5,0,15 }, YarnEnemyType::WaveVertical, 0.0f },
                    { { 5,0,18 }, YarnEnemyType::RescueEnemy, 0.0f },

                    #endif // 0
                },
                false,
               1
            },

            {
                // Wave 2（ちょい圧）
        {
                    // 左上　から　右下
                                            { { 18,0,5 },  YarnEnemyType::Static, 0.0f },
                            { { 15,0,7 },  YarnEnemyType::Static, 0.3f },
                            { { 12,0,10 }, YarnEnemyType::RescueEnemy, 0.5f },
                            { { 10,0,12 }, YarnEnemyType::Static, 0.8f },
                            { { 7,0,15 }, YarnEnemyType::Static, 1.2f },
                            { { 5,0,18 }, YarnEnemyType::RescueEnemy, 1.5f },

            #if 0
                            // 右上　から　左下
                    { { 5,0,5 },  YarnEnemyType::Static, 0.0f },
                    { { 7,0,7 },  YarnEnemyType::Static, 0.3f },
                    { { 10,0,10 }, YarnEnemyType::Static, 0.5f },
                    { { 12,0,12 }, YarnEnemyType::Static, 0.8f },
                    { { 15,0,15 }, YarnEnemyType::Static, 1.2f },
                    { { 18,0,18 }, YarnEnemyType::Static, 1.5f },

                    #endif // 0
                },
                        false
                    },

                    {
                        // Wave 3（追い込み）
                        {
                            { {21,0,11}, YarnEnemyType::MoveHorizontal, 4.0f },
                            { {19,0,12}, YarnEnemyType::MoveHorizontal, 3.5f },
                            { {21,0,13}, YarnEnemyType::MoveHorizontal, 4.0f },
                        },
                        false
                    },
                    {
                        // Wave 3（追い込み）
                        {
                            { {0,0,4}, YarnEnemyType::MoveHorizontal, 4.0f },
                            { {1,0,5}, YarnEnemyType::MoveHorizontal, 3.5f },
                            { {0,0,6}, YarnEnemyType::MoveHorizontal, 4.0f },
                        },
                        false
                    },

                    {
                        // Wave 3（追い込み）
                        {
                            { {14,0,21}, YarnEnemyType::MoveVertical, 4.0f },
                            { {16,0,21}, YarnEnemyType::MoveVertical, 4.0f },
                            { {4,0,0}, YarnEnemyType::MoveVertical, 4.5f },
                            { {6,0,0}, YarnEnemyType::MoveVertical, 4.5f },
                        },
                        false
                    }
        };
        break;
    }

    return data;
}