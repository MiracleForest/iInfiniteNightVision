#pragma once
#include <ll/api/mod/NativeMod.h>

namespace infinite_night_vision {

class iInfiniteNightVision {
public:
    struct SimpleHook;
    struct DeferredHook;
    struct UnderwaterHook;

public:
    static iInfiniteNightVision& getInstance();

    iInfiniteNightVision() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    bool load();
    bool enable();
    bool disable();
    bool unload();

private:
    ll::mod::NativeMod& mSelf;
};

} // namespace infinite_night_vision
