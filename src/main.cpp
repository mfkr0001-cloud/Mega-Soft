#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/GameStatsManager.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>

#include "ui_theme.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace geode::prelude;

namespace {
    constexpr int kVETERButtonTag = 0x4D534831;
    constexpr int kVETERToggleOnTag = 0x564F4E31;
    constexpr int kVETERToggleOffTag = 0x564F4646;
    constexpr std::size_t kMaxMacroEvents = 500000;
    constexpr double kMaxMacroDurationSeconds = 24.0 * 60.0 * 60.0;

    // Keep the image switch synchronized whenever existing callbacks update a label.
    void setVETERControlLabel(CCLabelBMFont* label, const std::string& value) {
        if (!label) return;
        label->setString(value.c_str());
        auto* parent = label->getParent();
        if (!parent) return;
        const std::string& text = value;
        const bool isOn = text.size() >= 3 && text.ends_with(" ON");
        const bool isOff = text.size() >= 4 && text.ends_with(" OFF");
        if (auto* toggleOn = parent->getChildByTag(kVETERToggleOnTag)) toggleOn->setVisible(isOn);
        if (auto* toggleOff = parent->getChildByTag(kVETERToggleOffTag)) toggleOff->setVisible(isOff);
    }

    // User-adjustable UI preferences. They are saved across launches.
    float uiButtonScale() {
        return std::clamp(Mod::get()->getSavedValue<float>("ui-button-scale", 1.0f), 0.70f, 1.0f);
    }
    void setUIButtonScale(float value) {
        Mod::get()->setSavedValue("ui-button-scale", std::clamp(value, 0.70f, 1.0f));
    }
    float uiMenuScale() {
        return std::clamp(Mod::get()->getSavedValue<float>("ui-menu-scale", 1.0f), 0.75f, 1.0f);
    }
    void setUIMenuScale(float value) {
        Mod::get()->setSavedValue("ui-menu-scale", std::clamp(value, 0.75f, 1.0f));
    }
    int uiButtonOpacity() {
        return std::clamp(Mod::get()->getSavedValue<int>("ui-button-opacity", 100), 50, 100);
    }
    void setUIButtonOpacity(int value) {
        Mod::get()->setSavedValue("ui-button-opacity", std::clamp(value, 50, 100));
    }
    int percent(float value) {
        return static_cast<int>(std::lround(value * 100.f));
    }
    float nextScale(float current, const std::vector<float>& values) {
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (std::fabs(current - values[i]) < 0.025f) return values[(i + 1) % values.size()];
        }
        return values.front();
    }
    int nextOpacity(int current) {
        constexpr int values[] = {50, 65, 80, 90, 100};
        for (std::size_t i = 0; i < std::size(values); ++i) {
            if (current == values[i]) return values[(i + 1) % std::size(values)];
        }
        return 50;
    }

    bool autoRetryEnabled() {
        return Mod::get()->getSavedValue<bool>("auto-retry-enabled", false);
    }
    void setAutoRetryEnabled(bool enabled) {
        Mod::get()->setSavedValue("auto-retry-enabled", enabled);
    }
    bool fpsBypassEnabled() {
        return Mod::get()->getSavedValue<bool>("fps-bypass-enabled", false);
    }
    void setFPSBypassEnabled(bool enabled) {
        Mod::get()->setSavedValue("fps-bypass-enabled", enabled);
    }
    int targetFPS() {
        int value = Mod::get()->getSavedValue<int>("fps-target", 120);
        return std::clamp(value, 60, 360);
    }
    void setTargetFPS(int value) {
        value = std::clamp(value, 60, 360);
        value = ((value + 15) / 30) * 30;
        Mod::get()->setSavedValue("fps-target", std::clamp(value, 60, 360));
    }
    bool fpsCounterEnabled() {
        return Mod::get()->getSavedValue<bool>("fps-counter-enabled", false);
    }
    void setFPSCounterEnabled(bool enabled) {
        Mod::get()->setSavedValue("fps-counter-enabled", enabled);
    }
    bool hitboxesEnabled() {
        return Mod::get()->getSavedValue<bool>("hitboxes-enabled", false);
    }
    void setHitboxesEnabled(bool enabled) {
        Mod::get()->setSavedValue("hitboxes-enabled", enabled);
    }
    bool cpsEnabled() {
        return Mod::get()->getSavedValue<bool>("cps-enabled", false);
    }
    void setCPSEnabled(bool enabled) {
        Mod::get()->setSavedValue("cps-enabled", enabled);
    }
    bool practiceMusicEnabled() {
        return Mod::get()->getSavedValue<bool>("practice-music-enabled", false);
    }
    void setPracticeMusicEnabled(bool enabled) {
        Mod::get()->setSavedValue("practice-music-enabled", enabled);
    }

    bool frameStepEnabled() {
        return Mod::get()->getSavedValue<bool>("frame-step-enabled", false);
    }
    void setFrameStepEnabled(bool enabled) {
        Mod::get()->setSavedValue("frame-step-enabled", enabled);
    }
    bool noDeathEffectEnabled() {
        return Mod::get()->getSavedValue<bool>("no-death-effect-enabled", false);
    }
    void setNoDeathEffectEnabled(bool enabled) {
        Mod::get()->setSavedValue("no-death-effect-enabled", enabled);
    }
    bool hitboxesOnDeathEnabled() {
        return Mod::get()->getSavedValue<bool>("hitboxes-on-death-enabled", false);
    }
    void setHitboxesOnDeathEnabled(bool enabled) {
        Mod::get()->setSavedValue("hitboxes-on-death-enabled", enabled);
    }
    bool confirmExitEnabled() {
        return Mod::get()->getSavedValue<bool>("confirm-exit-enabled", false);
    }
    void setConfirmExitEnabled(bool enabled) {
        Mod::get()->setSavedValue("confirm-exit-enabled", enabled);
    }
    bool noclipLimitEnabled() {
        return Mod::get()->getSavedValue<bool>("noclip-limit-enabled", false);
    }
    void setNoclipLimitEnabled(bool enabled) {
        Mod::get()->setSavedValue("noclip-limit-enabled", enabled);
    }
    int noclipMaxDeaths() {
        return std::clamp(Mod::get()->getSavedValue<int>("noclip-max-deaths", 5), 1, 50);
    }
    void setNoclipMaxDeaths(int value) {
        Mod::get()->setSavedValue("noclip-max-deaths", std::clamp(value, 1, 50));
    }
    bool unlockAllEnabled() {
        return Mod::get()->getSavedValue<bool>("unlock-all-enabled", false);
    }
    void setUnlockAllEnabled(bool enabled) {
        Mod::get()->setSavedValue("unlock-all-enabled", enabled);
    }
    bool shopUnlockBypassEnabled() {
        return Mod::get()->getSavedValue<bool>("shop-unlock-bypass-enabled", false);
    }
    void setShopUnlockBypassEnabled(bool enabled) {
        Mod::get()->setSavedValue("shop-unlock-bypass-enabled", enabled);
    }
    bool vaultBypassEnabled() {
        return Mod::get()->getSavedValue<bool>("vault-bypass-enabled", false);
    }
    void setVaultBypassEnabled(bool enabled) {
        Mod::get()->setSavedValue("vault-bypass-enabled", enabled);
    }
    bool iconKitBypassEnabled() {
        return Mod::get()->getSavedValue<bool>("icon-kit-bypass-enabled", false);
    }
    void setIconKitBypassEnabled(bool enabled) {
        Mod::get()->setSavedValue("icon-kit-bypass-enabled", enabled);
    }
    bool mainLevelsBypassEnabled() {
        return Mod::get()->getSavedValue<bool>("main-levels-bypass-enabled", false);
    }
    void setMainLevelsBypassEnabled(bool enabled) {
        Mod::get()->setSavedValue("main-levels-bypass-enabled", enabled);
    }
    bool textLengthBypassEnabled() {
        return Mod::get()->getSavedValue<bool>("text-length-bypass-enabled", false);
    }
    void setTextLengthBypassEnabled(bool enabled) {
        Mod::get()->setSavedValue("text-length-bypass-enabled", enabled);
    }
    bool trajectoryEnabled() {
        return Mod::get()->getSavedValue<bool>("trajectory-enabled", false);
    }
    void setTrajectoryEnabled(bool enabled) {
        Mod::get()->setSavedValue("trajectory-enabled", enabled);
    }
    bool trajectoryPredictorEnabled() {
        return Mod::get()->getSavedValue<bool>("trajectory-predictor-enabled", false);
    }
    void setTrajectoryPredictorEnabled(bool enabled) {
        Mod::get()->setSavedValue("trajectory-predictor-enabled", enabled);
    }
    bool showClicksEnabled() {
        return Mod::get()->getSavedValue<bool>("show-clicks-enabled", false);
    }
    void setShowClicksEnabled(bool enabled) {
        Mod::get()->setSavedValue("show-clicks-enabled", enabled);
    }
    bool customMenuMusicEnabled() {
        return Mod::get()->getSavedValue<bool>("custom-menu-music-enabled", false);
    }
    void setCustomMenuMusicEnabled(bool enabled) {
        Mod::get()->setSavedValue("custom-menu-music-enabled", enabled);
    }
    bool transparentPauseEnabled() {
        return Mod::get()->getSavedValue<bool>("transparent-pause-enabled", false);
    }
    void setTransparentPauseEnabled(bool enabled) {
        Mod::get()->setSavedValue("transparent-pause-enabled", enabled);
    }
    bool hidePauseButtonEnabled() {
        return Mod::get()->getSavedValue<bool>("hide-pause-button-enabled", false);
    }
    void setHidePauseButtonEnabled(bool enabled) {
        Mod::get()->setSavedValue("hide-pause-button-enabled", enabled);
    }
    bool layoutModeEnabled() {
        return Mod::get()->getSavedValue<bool>("layout-mode-enabled", false);
    }
    void setLayoutModeEnabled(bool enabled) {
        Mod::get()->setSavedValue("layout-mode-enabled", enabled);
    }

    double g_originalAnimationInterval = 1.0 / 60.0;
    PlayLayer* g_debugDrawLayer = nullptr;
    PlayLayer* g_practiceMusicLayer = nullptr;
    bool g_autoRetryPending = false;
    int g_cpsInputCount = 0;
    int g_currentCPS = 0;
    double g_cpsWindowElapsed = 0.0;
    PlayLayer* g_fpsCounterLayer = nullptr;
    CCLabelBMFont* g_fpsCounterLabel = nullptr;
    std::chrono::steady_clock::time_point g_fpsWindowStart{};
    int g_fpsWindowFrames = 0;
    double g_measuredGameFPS = 0.0;
    bool g_fpsWindowStarted = false;
    PlayLayer* g_cpsLayer = nullptr;
    CCLabelBMFont* g_cpsLabel = nullptr;
    PlayLayer* g_noclipStatusLayer = nullptr;
    CCLabelBMFont* g_noclipStatusLabel = nullptr;
    int g_noclipDeathCount = 0;
    int g_noclipMaxProgress = 0;
    bool g_noclipDeathDrawActive = false;
    bool g_frameStepPending = false;
    bool g_confirmExitProceed = false;
    int g_checkpointIndex = -1;

    constexpr int kTrajectoryDrawTag = 0x4D535833;
    constexpr int kClickLabelTag = 0x4D535834;
    PlayLayer* g_trajectoryLayer = nullptr;
    CCDrawNode* g_trajectoryDrawNode = nullptr;
    std::vector<CCPoint> g_trajectoryPoints;

    void clearTrajectoryTrail() {
        // The old PlayLayer can already be destroyed during scene changes. Only
        // dereference the node if it is still attached to the currently live layer.
        if (g_trajectoryDrawNode && g_trajectoryLayer && PlayLayer::get() == g_trajectoryLayer) {
            g_trajectoryDrawNode->removeFromParentAndCleanup(true);
        }
        g_trajectoryDrawNode = nullptr;
        g_trajectoryLayer = nullptr;
        g_trajectoryPoints.clear();
    }

    PlayLayer* g_pauseButtonLayer = nullptr;
    CCNode* g_pauseButtonNode = nullptr;
    PlayLayer* g_clickLayer = nullptr;
    CCLabelBMFont* g_clickLabel = nullptr;
    float g_clickFlashRemaining = 0.f;
    std::string g_customMenuMusicPath;
    bool g_customMenuMusicApplied = false;
    // Remembers that custom music replaced the menu track, even while a level is active.
    // This lets us restore menuLoop.mp3 if the toggle is turned off during gameplay.
    bool g_customMenuMusicNeedsRestore = false;
    // Track the game's requested pitch separately from our speed multiplier.
    // This prevents a speed change from erasing pitch changes requested by the level.
    std::unordered_map<int, float> g_musicBasePitchByChannel;
    std::unordered_map<int, float> g_musicAppliedPitchByChannel;
    bool g_settingMusicPitch = false;

    void applyFPSBypass() {
        auto* director = CCDirector::sharedDirector();
        if (!director) return;
        const double desired = fpsBypassEnabled()
            ? 1.0 / static_cast<double>(targetFPS())
            : (g_originalAnimationInterval > 0.0 ? g_originalAnimationInterval : 1.0 / 60.0);
        // Avoid resetting the director's frame timer on every drawScene call.
        if (std::abs(director->getAnimationInterval() - desired) > 0.00001) {
            director->setAnimationInterval(desired);
        }
    }

    void applyHitboxes(PlayLayer* layer) {
        if (!layer) return;
        // toggleDebugDraw() is a toggle, not a setter. Compare the actual game
        // state first so menu callbacks and new PlayLayer instances cannot invert it.
        if (layer->shouldDebugDraw() != hitboxesEnabled()) {
            layer->toggleDebugDraw();
        }
        g_debugDrawLayer = layer;
    }

    void updateCPSOverlay(PlayLayer* layer, float dt) {
        if (!layer) return;
        if (g_cpsLayer != layer) {
            // The previous level owns the previous label; never touch that pointer
            // after Geometry Dash replaces the PlayLayer.
            g_cpsLayer = layer;
            g_cpsLabel = nullptr;
            g_cpsWindowElapsed = 0.0;
            g_cpsInputCount = 0;
            g_currentCPS = 0;
        }
        if (!cpsEnabled()) {
            if (g_cpsLabel) g_cpsLabel->setVisible(false);
            return;
        }
        if (g_cpsLayer != layer || !g_cpsLabel) {
            g_cpsLayer = layer;
            g_cpsLabel = CCLabelBMFont::create("CPS 0", "chatFont.fnt");
            if (g_cpsLabel) {
                const auto size = CCDirector::sharedDirector()->getWinSize();
                g_cpsLabel->setAnchorPoint({0.f, 1.f});
                g_cpsLabel->setPosition({12.f, size.height - 12.f});
                g_cpsLabel->setScale(0.55f);
                layer->addChild(g_cpsLabel, 9998);
            }
        }
        if (g_cpsLabel) {
            g_cpsLabel->setVisible(true);
            char label[32];
            std::snprintf(label, sizeof(label), "CPS %d", g_currentCPS);
            g_cpsLabel->setString(label);
        }
        g_cpsWindowElapsed += std::max(0.0, static_cast<double>(dt));
        if (g_cpsWindowElapsed >= 1.0) {
            g_currentCPS = g_cpsInputCount;
            g_cpsInputCount = 0;
            g_cpsWindowElapsed = 0.0;
        }
    }

    void updateFPSCounterOverlay(PlayLayer* layer) {
        if (!layer) return;
        if (g_fpsCounterLayer != layer) {
            // Do not touch a label owned by a PlayLayer that may already be destroyed.
            g_fpsCounterLayer = layer;
            g_fpsCounterLabel = nullptr;
            g_fpsWindowFrames = 0;
            g_measuredGameFPS = 0.0;
            g_fpsWindowStarted = false;
        }
        if (!fpsCounterEnabled()) {
            if (g_fpsCounterLabel) g_fpsCounterLabel->setVisible(false);
            return;
        }
        if (!g_fpsCounterLabel) {
            g_fpsCounterLabel = CCLabelBMFont::create("GAME FPS 0", "chatFont.fnt");
            if (g_fpsCounterLabel) {
                const auto size = CCDirector::sharedDirector()->getWinSize();
                g_fpsCounterLabel->setAnchorPoint({0.f, 1.f});
                g_fpsCounterLabel->setPosition({12.f, size.height - 56.f});
                g_fpsCounterLabel->setScale(0.46f);
                layer->addChild(g_fpsCounterLabel, 9998);
            }
        }

        const auto now = std::chrono::steady_clock::now();
        if (!g_fpsWindowStarted) {
            g_fpsWindowStart = now;
            g_fpsWindowFrames = 0;
            g_fpsWindowStarted = true;
        }
        ++g_fpsWindowFrames;
        const double elapsed = std::chrono::duration<double>(now - g_fpsWindowStart).count();
        if (elapsed >= 0.5) {
            g_measuredGameFPS = elapsed > 0.0 ? static_cast<double>(g_fpsWindowFrames) / elapsed : 0.0;
            g_fpsWindowFrames = 0;
            g_fpsWindowStart = now;
        }
        if (g_fpsCounterLabel) {
            char text[40];
            std::snprintf(text, sizeof(text), "GAME FPS %.0f", g_measuredGameFPS);
            g_fpsCounterLabel->setString(text);
            g_fpsCounterLabel->setVisible(true);
        }
    }

    bool noclipEnabled();
    bool noclipLimitEnabled();
    int noclipMaxDeaths();

    void updateNoclipStatusOverlay(PlayLayer* layer) {
        if (!layer) return;
        if (g_noclipStatusLayer != layer) {
            g_noclipStatusLayer = layer;
            g_noclipStatusLabel = nullptr;
            g_noclipDeathCount = 0;
            g_noclipMaxProgress = 0;
        }
        const bool visible = noclipEnabled() || noclipLimitEnabled();
        if (!visible) {
            if (g_noclipStatusLabel) g_noclipStatusLabel->setVisible(false);
            return;
        }
        if (!g_noclipStatusLabel) {
            g_noclipStatusLabel = CCLabelBMFont::create("NC 0% | HITS 0", "chatFont.fnt");
            if (g_noclipStatusLabel) {
                const auto size = CCDirector::sharedDirector()->getWinSize();
                g_noclipStatusLabel->setAnchorPoint({0.f, 1.f});
                g_noclipStatusLabel->setPosition({12.f, size.height - 34.f});
                g_noclipStatusLabel->setScale(0.48f);
                layer->addChild(g_noclipStatusLabel, 9998);
            }
        }
        const int progress = std::clamp(layer->getCurrentPercentInt(), 0, 100);
        g_noclipMaxProgress = std::max(g_noclipMaxProgress, progress);
        if (g_noclipStatusLabel) {
            char label[80];
            if (noclipLimitEnabled()) {
                std::snprintf(label, sizeof(label), "NC %d%% | HITS %d/%d", g_noclipMaxProgress,
                              g_noclipDeathCount, noclipMaxDeaths());
            } else {
                std::snprintf(label, sizeof(label), "NC %d%% | HITS %d", g_noclipMaxProgress,
                              g_noclipDeathCount);
            }
            g_noclipStatusLabel->setString(label);
            g_noclipStatusLabel->setVisible(true);
        }
    }

    // Screen-space reference grid for Layout Mode. It is attached to the active
    // scene (rather than PlayLayer) so camera scrolling does not move the grid.
    constexpr int kLayoutGridTag = 0x4D534832;
    void updateLayoutGrid(CCScene* scene) {
        if (!scene) return;
        auto* existing = scene->getChildByTag(kLayoutGridTag);
        if (!layoutModeEnabled() || !PlayLayer::get()) {
            if (existing) existing->removeFromParentAndCleanup(true);
            return;
        }
        if (existing) return;
        auto* grid = CCDrawNode::create();
        if (!grid) return;
        grid->setTag(kLayoutGridTag);
        const auto win = CCDirector::sharedDirector()->getWinSize();
        const ccColor4F gridColor = {0.12f, 0.82f, 1.0f, 0.24f};
        constexpr float spacing = 32.f;
        for (float x = 0.f; x <= win.width; x += spacing) {
            grid->drawSegment({x, 0.f}, {x, win.height}, 0.55f, gridColor);
        }
        for (float y = 0.f; y <= win.height; y += spacing) {
            grid->drawSegment({0.f, y}, {win.width, y}, 0.55f, gridColor);
        }
        scene->addChild(grid, 10000);
    }

    CCNode* findPauseButtonNode(CCNode* root) {
        if (!root) return nullptr;
        auto* cache = CCSpriteFrameCache::sharedSpriteFrameCache();
        auto* pauseFrame = cache ? cache->spriteFrameByName("GJ_pauseBtn_001.png") : nullptr;
        if (pauseFrame) {
            if (auto* sprite = typeinfo_cast<CCSprite*>(root)) {
                if (sprite->displayFrame() == pauseFrame) {
                    CCNode* candidate = sprite;
                    while (candidate && candidate->getParent() && !typeinfo_cast<CCMenuItem*>(candidate)) {
                        candidate = candidate->getParent();
                    }
        
