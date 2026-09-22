#include "infinite_night_vision/iInfiniteNightVision.h"
#include <ll/api/memory/Hook.h>
#include <ll/api/mod/RegisterHelper.h>
#include <mc/client/renderer/game/LevelRendererCamera.h>
#include <mc/client/renderer/ptexture/BaseLightData.h>
#include <mc/client/renderer/ptexture/BaseLightTextureImageBuilder.h>
#include <mc/client/world/level/dimension/NetherLightTextureImageBuilder.h>
#include <mc/deps/core/utility/ServiceLocator.h>
#include <mc/deps/minecraft_renderer/framebuilder/BlitFlipbookTextureDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/EditorHighlightConfiguration.h>
#include <mc/deps/minecraft_renderer/framebuilder/FadeToBlackDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/FrameBuilder.h>
#include <mc/deps/minecraft_renderer/framebuilder/FullscreenEffectDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/RenderCameraAimAssistHighlightDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/RenderFlameBillboardDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/RenderParticleDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/RenderPlayerVisionDescription.h>
#include <mc/deps/minecraft_renderer/framebuilder/RenderShadowDescription.h>

namespace infinite_night_vision {

iInfiniteNightVision& iInfiniteNightVision::getInstance() {
    static iInfiniteNightVision instance;
    return instance;
}

bool iInfiniteNightVision::load() { return true; }

bool iInfiniteNightVision::enable() {
    ll::memory::HookRegistrar<SimpleHook1, SimpleHook2, DeferredHook, UnderwaterHook>::hook();
    return true;
}

bool iInfiniteNightVision::disable() {
    ll::memory::HookRegistrar<SimpleHook1, SimpleHook2, DeferredHook, UnderwaterHook>::unhook();
    return true;
}

bool iInfiniteNightVision::unload() { return true; }

LL_REGISTER_MOD(iInfiniteNightVision, iInfiniteNightVision::getInstance());

using namespace ll::memory_literals;

LL_TYPE_INSTANCE_HOOK(
    iInfiniteNightVision::SimpleHook1,
    HookPriority::Normal,
    BaseLightTextureImageBuilder,
    &BaseLightTextureImageBuilder::$createBaseLightTextureData,
    std::unique_ptr<BaseLightData>,
    IClientInstance*     client,
    BaseLightData const& currentData
) {
    auto result                = origin(client, currentData);
    result->mNightvisionActive = true;
    result->mNightvisionScale  = 1.0f;
    result->mUnderwaterVision  = true;
    result->mUnderwaterScale   = 1.0f;
    return result;
}

LL_TYPE_INSTANCE_HOOK(
    iInfiniteNightVision::SimpleHook2,
    HookPriority::Normal,
    NetherLightTextureImageBuilder,
    &NetherLightTextureImageBuilder::$createBaseLightTextureData,
    std::unique_ptr<BaseLightData>,
    IClientInstance*     client,
    BaseLightData const& currentData
) {
    auto result                = origin(client, currentData);
    result->mNightvisionActive = true;
    result->mNightvisionScale  = 1.0f;
    result->mUnderwaterVision  = true;
    result->mUnderwaterScale   = 1.0f;
    return result;
}

LL_INSTANCE_HOOK(
    iInfiniteNightVision::DeferredHook,
    HookPriority::Normal,
    // "std::_Func_impl_no_alloc<`lambda at D:\\a\\_work\\1\\s\\handheld\\src-client\\common\\client\\renderer\\game\\LevelRendererCamera.cpp:4124:33',void,CommandListTaskContext &>::_Do_call"_sym,
    "55 41 57 41 56 41 55 41 54 56 57 53 48 81 EC ?? ?? ?? ?? 48 8D AC 24 ?? ?? ?? ?? 44 0F 29 95 ?? ?? ?? ?? 44 0F 29 8D ?? ?? ?? ?? 44 0F 29 85 ?? ?? ?? ?? 0F 29 BD ?? ?? ?? ?? 0F 29 B5 ?? ?? ?? ?? 48 C7 85 ?? ?? ?? ?? ?? ?? ?? ?? 4C 8B 69 ?? 48 8D 1D"_sig,
    void
) {
    // clang-format off
    auto frameBuilderRef = reinterpret_cast<Bedrock::NonOwnerPointer<mce::framebuilder::FrameBuilder>& (*)()>(
        "?mService@?$ServiceLocator@VFrameBuilder@framebuilder@mce@@@@SAAEAV?$NonOwnerPointer@VFrameBuilder@framebuilder@mce@@@Bedrock@@XZ"_sym.resolve()
    )();
    if (!frameBuilderRef.mControlBlock || !frameBuilderRef.mControlBlock->mIsValid) return;
    if (!frameBuilderRef.mPointer->enabled()) return;
    // clang-format on

    mce::framebuilder::RenderPlayerVisionDescription desc{
        .mNightVisionEnabled     = true,
        .mNightVisionScale       = 1.0f,
        .mMobEffectFogLevel      = 1.0f,
        .mSkyAmbientContribution = 1.0f,
        .mDarknessScale          = 0.0f
    };

    frameBuilderRef.mPointer->_insert(desc);
}

LL_TYPE_INSTANCE_HOOK(
    iInfiniteNightVision::UnderwaterHook,
    HookPriority::Normal,
    LevelRendererCamera,
    &LevelRendererCamera::$preRenderUpdate,
    void,
    ScreenContext&                        screenContext,
    LevelRenderPreRenderUpdateParameters& levelRenderPreRenderUpdateParameters
) {
    origin(screenContext, levelRenderPreRenderUpdateParameters);
    mCameraUnderPowderSnow = false;
    mCameraUnderWater      = false;
    mCameraUnderLava       = false;
    mCameraUnderLiquid     = false;
}

} // namespace infinite_night_vision