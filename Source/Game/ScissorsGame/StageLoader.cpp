#include "pch.h"
#include "StageLoader.h"

StageData StageLoader::Load(int stageId)
{
    StageData data;

    switch (stageId)
    {
    case 1:

        data.waves = {
            // Wave1：横一列
          {
              {
                  { {6,0,12}, YarnEnemyType::Static ,0.0f},
                  { {12,0,12}, YarnEnemyType::Static ,0.0f},
                  { {18,0,12}, YarnEnemyType::Static ,0.0f},
              },

          },

          // Wave2：直線
          {
              {
                  { {3,0,0}, YarnEnemyType::MoveVertical,2.0f },
                  { {9,0,0}, YarnEnemyType::MoveVertical,2.0f  },
                  { {15,0,24}, YarnEnemyType::MoveVertical,2.0f },
                  { {21,0,24}, YarnEnemyType::MoveVertical,2.0f},
              },

          },

          // Wave3：
          {
              {
                  { {0,0,23.5}, YarnEnemyType::RescueEnemy,6.0f,false,3.0f },
                  { {23.5,0,23.5}, YarnEnemyType::RescueEnemy,6.0f, false,3.0f},
              },

          },
          // Wave4：ミッキー
        {
            {
                { { 23.5,0,18 }, YarnEnemyType::MoveHorizontal,8.0f,true, },
                { {21,0,19}, YarnEnemyType::MoveHorizontal,8.0f, false,},
                { {21,0,17}, YarnEnemyType::MoveHorizontal,8.0f, false,},

                { { 1,0,6 }, YarnEnemyType::MoveHorizontal,8.0f,true,},
                { {0,0,5}, YarnEnemyType::MoveHorizontal,8.0f, false,},
                { {0,0,7}, YarnEnemyType::MoveHorizontal,8.0f, false,},
            },

        },
        // Wave5：波波
    {
    {
        // 左上
        { {3,0,21}, YarnEnemyType::WaveMoveBehavior,6.0f,false,2.0f ,1.0f,{1,0,-1}},
        { {3,0,22.5}, YarnEnemyType::WaveMoveBehavior,6.0f, false,2.0f,1.0f,{1,0,-1}},
        { {1.5,0,21}, YarnEnemyType::WaveMoveBehavior,6.0f, false,2.0f,1.0f,{1,0,-1}},

        // 左下
                { {3,0,3}, YarnEnemyType::WaveMoveBehavior,6.0f,false,2.0f ,1.0f,{1,0,1}},
        { {3,0,1.5}, YarnEnemyType::WaveMoveBehavior,6.0f, false,2.0f,1.0f,{1,0,1}},
        { {1.5,0,3}, YarnEnemyType::WaveMoveBehavior,6.0f, false,2.0f,1.0f,{1,0,1}},

        // 右上
                { {21,0,21}, YarnEnemyType::WaveMoveBehavior,6.0f,false,2.0f ,1.0f,{-1,0,-1}},
        { {21,0,22.5}, YarnEnemyType::WaveMoveBehavior,6.0f, false,2.0f,1.0f,{-1,0,-1}},
        { {22.5,0,21}, YarnEnemyType::WaveMoveBehavior,6.0f, false,2.0f,1.0f,{-1,0,-1}},

        // 右下
        { {21,0,3}, YarnEnemyType::WaveMoveBehavior,6.0f,false,2.0f ,1.0f,{-1,0,1}},
        { {21,0,1.5}, YarnEnemyType::WaveMoveBehavior,6.0f, false,2.0f,1.0f,{-1,0,1}},
        { {22.5,0,3}, YarnEnemyType::WaveMoveBehavior,6.0f, false,2.0f,1.0f,{-1,0,1}},

    },

    },
    // Wave6：急かす
{
    {
        { {0,0,12}, YarnEnemyType::RescueEnemy,5.0f,false,3.0f },
        { {23,0,12}, YarnEnemyType::RescueEnemy,5.0f, false,3.0f},
    },
},


        };


        break;

    case 2:
        data.waves = {
            // Wave1：近くて簡単
          {
              {
                  { {3,0,10}, YarnEnemyType::Static ,0.0f,true},
                  { {18,0,10}, YarnEnemyType::Static ,0.0f,},
              },

          },

          // Wave2：少し離す
          {
              {
                  { {5,0,21}, YarnEnemyType::MoveVertical,2.0f,true },
                  { {8,0,0}, YarnEnemyType::MoveVertical,2.0f , },
                  { {12,0,21}, YarnEnemyType::MoveVertical,2.0f, },
                  { {15,0,0}, YarnEnemyType::MoveVertical,2.0f,true },
              },

          },

          // Wave3：溜めさせる
          {
              {
                  { {5,0,11}, YarnEnemyType::MoveHorizontal,5.0f, },
                  { {7,0,12}, YarnEnemyType::MoveHorizontal,5.0f,true },
                  { {9,0,10}, YarnEnemyType::MoveHorizontal,5.0f,true },
                  { {6,0,9}, YarnEnemyType::MoveHorizontal,5.0f,  },
              },

          },

          // Wave4：急かす
          {
              {
                  { {5,0,5},YarnEnemyType::ChasePlayer ,2.0f,true },
                  { {7,0,5}, YarnEnemyType::ChasePlayer,3.0f, },
                  { {9,0,5}, YarnEnemyType::ChasePlayer,4.0f, },
                  { {6,0,7}, YarnEnemyType::ChasePlayer,5.0f,true },
                  { {10,0,10}, YarnEnemyType::ChasePlayer,6.0f,  },
              },

          },

          // Wave 1
          {
              {
              { { 5,0,5 },  YarnEnemyType::Static, 0.0f },
              { { 8,0,5 },  YarnEnemyType::WaveVertical, 0.0f,true },
              { { 10,0,5 }, YarnEnemyType::Static, 0.0f ,true},
              { { 12,0,5 }, YarnEnemyType::Static, 0.0f },
              { { 15,0,5 }, YarnEnemyType::WaveHorizontal, 0.0f },
              { { 18,0,5 }, YarnEnemyType::WaveVertical, 0.0f },

              { {  5,0,8 }, YarnEnemyType::LongRangeAttack, 0.0f },
              { { 5,0,10 }, YarnEnemyType::ChasePlayer, 0.0f },
              { { 5,0,12 }, YarnEnemyType::WaveHorizontal, 0.0f },
              { { 5,0,15 }, YarnEnemyType::WaveVertical, 0.0f },
              { { 5,0,18 }, YarnEnemyType::RescueEnemy, 0.0f },

          },
          false,
               },
          {
              // Wave 2（ちょい圧）
      {
                  // 左上　から　右下
                                          { { 18,0,5 },  YarnEnemyType::Static, 0.0f },
                          { { 15,0,7 },  YarnEnemyType::LongRangeAttack, 0.3f },
                          { { 12,0,10 }, YarnEnemyType::RescueEnemy, 0.5f },
                          { { 10,0,12 }, YarnEnemyType::Static, 0.8f },
                          { { 7,0,15 }, YarnEnemyType::Static, 1.2f },
                          { { 5,0,18 }, YarnEnemyType::RescueEnemy, 1.5f },

                          // 右上　から　左下
                  { { 5,0,5 },  YarnEnemyType::Static, 0.0f },
                  { { 7,0,7 },  YarnEnemyType::Static, 0.3f },
                  { { 10,0,10 }, YarnEnemyType::Static, 0.5f },
                  { { 12,0,12 }, YarnEnemyType::Static, 0.8f },
                  { { 15,0,15 }, YarnEnemyType::Static, 1.2f },
                  { { 18,0,18 }, YarnEnemyType::Static, 1.5f },

              },
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
                      false,
                  }

        };
        break;
    case 3:
        data.waves = {
       {
                // Wave 1
                {
                    { { 5,0,5 },  YarnEnemyType::Static, 0.0f },
                    { { 8,0,5 },  YarnEnemyType::WaveVertical, 0.0f,true },
    #if 1
                    { { 10,0,5 }, YarnEnemyType::WaveVertical, 0.0f ,true},
                    { { 12,0,5 }, YarnEnemyType::Static, 0.0f },
                    { { 15,0,5 }, YarnEnemyType::WaveHorizontal, 0.0f },
                    { { 18,0,5 }, YarnEnemyType::WaveVertical, 0.0f },

                    //{ {  5,0,8 }, YarnEnemyType::LongRangeAttack, 0.0f },
                    { { 5,0,10 }, YarnEnemyType::ChasePlayer, 0.0f },
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
                                            //{ { 12,0,10 }, YarnEnemyType::RescueEnemy, 0.5f },
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
                                            { {14,0,21}, YarnEnemyType::MoveHorizontal, 4.0f },
                                            { {16,0,21}, YarnEnemyType::MoveHorizontal, 4.0f },
                                            { {4,0,0}, YarnEnemyType::MoveVertical, 4.5f },
                                            { {6,0,0}, YarnEnemyType::MoveVertical, 4.5f },
                                        },
                                        false
                                    }
        };
        break;

    case 4:
#if 1
        data.waves =
        {
           {
                // Wave 1
                {
                    { { 5,0,5 },  YarnEnemyType::LongRangeAttack, 0.0f,true },
                    { { 8,0,5 },  YarnEnemyType::ChasePlayer, 0.0f},
                    { { 10,0,5 }, YarnEnemyType::ChasePlayer, 0.0f ,true},
                    { { 12,0,5 }, YarnEnemyType::ChasePlayer, 0.0f },
                    { { 15,0,5 }, YarnEnemyType::WaveHorizontal, 0.0f,true },
                    { { 18,0,5 }, YarnEnemyType::WaveVertical, 0.0f },
    #if 0

                    { {  5,0,8 }, YarnEnemyType::LongRangeAttack, 0.0f },
                    { { 5,0,10 }, YarnEnemyType::ChasePlayer, 0.0f },
                    { { 5,0,12 }, YarnEnemyType::WaveHorizontal, 0.0f },
                    { { 5,0,15 }, YarnEnemyType::WaveVertical, 0.0f },
                    { { 5,0,18 }, YarnEnemyType::RescueEnemy, 0.0f },

                    #endif // 0
                },
                false,
               1,
            },

            {
                // Wave 
        {

                                            { { 18,0,5 },  YarnEnemyType::Static, 0.0f },
                            { { 15,0,7 },  YarnEnemyType::Static, 0.3f },
                            { { 12,0,10 }, YarnEnemyType::RescueEnemy, 0.5f },
                            { { 10,0,12 }, YarnEnemyType::Static, 0.8f },
                            { { 7,0,15 }, YarnEnemyType::Static, 1.2f },
                            { { 5,0,18 }, YarnEnemyType::RescueEnemy, 1.5f },

            #if 0
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
                        // Wave 3
                        {
                            { {21,0,11}, YarnEnemyType::MoveHorizontal, 1.0f },
                            { {19,0,12}, YarnEnemyType::MoveHorizontal, 1.5f },
                            { {21,0,13}, YarnEnemyType::MoveHorizontal, 1.0f },
                        },
                        false
                    },
                    {
                        // Wave 
                        {
                            { {0,0,4}, YarnEnemyType::MoveHorizontal, 1.0f },
                            { {1,0,5}, YarnEnemyType::MoveHorizontal, 1.5f },
                            { {0,0,6}, YarnEnemyType::MoveHorizontal, 1.0f },
                            //{ { 5,0,5 },  YarnEnemyType::Static, 0.0f },
                    //{ { 7,0,7 },  YarnEnemyType::Static, 0.3f },
                    //{ { 10,0,10 }, YarnEnemyType::Static, 0.5f },

                        },
                        false
                    },

#if 0
                                    {
                // Wave 3
                {
                    { {14,0,21}, YarnEnemyType::MoveVertical, 1.0f },
                    { {16,0,21}, YarnEnemyType::MoveVertical, 1.0f },
                    { {4,0,0}, YarnEnemyType::MoveVertical, 1.5f },
                    { {6,0,0}, YarnEnemyType::MoveVertical, 1.5f },
                },
                false
            },

        #endif // 0

        };

#endif // 0
        break;
    case 5:
        data.waves =
        {

        };

        data.bossData.hasBoss = true;
        data.bossData.position = { 10.5f, 0.0f, 12.7f };
        break;
    }

    return data;
}