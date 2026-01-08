#ifndef COLLISION_HELPER_H
#define COLLISION_HELPER_H

#include <cmath>
#include <list>

enum class CollisionLayer :uint32_t
{
    None = 0,
    WorldStatic = 1 << 0,
    Player = 1 << 1,
    Enemy = 1 << 2,
    Oden = 1 << 3,      // おでん
    Convex = 1 << 4,

    WorldProps = 1 << 10, // stage の object
    Camera = 1 << 12,   // カメラ

    Everything = 0xFFFFFF,
};



namespace CollisionHelper
{
    // 単一レイヤーをビットに変換
    inline uint32_t ToBit(CollisionLayer layer)
    {
        uint32_t bit = (1u << static_cast<uint32_t>(layer));
        return bit;
    }

    // 複数レイヤーをまとめてマスクに変換
    inline uint32_t MakeMask(std::initializer_list<CollisionLayer> layers)
    {
        uint32_t mask = 0;
        for (auto layer : layers)
        {
            mask |= ToBit(layer);
        }
        return mask;
    }

    // マスクにレイヤーを追加
    inline void AddToMask(uint32_t& mask, CollisionLayer layer)
    {
        mask |= ToBit(layer);
    }

    // マスクからレイヤーを除去
    inline void RemoveFromMask(uint32_t& mask, CollisionLayer layer)
    {
        mask &= ~ToBit(layer);
    }

    // マスクに含まれているか確認
    inline bool HasLayer(uint32_t mask, CollisionLayer layer)
    {
        return (mask & ToBit(layer)) != 0;
    }
}



#endif //COLLISION_HELPER_H