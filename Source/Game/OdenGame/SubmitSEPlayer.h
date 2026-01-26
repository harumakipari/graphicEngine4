#pragma once
#include "Engine/Audio/CoreAudio.h"


class SubmitSEPlayer
{
public:
    enum class ESubmitSE
    {
        Twinkle,
        TwinkleLong,
    };

    static void Play()
    {
        ESubmitSE next = DecideNextSE();
        PlaySE(next);

        if (next == lastSE)
        {
            sameCount++;
        }
        else
        {
            sameCount = 1;
            lastSE = next;
        }
    }

private:
    static inline ESubmitSE lastSE = ESubmitSE::Twinkle;
    static inline int sameCount = 0;

    static ESubmitSE DecideNextSE()
    {
        // 3âÒë±Ç¢ÇΩÇÁïKÇ∏à·Ç§SE
        if (sameCount >= 3)
        {
            return GetDifferentSE(lastSE);
        }

        // í èÌÇÕÉâÉìÉ_ÉÄ
        return (rand() % 2 == 0)
            ? ESubmitSE::Twinkle
            : ESubmitSE::TwinkleLong;
    }

    static ESubmitSE GetDifferentSE(ESubmitSE se)
    {
        return (se == ESubmitSE::Twinkle)
            ? ESubmitSE::TwinkleLong
            : ESubmitSE::Twinkle;
    }

    static void PlaySE(ESubmitSE se)
    {
        switch (se)
        {
        case ESubmitSE::Twinkle:
            CoreAudio::PlayOneShot(L"./Data/Sound/SE/se_twinkle.wav");
            break;
        case ESubmitSE::TwinkleLong:
            CoreAudio::PlayOneShot(L"./Data/Sound/SE/se_twinkle_tree.wav");
            break;
        }
    }
};
