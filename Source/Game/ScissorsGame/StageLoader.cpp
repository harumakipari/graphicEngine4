#include "pch.h"
#include "StageLoader.h"

StageData StageLoader::Load(STAGE_NAME stageId)
{
    StageData data;

    switch (stageId)
    {
    case STAGE_NAME::FIRST:

        data.waves = {
            // Wave1：横一列
          {
              {
                  { {12,0,10}, YarnEnemyType::Static ,0.0f},
                  { {12,0,12}, YarnEnemyType::Static ,0.0f},
                  { {12,0,14}, YarnEnemyType::Static ,0.0f},
                  { {12,0,16}, YarnEnemyType::Static ,0.0f},
                  { {12,0,18}, YarnEnemyType::Static ,0.0f},
                  //{ {8,0,12}, YarnEnemyType::Static ,0.0f},
                  //{ {12,0,12}, YarnEnemyType::Static ,0.0f},
                  //{ {16,0,12}, YarnEnemyType::Static ,0.0f},
              },
          },

#if 0
          // Wave2：直線
{
    {
        { {6,0,0}, YarnEnemyType::MoveVertical,0.0f },
        { {10,0,0}, YarnEnemyType::MoveVertical,0.0f },
        { {14,0,24}, YarnEnemyType::MoveVertical,0.0f },
        { {18,0,24}, YarnEnemyType::MoveVertical,0.0f },
    },
    false,
    -1,
    1.0f,
},

// Wave3：
{
    {
        { {0,0,23.5}, YarnEnemyType::RescueEnemy,0.0f,false,3.0f },
        { {23.5,0,23.5}, YarnEnemyType::RescueEnemy,0.0f, false,3.0f},
    },
     false,
    -1,
    5.0f,
},
// Wave4：ミッキー
{
    {
        { { 23.5,0,16 }, YarnEnemyType::MoveLinear,0.0f,false,2.0f,1.0f,{-1,0,0}},
        { {21,0,15}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{-1,0,0}},
        { {21,0,17}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{-1,0,0}},

        { { 1,0,8 }, YarnEnemyType::MoveLinear,0.0f,false,2.0f,1.0f,{1,0,0}},
        { {0,0,9}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{1,0,0}},
        { {0,0,7}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{1,0,0}},
    },
     false,
      -1,
      7.5f,
},
// Wave5：波波
{
{
        // 左上
        { {3,0,21}, YarnEnemyType::WaveMoveBehavior,0.0f,false,2.0f ,1.0f,{1,0,-1}},
        { {3,0,22.5}, YarnEnemyType::WaveMoveBehavior,0.0f, false,2.0f,1.0f,{1,0,-1}},
        { {1.5,0,21}, YarnEnemyType::WaveMoveBehavior,0.0f, false,2.0f,1.0f,{1,0,-1}},

        // 左下
                { {3,0,3}, YarnEnemyType::WaveMoveBehavior,0.0f,false,2.0f ,1.0f,{1,0,1}},
        { {3,0,1.5}, YarnEnemyType::WaveMoveBehavior,0.0f, false,2.0f,1.0f,{1,0,1}},
        { {1.5,0,3}, YarnEnemyType::WaveMoveBehavior,0.0f, false,2.0f,1.0f,{1,0,1}},

        // 右上
                { {21,0,21}, YarnEnemyType::WaveMoveBehavior,0.0f,false,2.0f ,1.0f,{-1,0,-1}},
        { {21,0,22.5}, YarnEnemyType::WaveMoveBehavior,0.0f, false,2.0f,1.0f,{-1,0,-1}},
        { {22.5,0,21}, YarnEnemyType::WaveMoveBehavior,0.0f, false,2.0f,1.0f,{-1,0,-1}},

        // 右下
        { {21,0,3}, YarnEnemyType::WaveMoveBehavior,0.0f,false,2.0f ,1.0f,{-1,0,1}},
        { {21,0,1.5}, YarnEnemyType::WaveMoveBehavior,0.0f, false,2.0f,1.0f,{-1,0,1}},
        { {22.5,0,3}, YarnEnemyType::WaveMoveBehavior,0.0f, false,2.0f,1.0f,{-1,0,1}},

    },
     false,
          -1,
          5.0f,
    },

    // Wave6：急かす
{
    {
              { {12,0,12}, YarnEnemyType::Static ,0.0f},
        { {0,0,12}, YarnEnemyType::RescueEnemy,2.0f,false,3.0f },
        { {23,0,12}, YarnEnemyType::RescueEnemy,2.0f, false,3.0f},
#if 0
                            { { 23.5,0,15 }, YarnEnemyType::MoveLinear,0.0f,false,2.0f,1.0f,{-1,0,0}},
                { {21,0,16}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{-1,0,0}},
                { {21,0,14}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{-1,0,0}},

                { { 1,0,8 }, YarnEnemyType::MoveLinear,0.0f,false,2.0f,1.0f,{1,0,0}},
                { {0,0,6}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{1,0,0}},
                { {0,0,7}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{1,0,0}},
#endif // 0


        },
         false,
              -1,
              3.0f,
    },

                #endif // 0


        };


        break;

    case STAGE_NAME::BOBBIN_FIRST: // 中央糸巻ステージ
        data.waves = {
          {
              {
                  { {12,0,15}, YarnEnemyType::Static ,0.0f,true},
                  { {9,0,12}, YarnEnemyType::Static ,0.0f,true},
                  { {15,0,12}, YarnEnemyType::Static ,0.0f,true},

              },
              true,
          },
          // Wave2：ミッキー　中央に集まる
            {
    {
                    // 左上
                    { {3,0,21}, YarnEnemyType::MoveLinear,1.0f,true,2.0f ,1.0f,{1,0,-1}},
                    { {3,0,22.5}, YarnEnemyType::MoveLinear,1.0f, false,2.0f,1.0f,{1,0,-1}},
                    { {1.5,0,21}, YarnEnemyType::MoveLinear,1.0f, false,2.0f,1.0f,{1,0,-1}},

                    // 右上
                            { {21,0,21}, YarnEnemyType::MoveLinear,6.0f,true,2.0f ,1.0f,{-1,0,-1}},
                    { {21,0,22.5}, YarnEnemyType::MoveLinear,6.0f, false,2.0f,1.0f,{-1,0,-1}},
                    { {22.5,0,21}, YarnEnemyType::MoveLinear,6.0f, false,2.0f,1.0f,{-1,0,-1}},

                    // 左下
                            { {3,0,3}, YarnEnemyType::MoveLinear,6.0f,true,2.0f ,1.0f,{1,0,1}},
                    { {3,0,1.5}, YarnEnemyType::MoveLinear,6.0f, false,2.0f,1.0f,{1,0,1}},
                    { {1.5,0,3}, YarnEnemyType::MoveLinear,6.0f, false,2.0f,1.0f,{1,0,1}},

                    // 右下
                    { {21,0,3}, YarnEnemyType::MoveLinear,1.0f,true,2.0f ,1.0f,{-1,0,1}},
                    { {21,0,1.5}, YarnEnemyType::MoveLinear,1.0f, false,2.0f,1.0f,{-1,0,1}},
                    { {22.5,0,3}, YarnEnemyType::MoveLinear,1.0f, false,2.0f,1.0f,{-1,0,1}},

                    },
                    false,-1,5.0f,false

                    },
            // Wave3：ミッキー 追いかける
    {
        {
            // 左　
            { { 1,0,12 }, YarnEnemyType::ChasePlayer,0.0f,false,2.0f,1.0f,{1,0,0}},
            { {0,0,13}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{1,0,0}},
            { {0,0,11}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{1,0,0}},

            // 右
            { {23,0,12 }, YarnEnemyType::ChasePlayer,0.0f,false,2.0f,1.0f,{-1,0,0}},
            { {24,0,13}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{-1,0,0}},
            { {24,0,11}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{-1,0,0}},

            // 上
    { { 12,0,23 }, YarnEnemyType::ChasePlayer,0.0f,false,2.0f,1.0f,{1,0,0}},
    { {11,0,24}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{1,0,0}},
    { {13,0,24}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{1,0,0}},

    // 下
    { { 12,0,1 }, YarnEnemyType::ChasePlayer,0.0f,false,2.0f,1.0f,{1,0,0}},
    { {11,0,0}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{1,0,0}},
    { {13,0,0}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{1,0,0}},

        },
        false,-1,8.0f,


},
// Wave4：直線
{
    {
        { { 2.5f,0,2 }, YarnEnemyType::MoveLinear,0.0f,false,2.0f,1.0f,{0,0,1}},
        { {3.5f,0,2}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{0,0,1}},

        { { 20.5f,0,22.0f }, YarnEnemyType::MoveLinear,0.0f,false,2.0f,1.0f,{0,0,-1}},
        { {21.5f,0,22.0f}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{0,0,-1}},
    },
       false,-1,8.0f,
},
// Wave5：波波 パレード
{
{
        // 左上
        { {3,0,21}, YarnEnemyType::WaveMoveBehavior,0.0f,true,2.0f ,1.0f,{1,0,-1}},

        // 真ん中上
            { {12,0,21}, YarnEnemyType::WaveMoveBehavior,2.0f,false,2.0f ,1.0f,{0,0,-1}},

            // 右上
                { {21,0,21}, YarnEnemyType::WaveMoveBehavior,0.0f,true,2.0f ,1.0f,{-1,0,-1}},

                // 右横
        { {21,0,12}, YarnEnemyType::WaveMoveBehavior,2.0f,false,2.0f ,1.0f,{-1,0,0}},

        // 右下
    { {21,0,3}, YarnEnemyType::WaveMoveBehavior,0.0f,true,2.0f ,1.0f,{-1,0,1}},

    // 真ん中上
    { {12,0,3}, YarnEnemyType::WaveMoveBehavior,2.0f,false,2.0f ,1.0f,{0,0,1}},

    // 左下
            { {3,0,3}, YarnEnemyType::WaveMoveBehavior,0.0f,true,2.0f ,1.0f,{1,0,1}},

            // 左横
    { {3,0,12}, YarnEnemyType::WaveMoveBehavior,2.0f,false,2.0f ,1.0f,{1,0,0}},


},
  false,-1,6.0f,
},
        };
        break;
    case STAGE_NAME::DIFFICULT: // ハリネズミ敵ステージ
        data.waves =
        {
                       {
                // Wave 1 ハリネズミ
                {
                    { { 12,0,12},  YarnEnemyType::LongRangeAttack, 0.0f },
                },
            },
#if 1
            {
                // Wave 2　ハリネズミ＋追尾4体
        {
                    { { 12,0,12},  YarnEnemyType::LongRangeAttack, 0.0f },

                    // 左下
                { { 2.5f,0,2 }, YarnEnemyType::ChasePlayer,2.0f,false,2.0f,1.0f,{0,0,1}},
                { {3.5f,0,2}, YarnEnemyType::ChasePlayer,2.0f, false,2.0f,1.0f,{0,0,1}},
                // 右上　
                { { 20.5f,0,22.0f }, YarnEnemyType::ChasePlayer,2.0f,false,2.0f,1.0f,{0,0,-1}},
                { {21.5f,0,22.0f}, YarnEnemyType::ChasePlayer,2.0f, false,2.0f,1.0f,{0,0,-1}},

                // 右下
                    { { 20.5f,0,2 }, YarnEnemyType::ChasePlayer,4.0f,false,2.0f,1.0f,{0,0,1}},
        { {21.5f,0,2}, YarnEnemyType::ChasePlayer,4.0f, false,2.0f,1.0f,{0,0,1}},
        // 左上　
        { { 2.5f,0,22.0f }, YarnEnemyType::ChasePlayer,4.0f,false,2.0f,1.0f,{0,0,-1}},
        { {3.5f,0,22.0f}, YarnEnemyType::ChasePlayer,4.0f, false,2.0f,1.0f,{0,0,-1}},

                                        },false, -1,5.0f

                                            },
                     #endif // 0

#if 0
                       {
                // Wave 3 ハリネズミ ハサミ
                {
                    { { 12,0,12},  YarnEnemyType::LongRangeAttack, 0.0f },
                                      { {1.0f,0,12.0f}, YarnEnemyType::RescueEnemy,0.0f,false,3.0f },
                       { {23.0f,0,12.0f}, YarnEnemyType::RescueEnemy,0.0f, false,3.0f},
                },
                    false,
                       -1,
                       10.0f,
                       true
            },

                     #endif // 0
              {
                // Wave ４　波波　ハサミ
                {
                      // 右下
                      { { 18,0,2},  YarnEnemyType::WaveMoveBehavior, 0.f ,false,2.0f ,1.0f,{0,0,1}},
                      { { 19,0,1},  YarnEnemyType::WaveMoveBehavior, 0.0f ,false,2.0f ,1.0f,{0,0,1}},
                      { { 17,0,1},  YarnEnemyType::WaveMoveBehavior, 0.0f ,false,2.0f ,1.0f,{0,0,1}},

                      // 左上
                      { { 6,0,22},  YarnEnemyType::WaveMoveBehavior, 0.0f ,false,2.0f ,1.0f,{0,0,-1}},
                      { { 7,0,23},  YarnEnemyType::WaveMoveBehavior, 0.0f ,false,2.0f ,1.0f,{0,0,-1}},
                      { { 5,0,23},  YarnEnemyType::WaveMoveBehavior, 0.0f ,false,2.0f ,1.0f,{0,0,-1}},

                      // ハサミ
                                 { {12.0f,0,1.0f}, YarnEnemyType::RescueEnemy,2.0f,false,3.0f },
                  { {12.0f,0,23.0f}, YarnEnemyType::RescueEnemy,2.0f, false,3.0f},
                  },
                  false,
                  -1,
                  7.0f,
                  true
              },


            // Wave 5　ひん死の敵
        {
                      {

                        { { 6.0,0,11.0},  YarnEnemyType::Static, .0f ,false,2.0f ,1.0f,{0,0,1},true},
                       {{10.0,0,11.0},  YarnEnemyType::Static,0.0f + 0.3f ,false,2.0f ,1.0f,{0,0,1},true},
                       {{12.0,0,12.0},  YarnEnemyType::Static,0.0f  ,false,2.0f ,1.0f,{0,0,1},true},
                       {{14.0,0,11.0},  YarnEnemyType::Static,0.0f + 0.3f ,false,2.0f ,1.0f,{0,0,1},true },
                       {{18.0,0,11.0},  YarnEnemyType::Static,0.0f  ,false,2.0f ,1.0f,{0,0,1},true },
                       {{16.0,0,8.0},  YarnEnemyType::Static,0.0f + 0.3f ,false,2.0f ,1.0f,{0,0,1},true },
                       {{18.0,0,5.0},  YarnEnemyType::Static, 0.0f ,false,2.0f ,1.0f,{0,0,1},true },
                       {{14.0,0,5.0},  YarnEnemyType::Static, 0.0f + 0.3f ,false,2.0f ,1.0f,{0,0,1},true },
                       {{12.0,0,2.0},  YarnEnemyType::Static, 0.0f  ,false,2.0f ,1.0f,{0,0,1},true },
                       {{10.0,0,5.0},  YarnEnemyType::Static, 0.0f + 0.3f ,false,2.0f ,1.0f,{0,0,1},true },
                       {{6.0,0,5.0},  YarnEnemyType::Static, 0.0f  ,false,2.0f ,1.0f,{0,0,1},true },
                       {{8.0,0,8.0},  YarnEnemyType::Static, 0.0f + 0.3f ,false,2.0f ,1.0f,{0,0,1},true },
                       {{12.0,0,8.0},  YarnEnemyType::Static, 0.0f + 0.6f ,true,2.0f ,1.0f,{0,0,1},true },
                       // { { 6.0,0,15.0},  YarnEnemyType::Static, .0f ,false,2.0f ,1.0f,{0,0,1},true},
                       //{{10.0,0,15.0},  YarnEnemyType::Static,0.0f + 0.2f ,false,2.0f ,1.0f,{0,0,1},true},
                       //{{12.0,0,18.0},  YarnEnemyType::Static,0.0f + 0.4f ,false,2.0f ,1.0f,{0,0,1},true},
                       //{{14.0,0,15.0},  YarnEnemyType::Static,0.0f + 0.6f ,false,2.0f ,1.0f,{0,0,1},true },
                       //{{18.0,0,15.0},  YarnEnemyType::Static,0.0f + 0.8f ,false,2.0f ,1.0f,{0,0,1},true },
                       //{{16.0,0,12.0},  YarnEnemyType::Static,0.0f + 1.0f ,false,2.0f ,1.0f,{0,0,1},true },
                       //{{18.0,0,9.0},  YarnEnemyType::Static, 0.0f + 1.2f ,false,2.0f ,1.0f,{0,0,1},true },
                       //{{14.0,0,9.0},  YarnEnemyType::Static, 0.0f + 1.4f ,false,2.0f ,1.0f,{0,0,1},true },
                       //{{12.0,0,6.0},  YarnEnemyType::Static, 0.0f + 1.6f ,false,2.0f ,1.0f,{0,0,1},true },
                       //{{10.0,0,9.0},  YarnEnemyType::Static, 0.0f + 1.8f ,false,2.0f ,1.0f,{0,0,1},true },
                       //{{6.0,0,9.0},  YarnEnemyType::Static, 0.0f + 2.0f ,false,2.0f ,1.0f,{0,0,1},true },
                       //{{8.0,0,12.0},  YarnEnemyType::Static, 0.0f + 2.2f ,false,2.0f ,1.0f,{0,0,1},true },
                       //{{12.0,0,12.0},  YarnEnemyType::Static, 0.0f + 2.4f ,true,2.0f ,1.0f,{0,0,1},true },

    },   false,
                  -1,
                  15.0f,
                  true
        },
            // Wave 6 ハリネズミ　大直線４　　ハサミ
#if 0
                            {
                {
                   { { 12,0,12},  YarnEnemyType::LongRangeAttack, 0.0f },

                   // // 大きい
                   //{ { 4.5,0,4.5},  YarnEnemyType::MoveLinear, 0.0f ,false,2.0f,1.0f,{1.0,0.0,1.0}},
                   //{ { 4.5,0,19.5},  YarnEnemyType::MoveLinear, 0.0f ,false,2.0f,1.0f,{1.0,0.0,-1.0}},
                   //{ { 19.5,0,19.5},  YarnEnemyType::MoveLinear, 0.0f ,false,2.0f,1.0f,{-1.0,0.0,-1.0}},
                   //{ { 19.5,0,4.5},  YarnEnemyType::MoveLinear, 0.0f ,false,2.0f,1.0f,{-1.0,0.0,1.0}},

                   // ハサミ
                    // 右上
               { {22.5f,0,22.5f}, YarnEnemyType::RescueEnemy,1.0f,false,3.0f },
               // 左下
           { {1.5f,0,1.5f}, YarnEnemyType::RescueEnemy,1.0f, false,3.0f},

           }, false,
                 -1,
                 15.0f,
                 true
       },

                 #endif // 0
            // Wave 7 ハリネズミ　追尾
    {
        {
           { { 12,0,12},  YarnEnemyType::LongRangeAttack, 0.0f },


           // 左上
           { {3,0,21}, YarnEnemyType::ChasePlayer,0.0f,false,2.0f ,1.0f,{1,0,-1}},
           { {3,0,22.5}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{1,0,-1}},
           { {1.5,0,21}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{1,0,-1}},

           // 左下
                   { {3,0,3}, YarnEnemyType::ChasePlayer,0.0f,false,2.0f ,1.0f,{1,0,1}},
           { {3,0,1.5}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{1,0,1}},
           { {1.5,0,3}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{1,0,1}},

           // 右上
                   { {21,0,21}, YarnEnemyType::ChasePlayer,0.0f,false,2.0f ,1.0f,{-1,0,-1}},
           { {21,0,22.5}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{-1,0,-1}},
           { {22.5,0,21}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{-1,0,-1}},

           // 右下
           { {21,0,3}, YarnEnemyType::ChasePlayer,0.0f,false,2.0f ,1.0f,{-1,0,1}},
           { {21,0,1.5}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{-1,0,1}},
           { {22.5,0,3}, YarnEnemyType::ChasePlayer,0.0f, false,2.0f,1.0f,{-1,0,1}},


           // ハサミ
       { {1.0f,0,12.0f}, YarnEnemyType::RescueEnemy,0.0f,false,3.0f },
        { {23.0f,0,12.0f}, YarnEnemyType::RescueEnemy,0.0f, false,3.0f},

        },
         false,
                       -1,
                       15.0f,
                       true
     },


        };
        break;

    case STAGE_NAME::BOBBIN_SECOND:
        data.waves =
        {
           {
                // Wave 1
                {
                    { { 10.5,0,11 },  YarnEnemyType::MoveLinear, 1.0f,false,2.0f ,1.0f,{-1,0,-1}},
                    { { 10.5,0,12 },  YarnEnemyType::MoveLinear, 1.0f,false,2.0f ,1.0f,{-1,0,-1}},
                    { { 12,0,11 },  YarnEnemyType::MoveLinear, 1.0f,false,2.0f ,1.0f,{-1,0,-1}},

                    { { 1,0,23 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{0,0,-1}},
                    { { 2.5,0,23 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{0,0,-1}},
                    { { 4,0,23 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{0,0,-1}},

                },
                false,
               -1,0.0f,true
            },
           {
               // Wave 2
               {
                   { { 11.5,0,11 },  YarnEnemyType::MoveLinear, 1.0f,false,2.0f ,1.0f,{1,0,1}},
                   { { 11.5,0,12 },  YarnEnemyType::MoveLinear, 1.0f,false,2.0f ,1.0f,{1,0,1}},
                   { { 13,0,11 },  YarnEnemyType::MoveLinear, 1.0f,false,2.0f ,1.0f,{1,0,1}},

                    { { 20,0,1 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{0,0,1}},
                    { { 21.5,0,1 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{0,0,1}},
                    { { 23,0,1 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{0,0,1}},

               },
               false,
              -1,7.0f,true
           },
           {
               // Wave 3
               {
                   { { 12,0,12 },  YarnEnemyType::LongRangeAttack, 0.0f,false,2.0f ,1.0f,{1,0,1}},
               },
               false,
              -1,5.0f,true
           },
           {
               // Wave 4
        {
                    { { 2.0,0,23 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{0,0,-1}},
                    { { 4,0,23 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{0,0,-1}},

                    { { 20,0,1 },  YarnEnemyType::MoveLinear, 0.5f,false,2.0f ,1.0f,{0,0,1}},
                    { { 22,0,1 },  YarnEnemyType::MoveLinear, 0.5f,false,2.0f ,1.0f,{0,0,1}},

               },
               false,
              -1,5.0f,true
           },
           {
               // Wave 5
        {
                    { { 1.0,0,13 },  YarnEnemyType::ChasePlayer, 0.0f,false,2.0f ,1.0f,{0,0,-1}},
                    { { 1.0,0,11 },  YarnEnemyType::ChasePlayer, 0.0f,false,2.0f ,1.0f,{0,0,-1}},

                    { { 23,0,13 },  YarnEnemyType::ChasePlayer, 0.0f,false,2.0f ,1.0f,{0,0,1}},
                    { { 23,0,11 },  YarnEnemyType::ChasePlayer, 0.0f,false,2.0f ,1.0f,{0,0,1}},

                    { { 12,0,23 },  YarnEnemyType::RescueEnemy, 2.0f,false,3.0f ,1.0f,{0,0,1}},
                    { { 12,0,1 },  YarnEnemyType::RescueEnemy, 2.0f,false,3.0f ,1.0f,{0,0,1}},

               },
               false,
              -1,7.0f,true
           },
           {
               // Wave 6
               {
                   { { 12,0,12 },  YarnEnemyType::LongRangeAttack, 0.0f,false,2.0f ,1.0f,{1,0,1}},
                   { { 1.5f,0,22.5f },  YarnEnemyType::ChasePlayer, 0.0f,true,2.0f ,1.0f,{1,0,1}},
                   { { 22.5f,0,1.5f },  YarnEnemyType::ChasePlayer, 0.0f,true,2.0f ,1.0f,{1,0,1}},
               },
               false,
              -1,10.0f,true
           },
        };

        break;
    case STAGE_NAME::BOSS: // ボスステージ
        data.waves =
        {

        };

        data.bossData.hasBoss = true;
        data.bossData.position = { 12.0f, 0.0f, 18.0f };
        break;
    case STAGE_NAME::REFLECT_WALL: // 反射ステージ
        data.waves =
        {
            {
                // Wave 1
        {
                    //                    { { 12,0,21 },  YarnEnemyType::Static, 0.0f,false,2.0f ,1.0f,{1,0,0}},
                    //{ { 12,0,3 },  YarnEnemyType::Static, 0.0f,false,2.0f ,1.0f,{1,0,0}},


                    { { 1,0,16 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{1,0,0}},
                    { { 1,0,14 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{1,0,0}},

                    { { 23,0,10 },  YarnEnemyType::MoveLinear, 0.5f,false,2.0f ,1.0f,{-1,0,0}},
                    { { 23,0,12 },  YarnEnemyType::MoveLinear, 0.5f,false,2.0f ,1.0f,{-1,0,0}},
                },
                false,
               -1,0.0f,true
            },
            {
                // Wave 2
                {


                    { { 1,0,15 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{1,0,0}},
                    { { 1,0,11 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{1,0,0}},

                    { { 23,0,13 },  YarnEnemyType::MoveLinear, 0.5f,false,2.0f ,1.0f,{-1,0,0}},
                    { { 23,0,10 },  YarnEnemyType::MoveLinear, 0.5f,false,2.0f ,1.0f,{-1,0,0}},
                },
                true,
               -1,7.0f,true
            },
            {
                // Wave 2
        {
                },
                false,
               -1,2.0f,false
            },

            {
                // Wave 3
        {
                    { { 16,0,1 },  YarnEnemyType::MoveLinear, 0.5f,false,2.0f ,1.0f,{0,0,1}},
                    { { 14,0,1 },  YarnEnemyType::MoveLinear, 0.5f,false,2.0f ,1.0f,{0,0,1}},

                    { { 8,0,23 },  YarnEnemyType::MoveLinear, 1.0f,false,2.0f ,1.0f,{0,0,-1}},
                    { { 10,0,23 },  YarnEnemyType::MoveLinear, 1.0f,false,2.0f ,1.0f,{0,0,-1}},
                },
                false,
               -1,0.5f,true
            },
            {
                // Wave 5
                {
                    { { 15,0,1 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{0,0,1}},
                    { { 11,0,1 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{0,0,1}},

                    { { 13,0,23 },  YarnEnemyType::MoveLinear, 0.5f,false,2.0f ,1.0f,{0,0,-1}},
                    { { 9,0,23 },  YarnEnemyType::MoveLinear, 0.5f,false,2.0f ,1.0f,{0,0,-1}},
                },
                false,
               -1,7.0f,true
            },
            {
                // Wave 3
        {
                    { { 16,0,1 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{0,0,1}},
                    { { 14,0,1 },  YarnEnemyType::MoveLinear, 0.0f,false,2.0f ,1.0f,{0,0,1}},

                    { { 8,0,23 },  YarnEnemyType::MoveLinear, 0.5f,false,2.0f ,1.0f,{0,0,-1}},
                    { { 10,0,23 },  YarnEnemyType::MoveLinear, 0.5f,false,2.0f ,1.0f,{0,0,-1}},
                },
                false,
               -1,4.0f,true
            },

#if 0
            // Wave5：波波
                        {
        {
                                // 左上
                                { {3,0,21}, YarnEnemyType::MoveLinear,0.0f,false,2.0f ,1.0f,{1,0,-1}},
                                { {3,0,22.5}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{1,0,-1}},
                                { {1.5,0,21}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{1,0,-1}},

                                // 左下
                                        { {3,0,3}, YarnEnemyType::MoveLinear,0.0f,false,2.0f ,1.0f,{1,0,1}},
                                { {3,0,1.5}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{1,0,1}},
                                { {1.5,0,3}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{1,0,1}},

                                // 右上
                                        { {21,0,21}, YarnEnemyType::MoveLinear,0.0f,false,2.0f ,1.0f,{-1,0,-1}},
                                { {21,0,22.5}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{-1,0,-1}},
                                { {22.5,0,21}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{-1,0,-1}},

                                // 右下
                                { {21,0,3}, YarnEnemyType::MoveLinear,0.0f,false,2.0f ,1.0f,{-1,0,1}},
                                { {21,0,1.5}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{-1,0,1}},
                                { {22.5,0,3}, YarnEnemyType::MoveLinear,0.0f, false,2.0f,1.0f,{-1,0,1}},

                            },
                             false,
                                  -1,
                                  10.0f,
                            },
                                #endif // 0

        };
        break;
    }

    return data;
}

