#include "infinite_night_vision/iInfiniteNightVision.h"
#include <ll/api/memory/Hook.h>
#include <ll/api/mod/RegisterHelper.h>
#include <mc/client/renderer/game/LevelRendererCamera.h>
#include <mc/client/renderer/ptexture/BaseLightTextureImageBuilder.h>
#include <mc/deps/core/utility/ServiceLocator.h>
#include <mc/deps/minecraft_renderer/framebuilder/BlitFlipbookTextureDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/EditorHighlightConfiguration.h>
#include <mc/deps/minecraft_renderer/framebuilder/FadeToBlackDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/FrameBuilder.h>
#include <mc/deps/minecraft_renderer/framebuilder/FullscreenEffectDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/RenderCameraAimAssistHighlightDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/RenderFlameBillboardDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/RenderParticleDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/RenderShadowDescription.h>

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
    ll::memory::HookRegistrar<SimpleHook, DeferredHook, UnderwaterHook>::hook();
    return true;
}

bool iInfiniteNightVision::disable() {
    ll::memory::HookRegistrar<SimpleHook, DeferredHook, UnderwaterHook>::unhook();
    return true;
}

bool iInfiniteNightVision::unload() { return true; }

LL_REGISTER_MOD(iInfiniteNightVision, iInfiniteNightVision::getInstance());

LL_TYPE_INSTANCE_HOOK(
    iInfiniteNightVision::SimpleHook,
    HookPriority::Normal,
    BaseLightTextureImageBuilder,
    &BaseLightTextureImageBuilder::refreshData,
    bool,
    IClientInstance* client,
    BaseLightData&   lightData
) {
    auto result                                = origin(client, lightData);
    ll::memory::dAccess<bool>(&lightData, 36)  = true;
    ll::memory::dAccess<float>(&lightData, 40) = 1.0f;
    ll::memory::dAccess<bool>(&lightData, 44)  = true;
    ll::memory::dAccess<float>(&lightData, 48) = 1.0f;
    return result;
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
    auto frameBuilderRef = *reinterpret_cast<Bedrock::NonOwnerPointer<mce::framebuilder::FrameBuilder>*>(
        reinterpret_cast<uintptr_t>(ll::sys_utils::getImageRange().data()) + 0xDCE1DD8
    );
    if (!frameBuilderRef.mControlBlock->mIsValid) return;
    if (!ll::memory::virtualCall<bool>(frameBuilderRef.mPointer, 1)) return;
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
            std::reference_wrapper<mce::framebuilder::gamecomponents::EditorHighlightConfiguration const>>>(
        frameBuilderRef.mPointer,
        99,
        desc
    );
}

LL_TYPE_INSTANCE_HOOK(
    iInfiniteNightVision::UnderwaterHook,
    HookPriority::Normal,
    LevelRendererCamera,
    &LevelRendererCamera::determineUnderwaterStatus,
    void,
    BlockSource& region
) {
    origin(region);
    mCameraUnderPowderSnow = false;
    mCameraUnderWater      = false;
    mCameraUnderLava       = false;
    mCameraUnderLiquid     = false;
}

} // namespace infinite_night_vision