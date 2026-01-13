#pragma once

class IBeatReactive
{
public:
    virtual ~IBeatReactive() = default;

    // ”‚ª—ˆ‚½uŠÔ
    virtual void OnBeat(bool isStrong) {}

    // –ˆƒtƒŒ[ƒ€ŒÄ‚Î‚ê‚é (0 ~ 1)
    virtual void OnBeatPhase(float phase) {}
};