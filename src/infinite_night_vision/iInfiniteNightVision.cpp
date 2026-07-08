#include "infinite_night_vision/iInfiniteNightVision.h"
#include <ll/api/memory/Hook.h>
#include <ll/api/mod/RegisterHelper.h>
#include <mc/client/renderer/game/LevelRendererCamera.h>
#include <mc/client/renderer/ptexture/BaseLightTextureImageBuilder.h>
#include <mc/deps/core/utility/ServiceLocator.h>
#include <mc/deps/minecraft_renderer/framebuilder/BlitFlipbookTextureDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/FadeToBlackDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/FrameBuilder.h>
#include <mc/deps/minecraft_renderer/framebuilder/FullscreenEffectDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/RenderCameraAimAssistHighlightDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/RenderFlameBillboardDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/RenderParticleDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/RenderShadowDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/gamecomponents/mfc/EditorHighlightConfiguration.h>

namespace mce::framebuilder {
struct RenderPlayerVisionDescription {
    bool  mNightVisionEnabled;
    float mNightVisionScale;
    float mMobEffectFogLevel;
    float mSkyAmbientContribution;
    float mDarknessScale;
};
} // namespace mce::framebuilder

namespace infinite_night_vision {

iInfiniteNightVision& iInfiniteNightVision::getInstance() {
    static iInfiniteNightVision instance;
    return instance;
}

bool iInfiniteNightVision::load() { return true; }

bool iInfiniteNightVision::enable() {
    ll::memory::HookRegistrar<SimpleHook, DeferredHook>::hook();
    return true;
}

bool iInfiniteNightVision::disable() {
    ll::memory::HookRegistrar<SimpleHook, DeferredHook>::unhook();
    return true;
}

bool iInfiniteNightVision::unload() { return true; }

LL_REGISTER_MOD(iInfiniteNightVision, iInfiniteNightVision::getInstance());

LL_STATIC_HOOK(
    iInfiniteNightVision::SimpleHook,
    HookPriority::Normal,
    ll::memory::unchecked(&BaseLightTextureImageBuilder::_updateDarknessLightData),
    void,
    BaseLightData&  baseLightData,
    Player const&   player,
    IOptions const& options
) {
    ll::memory::dAccess<bool>(&baseLightData, 36)  = true;
    ll::memory::dAccess<float>(&baseLightData, 40) = 1.0f;
    origin(baseLightData, player, options);
}

LL_TYPE_INSTANCE_HOOK(
    iInfiniteNightVision::DeferredHook,
    HookPriority::Normal,
    LevelRendererCamera,
    &LevelRendererCamera::renderPlayerVision,
    void,
    ScreenContext&
) {
    using namespace ll::memory_literals;
    // clang-format off
    auto frameBuilderRef = reinterpret_cast<ServiceReference<mce::framebuilder::FrameBuilder> (*)()>(
        "48 89 5C 24 10 57 48 83 EC 20 48 8B D9 48 8D 3D 14 BB 2F 0A"_sig.resolve()
    )();
    if (!frameBuilderRef.mService.mControlBlock->mIsValid) return;
    if (!ll::memory::virtualCall<bool>(frameBuilderRef.mService.mPointer, 1)) return;
    // clang-format on

    mce::framebuilder::RenderPlayerVisionDescription desc{
        .mNightVisionEnabled     = true,
        .mNightVisionScale       = 1.0f,
        .mMobEffectFogLevel      = 1.0f,
        .mSkyAmbientContribution = 1.0f,
        .mDarknessScale          = 0.0f
    };

    ll::memory::virtualCall<
        void,
        std::variant<
            std::reference_wrapper<mce::framebuilder::RenderFlameBillboardDescription const>,
            std::reference_wrapper<mce::framebuilder::BlitFlipbookTextureDescription const>,
            std::reference_wrapper<mce::framebuilder::RenderParticleDescription const>,
            std::reference_wrapper<mce::framebuilder::RenderPlayerVisionDescription const>,
            std::reference_wrapper<mce::framebuilder::RenderShadowDescription const>,
            std::reference_wrapper<mce::framebuilder::FadeToBlackDescription const>,
            std::reference_wrapper<mce::framebuilder::RenderCameraAimAssistHighlightDescription const>,
            std::reference_wrapper<mce::framebuilder::FullscreenEffectDescription const>,
            std::reference_wrapper<MFC::EditorHighlightConfiguration const>>>(
        frameBuilderRef.mService.mPointer,
        99,
        desc
    );
}

} // namespace infinite_night_vision
