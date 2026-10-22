include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/GameStatsManager.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>

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
    constexpr std::size_t kMaxMacroEvents = 500000;
    constexpr double kMaxMacroDurationSeconds = 24.0 * 60.0 * 60.0;

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
        if (fpsBypassEnabled()) {
            director->setAnimationInterval(1.0 / static_cast<double>(targetFPS()));
        } else {
            director->setAnimationInterval(g_originalAnimationInterval > 0.0
                ? g_originalAnimationInterval : 1.0 / 60.0);
        }
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
                    if (candidate && typeinfo_cast<CCMenuItem*>(candidate)) return candidate;
                }
            }
        }
        if (auto* children = root->getChildren()) {
            for (unsigned int i = 0; i < children->count(); ++i) {
                if (auto* found = findPauseButtonNode(static_cast<CCNode*>(children->objectAtIndex(i)))) return found;
            }
        }
        return nullptr;
    }

    void updatePauseButtonVisibility(PlayLayer* layer) {
        if (!layer) return;
        if (g_pauseButtonLayer != layer) {
            g_pauseButtonLayer = layer;
            g_pauseButtonNode = nullptr;
        }
        // Only search the scene until the pause item is found. The old version
        // recursively traversed every child node on every frame.
        if (!g_pauseButtonNode) g_pauseButtonNode = findPauseButtonNode(layer);
        if (g_pauseButtonNode) g_pauseButtonNode->setVisible(!hidePauseButtonEnabled());
    }

    void updateTrajectoryTrail(PlayLayer* layer) {
        if (!layer) return;
        const bool showTrail = trajectoryEnabled();
        const bool showPrediction = trajectoryPredictorEnabled();
        if (!showTrail && !showPrediction) {
            if (g_trajectoryDrawNode) g_trajectoryDrawNode->removeFromParentAndCleanup(true);
            g_trajectoryDrawNode = nullptr;
            g_trajectoryLayer = nullptr;
            g_trajectoryPoints.clear();
            return;
        }
        if (g_trajectoryLayer != layer) {
            g_trajectoryLayer = layer;
            g_trajectoryPoints.clear();
            g_trajectoryDrawNode = CCDrawNode::create();
            if (g_trajectoryDrawNode) {
                g_trajectoryDrawNode->setTag(kTrajectoryDrawTag);
                layer->addChild(g_trajectoryDrawNode, 9996);
            }
        }
        if (!g_trajectoryDrawNode || !layer->m_player1) return;

        const CCPoint current = layer->m_player1->getPosition();
        // Discard history after a teleport, restart, or large camera/physics discontinuity.
        if (!g_trajectoryPoints.empty()) {
            const auto previous = g_trajectoryPoints.back();
            if (std::hypot(current.x - previous.x, current.y - previous.y) > 180.f) {
                g_trajectoryPoints.clear();
            }
        }
        g_trajectoryPoints.push_back(current);
        if (g_trajectoryPoints.size() > 72) g_trajectoryPoints.erase(g_trajectoryPoints.begin());
        g_trajectoryDrawNode->clear();

        if (showTrail && g_trajectoryPoints.size() >= 2) {
            const ccColor4F trailColor = {0.10f, 0.88f, 1.0f, 0.78f};
            for (std::size_t i = 1; i < g_trajectoryPoints.size(); ++i) {
                const float fade = static_cast<float>(i) / static_cast<float>(g_trajectoryPoints.size());
                const ccColor4F color = {trailColor.r, trailColor.g, trailColor.b, trailColor.a * fade};
                g_trajectoryDrawNode->drawSegment(g_trajectoryPoints[i - 1], g_trajectoryPoints[i], 2.1f, color);
            }
        }

        // This is an extrapolation of recent movement, not a full physics simulation:
        // triggers, collisions, gravity changes, portals, and mode switches can invalidate it.
        if (showPrediction && g_trajectoryPoints.size() >= 3) {
            const std::size_t count = g_trajectoryPoints.size();
            const CCPoint p0 = g_trajectoryPoints[count - 3];
            const CCPoint p1 = g_trajectoryPoints[count - 2];
            const CCPoint p2 = g_trajectoryPoints[count - 1];
            float vx = std::clamp(p2.x - p1.x, -80.f, 80.f);
            float vy = std::clamp(p2.y - p1.y, -80.f, 80.f);
            const float ax = std::clamp((p2.x - p1.x) - (p1.x - p0.x), -8.f, 8.f);
            const float ay = std::clamp((p2.y - p1.y) - (p1.y - p0.y), -8.f, 8.f);
            CCPoint previous = p2;
            const ccColor4F predictionColor = {1.0f, 0.48f, 0.12f, 0.78f};
            for (int step = 1; step <= 14; ++step) {
                vx = std::clamp(vx + ax * 0.35f, -80.f, 80.f);
                vy = std::clamp(vy + ay * 0.35f, -80.f, 80.f);
                CCPoint next = {previous.x + vx, previous.y + vy};
                if (!std::isfinite(next.x) || !std::isfinite(next.y)) break;
                // Short alternating segments distinguish the forecast from the actual trail.
                if ((step % 2) == 1) {
                    g_trajectoryDrawNode->drawSegment(previous, next, 1.5f, predictionColor);
                }
                previous = next;
            }
        }
    }

    void updateClickIndicator(PlayLayer* layer, float dt) {
        if (!layer) return;
        if (g_clickLayer != layer) {
            g_clickLayer = layer;
            g_clickLabel = nullptr;
            g_clickFlashRemaining = 0.f;
        }
        if (!showClicksEnabled()) {
            if (g_clickLabel) g_clickLabel->setVisible(false);
            g_clickFlashRemaining = 0.f;
            return;
        }
        if (!g_clickLabel) {
            g_clickLabel = CCLabelBMFont::create("CLICK", "chatFont.fnt");
            if (g_clickLabel) {
                g_clickLabel->setTag(kClickLabelTag);
                g_clickLabel->setScale(0.48f);
                g_clickLabel->setColor({255, 228, 80});
                g_clickLabel->setVisible(false);
                layer->addChild(g_clickLabel, 9997);
            }
        }
        g_clickFlashRemaining = std::max(0.f, g_clickFlashRemaining - std::max(0.f, dt));
        if (g_clickLabel && layer->m_player1 && g_clickFlashRemaining > 0.f) {
            const auto pos = layer->m_player1->getPosition();
            g_clickLabel->setPosition({pos.x, pos.y + 28.f});
            g_clickLabel->setVisible(true);
        } else if (g_clickLabel) {
            g_clickLabel->setVisible(false);
        }
    }

    std::filesystem::path customMenuMusicFile() {
        return Mod::get()->getSaveDir() / "custom-menu.mp3";
    }

    void applyCustomMenuMusic() {
        // Custom menu music is optional: the user places custom-menu.mp3 inside
        // this mod's save directory. It is never started while a level is active.
        auto* audio = FMODAudioEngine::sharedEngine();
        if (!audio) return;
        if (PlayLayer::get()) {
            // The level owns music playback now, but remember whether a custom
            // menu track will need to be restored when returning to the menu.
            g_customMenuMusicApplied = false;
            return;
        }

        if (customMenuMusicEnabled()) {
            const auto path = customMenuMusicFile();
            std::error_code ec;
            const bool exists = std::filesystem::exists(path, ec) && !ec;
            if (exists) {
                const auto nativePath = path.string();
                if (!g_customMenuMusicApplied || g_customMenuMusicPath != nativePath) {
                    audio->playMusic(gd::string(nativePath.c_str()), true, 0.35f, 0);
                    g_customMenuMusicPath = nativePath;
                    g_customMenuMusicApplied = true;
                    g_customMenuMusicNeedsRestore = true;
                }
                return;
            }
        }

        // Restore the stock menu loop if custom music was used before the last
        // level transition, including when the toggle was disabled in gameplay.
        if (g_customMenuMusicApplied || g_customMenuMusicNeedsRestore) {
            audio->playMusic("menuLoop.mp3", true, 0.35f, 0);
            g_customMenuMusicApplied = false;
            g_customMenuMusicNeedsRestore = false;
            g_customMenuMusicPath.clear();
        }
    }

    void setPauseSpritesAlpha(CCNode* node, GLubyte alpha) {
        if (!node) return;
        if (auto* sprite = typeinfo_cast<CCSprite*>(node)) sprite->setOpacity(alpha);
        else if (auto* label = typeinfo_cast<CCLabelBMFont*>(node)) label->setOpacity(alpha);
        else if (auto* layerColor = typeinfo_cast<CCLayerColor*>(node)) layerColor->setOpacity(alpha);
        if (auto* children = node->getChildren()) {
            for (unsigned int i = 0; i < children->count(); ++i) {
                setPauseSpritesAlpha(static_cast<CCNode*>(children->objectAtIndex(i)), alpha);
            }
        }
    }

    enum class BotMode { Idle, Recording, Playing, Continuing };

    struct BotEvent {
        double time = 0.0;
        int button = 0;
        bool pressed = false;
    };

    BotMode g_botMode = BotMode::Idle;
    std::vector<BotEvent> g_events;
    std::vector<int> g_playbackHeldButtons;
    double g_botClock = 0.0;
    std::size_t g_playIndex = 0;
    int g_macroLevelID = 0;
    PlayLayer* g_botLayer = nullptr;
    bool g_injectedInput = false;
    // True from Continue start until the appended recording is saved or cancelled.
    bool g_botContinueSession = false;
    // Event count in the saved macro before Continue appended its release boundary
    // and new input events. Used to discard only an incomplete appended segment.
    std::size_t g_botContinueOriginalEventCount = 0;
    // Ownership of the in-memory event list. A slot switch must never silently
    // play or save an older slot's buffer under the newly selected slot.
    int g_macroSlotInMemory = 1;
    bool g_botContinueHandoffPending = false;
    bool g_botPlaybackFinishPending = false;
    bool g_botLoopRestartPending = false;
    // Set when a handoff/retry/loop reset already established this frame's timestamp.
    bool g_botClockAdvancedThisFrame = false;

    bool saveMacro();
    bool finalizeContinueSession();

    bool noclipEnabled() {
        return Mod::get()->getSavedValue<bool>("noclip-enabled", false);
    }

    void setNoclipEnabled(bool enabled) {
        // Store our active toggle independently, and keep the Geode setting in sync.
        Mod::get()->setSavedValue("noclip-enabled", enabled);
        Mod::get()->setSettingValue("noclip-enabled", enabled);
    }

    bool speedhackEnabled() {
        return Mod::get()->getSavedValue<bool>("speedhack-enabled", false);
    }

    void setSpeedhackEnabled(bool enabled) {
        Mod::get()->setSavedValue("speedhack-enabled", enabled);
    }

    float speedMultiplier() {
        float value = Mod::get()->getSavedValue<float>("speedhack-multiplier", 1.0f);
        if (!std::isfinite(value)) value = 1.0f;
        return std::clamp(value, 0.1f, 5.0f);
    }

    void setSpeedMultiplier(float value) {
        if (!std::isfinite(value)) value = 1.0f;
        value = std::round(value * 10.0f) / 10.0f;
        Mod::get()->setSavedValue("speedhack-multiplier", std::clamp(value, 0.1f, 5.0f));
    }

    void notify(const std::string& message);

    bool musicMuted() {
        return Mod::get()->getSavedValue<bool>("music-muted", false);
    }

    bool musicSpeedSyncEnabled() {
        return Mod::get()->getSavedValue<bool>("music-speed-sync", true);
    }

    float musicPitchScale() {
        if (!PlayLayer::get() || !speedhackEnabled() || !musicSpeedSyncEnabled()) return 1.0f;
        return speedMultiplier();
    }

    void applyMusicPitch() {
        auto* audio = FMODAudioEngine::sharedEngine();
        if (!audio) return;

        // Music ID 0 is the standard level music channel used by Geometry Dash.
        // A missing channel is normal while menus/loading screens are active.
        const int channel = audio->getMusicChannelID(0);
        if (channel < 0) return;

        auto baseIt = g_musicBasePitchByChannel.find(channel);
        const float basePitch = baseIt != g_musicBasePitchByChannel.end() &&
            std::isfinite(baseIt->second) && baseIt->second > 0.0f
                ? baseIt->second : 1.0f;
        const float desiredPitch = std::clamp(basePitch * musicPitchScale(), 0.05f, 10.0f);

        auto appliedIt = g_musicAppliedPitchByChannel.find(channel);
        if (appliedIt != g_musicAppliedPitchByChannel.end() &&
            std::isfinite(appliedIt->second) && std::abs(appliedIt->second - desiredPitch) < 0.001f) {
            return;
        }

        // The FMOD hook below recognizes this as our own write and won't mistake
        // the multiplied value for the game's original/base pitch.
        g_settingMusicPitch = true;
        audio->setChannelPitch(channel, AudioTargetType::MusicChannel, desiredPitch);
        g_settingMusicPitch = false;
        g_musicAppliedPitchByChannel[channel] = desiredPitch;
    }

    void setMusicSpeedSyncEnabled(bool enabled) {
        Mod::get()->setSavedValue("music-speed-sync", enabled);
        applyMusicPitch();
        notify(std::string("VETER: music speed sync ") +
            (enabled ? "ON (with Speedhack)" : "OFF"));
    }

    void applyMusicMute() {
        auto* audio = FMODAudioEngine::sharedEngine();
        if (!audio) return;
        if (musicMuted()) {
            audio->pauseAllMusic(true);
        } else {
            audio->resumeAllMusic();
        }
    }

    void setMusicMuted(bool muted) {
        Mod::get()->setSavedValue("music-muted", muted);
        applyMusicMute();
        notify(muted ? "VETER: Music OFF" : "VETER: Music ON");
    }

    void applySpeedhack() {
        auto* director = CCDirector::sharedDirector();
        auto* scheduler = director ? director->getScheduler() : nullptr;
        if (scheduler) {
            scheduler->setTimeScale((PlayLayer::get() && speedhackEnabled()) ? speedMultiplier() : 1.0f);
        }
        applyMusicPitch();
    }

    std::string speedLabel() {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "%.1fx", static_cast<double>(speedMultiplier()));
        return std::string("SPEED ") + buffer;
    }

    void notify(const std::string& message) {
        Notification::create(message, NotificationIcon::Info)->show();
    }

    int currentLevelID() {
        auto* layer = PlayLayer::get();
        return layer && layer->m_level ? layer->m_level->m_levelID : 0;
    }

    bool botLoopEnabled() {
        return Mod::get()->getSavedValue<bool>("gdbot-loop-enabled", false);
    }
    void setBotLoopEnabled(bool enabled) {
        Mod::get()->setSavedValue("gdbot-loop-enabled", enabled);
    }
    float botPlaybackRate() {
        float value = Mod::get()->getSavedValue<float>("gdbot-playback-rate", 1.0f);
        if (!std::isfinite(value)) value = 1.0f;
        return std::clamp(std::round(value * 4.0f) / 4.0f, 0.25f, 2.0f);
    }
    void setBotPlaybackRate(float value) {
        if (!std::isfinite(value)) value = 1.0f;
        value = std::round(value * 4.0f) / 4.0f;
        Mod::get()->setSavedValue("gdbot-playback-rate", std::clamp(value, 0.25f, 2.0f));
    }
    int botMacroSlot() {
        return std::clamp(Mod::get()->getSavedValue<int>("gdbot-active-slot", 1), 1, 3);
    }
    void setBotMacroSlot(int slot) {
        Mod::get()->setSavedValue("gdbot-active-slot", std::clamp(slot, 1, 3));
    }
    std::string botPlaybackRateLabel() {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "RATE %.2fx", static_cast<double>(botPlaybackRate()));
        return buffer;
    }
    std::filesystem::path macroPath() {
        const int slot = botMacroSlot();
        // Slot 1 deliberately retains the original filename for backward compatibility.
        std::string filename = "gdbot.gdm";
        if (slot > 1) filename = "gdbot-slot" + std::to_string(slot) + ".gdm";
        return Mod::get()->getSaveDir() / filename;
    }

    void injectBotInput(PlayLayer* layer, int button, bool down) {
        if (!layer || !layer->m_player1 || button < 0 || button > 32) return;
        // Route playback through the same game input dispatcher as real/touch input.
        // The hook checks g_injectedInput to keep these events out of the recorder.
        g_injectedInput = true;
        layer->handleButton(down, button, true);
        g_injectedInput = false;
    }

    void releasePlaybackButtons() {
        auto* currentLayer = PlayLayer::get();
        // Only send releases to the exact PlayLayer that started playback. A level
        // transition can destroy that layer, so never send old button-up events to
        // a new level just because it happens to be the current PlayLayer.
        if (!g_botLayer || currentLayer != g_botLayer || !g_botLayer->m_player1) {
            g_playbackHeldButtons.clear();
            return;
        }

        for (const int value : g_playbackHeldButtons) {
            injectBotInput(g_botLayer, value, false);
        }
        g_playbackHeldButtons.clear();
    }

    void stopBot(bool showNotification = true) {
        const auto oldMode = g_botMode;
        if (oldMode == BotMode::Playing || oldMode == BotMode::Continuing) releasePlaybackButtons();
        g_botMode = BotMode::Idle;
        g_botLayer = nullptr;
        g_botContinueHandoffPending = false;
        g_botPlaybackFinishPending = false;
        g_botLoopRestartPending = false;
        g_botClockAdvancedThisFrame = false;

        if (!showNotification || oldMode == BotMode::Idle) return;
        if (oldMode == BotMode::Recording) {
            notify("GD Bot: recording stopped");
        } else {
            notify("GD Bot: playback stopped");
        }
    }

    void recordInput(int button, bool pressed) {
        if (g_botMode != BotMode::Recording || g_injectedInput) return;
        auto* layer = PlayLayer::get();
        if (!layer || !layer->m_player1) return;
        if (layer != g_botLayer) {
            if (g_botContinueSession) finalizeContinueSession();
            else stopBot(false);
            notify("GD Bot: recording stopped because the level changed");
            return;
        }
        if (button < 0 || button > 32) return;
        if (g_events.size() >= kMaxMacroEvents) {
            if (g_botContinueSession) finalizeContinueSession();
            else stopBot(false);
            notify("GD Bot: event limit reached; recording stopped");
            return;
        }
        g_events.push_back({g_botClock, button, pressed});
    }

    bool saveMacro() {
        if (g_botMode == BotMode::Recording) stopBot(false);
        if (g_events.empty()) {
            notify("GD Bot: nothing recorded yet");
            return false;
        }
        if (g_macroSlotInMemory != botMacroSlot()) {
            notify("GD Bot: active buffer belongs to slot " + std::to_string(g_macroSlotInMemory) +
                "; load the selected slot or start a new recording before saving");
            return false;
        }
        if (g_events.size() > kMaxMacroEvents) {
            notify("GD Bot: macro exceeds the event limit");
            return false;
        }
        double previousTime = 0.0;
        for (const auto& event : g_events) {
            if (!std::isfinite(event.time) || event.time < previousTime ||
                event.time > kMaxMacroDurationSeconds || event.button < 0 || event.button > 32) {
                notify("GD Bot: macro data is invalid; not saved");
                return false;
            }
            previousTime = event.time;
        }

        const auto path = macroPath();
        auto tempPath = path;
        tempPath += ".tmp";
        std::error_code error;
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) {
            log::error("GD Bot: could not create save folder: {}", error.message());
            notify("GD Bot: could not create save folder");
            return false;
        }
        std::filesystem::remove(tempPath, error);
        error.clear();

        std::ofstream file(tempPath, std::ios::trunc);
        if (!file) {
            notify("GD Bot: could not open the temporary save file");
            return false;
        }
        file << "GDBOT1\n" << g_macroLevelID << '\n';
        file.precision(8);
        for (const auto& event : g_events) {
            file << event.time << ' ' << (event.pressed ? 1 : 0) << ' ' << event.button << '\n';
        }
        file.flush();
        const bool writeOk = file.good();
        file.close();
        if (!writeOk || file.fail()) {
            std::filesystem::remove(tempPath, error);
            notify("GD Bot: saving failed; previous macro kept");
            return false;
        }

        // Rename only after the complete file is written, so an interrupted save
        // cannot truncate the previous valid macro.
        error.clear();
        std::filesystem::rename(tempPath, path, error);
        if (error) {
            std::error_code cleanupError;
            std::filesystem::remove(tempPath, cleanupError);
            log::error("GD Bot: could not replace macro file: {}", error.message());
            notify("GD Bot: could not replace macro file; previous macro kept");
            return false;
        }
        g_macroSlotInMemory = botMacroSlot();
        log::info("GD Bot: saved {} input events to slot {}", g_events.size(), botMacroSlot());
        notify("GD Bot: macro saved to slot " + std::to_string(botMacroSlot()));
        return true;
    }

    // Stop a Continue session and persist the combined old + newly recorded events.
    // saveMacro writes through a temporary file, so an unsuccessful save preserves the
    // previous on-disk macro and the user can retry saving.
    bool finalizeContinueSession() {
        if (!g_botContinueSession) return true;
        if (g_botMode != BotMode::Idle) stopBot(false);
        if (!saveMacro()) {
            notify("GD Bot: could not save Continue session; original macro file is kept");
            return false;
        }
        g_botContinueSession = false;
        g_botContinueOriginalEventCount = 0;
        notify("GD Bot: Continue session saved; previous inputs were preserved");
        return true;
    }

    bool loadMacro() {
        stopBot(false);
        // Keep the current in-memory macro until the selected file is fully
        // validated. A missing/corrupt slot must not destroy a good recording.
        std::ifstream file(macroPath());
        if (!file) {
            notify("GD Bot: saved macro not found; current macro kept");
            return false;
        }

        std::string signature;
        int levelID = 0;
        if (!(file >> signature >> levelID) || signature != "GDBOT1") {
            notify("GD Bot: invalid macro file; current macro kept");
            return false;
        }

        std::vector<BotEvent> loaded;
        double timestamp = 0.0;
        double previousTimestamp = 0.0;
        int pressed = 0;
        int button = 0;

        while (file >> timestamp >> pressed >> button) {
            if (!std::isfinite(timestamp) || timestamp < 0.0 || timestamp < previousTimestamp ||
                timestamp > kMaxMacroDurationSeconds ||
                (pressed != 0 && pressed != 1) || button < 0 || button > 32 ||
                loaded.size() >= kMaxMacroEvents) {
                notify("GD Bot: invalid or oversized macro; current macro kept");
                return false;
            }
            loaded.push_back({timestamp, button, pressed != 0});
            previousTimestamp = timestamp;
        }

        if (!file.eof()) {
            notify("GD Bot: malformed macro data; current macro kept");
            return false;
        }
        if (loaded.empty()) {
            notify("GD Bot: macro contains no inputs; current macro kept");
            return false;
        }

        g_events = std::move(loaded);
        g_macroLevelID = levelID;
        g_macroSlotInMemory = botMacroSlot();
        g_botClock = 0.0;
        g_playIndex = 0;
        log::info("GD Bot: loaded {} input events", g_events.size());
        notify("GD Bot: macro loaded");
        return true;
    }

    void processPlayback(PlayLayer* layer) {
        if ((g_botMode != BotMode::Playing && g_botMode != BotMode::Continuing) || !layer || !layer->m_player1) return;
        if (layer != g_botLayer) {
            if (g_botContinueSession) finalizeContinueSession();
            else stopBot(false);
            notify("GD Bot: playback stopped because the level changed");
            return;
        }

        while (g_playIndex < g_events.size() && g_events[g_playIndex].time <= g_botClock) {
            const auto event = g_events[g_playIndex++];

            if (event.pressed) {
                injectBotInput(layer, event.button, true);
                if (std::find(g_playbackHeldButtons.begin(), g_playbackHeldButtons.end(), event.button) == g_playbackHeldButtons.end()) {
                    g_playbackHeldButtons.push_back(event.button);
                }
            } else {
                injectBotInput(layer, event.button, false);
                std::erase(g_playbackHeldButtons, event.button);
            }
        }

        if (g_playIndex >= g_events.size()) {
            if (g_botMode == BotMode::Continuing) {
                // Complete the final physics update first, then checkpoint. Otherwise
                // the checkpoint can be one frame before the visible macro endpoint.
                g_botContinueHandoffPending = true;
                return;
            }

            // Defer the end-of-pass release/loop action until after the current
            // PlayLayer update so a final press gets one complete physics tick.
            g_botPlaybackFinishPending = true;
            return;
        }
    }

    void finishRegularPlayback(PlayLayer* layer) {
        if (!g_botPlaybackFinishPending) return;
        g_botPlaybackFinishPending = false;
        if (g_botMode != BotMode::Playing || !layer || layer != g_botLayer ||
            PlayLayer::get() != layer) {
            return;
        }

        releasePlaybackButtons();
        if (botLoopEnabled() && !g_events.empty() && layer->m_player1) {
            // BOT LOOP begins a fresh attempt after the just-completed physics tick.
            g_botLoopRestartPending = true;
        } else {
            g_botMode = BotMode::Idle;
            g_botLayer = nullptr;
            notify("GD Bot: playback finished");
        }
    }

    void finishContinuePlayback(PlayLayer* layer, double frameDelta) {
        if (!g_botContinueHandoffPending) return;
        g_botContinueHandoffPending = false;
        if (g_botMode != BotMode::Continuing || !layer || !layer->m_player1 ||
            layer != g_botLayer || PlayLayer::get() != layer) {
            return;
        }

        const auto heldToRelease = g_playbackHeldButtons;
        // The current physics tick has completed. Use its end-time for the release
        // boundary and suppress the normal clock increment later in this update.
        const double appendTime = std::max(g_botClock + std::max(0.0, frameDelta),
            g_events.empty() ? 0.0 : g_events.back().time);
        if (g_events.size() + heldToRelease.size() >= kMaxMacroEvents ||
            !std::isfinite(appendTime) || appendTime >= kMaxMacroDurationSeconds) {
            if (g_botContinueSession) finalizeContinueSession();
            else stopBot(false);
            notify("GD Bot: Continue stopped at a macro limit; previous events were kept");
            return;
        }

        // Store matching button-up events in the combined macro. A later full playback
        // will then reproduce the release boundary instead of leaving a key held.
        for (const int button : heldToRelease) {
            g_events.push_back({appendTime, button, false});
        }
        releasePlaybackButtons();
        g_botClock = appendTime;
        g_playIndex = g_events.size();
        g_botMode = BotMode::Recording;
        g_botClockAdvancedThisFrame = true;

        // A checkpoint allows subsequent Practice deaths to return to the exact point
        // where the appended section begins.
        auto* checkpoint = layer->createCheckpoint();
        if (checkpoint) {
            notify("GD Bot: reached saved endpoint — recording continued; checkpoint placed");
        } else {
            notify("GD Bot: reached saved endpoint — recording continued, but checkpoint failed");
        }
    }

    void toggleNoClip() {
        const bool enabled = !noclipEnabled();
        setNoclipEnabled(enabled);
        log::info("VETER: NoClip {}", enabled ? "ON" : "OFF");
        notify(enabled ? "VETER: NoClip ON" : "VETER: NoClip OFF");
    }

    void toggleSpeedhack() {
        const bool enabled = !speedhackEnabled();
        setSpeedhackEnabled(enabled);
        applySpeedhack();
        log::info("VETER: Speedhack {} at {}x", enabled ? "ON" : "OFF", speedMultiplier());
        notify(enabled ? "VETER: Speedhack ON" : "VETER: Speedhack OFF");
    }
}

class VETERToolsPopup;
class VETERUnlockPopup;
class VETEREditorPopup;
class VETERVisualsPopup;

class VETERPopup : public Popup {
private:
    enum class MenuTab { Player = 0, Bot = 1, Visuals = 2, Settings = 3 };
    CCNode* m_pages[4] = {nullptr, nullptr, nullptr, nullptr};
    CCMenu* m_pageMenus[4] = {nullptr, nullptr, nullptr, nullptr};
    ButtonSprite* m_tabSprites[4] = {nullptr, nullptr, nullptr, nullptr};
    int m_activeTab = 0;

    ButtonSprite* m_noClipSprite = nullptr;
    ButtonSprite* m_speedhackSprite = nullptr;
    ButtonSprite* m_musicSprite = nullptr;
    ButtonSprite* m_musicSyncSprite = nullptr;
    ButtonSprite* m_speedValueSprite = nullptr;
    ButtonSprite* m_botLoopSprite = nullptr;
    ButtonSprite* m_botSlotSprite = nullptr;
    ButtonSprite* m_botRateSprite = nullptr;

    ButtonSprite* m_fpsEnabledSprite = nullptr;
    ButtonSprite* m_fpsValueSprite = nullptr;
    ButtonSprite* m_retrySprite = nullptr;
    ButtonSprite* m_hitboxSprite = nullptr;
    ButtonSprite* m_cpsSprite = nullptr;
    ButtonSprite* m_practiceMusicSprite = nullptr;
    ButtonSprite* m_noclipLimitSprite = nullptr;
    ButtonSprite* m_noclipLimitValueSprite = nullptr;
    ButtonSprite* m_frameStepSprite = nullptr;
    ButtonSprite* m_noDeathEffectSprite = nullptr;
    ButtonSprite* m_hitboxesOnDeathSprite = nullptr;
    ButtonSprite* m_confirmExitSprite = nullptr;
    ButtonSprite* m_layoutModeSprite = nullptr;

    ButtonSprite* m_trajectorySprite = nullptr;
    ButtonSprite* m_predictionSprite = nullptr;
    ButtonSprite* m_clicksSprite = nullptr;
    ButtonSprite* m_fpsCounterSprite = nullptr;
    ButtonSprite* m_customMusicSprite = nullptr;
    ButtonSprite* m_transparentPauseSprite = nullptr;
    ButtonSprite* m_hidePauseSprite = nullptr;

    void refreshTabs() {
        static const char* labels[] = {"PLAYER", "GD BOT", "VISUALS", "SETTINGS"};
        for (int i = 0; i < 4; ++i) {
            if (!m_tabSprites[i]) continue;
            const std::string label = labels[i];
            m_tabSprites[i]->setString(label.c_str());
            m_tabSprites[i]->setColor(i == m_activeTab ? ccc3(60, 245, 255) : ccc3(235, 240, 255));
        }
    }

    void selectTab(int tab) {
        m_activeTab = std::clamp(tab, 0, 3);
        for (int i = 0; i < 4; ++i) {
            const bool active = i == m_activeTab;
            if (m_pages[i]) m_pages[i]->setVisible(active);
            // Hide the CCMenu itself too: some Cocos2d touch dispatch paths check
            // a menu's own visibility rather than walking invisible ancestors.
            if (m_pageMenus[i]) m_pageMenus[i]->setVisible(active);
        }
        refreshTabs();
    }

    void refreshSettingsLabels() {
        if (m_fpsEnabledSprite) m_fpsEnabledSprite->setString(fpsBypassEnabled() ? "FPS ON" : "FPS OFF");
        if (m_fpsValueSprite) {
            char value[24];
            std::snprintf(value, sizeof(value), "%d FPS", targetFPS());
            m_fpsValueSprite->setString(value);
        }
        if (m_retrySprite) m_retrySprite->setString(autoRetryEnabled() ? "RETRY ON" : "RETRY OFF");
        if (m_hitboxSprite) m_hitboxSprite->setString(hitboxesEnabled() ? "HITBOX ON" : "HITBOX OFF");
        if (m_cpsSprite) m_cpsSprite->setString(cpsEnabled() ? "CPS ON" : "CPS OFF");
        if (m_practiceMusicSprite) m_practiceMusicSprite->setString(practiceMusicEnabled() ? "PRACTICE ON" : "PRACTICE OFF");
        if (m_noclipLimitSprite) m_noclipLimitSprite->setString(noclipLimitEnabled() ? "NC LIMIT ON" : "NC LIMIT OFF");
        if (m_noclipLimitValueSprite) m_noclipLimitValueSprite->setString(("NC MAX " + std::to_string(noclipMaxDeaths())).c_str());
        if (m_frameStepSprite) m_frameStepSprite->setString(frameStepEnabled() ? "FRAME STEP ON" : "FRAME STEP OFF");
        if (m_noDeathEffectSprite) m_noDeathEffectSprite->setString(noDeathEffectEnabled() ? "NO DEATH FX ON" : "NO DEATH FX OFF");
        if (m_hitboxesOnDeathSprite) m_hitboxesOnDeathSprite->setString(hitboxesOnDeathEnabled() ? "DEATH BOX ON" : "DEATH BOX OFF");
        if (m_confirmExitSprite) m_confirmExitSprite->setString(confirmExitEnabled() ? "EXIT CONFIRM ON" : "EXIT CONFIRM OFF");
        if (m_layoutModeSprite) m_layoutModeSprite->setString(layoutModeEnabled() ? "GRID ON" : "GRID OFF");
    }

    void refreshVisualLabels() {
        if (m_trajectorySprite) m_trajectorySprite->setString(trajectoryEnabled() ? "TRAIL ON" : "TRAIL OFF");
        if (m_predictionSprite) m_predictionSprite->setString(trajectoryPredictorEnabled() ? "PREDICT ON" : "PREDICT OFF");
        if (m_clicksSprite) m_clicksSprite->setString(showClicksEnabled() ? "CLICKS ON" : "CLICKS OFF");
        if (m_fpsCounterSprite) m_fpsCounterSprite->setString(fpsCounterEnabled() ? "FPS COUNT ON" : "FPS COUNT OFF");
        if (m_customMusicSprite) m_customMusicSprite->setString(customMenuMusicEnabled() ? "MENU MUSIC ON" : "MENU MUSIC OFF");
        if (m_transparentPauseSprite) m_transparentPauseSprite->setString(transparentPauseEnabled() ? "PAUSE ALPHA ON" : "PAUSE ALPHA OFF");
        if (m_hidePauseSprite) m_hidePauseSprite->setString(hidePauseButtonEnabled() ? "HIDE PAUSE ON" : "HIDE PAUSE OFF");
    }

    void cycleCheckpoint(int direction) {
        auto* layer = PlayLayer::get();
        if (!layer || !layer->m_checkpointArray || layer->m_checkpointArray->count() == 0) {
            notify("Checkpoint Switcher: no checkpoints yet; use Practice Mode first");
            return;
        }
        const int count = static_cast<int>(layer->m_checkpointArray->count());
        if (g_checkpointIndex < 0 || g_checkpointIndex >= count) {
            g_checkpointIndex = direction > 0 ? 0 : count - 1;
        } else {
            g_checkpointIndex = (g_checkpointIndex + direction + count) % count;
        }
        auto* checkpoint = static_cast<CheckpointObject*>(layer->m_checkpointArray->objectAtIndex(g_checkpointIndex));
        if (checkpoint) {
            layer->loadFromCheckpoint(checkpoint);
            notify("Checkpoint " + std::to_string(g_checkpointIndex + 1) + "/" + std::to_string(count));
        }
    }

protected:
    bool init() {
        // A short window leaves enough vertical room for a wide-screen Android layout.
        if (!Popup::init(350.f, 305.f)) return false;
        setTitle("VETER");

        if (!m_mainLayer) return false;
        const auto size = m_mainLayer->getContentSize();
        auto* darkPanel = CCLayerColor::create(ccc4(16, 20, 34, 250), size.width, size.height);
        if (darkPanel) m_mainLayer->addChild(darkPanel, -5);
        auto* accent = CCLayerColor::create(ccc4(0, 220, 255, 255), size.width - 18.f, 3.f);
        if (accent) {
            accent->setPosition({9.f, size.height - 28.f});
            m_mainLayer->addChild(accent, -4);
        }

        auto makeMenu = [&](int tabIndex) -> CCMenu* {
            m_pages[tabIndex] = CCNode::create();
            if (!m_pages[tabIndex]) return nullptr;
            m_mainLayer->addChild(m_pages[tabIndex], 1);
            auto* menu = CCMenu::create();
            if (!menu) return nullptr;
            menu->setPosition({0.f, 0.f});
            m_pageMenus[tabIndex] = menu;
            m_pages[tabIndex]->addChild(menu);
            return menu;
        };
        auto addButton = [&](CCMenu* menu, const std::string& text, SEL_MenuHandler selector,
                             float x, float y, const char* frame, float fontScale = 0.27f) -> ButtonSprite* {
            if (!menu) return nullptr;
            auto* sprite = ButtonSprite::create(text.c_str(), "bigFont.fnt", frame, fontScale);
            if (!sprite) return nullptr;
            auto* item = CCMenuItemSpriteExtra::create(sprite, this, selector);
            if (!item) return nullptr;
            item->setPosition({x, y});
            menu->addChild(item);
            return sprite;
        };
        auto* nav = CCMenu::create();
        if (!nav) return false;
        nav->setPosition({0.f, 0.f});
        m_mainLayer->addChild(nav, 5);
        auto tabButton = [&](int idx, const char* label, SEL_MenuHandler selector, float x) {
            auto* sprite = ButtonSprite::create(label, "bigFont.fnt",
                idx == 0 ? "GJ_button_02.png" : "GJ_button_01.png", 0.25f);
            if (!sprite) return;
            auto* item = CCMenuItemSpriteExtra::create(sprite, this, selector);
            if (!item) return;
            item->setPosition({x, size.height - 49.f});
            m_tabSprites[idx] = sprite;
            nav->addChild(item);
        };
        tabButton(0, "PLAYER", menu_selector(VETERPopup::onTabPlayer), size.width * 0.14f);
        tabButton(1, "GD BOT", menu_selector(VETERPopup::onTabBot), size.width * 0.38f);
        tabButton(2, "VISUALS", menu_selector(VETERPopup::onTabVisuals), size.width * 0.63f);
        tabButton(3, "SETTINGS", menu_selector(VETERPopup::onTabSettings), size.width * 0.87f);

        const float left = size.width * 0.19f;
        const float center = size.width * 0.50f;
        const float right = size.width * 0.81f;
        const float top = size.height - 92.f;

        // PLAYER tab: gameplay switches and speed controls.
        auto* player = makeMenu(0);
        m_noClipSprite = addButton(player, noclipEnabled() ? "NOCLIP ON" : "NOCLIP OFF",
            menu_selector(VETERPopup::onToggleNoClip), left, top, noclipEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_speedhackSprite = addButton(player, speedhackEnabled() ? "SPEED ON" : "SPEED OFF",
            menu_selector(VETERPopup::onToggleSpeedhack), center, top, speedhackEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_musicSprite = addButton(player, musicMuted() ? "MUSIC OFF" : "MUSIC ON",
            menu_selector(VETERPopup::onToggleMusic), right, top, musicMuted() ? "GJ_button_04.png" : "GJ_button_02.png");
        addButton(player, "-0.1x", menu_selector(VETERPopup::onSpeedDown), left, top - 50.f, "GJ_button_01.png");
        m_speedValueSprite = addButton(player, speedLabel(), menu_selector(VETERPopup::onSpeedLabel), center, top - 50.f, "GJ_button_02.png");
        addButton(player, "+0.1x", menu_selector(VETERPopup::onSpeedUp), right, top - 50.f, "GJ_button_01.png");
        m_musicSyncSprite = addButton(player, musicSpeedSyncEnabled() ? "MUSIC SYNC ON" : "MUSIC SYNC OFF",
            menu_selector(VETERPopup::onToggleMusicSync), center, top - 101.f,
            musicSpeedSyncEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", 0.25f);

        // GD BOT tab: all macro controls stay together and fit in four rows.
        auto* bot = makeMenu(1);
        const float botGap = 43.f;
        auto botRow = [&](int i) { return top - 3.f - botGap * static_cast<float>(i); };
        addButton(bot, "REC", menu_selector(VETERPopup::onRecord), left, botRow(0), "GJ_button_01.png");
        addButton(bot, "STOP", menu_selector(VETERPopup::onStopBot), center, botRow(0), "GJ_button_04.png");
        addButton(bot, "PLAY", menu_selector(VETERPopup::onPlay), right, botRow(0), "GJ_button_01.png");
        addButton(bot, "CONTINUE", menu_selector(VETERPopup::onContinue), left, botRow(1), "GJ_button_02.png", 0.25f);
        addButton(bot, "SAVE", menu_selector(VETERPopup::onSave), center, botRow(1), "GJ_button_02.png");
        addButton(bot, "LOAD", menu_selector(VETERPopup::onLoad), right, botRow(1), "GJ_button_02.png");
        addButton(bot, "STATUS", menu_selector(VETERPopup::onBotStatus), left, botRow(2), "GJ_button_01.png");
        m_botLoopSprite = addButton(bot, botLoopEnabled() ? "LOOP ON" : "LOOP OFF",
            menu_selector(VETERPopup::onToggleBotLoop), center, botRow(2), botLoopEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_botSlotSprite = addButton(bot, "SLOT " + std::to_string(botMacroSlot()), menu_selector(VETERPopup::onNextBotSlot),
            right, botRow(2), "GJ_button_01.png");
        addButton(bot, "RATE -", menu_selector(VETERPopup::onBotRateDown), left, botRow(3), "GJ_button_01.png");
        m_botRateSprite = addButton(bot, botPlaybackRateLabel(), menu_selector(VETERPopup::onBotRateInfo), center, botRow(3), "GJ_button_02.png");
        addButton(bot, "RATE +", menu_selector(VETERPopup::onBotRateUp), right, botRow(3), "GJ_button_01.png");

        // VISUALS tab: these are direct controls, not another oversized popup.
        auto* visuals = makeMenu(2);
        const float visualGap = 47.f;
        auto visualRow = [&](int i) { return top - visualGap * static_cast<float>(i); };
        m_trajectorySprite = addButton(visuals, trajectoryEnabled() ? "TRAIL ON" : "TRAIL OFF",
            menu_selector(VETERPopup::onToggleTrajectory), left, visualRow(0), trajectoryEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_predictionSprite = addButton(visuals, trajectoryPredictorEnabled() ? "PREDICT ON" : "PREDICT OFF",
            menu_selector(VETERPopup::onTogglePrediction), center, visualRow(0), trajectoryPredictorEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_clicksSprite = addButton(visuals, showClicksEnabled() ? "CLICKS ON" : "CLICKS OFF",
            menu_selector(VETERPopup::onToggleClicks), right, visualRow(0), showClicksEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_fpsCounterSprite = addButton(visuals, fpsCounterEnabled() ? "FPS COUNT ON" : "FPS COUNT OFF",
            menu_selector(VETERPopup::onToggleFPSCounter), left, visualRow(1), fpsCounterEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", 0.24f);
        m_customMusicSprite = addButton(visuals, customMenuMusicEnabled() ? "MENU MUSIC ON" : "MENU MUSIC OFF",
            menu_selector(VETERPopup::onToggleCustomMusic), center, visualRow(1), customMenuMusicEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", 0.24f);
        m_transparentPauseSprite = addButton(visuals, transparentPauseEnabled() ? "PAUSE ALPHA ON" : "PAUSE ALPHA OFF",
            menu_selector(VETERPopup::onToggleTransparentPause), right, visualRow(1), transparentPauseEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", 0.23f);
        m_hidePauseSprite = addButton(visuals, hidePauseButtonEnabled() ? "HIDE PAUSE ON" : "HIDE PAUSE OFF",
            menu_selector(VETERPopup::onToggleHidePause), left, visualRow(2), hidePauseButtonEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", 0.24f);
        addButton(visuals, "MUSIC FILE INFO", menu_selector(VETERPopup::onMusicInfo), center, visualRow(2), "GJ_button_01.png", 0.24f);

        // SETTINGS tab: compact controls for the remaining toggles and utilities.
        auto* settings = makeMenu(3);
        const float setTop = size.height - 91.f;
        const float setGap = 25.3f;
        auto setRow = [&](int i) { return setTop - setGap * static_cast<float>(i); };
        const float small = 0.215f;
        m_fpsEnabledSprite = addButton(settings, fpsBypassEnabled() ? "FPS ON" : "FPS OFF", menu_selector(VETERPopup::onSetFPS), left, setRow(0), fpsBypassEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", small);
        m_retrySprite = addButton(settings, autoRetryEnabled() ? "RETRY ON" : "RETRY OFF", menu_selector(VETERPopup::onSetRetry), center, setRow(0), autoRetryEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", small);
        m_hitboxSprite = addButton(settings, hitboxesEnabled() ? "HITBOX ON" : "HITBOX OFF", menu_selector(VETERPopup::onSetHitboxes), right, setRow(0), hitboxesEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", small);
        addButton(settings, "FPS -30", menu_selector(VETERPopup::onFPSDown), left, setRow(1), "GJ_button_01.png", small);
        m_fpsValueSprite = addButton(settings, "", menu_selector(VETERPopup::onFPSInfo), center, setRow(1), "GJ_button_02.png", small);
        addButton(settings, "FPS +30", menu_selector(VETERPopup::onFPSUp), right, setRow(1), "GJ_button_01.png", small);
        m_cpsSprite = addButton(settings, cpsEnabled() ? "CPS ON" : "CPS OFF", menu_selector(VETERPopup::onSetCPS), left, setRow(2), cpsEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", small);
        m_practiceMusicSprite = addButton(settings, practiceMusicEnabled() ? "PRACTICE ON" : "PRACTICE OFF", menu_selector(VETERPopup::onSetPracticeMusic), center, setRow(2), practiceMusicEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", small);
        m_noclipLimitSprite = addButton(settings, noclipLimitEnabled() ? "NC LIMIT ON" : "NC LIMIT OFF", menu_selector(VETERPopup::onSetNoclipLimit), right, setRow(2), noclipLimitEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", small);
        m_noclipLimitValueSprite = addButton(settings, "", menu_selector(VETERPopup::onSetNoclipMax), left, setRow(3), "GJ_button_01.png", small);
        m_noDeathEffectSprite = addButton(settings, noDeathEffectEnabled() ? "NO DEATH FX ON" : "NO DEATH FX OFF", menu_selector(VETERPopup::onSetNoDeathEffect), center, setRow(3), noDeathEffectEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", small);
        m_hitboxesOnDeathSprite = addButton(settings, hitboxesOnDeathEnabled() ? "DEATH BOX ON" : "DEATH BOX OFF", menu_selector(VETERPopup::onSetHitboxesOnDeath), right, setRow(3), hitboxesOnDeathEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", small);
        m_frameStepSprite = addButton(settings, frameStepEnabled() ? "FRAME STEP ON" : "FRAME STEP OFF", menu_selector(VETERPopup::onSetFrameStep), left, setRow(4), frameStepEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", small);
        addButton(settings, "STEP +1", menu_selector(VETERPopup::onStepFrame), center, setRow(4), "GJ_button_01.png", small);
        m_layoutModeSprite = addButton(settings, layoutModeEnabled() ? "GRID ON" : "GRID OFF", menu_selector(VETERPopup::onSetLayoutMode), right, setRow(4), layoutModeEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", small);
        addButton(settings, "CP PREV", menu_selector(VETERPopup::onPreviousCheckpoint), left, setRow(5), "GJ_button_01.png", small);
        addButton(settings, "CP NEXT", menu_selector(VETERPopup::onNextCheckpoint), center, setRow(5), "GJ_button_01.png", small);
        m_confirmExitSprite = addButton(settings, confirmExitEnabled() ? "EXIT CONFIRM ON" : "EXIT CONFIRM OFF", menu_selector(VETERPopup::onSetConfirmExit), right, setRow(5), confirmExitEnabled() ? "GJ_button_02.png" : "GJ_button_04.png", small);
        addButton(settings, "EDITOR", menu_selector(VETERPopup::onOpenEditor), left, setRow(6), "GJ_button_01.png", small);
        addButton(settings, "UNLOCKS", menu_selector(VETERPopup::onOpenUnlocks), center, setRow(6), "GJ_button_01.png", small);
        addButton(settings, "FAST RESET", menu_selector(VETERPopup::onFastReset), right, setRow(6), "GJ_button_01.png", small);

        selectTab(0);
        refreshSettingsLabels();
        refreshVisualLabels();
        return true;
    }

public:
    void onMore(CCObject*);
    void onOpenEditor(CCObject*);
    void onOpenUnlocks(CCObject*);

    static VETERPopup* create() {
        auto* popup = new VETERPopup();
        if (popup && popup->init()) { popup->autorelease(); return popup; }
        delete popup;
        return nullptr;
    }

    void onTabPlayer(CCObject*) { selectTab(0); }
    void onTabBot(CCObject*) { selectTab(1); }
    void onTabVisuals(CCObject*) { selectTab(2); }
    void onTabSettings(CCObject*) { selectTab(3); }

    void onToggleNoClip(CCObject*) {
        toggleNoClip();
        if (m_noClipSprite) m_noClipSprite->setString(noclipEnabled() ? "NOCLIP ON" : "NOCLIP OFF");
    }
    void onToggleSpeedhack(CCObject*) {
        toggleSpeedhack();
        if (m_speedhackSprite) m_speedhackSprite->setString(speedhackEnabled() ? "SPEED ON" : "SPEED OFF");
    }
    void onToggleMusic(CCObject*) {
        setMusicMuted(!musicMuted());
        if (m_musicSprite) m_musicSprite->setString(musicMuted() ? "MUSIC OFF" : "MUSIC ON");
    }
    void onToggleMusicSync(CCObject*) {
        setMusicSpeedSyncEnabled(!musicSpeedSyncEnabled());
        if (m_musicSyncSprite) m_musicSyncSprite->setString(musicSpeedSyncEnabled() ? "MUSIC SYNC ON" : "MUSIC SYNC OFF");
    }
    void onSpeedLabel(CCObject*) { notify("Tap -0.1x or +0.1x to set speed"); }
    void onSpeedDown(CCObject*) {
        setSpeedMultiplier(speedMultiplier() - 0.1f);
        if (speedhackEnabled()) applySpeedhack();
        if (m_speedValueSprite) m_speedValueSprite->setString(speedLabel().c_str());
        notify("VETER: speed set to " + speedLabel().substr(6));
    }
    void onSpeedUp(CCObject*) {
        setSpeedMultiplier(speedMultiplier() + 0.1f);
        if (speedhackEnabled()) applySpeedhack();
        if (m_speedValueSprite) m_speedValueSprite->setString(speedLabel().c_str());
        notify("VETER: speed set to " + speedLabel().substr(6));
    }
    void onRecord(CCObject*) {
        if (g_botMode != BotMode::Idle) { notify("GD Bot: stop the current action before starting a new recording"); onClose(nullptr); return; }
        auto* layer = PlayLayer::get();
        if (!layer || !layer->m_player1) { notify("GD Bot: enter a level first"); onClose(nullptr); return; }
        if (g_botContinueSession && !finalizeContinueSession()) { onClose(nullptr); return; }
        stopBot(false);
        g_events.clear();
        g_macroSlotInMemory = botMacroSlot();
        g_playbackHeldButtons.clear();
        g_botClock = 0.0;
        g_playIndex = 0;
        g_botContinueHandoffPending = false;
        g_botPlaybackFinishPending = false;
        g_botLoopRestartPending = false;
        g_botClockAdvancedThisFrame = false;
        layer->resetLevel();
        g_macroLevelID = currentLevelID();
        g_botLayer = layer;
        g_botMode = BotMode::Recording;
        notify("GD Bot: recording started; play the level now");
        onClose(nullptr);
    }
    void onStopBot(CCObject*) {
        if (g_botContinueSession) finalizeContinueSession(); else stopBot();
        onClose(nullptr);
    }
    void onPlay(CCObject*) {
        if (g_botMode != BotMode::Idle) { notify("GD Bot: stop the current action before starting playback"); onClose(nullptr); return; }
        auto* layer = PlayLayer::get();
        if (!layer || !layer->m_player1) { notify("GD Bot: enter the recorded level first"); onClose(nullptr); return; }
        if (g_botContinueSession && !finalizeContinueSession()) { onClose(nullptr); return; }
        if (g_events.empty() || g_macroSlotInMemory != botMacroSlot()) {
            if (!loadMacro()) { notify("GD Bot: no valid macro in the selected slot; old in-memory data was not played"); onClose(nullptr); return; }
        }
        if (g_events.empty()) { notify("GD Bot: record or load a macro first"); onClose(nullptr); return; }
        const int levelID = currentLevelID();
        if (g_macroLevelID != 0 && levelID != 0 && g_macroLevelID != levelID) { notify("GD Bot: this macro belongs to another level"); onClose(nullptr); return; }
        stopBot(false);
        g_botClock = 0.0; g_playIndex = 0; g_playbackHeldButtons.clear();
        g_botContinueHandoffPending = false; g_botPlaybackFinishPending = false; g_botLoopRestartPending = false;
        g_botClockAdvancedThisFrame = false;
        layer->resetLevel();
        g_botLayer = layer; g_botMode = BotMode::Playing; g_botContinueSession = false;
        notify("GD Bot: playback started");
        onClose(nullptr);
    }
    void onContinue(CCObject*) {
        if (g_botContinueSession) { notify("GD Bot: a Continue session is already active; stop/save it first"); onClose(nullptr); return; }
        if (g_botMode != BotMode::Idle) { notify("GD Bot: stop the current bot action first"); onClose(nullptr); return; }
        if (!loadMacro()) { notify("GD Bot Continue: save a macro in the selected slot first"); onClose(nullptr); return; }
        auto* layer = PlayLayer::get();
        if (!layer || !layer->m_player1) { notify("GD Bot Continue: open the macro's level first; Practice activates automatically"); onClose(nullptr); return; }
        const int levelID = currentLevelID();
        if (g_macroLevelID != 0 && levelID != 0 && g_macroLevelID != levelID) { notify("GD Bot Continue: open the same level as the saved macro"); onClose(nullptr); return; }
        if (g_events.size() >= kMaxMacroEvents) { notify("GD Bot Continue: event limit reached; old macro kept"); onClose(nullptr); return; }
        layer->togglePracticeMode(true);
        stopBot(false);
        g_botClock = 0.0; g_playIndex = 0; g_playbackHeldButtons.clear();
        g_botContinueHandoffPending = false; g_botPlaybackFinishPending = false; g_botLoopRestartPending = false;
        g_botClockAdvancedThisFrame = false;
        layer->resetLevelFromStart();
        g_botLayer = layer; g_botMode = BotMode::Continuing;
        g_botContinueOriginalEventCount = g_events.size(); g_botContinueSession = true;
        notify("GD Bot Continue: Practice ON; replaying saved inputs, then recording will append without deleting them");
        onClose(nullptr);
    }
    void onSave(CCObject*) {
        const bool continuePlaybackInProgress = g_botMode == BotMode::Continuing;
        if (saveMacro() && !continuePlaybackInProgress) g_botContinueSession = false;
        onClose(nullptr);
    }
    void onLoad(CCObject*) {
        if (g_botMode != BotMode::Idle) { notify("GD Bot: stop the current action before loading a macro"); onClose(nullptr); return; }
        if (g_botContinueSession && !finalizeContinueSession()) { onClose(nullptr); return; }
        loadMacro();
        onClose(nullptr);
    }
    void onBotStatus(CCObject*) {
        const char* mode = g_botMode == BotMode::Recording ? "RECORDING" : g_botMode == BotMode::Continuing ? "CONTINUE PLAYBACK" : g_botMode == BotMode::Playing ? "PLAYING" : "IDLE";
        notify(std::string("GD Bot: ") + mode + " | events: " + std::to_string(g_events.size()) + " | slot: " + std::to_string(botMacroSlot()) + " | buffer: " + std::to_string(g_macroSlotInMemory) + (botLoopEnabled() ? " | loop: ON" : " | loop: OFF") + (g_botContinueSession ? " | append session: ON" : ""));
    }
    void onToggleBotLoop(CCObject*) {
        setBotLoopEnabled(!botLoopEnabled());
        if (m_botLoopSprite) m_botLoopSprite->setString(botLoopEnabled() ? "LOOP ON" : "LOOP OFF");
        notify(std::string("GD Bot loop ") + (botLoopEnabled() ? "ON" : "OFF"));
    }
    void onNextBotSlot(CCObject*) {
        if (g_botMode != BotMode::Idle || g_botContinueSession) { notify("GD Bot: stop/save the current bot action before switching macro slots"); return; }
        const int nextSlot = botMacroSlot() >= 3 ? 1 : botMacroSlot() + 1;
        setBotMacroSlot(nextSlot);
        if (m_botSlotSprite) m_botSlotSprite->setString(("SLOT " + std::to_string(nextSlot)).c_str());
        notify(g_macroSlotInMemory == nextSlot ? "GD Bot: active slot " + std::to_string(nextSlot) + " (buffer belongs to this slot)" : "GD Bot: active slot " + std::to_string(nextSlot) + "; use LOAD or PLAY to load its saved macro");
    }
    void onBotRateDown(CCObject*) {
        setBotPlaybackRate(botPlaybackRate() - 0.25f);
        if (m_botRateSprite) m_botRateSprite->setString(botPlaybackRateLabel().c_str());
        notify("GD Bot: playback " + botPlaybackRateLabel().substr(5));
    }
    void onBotRateUp(CCObject*) {
        setBotPlaybackRate(botPlaybackRate() + 0.25f);
        if (m_botRateSprite) m_botRateSprite->setString(botPlaybackRateLabel().c_str());
        notify("GD Bot: playback " + botPlaybackRateLabel().substr(5));
    }
    void onBotRateInfo(CCObject*) { notify("GD Bot playback rate: " + botPlaybackRateLabel().substr(5) + ". Range 0.25x to 2.00x."); }

    // SETTINGS tab callbacks retain the same saved values as the old Tools popup.
    void onSetFPS(CCObject*) { setFPSBypassEnabled(!fpsBypassEnabled()); applyFPSBypass(); refreshSettingsLabels(); notify(std::string("VETER: FPS bypass ") + (fpsBypassEnabled() ? "ON" : "OFF")); }
    void onFPSDown(CCObject*) { setTargetFPS(targetFPS() - 30); applyFPSBypass(); refreshSettingsLabels(); }
    void onFPSUp(CCObject*) { setTargetFPS(targetFPS() + 30); applyFPSBypass(); refreshSettingsLabels(); }
    void onFPSInfo(CCObject*) { notify("FPS target: " + std::to_string(targetFPS()) + ". Actual FPS depends on device/display."); }
    void onSetRetry(CCObject*) { setAutoRetryEnabled(!autoRetryEnabled()); refreshSettingsLabels(); notify(std::string("VETER: Auto-Retry ") + (autoRetryEnabled() ? "ON" : "OFF")); }
    void onSetHitboxes(CCObject*) {
        const bool enabled = !hitboxesEnabled(); setHitboxesEnabled(enabled);
        if (auto* layer = PlayLayer::get()) { layer->toggleDebugDraw(); g_debugDrawLayer = layer; }
        refreshSettingsLabels(); notify(enabled ? "VETER: Hitboxes ON" : "VETER: Hitboxes OFF");
    }
    void onSetCPS(CCObject*) { setCPSEnabled(!cpsEnabled()); if (!cpsEnabled() && g_cpsLabel) g_cpsLabel->setVisible(false); refreshSettingsLabels(); notify(std::string("VETER: CPS ") + (cpsEnabled() ? "ON" : "OFF")); }
    void onSetPracticeMusic(CCObject*) {
        auto* layer = PlayLayer::get(); if (!layer) { notify("Practice Music: enter a level first"); return; }
        const bool enabled = !practiceMusicEnabled(); layer->toggleMusicInPractice(); setPracticeMusicEnabled(enabled); g_practiceMusicLayer = layer;
        refreshSettingsLabels(); notify(enabled ? "Practice Music ON" : "Practice Music OFF");
    }
    void onSetNoclipLimit(CCObject*) { setNoclipLimitEnabled(!noclipLimitEnabled()); refreshSettingsLabels(); notify(std::string("Noclip limit ") + (noclipLimitEnabled() ? "ON" : "OFF")); }
    void onSetNoclipMax(CCObject*) { int next = noclipMaxDeaths() + 1; if (next > 10) next = 1; setNoclipMaxDeaths(next); refreshSettingsLabels(); notify("Noclip collision limit: " + std::to_string(noclipMaxDeaths())); }
    void onSetNoDeathEffect(CCObject*) { setNoDeathEffectEnabled(!noDeathEffectEnabled()); refreshSettingsLabels(); notify(std::string("No Death Effect ") + (noDeathEffectEnabled() ? "ON" : "OFF")); }
    void onSetHitboxesOnDeath(CCObject*) { setHitboxesOnDeathEnabled(!hitboxesOnDeathEnabled()); refreshSettingsLabels(); notify(std::string("Hitboxes on Death ") + (hitboxesOnDeathEnabled() ? "ON" : "OFF")); }
    void onSetFrameStep(CCObject*) { setFrameStepEnabled(!frameStepEnabled()); g_frameStepPending = false; refreshSettingsLabels(); notify(frameStepEnabled() ? "Frame Step ON: use STEP +1" : "Frame Step OFF"); }
    void onStepFrame(CCObject*) { if (!frameStepEnabled()) { notify("Turn FRAME STEP ON first"); return; } g_frameStepPending = true; notify("Frame Step: one update queued"); }
    void onPreviousCheckpoint(CCObject*) { cycleCheckpoint(-1); }
    void onNextCheckpoint(CCObject*) { cycleCheckpoint(1); }
    void onSetConfirmExit(CCObject*) { setConfirmExitEnabled(!confirmExitEnabled()); refreshSettingsLabels(); notify(std::string("Confirm Exit ") + (confirmExitEnabled() ? "ON" : "OFF")); }
    void onSetLayoutMode(CCObject*) { setLayoutModeEnabled(!layoutModeEnabled()); refreshSettingsLabels(); notify(layoutModeEnabled() ? "Layout Mode ON" : "Layout Mode OFF"); }
    void onFastReset(CCObject*) { auto* layer = PlayLayer::get(); if (!layer) { notify("Fast Reset: enter a level first"); onClose(nullptr); return; } stopBot(false); onClose(nullptr); layer->resetLevel(); }

    // VISUALS tab callbacks share the existing settings and overlay routines.
    void onToggleTrajectory(CCObject*) {
        setTrajectoryEnabled(!trajectoryEnabled());
        if (!trajectoryEnabled() && !trajectoryPredictorEnabled() && g_trajectoryDrawNode) { g_trajectoryDrawNode->removeFromParentAndCleanup(true); g_trajectoryDrawNode = nullptr; g_trajectoryLayer = nullptr; g_trajectoryPoints.clear(); }
        refreshVisualLabels(); notify(std::string("Position trail ") + (trajectoryEnabled() ? "ON" : "OFF"));
    }
    void onTogglePrediction(CCObject*) {
        setTrajectoryPredictorEnabled(!trajectoryPredictorEnabled());
        if (!trajectoryEnabled() && !trajectoryPredictorEnabled() && g_trajectoryDrawNode) { g_trajectoryDrawNode->removeFromParentAndCleanup(true); g_trajectoryDrawNode = nullptr; g_trajectoryLayer = nullptr; g_trajectoryPoints.clear(); }
        refreshVisualLabels(); notify(trajectoryPredictorEnabled() ? "Trajectory prediction ON (approximate)" : "Trajectory prediction OFF");
    }
    void onToggleClicks(CCObject*) { setShowClicksEnabled(!showClicksEnabled()); if (!showClicksEnabled() && g_clickLabel) g_clickLabel->setVisible(false); refreshVisualLabels(); notify(std::string("Show Clicks ") + (showClicksEnabled() ? "ON" : "OFF")); }
    void onToggleFPSCounter(CCObject*) { setFPSCounterEnabled(!fpsCounterEnabled()); if (!fpsCounterEnabled() && g_fpsCounterLabel) g_fpsCounterLabel->setVisible(false); refreshVisualLabels(); notify(std::string("FPS counter ") + (fpsCounterEnabled() ? "ON" : "OFF")); }
    void onToggleCustomMusic(CCObject*) {
        if (!customMenuMusicEnabled()) {
            std::error_code ec;
            if (!std::filesystem::exists(customMenuMusicFile(), ec) || ec) { notify("Copy custom-menu.mp3 into VETER save folder first"); return; }
            setCustomMenuMusicEnabled(true);
        } else setCustomMenuMusicEnabled(false);
        applyCustomMenuMusic(); refreshVisualLabels(); notify(std::string("Custom menu music ") + (customMenuMusicEnabled() ? "ON" : "OFF"));
    }
    void onToggleTransparentPause(CCObject*) { setTransparentPauseEnabled(!transparentPauseEnabled()); refreshVisualLabels(); notify(std::string("Transparent Pause ") + (transparentPauseEnabled() ? "ON (next pause)" : "OFF (next pause)")); }
    void onToggleHidePause(CCObject*) { setHidePauseButtonEnabled(!hidePauseButtonEnabled()); refreshVisualLabels(); notify(std::string("Hide Pause Button ") + (hidePauseButtonEnabled() ? "ON" : "OFF")); }
    void onMusicInfo(CCObject*) { const auto path = customMenuMusicFile().string(); FLAlertLayer::create("Custom Menu Music", ("Place your own MP3 file here:\n" + path + "\nThen turn MENU MUSIC ON from the Visuals tab.").c_str(), "OK")->show(); }
};

class VETERToolsPopup : public Popup {
private:
    ButtonSprite* m_fpsEnabledSprite = nullptr;
    ButtonSprite* m_fpsValueSprite = nullptr;
    ButtonSprite* m_retrySprite = nullptr;
    ButtonSprite* m_hitboxSprite = nullptr;
    ButtonSprite* m_cpsSprite = nullptr;
    ButtonSprite* m_practiceMusicSprite = nullptr;
    ButtonSprite* m_noclipLimitSprite = nullptr;
    ButtonSprite* m_noclipLimitValueSprite = nullptr;
    ButtonSprite* m_frameStepSprite = nullptr;
    ButtonSprite* m_noDeathEffectSprite = nullptr;
    ButtonSprite* m_hitboxesOnDeathSprite = nullptr;
    ButtonSprite* m_confirmExitSprite = nullptr;
    ButtonSprite* m_layoutModeSprite = nullptr;

    void refreshLabels() {
        if (m_fpsEnabledSprite) m_fpsEnabledSprite->setString(fpsBypassEnabled() ? "FPS ON" : "FPS OFF");
        if (m_fpsValueSprite) {
            char value[24];
            std::snprintf(value, sizeof(value), "%d FPS", targetFPS());
            m_fpsValueSprite->setString(value);
        }
        if (m_retrySprite) m_retrySprite->setString(autoRetryEnabled() ? "RETRY ON" : "RETRY OFF");
        if (m_hitboxSprite) m_hitboxSprite->setString(hitboxesEnabled() ? "HITBOXES ON" : "HITBOXES OFF");
        if (m_cpsSprite) m_cpsSprite->setString(cpsEnabled() ? "CPS ON" : "CPS OFF");
        if (m_practiceMusicSprite) m_practiceMusicSprite->setString(practiceMusicEnabled() ? "PRACTICE MUSIC ON" : "PRACTICE MUSIC OFF");
        if (m_noclipLimitSprite) m_noclipLimitSprite->setString(noclipLimitEnabled() ? "NC LIMIT ON" : "NC LIMIT OFF");
        if (m_noclipLimitValueSprite) m_noclipLimitValueSprite->setString(("NC MAX " + std::to_string(noclipMaxDeaths())).c_str());
        if (m_frameStepSprite) m_frameStepSprite->setString(frameStepEnabled() ? "FRAME STEP ON" : "FRAME STEP OFF");
        if (m_noDeathEffectSprite) m_noDeathEffectSprite->setString(noDeathEffectEnabled() ? "NO DEATH FX ON" : "NO DEATH FX OFF");
        if (m_hitboxesOnDeathSprite) m_hitboxesOnDeathSprite->setString(hitboxesOnDeathEnabled() ? "DEATH HITBOX ON" : "DEATH HITBOX OFF");
        if (m_confirmExitSprite) m_confirmExitSprite->setString(confirmExitEnabled() ? "CONFIRM EXIT ON" : "CONFIRM EXIT OFF");
        if (m_layoutModeSprite) m_layoutModeSprite->setString(layoutModeEnabled() ? "LAYOUT GRID ON" : "LAYOUT GRID OFF");
    }

protected:
    bool init() {
        if (!Popup::init(400.f, 545.f)) return false;
        setTitle("VETER - Tools");
        auto* menu = CCMenu::create();
        if (!menu || !m_mainLayer) return false;
        menu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(menu);
        const auto size = m_mainLayer->getContentSize();
        const float left = size.width * 0.36f;
        const float center = size.width * 0.50f;
        const float right = size.width * 0.64f;
        auto addButton = [&](const std::string& text, SEL_MenuHandler selector,
                             float x, float y, const char* frame) -> ButtonSprite* {
            auto* sprite = ButtonSprite::create(text.c_str(), "bigFont.fnt", frame, 0.29f);
            if (!sprite) return nullptr;
            auto* item = CCMenuItemSpriteExtra::create(sprite, this, selector);
            if (!item) return nullptr;
            item->setPosition({x, y});
            menu->addChild(item);
            return sprite;
        };
        const float row1 = size.height - 58.f;
        const float row2 = size.height - 102.f;
        const float row3 = size.height - 146.f;
        const float row4 = size.height - 190.f;
        const float row5 = size.height - 234.f;
        const float row6 = size.height - 278.f;
        const float row7 = size.height - 322.f;
        const float row8 = size.height - 366.f;
        const float row9 = size.height - 410.f;
        m_fpsEnabledSprite = addButton(fpsBypassEnabled() ? "FPS ON" : "FPS OFF",
            menu_selector(VETERToolsPopup::onToggleFPS), left, row1,
            fpsBypassEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_retrySprite = addButton(autoRetryEnabled() ? "RETRY ON" : "RETRY OFF",
            menu_selector(VETERToolsPopup::onToggleRetry), right, row1,
            autoRetryEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        addButton("FPS -30", menu_selector(VETERToolsPopup::onFPSDown), left, row2, "GJ_button_01.png");
        m_fpsValueSprite = addButton("", menu_selector(VETERToolsPopup::onFPSInfo), center, row2, "GJ_button_02.png");
        addButton("FPS +30", menu_selector(VETERToolsPopup::onFPSUp), right, row2, "GJ_button_01.png");
        m_hitboxSprite = addButton(hitboxesEnabled() ? "HITBOXES ON" : "HITBOXES OFF",
            menu_selector(VETERToolsPopup::onToggleHitboxes), left, row3,
            hitboxesEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_cpsSprite = addButton(cpsEnabled() ? "CPS ON" : "CPS OFF",
            menu_selector(VETERToolsPopup::onToggleCPS), right, row3,
            cpsEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_practiceMusicSprite = addButton(practiceMusicEnabled() ? "PRACTICE MUSIC ON" : "PRACTICE MUSIC OFF",
            menu_selector(VETERToolsPopup::onTogglePracticeMusic), center, row4,
            practiceMusicEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_noclipLimitSprite = addButton(noclipLimitEnabled() ? "NC LIMIT ON" : "NC LIMIT OFF",
            menu_selector(VETERToolsPopup::onToggleNoclipLimit), left, row5,
            noclipLimitEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_noclipLimitValueSprite = addButton("", menu_selector(VETERToolsPopup::onChangeNoclipMax), right, row5, "GJ_button_01.png");
        m_noDeathEffectSprite = addButton(noDeathEffectEnabled() ? "NO DEATH FX ON" : "NO DEATH FX OFF",
            menu_selector(VETERToolsPopup::onToggleNoDeathEffect), left, row6,
            noDeathEffectEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_hitboxesOnDeathSprite = addButton(hitboxesOnDeathEnabled() ? "DEATH HITBOX ON" : "DEATH HITBOX OFF",
            menu_selector(VETERToolsPopup::onToggleHitboxesOnDeath), right, row6,
            hitboxesOnDeathEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_frameStepSprite = addButton(frameStepEnabled() ? "FRAME STEP ON" : "FRAME STEP OFF",
            menu_selector(VETERToolsPopup::onToggleFrameStep), left, row7,
            frameStepEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        addButton("STEP +1", menu_selector(VETERToolsPopup::onStepFrame), right, row7, "GJ_button_01.png");
        addButton("CP PREV", menu_selector(VETERToolsPopup::onPreviousCheckpoint), left, row8, "GJ_button_01.png");
        addButton("CP NEXT", menu_selector(VETERToolsPopup::onNextCheckpoint), right, row8, "GJ_button_01.png");
        m_layoutModeSprite = addButton(layoutModeEnabled() ? "LAYOUT GRID ON" : "LAYOUT GRID OFF",
            menu_selector(VETERToolsPopup::onToggleLayoutMode), center, row9,
            layoutModeEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_confirmExitSprite = addButton(confirmExitEnabled() ? "CONFIRM EXIT ON" : "CONFIRM EXIT OFF",
            menu_selector(VETERToolsPopup::onToggleConfirmExit), left, 95.f,
            confirmExitEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        addButton("VISUALS", menu_selector(VETERToolsPopup::onVisuals), center, 95.f, "GJ_button_01.png");
        addButton("EDITOR", menu_selector(VETERToolsPopup::onEditor), right, 95.f, "GJ_button_01.png");
        addButton("FAST RESET", menu_selector(VETERToolsPopup::onFastReset), left, 60.f, "GJ_button_01.png");
        addButton("UNLOCKS", menu_selector(VETERToolsPopup::onUnlocks), right, 60.f, "GJ_button_01.png");
        addButton("BACK", menu_selector(VETERToolsPopup::onBack), center, 27.f, "GJ_button_01.png");
        refreshLabels();
        return true;
    }

public:
    static VETERToolsPopup* create() {
        auto* popup = new VETERToolsPopup();
        if (popup && popup->init()) { popup->autorelease(); return popup; }
        delete popup;
        return nullptr;
    }

    void onToggleFPS(CCObject*) {
        setFPSBypassEnabled(!fpsBypassEnabled());
        applyFPSBypass();
        refreshLabels();
        notify(std::string("VETER: FPS bypass ") + (fpsBypassEnabled() ? "ON" : "OFF"));
    }
    void onFPSDown(CCObject*) {
        setTargetFPS(targetFPS() - 30);
        applyFPSBypass();
        refreshLabels();
    }
    void onFPSUp(CCObject*) {
        setTargetFPS(targetFPS() + 30);
        applyFPSBypass();
        refreshLabels();
    }
    void onFPSInfo(CCObject*) {
        notify("FPS target: " + std::to_string(targetFPS()) + ". Actual FPS is limited by the device/display.");
    }
    void onToggleRetry(CCObject*) {
        setAutoRetryEnabled(!autoRetryEnabled());
        refreshLabels();
        notify(std::string("VETER: Auto-Retry ") + (autoRetryEnabled() ? "ON" : "OFF"));
    }
    void onToggleHitboxes(CCObject*) {
        const bool enabled = !hitboxesEnabled();
        setHitboxesEnabled(enabled);
        auto* layer = PlayLayer::get();
        if (layer) {
            layer->toggleDebugDraw();
            g_debugDrawLayer = layer;
        }
        refreshLabels();
        notify(enabled ? "VETER: Hitboxes ON" : "VETER: Hitboxes OFF");
    }
    void onToggleCPS(CCObject*) {
        setCPSEnabled(!cpsEnabled());
        if (!cpsEnabled() && g_cpsLabel) g_cpsLabel->setVisible(false);
        refreshLabels();
        notify(std::string("VETER: CPS ") + (cpsEnabled() ? "ON" : "OFF"));
    }
    void onTogglePracticeMusic(CCObject*) {
        auto* layer = PlayLayer::get();
        if (!layer) {
            notify("Practice Music Hack: enter a level first");
            return;
        }
        const bool enabled = !practiceMusicEnabled();
        layer->toggleMusicInPractice();
        setPracticeMusicEnabled(enabled);
        g_practiceMusicLayer = layer;
        refreshLabels();
        notify(enabled ? "Practice Music Hack: toggled ON" : "Practice Music Hack: toggled OFF");
    }
    void onFastReset(CCObject*) {
        auto* layer = PlayLayer::get();
        if (!layer) {
            notify("Fast Reset: enter a level first");
            onClose(nullptr);
            return;
        }
        stopBot(false);
        onClose(nullptr);
        layer->resetLevel();
    }
    void onToggleNoclipLimit(CCObject*) {
        setNoclipLimitEnabled(!noclipLimitEnabled());
        refreshLabels();
        notify(std::string("Noclip limit ") + (noclipLimitEnabled() ? "ON" : "OFF"));
    }
    void onChangeNoclipMax(CCObject*) {
        int next = noclipMaxDeaths() + 1;
        if (next > 10) next = 1;
        setNoclipMaxDeaths(next);
        refreshLabels();
        notify("Noclip collision limit: " + std::to_string(noclipMaxDeaths()));
    }
    void onToggleNoDeathEffect(CCObject*) {
        setNoDeathEffectEnabled(!noDeathEffectEnabled());
        refreshLabels();
        notify(std::string("No Death Effect ") + (noDeathEffectEnabled() ? "ON" : "OFF"));
    }
    void onToggleHitboxesOnDeath(CCObject*) {
        setHitboxesOnDeathEnabled(!hitboxesOnDeathEnabled());
        refreshLabels();
        notify(std::string("Show Hitboxes on Death ") + (hitboxesOnDeathEnabled() ? "ON" : "OFF"));
    }
    void onToggleFrameStep(CCObject*) {
        setFrameStepEnabled(!frameStepEnabled());
        g_frameStepPending = false;
        refreshLabels();
        notify(frameStepEnabled() ? "Frame Step ON: game updates only after STEP +1" : "Frame Step OFF");
    }
    void onStepFrame(CCObject*) {
        if (!frameStepEnabled()) {
            notify("Turn FRAME STEP ON first");
            return;
        }
        g_frameStepPending = true;
        notify("Frame Step: one update queued");
    }
    void onPreviousCheckpoint(CCObject*) { cycleCheckpoint(-1); }
    void onNextCheckpoint(CCObject*) { cycleCheckpoint(1); }
    void cycleCheckpoint(int direction) {
        auto* layer = PlayLayer::get();
        if (!layer || !layer->m_checkpointArray || layer->m_checkpointArray->count() == 0) {
            notify("Checkpoint Switcher: no checkpoints yet; use Practice Mode first");
            return;
        }
        const unsigned int count = layer->m_checkpointArray->count();
        if (g_checkpointIndex < 0 || g_checkpointIndex >= static_cast<int>(count)) {
            g_checkpointIndex = direction > 0 ? 0 : static_cast<int>(count) - 1;
        } else {
            g_checkpointIndex = (g_checkpointIndex + direction + static_cast<int>(count)) % static_cast<int>(count);
        }
        auto* checkpoint = static_cast<CheckpointObject*>(layer->m_checkpointArray->objectAtIndex(g_checkpointIndex));
        if (checkpoint) {
            layer->loadFromCheckpoint(checkpoint);
            notify("Checkpoint " + std::to_string(g_checkpointIndex + 1) + "/" + std::to_string(count));
        }
    }
    void onToggleConfirmExit(CCObject*) {
        setConfirmExitEnabled(!confirmExitEnabled());
        refreshLabels();
        notify(std::string("Confirm Exit ") + (confirmExitEnabled() ? "ON" : "OFF"));
    }
    void onToggleLayoutMode(CCObject*) {
        setLayoutModeEnabled(!layoutModeEnabled());
        refreshLabels();
        notify(layoutModeEnabled() ? "Layout Mode ON: 32px guide grid in levels" : "Layout Mode OFF");
    }
    void onUnlocks(CCObject*);
    void onEditor(CCObject*);
    void onVisuals(CCObject*);
    void onBack(CCObject*) {
        onClose(nullptr);
        if (auto* popup = VETERPopup::create()) popup->show();
    }
};

class VETERUnlockPopup : public Popup {
private:
    ButtonSprite* m_unlockAllSprite = nullptr;
    ButtonSprite* m_iconKitSprite = nullptr;
    ButtonSprite* m_mainLevelsSprite = nullptr;
    ButtonSprite* m_shopSprite = nullptr;
    ButtonSprite* m_vaultSprite = nullptr;
    ButtonSprite* m_textLengthSprite = nullptr;

    void refreshLabels() {
        if (m_unlockAllSprite) m_unlockAllSprite->setString(unlockAllEnabled() ? "UNLOCK ALL ON" : "UNLOCK ALL OFF");
        if (m_iconKitSprite) m_iconKitSprite->setString(iconKitBypassEnabled() ? "ICON KIT ON" : "ICON KIT OFF");
        if (m_mainLevelsSprite) m_mainLevelsSprite->setString(mainLevelsBypassEnabled() ? "MAIN LVL ON" : "MAIN LVL OFF");
        if (m_shopSprite) m_shopSprite->setString(shopUnlockBypassEnabled() ? "SHOP ON" : "SHOP OFF");
        if (m_vaultSprite) m_vaultSprite->setString(vaultBypassEnabled() ? "VAULT ON" : "VAULT OFF");
        if (m_textLengthSprite) m_textLengthSprite->setString(textLengthBypassEnabled() ? "TEXT LIMIT ON" : "TEXT LIMIT OFF");
    }

protected:
    bool init() {
        if (!Popup::init(350.f, 295.f)) return false;
        setTitle("VETER - Unlocks");
        auto* menu = CCMenu::create();
        if (!menu || !m_mainLayer) return false;
        menu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(menu);
        const auto size = m_mainLayer->getContentSize();
        auto addButton = [&](const std::string& text, SEL_MenuHandler selector, float x, float y, const char* frame) -> ButtonSprite* {
            auto* sprite = ButtonSprite::create(text.c_str(), "bigFont.fnt", frame, 0.30f);
            if (!sprite) return nullptr;
            auto* item = CCMenuItemSpriteExtra::create(sprite, this, selector);
            if (!item) return nullptr;
            item->setPosition({x, y});
            menu->addChild(item);
            return sprite;
        };
        const float left = size.width * 0.25f;
        const float center = size.width * 0.50f;
        const float right = size.width * 0.75f;
        m_unlockAllSprite = addButton("", menu_selector(VETERUnlockPopup::onToggleUnlockAll), center, size.height - 50.f,
            unlockAllEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_iconKitSprite = addButton("", menu_selector(VETERUnlockPopup::onToggleIconKit), left, size.height - 90.f,
            iconKitBypassEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_mainLevelsSprite = addButton("", menu_selector(VETERUnlockPopup::onToggleMainLevels), right, size.height - 90.f,
            mainLevelsBypassEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_shopSprite = addButton("", menu_selector(VETERUnlockPopup::onToggleShop), left, size.height - 130.f,
            shopUnlockBypassEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_vaultSprite = addButton("", menu_selector(VETERUnlockPopup::onToggleVault), right, size.height - 130.f,
            vaultBypassEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_textLengthSprite = addButton("", menu_selector(VETERUnlockPopup::onToggleTextLength), center, size.height - 170.f,
            textLengthBypassEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        addButton("INFO", menu_selector(VETERUnlockPopup::onInfo), left, size.height - 215.f, "GJ_button_01.png");
        addButton("BACK", menu_selector(VETERUnlockPopup::onBack), right, size.height - 215.f, "GJ_button_01.png");
        refreshLabels();
        return true;
    }
public:
    static VETERUnlockPopup* create() {
        auto* popup = new VETERUnlockPopup();
        if (popup && popup->init()) { popup->autorelease(); return popup; }
        delete popup;
        return nullptr;
    }
    void onToggleUnlockAll(CCObject*) {
        setUnlockAllEnabled(!unlockAllEnabled()); refreshLabels();
        notify(std::string("Unlock All checks ") + (unlockAllEnabled() ? "ON" : "OFF"));
    }
    void onToggleShop(CCObject*) {
        setShopUnlockBypassEnabled(!shopUnlockBypassEnabled()); refreshLabels();
        notify(std::string("Shop unlock check ") + (shopUnlockBypassEnabled() ? "ON" : "OFF"));
    }
    void onToggleVault(CCObject*) {
        setVaultBypassEnabled(!vaultBypassEnabled()); refreshLabels();
        notify(std::string("Vault unlock check ") + (vaultBypassEnabled() ? "ON" : "OFF"));
    }
    void onToggleIconKit(CCObject*) {
        setIconKitBypassEnabled(!iconKitBypassEnabled()); refreshLabels();
        notify(std::string("Icon Kit local unlock check ") + (iconKitBypassEnabled() ? "ON" : "OFF"));
    }
    void onToggleMainLevels(CCObject*) {
        setMainLevelsBypassEnabled(!mainLevelsBypassEnabled()); refreshLabels();
        notify(std::string("Main Levels local check ") + (mainLevelsBypassEnabled() ? "ON" : "OFF"));
    }
    void onToggleTextLength(CCObject*) {
        setTextLengthBypassEnabled(!textLengthBypassEnabled()); refreshLabels();
        notify(std::string("Text Length Bypass ") + (textLengthBypassEnabled() ? "ON" : "OFF") + " (new text fields)");
    }
    void onInfo(CCObject*) {
        FLAlertLayer::create("Unlock features", "These switches override local unlock-status checks. They do not grant currency, save permanent progress, or guarantee every shop/vault screen accepts the bypass.", "OK")->show();
    }
    void onBack(CCObject*);
};

void VETERToolsPopup::onUnlocks(CCObject*) {
    onClose(nullptr);
    if (auto* popup = VETERUnlockPopup::create()) popup->show();
}

void VETERUnlockPopup::onBack(CCObject*) {
    onClose(nullptr);
    if (auto* popup = VETERPopup::create()) popup->show();
}

void VETERPopup::onMore(CCObject*) {
    onClose(nullptr);
    if (auto* popup = VETERToolsPopup::create()) popup->show();
}

class VETEREditorPopup;
void reopenEditorPopup();

class VETERAbsolutePositionPopup : public Popup, public TextInputDelegate {
private:
    CCTextInputNode* m_xInput = nullptr;
    CCTextInputNode* m_yInput = nullptr;

protected:
    bool init() {
        if (!Popup::init(330.f, 230.f)) return false;
        setTitle("Absolute X / Y");
        const auto size = m_mainLayer->getContentSize();

        auto* xLabel = CCLabelBMFont::create("X", "bigFont.fnt");
        auto* yLabel = CCLabelBMFont::create("Y", "bigFont.fnt");
        if (!xLabel || !yLabel) return false;
        xLabel->setScale(0.55f);
        yLabel->setScale(0.55f);
        xLabel->setPosition({size.width * 0.13f, size.height - 82.f});
        yLabel->setPosition({size.width * 0.53f, size.height - 82.f});
        m_mainLayer->addChild(xLabel, 2);
        m_mainLayer->addChild(yLabel, 2);

        m_xInput = CCTextInputNode::create(105.f, 25.f, "X coordinate", "bigFont.fnt");
        m_yInput = CCTextInputNode::create(105.f, 25.f, "Y coordinate", "bigFont.fnt");
        if (!m_xInput || !m_yInput) return false;
        m_xInput->setPosition({size.width * 0.31f, size.height - 82.f});
        m_yInput->setPosition({size.width * 0.72f, size.height - 82.f});
        m_xInput->setDelegate(this);
        m_yInput->setDelegate(this);
        const int maxTextLength = textLengthBypassEnabled() ? 1000000 : 16;
        m_xInput->setMaxLabelLength(maxTextLength);
        m_yInput->setMaxLabelLength(maxTextLength);
        m_xInput->setString("0");
        m_yInput->setString("0");
        m_mainLayer->addChild(m_xInput, 3);
        m_mainLayer->addChild(m_yInput, 3);

        auto* menu = CCMenu::create();
        if (!menu) return false;
        menu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(menu);
        auto addButton = [&](const char* label, SEL_MenuHandler selector, float x) {
            auto* sprite = ButtonSprite::create(label, "bigFont.fnt", "GJ_button_01.png", 0.32f);
            if (!sprite) return;
            auto* item = CCMenuItemSpriteExtra::create(sprite, this, selector);
            if (!item) return;
            item->setPosition({x, 34.f});
            menu->addChild(item);
        };
        addButton("APPLY", menu_selector(VETERAbsolutePositionPopup::onApply), size.width * 0.32f);
        addButton("BACK", menu_selector(VETERAbsolutePositionPopup::onBack), size.width * 0.68f);
        return true;
    }

public:
    static VETERAbsolutePositionPopup* create() {
        auto* popup = new VETERAbsolutePositionPopup();
        if (popup && popup->init()) { popup->autorelease(); return popup; }
        delete popup;
        return nullptr;
    }
    void textChanged(CCTextInputNode*) override {}

    void onApply(CCObject*) {
        std::string xText;
        std::string yText;
        if (m_xInput) xText = m_xInput->getString();
        if (m_yInput) yText = m_yInput->getString();
        std::istringstream xStream(xText);
        std::istringstream yStream(yText);
        float x = 0.f;
        float y = 0.f;
        if (!(xStream >> x) || !(yStream >> y) || !std::isfinite(x) || !std::isfinite(y)) {
            notify("Absolute X/Y: enter valid numbers");
            return;
        }
        xStream >> std::ws;
        yStream >> std::ws;
        if (!xStream.eof() || !yStream.eof()) {
            notify("Absolute X/Y: enter only numeric values");
            return;
        }
        auto* editorLayer = LevelEditorLayer::get();
        auto* editor = editorLayer ? editorLayer->m_editorUI : nullptr;
        auto* selected = editor ? editor->getSelectedObjects() : nullptr;
        if (!selected || selected->count() == 0) {
            notify("Absolute X/Y: select one or more objects in the editor first");
            return;
        }
        unsigned int changed = 0;
        for (unsigned int i = 0; i < selected->count(); ++i) {
            auto* object = static_cast<GameObject*>(selected->objectAtIndex(i));
            if (!object) continue;
            object->setPosition({x, y});
            ++changed;
        }
        notify("Absolute X/Y: set " + std::to_string(changed) + " object(s) to X=" + xText + ", Y=" + yText);
        onClose(nullptr);
        reopenEditorPopup();
    }
    void onBack(CCObject*) {
        onClose(nullptr);
        reopenEditorPopup();
    }
};

class VETEREditorPopup : public Popup {
private:
    ButtonSprite* m_moveStepSprite = nullptr;

    float moveStep() const {
        float value = Mod::get()->getSavedValue<float>("editor-move-step", 1.f);
        if (value < 0.5f) return 0.1f;
        if (value < 5.f) return 1.f;
        return 10.f;
    }
    float scaleStep() const {
        float value = Mod::get()->getSavedValue<float>("editor-scale-step", 0.1f);
        return std::clamp(value, 0.01f, 1.f);
    }
    void refreshLabels() {
        if (!m_moveStepSprite) return;
        char label[32];
        std::snprintf(label, sizeof(label), "MOVE STEP %.1f", static_cast<double>(moveStep()));
        m_moveStepSprite->setString(label);
    }
    void cycleMoveStep() {
        const float current = moveStep();
        const float next = current < 0.5f ? 1.f : (current < 5.f ? 10.f : 0.1f);
        Mod::get()->setSavedValue("editor-move-step", next);
        refreshLabels();
        notify("Editor move step set to " + std::to_string(next));
    }
    void modifySelectedObjects(float dx, float dy, float scaleDelta, float rotationDelta) {
        auto* editorLayer = LevelEditorLayer::get();
        auto* editor = editorLayer ? editorLayer->m_editorUI : nullptr;
        auto* selected = editor ? editor->getSelectedObjects() : nullptr;
        if (!selected || selected->count() == 0) {
            notify("Editor tools: select one or more objects first");
            return;
        }
        unsigned int changed = 0;
        for (unsigned int i = 0; i < selected->count(); ++i) {
            auto* object = static_cast<GameObject*>(selected->objectAtIndex(i));
            if (!object) continue;
            if (dx != 0.f || dy != 0.f) {
                object->setPosition({object->getPositionX() + dx, object->getPositionY() + dy});
            }
            if (scaleDelta != 0.f) {
                object->setScale(std::clamp(object->getScale() + scaleDelta, 0.1f, 100.f));
            }
            if (rotationDelta != 0.f) {
                object->setRotation(object->getRotation() + rotationDelta);
            }
            ++changed;
        }
        if (changed == 0) notify("Editor tools: no editable objects selected");
        else notify("Editor tools: modified " + std::to_string(changed) + " object(s)");
    }

protected:
    bool init() {
        if (!Popup::init(350.f, 315.f)) return false;
        setTitle("VETER - Editor");
        auto* menu = CCMenu::create();
        if (!menu || !m_mainLayer) return false;
        menu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(menu);
        const auto size = m_mainLayer->getContentSize();
        const float left = size.width * 0.25f;
        const float center = size.width * 0.50f;
        const float right = size.width * 0.75f;
        auto addButton = [&](const std::string& text, SEL_MenuHandler selector,
                             float x, float y, const char* frame) -> ButtonSprite* {
            auto* sprite = ButtonSprite::create(text.c_str(), "bigFont.fnt", frame, 0.23f);
            if (!sprite) return nullptr;
            auto* item = CCMenuItemSpriteExtra::create(sprite, this, selector);
            if (!item) return nullptr;
            item->setPosition({x, y});
            menu->addChild(item);
            return sprite;
        };
        const float row1 = size.height - 48.f;
        const float row2 = size.height - 79.f;
        const float row3 = size.height - 110.f;
        const float row4 = size.height - 141.f;
        const float row5 = size.height - 172.f;
        const float row6 = size.height - 203.f;
        const float row7 = size.height - 234.f;
        const float row8 = size.height - 265.f;
        const float row9 = 28.f;
        addButton("X -", menu_selector(VETEREditorPopup::onXMinus), left, row1, "GJ_button_01.png");
        addButton("X +", menu_selector(VETEREditorPopup::onXPlus), right, row1, "GJ_button_01.png");
        addButton("Y -", menu_selector(VETEREditorPopup::onYMinus), left, row2, "GJ_button_01.png");
        addButton("Y +", menu_selector(VETEREditorPopup::onYPlus), right, row2, "GJ_button_01.png");
        addButton("ROT -15", menu_selector(VETEREditorPopup::onRotateMinus15), left, row3, "GJ_button_01.png");
        addButton("ROT +15", menu_selector(VETEREditorPopup::onRotatePlus15), right, row3, "GJ_button_01.png");
        addButton("ROT -90", menu_selector(VETEREditorPopup::onRotateMinus90), left, row4, "GJ_button_01.png");
        addButton("ROT +90", menu_selector(VETEREditorPopup::onRotatePlus90), right, row4, "GJ_button_01.png");
        addButton("SCALE -", menu_selector(VETEREditorPopup::onScaleMinus), left, row5, "GJ_button_01.png");
        addButton("SCALE +", menu_selector(VETEREditorPopup::onScalePlus), right, row5, "GJ_button_01.png");
        m_moveStepSprite = addButton("MOVE STEP 1.0", menu_selector(VETEREditorPopup::onCycleMoveStep), center, row6, "GJ_button_02.png");
        addButton("ABSOLUTE X/Y", menu_selector(VETEREditorPopup::onAbsolutePosition), center, row7, "GJ_button_02.png");
        addButton("COPY XFORM", menu_selector(VETEREditorPopup::onCopyTransform), left, row8, "GJ_button_01.png");
        addButton("PASTE XFORM", menu_selector(VETEREditorPopup::onPasteTransform), right, row8, "GJ_button_01.png");
        addButton("INFO", menu_selector(VETEREditorPopup::onInfo), left, row9, "GJ_button_01.png");
        addButton("BACK", menu_selector(VETEREditorPopup::onBack), right, row9, "GJ_button_01.png");
        refreshLabels();
        return true;
    }
public:
    static VETEREditorPopup* create() {
        auto* popup = new VETEREditorPopup();
        if (popup && popup->init()) { popup->autorelease(); return popup; }
        delete popup;
        return nullptr;
    }
    void onXMinus(CCObject*) { modifySelectedObjects(-moveStep(), 0.f, 0.f, 0.f); }
    void onXPlus(CCObject*) { modifySelectedObjects(moveStep(), 0.f, 0.f, 0.f); }
    void onYMinus(CCObject*) { modifySelectedObjects(0.f, -moveStep(), 0.f, 0.f); }
    void onYPlus(CCObject*) { modifySelectedObjects(0.f, moveStep(), 0.f, 0.f); }
    void onRotateMinus15(CCObject*) { modifySelectedObjects(0.f, 0.f, 0.f, -15.f); }
    void onRotatePlus15(CCObject*) { modifySelectedObjects(0.f, 0.f, 0.f, 15.f); }
    void onRotateMinus90(CCObject*) { modifySelectedObjects(0.f, 0.f, 0.f, -90.f); }
    void onRotatePlus90(CCObject*) { modifySelectedObjects(0.f, 0.f, 0.f, 90.f); }
    void onScaleMinus(CCObject*) { modifySelectedObjects(0.f, 0.f, -scaleStep(), 0.f); }
    void onScalePlus(CCObject*) { modifySelectedObjects(0.f, 0.f, scaleStep(), 0.f); }
    void onCopyTransform(CCObject*) {
        auto* editorLayer = LevelEditorLayer::get();
        auto* editor = editorLayer ? editorLayer->m_editorUI : nullptr;
        auto* selected = editor ? editor->getSelectedObjects() : nullptr;
        if (!selected || selected->count() == 0) {
            notify("Copy Transform: select an object first");
            return;
        }
        auto* object = static_cast<GameObject*>(selected->objectAtIndex(0));
        if (!object) {
            notify("Copy Transform: selected object is unavailable");
            return;
        }
        Mod::get()->setSavedValue("editor-transform-x", object->getPositionX());
        Mod::get()->setSavedValue("editor-transform-y", object->getPositionY());
        Mod::get()->setSavedValue("editor-transform-rotation", object->getRotation());
        Mod::get()->setSavedValue("editor-transform-scale", object->getScale());
        Mod::get()->setSavedValue("editor-transform-valid", true);
        notify("Editor: transform copied from first selected object");
    }
    void onPasteTransform(CCObject*) {
        if (!Mod::get()->getSavedValue<bool>("editor-transform-valid", false)) {
            notify("Paste Transform: copy a transform first");
            return;
        }
        auto* editorLayer = LevelEditorLayer::get();
        auto* editor = editorLayer ? editorLayer->m_editorUI : nullptr;
        auto* selected = editor ? editor->getSelectedObjects() : nullptr;
        if (!selected || selected->count() == 0) {
            notify("Paste Transform: select one or more objects first");
            return;
        }
        const float x = Mod::get()->getSavedValue<float>("editor-transform-x", 0.f);
        const float y = Mod::get()->getSavedValue<float>("editor-transform-y", 0.f);
        const float rotation = Mod::get()->getSavedValue<float>("editor-transform-rotation", 0.f);
        const float scale = Mod::get()->getSavedValue<float>("editor-transform-scale", 1.f);
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(rotation) ||
            !std::isfinite(scale) || scale < 0.1f || scale > 100.f) {
            notify("Paste Transform: saved transform is invalid");
            return;
        }
        unsigned int changed = 0;
        for (unsigned int i = 0; i < selected->count(); ++i) {
            auto* object = static_cast<GameObject*>(selected->objectAtIndex(i));
            if (!object) continue;
            object->setPosition({x, y});
            object->setRotation(rotation);
            object->setScale(scale);
            ++changed;
        }
        notify("Editor: pasted transform to " + std::to_string(changed) + " object(s)");
    }
    void onCycleMoveStep(CCObject*) { cycleMoveStep(); }
    void onAbsolutePosition(CCObject*) {
        onClose(nullptr);
        if (auto* popup = VETERAbsolutePositionPopup::create()) popup->show();
    }
    void onInfo(CCObject*) {
        FLAlertLayer::create("Editor helpers", "Select objects first. X/Y nudge the selection by 0.1, 1 or 10 units; MOVE STEP changes the amount. ROT buttons rotate selected objects by 15 or 90 degrees. SCALE changes uniform scale by 0.1. ABSOLUTE X/Y opens numeric fields to set the selected objects at the exact entered coordinate. COPY TRANSFORM saves position/rotation/scale from the first selected object; PASTE TRANSFORM applies that transform to every selected object.", "OK")->show();
    }
    void onBack(CCObject*) {
        onClose(nullptr);
        if (auto* popup = VETERPopup::create()) popup->show();
    }
};

void reopenEditorPopup() {
    if (auto* popup = VETEREditorPopup::create()) popup->show();
}

class VETERVisualsPopup : public Popup {
    ButtonSprite* m_trajectorySprite = nullptr;
    ButtonSprite* m_predictionSprite = nullptr;
    ButtonSprite* m_clicksSprite = nullptr;
    ButtonSprite* m_fpsCounterSprite = nullptr;
    ButtonSprite* m_customMusicSprite = nullptr;
    ButtonSprite* m_transparentPauseSprite = nullptr;
    ButtonSprite* m_hidePauseSprite = nullptr;
    void refreshLabels() {
        if (m_trajectorySprite) m_trajectorySprite->setString(trajectoryEnabled() ? "TRAIL ON" : "TRAIL OFF");
        if (m_predictionSprite) m_predictionSprite->setString(trajectoryPredictorEnabled() ? "PREDICT ON" : "PREDICT OFF");
        if (m_clicksSprite) m_clicksSprite->setString(showClicksEnabled() ? "CLICKS ON" : "CLICKS OFF");
        if (m_fpsCounterSprite) m_fpsCounterSprite->setString(fpsCounterEnabled() ? "FPS COUNT ON" : "FPS COUNT OFF");
        if (m_customMusicSprite) m_customMusicSprite->setString(customMenuMusicEnabled() ? "CUSTOM MUSIC ON" : "CUSTOM MUSIC OFF");
        if (m_transparentPauseSprite) m_transparentPauseSprite->setString(transparentPauseEnabled() ? "PAUSE ALPHA ON" : "PAUSE ALPHA OFF");
        if (m_hidePauseSprite) m_hidePauseSprite->setString(hidePauseButtonEnabled() ? "HIDE PAUSE ON" : "HIDE PAUSE OFF");
    }
protected:
    bool init() {
        if (!Popup::init(390.f, 425.f)) return false;
        setTitle("VETER - Visuals");
        auto* menu = CCMenu::create();
        if (!menu || !m_mainLayer) return false;
        menu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(menu);
        const auto size = m_mainLayer->getContentSize();
        const float left = size.width * 0.35f;
        const float center = size.width * 0.50f;
        const float right = size.width * 0.65f;
        auto addButton = [&](const std::string& text, SEL_MenuHandler selector,
                             float x, float y, const char* frame) -> ButtonSprite* {
            auto* sprite = ButtonSprite::create(text.c_str(), "bigFont.fnt", frame, 0.30f);
            if (!sprite) return nullptr;
            auto* item = CCMenuItemSpriteExtra::create(sprite, this, selector);
            if (!item) return nullptr;
            item->setPosition({x, y});
            menu->addChild(item);
            return sprite;
        };
        m_trajectorySprite = addButton("", menu_selector(VETERVisualsPopup::onToggleTrajectory), left, size.height - 58.f,
            trajectoryEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_predictionSprite = addButton("", menu_selector(VETERVisualsPopup::onTogglePrediction), right, size.height - 58.f,
            trajectoryPredictorEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_clicksSprite = addButton("", menu_selector(VETERVisualsPopup::onToggleClicks), left, size.height - 112.f,
            showClicksEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_fpsCounterSprite = addButton("", menu_selector(VETERVisualsPopup::onToggleFPSCounter), right, size.height - 112.f,
            fpsCounterEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_customMusicSprite = addButton("", menu_selector(VETERVisualsPopup::onToggleCustomMusic), center, size.height - 166.f,
            customMenuMusicEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_transparentPauseSprite = addButton("", menu_selector(VETERVisualsPopup::onToggleTransparentPause), left, size.height - 220.f,
            transparentPauseEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_hidePauseSprite = addButton("", menu_selector(VETERVisualsPopup::onToggleHidePause), right, size.height - 220.f,
            hidePauseButtonEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        addButton("MUSIC FILE INFO", menu_selector(VETERVisualsPopup::onMusicInfo), center, size.height - 278.f, "GJ_button_01.png");
        addButton("BACK", menu_selector(VETERVisualsPopup::onBack), center, 28.f, "GJ_button_01.png");
        refreshLabels();
        return true;
    }
public:
    static VETERVisualsPopup* create() {
        auto* popup = new VETERVisualsPopup();
        if (popup && popup->init()) { popup->autorelease(); return popup; }
        delete popup;
        return nullptr;
    }
    void onToggleTrajectory(CCObject*) {
        setTrajectoryEnabled(!trajectoryEnabled());
        if (!trajectoryEnabled() && !trajectoryPredictorEnabled() && g_trajectoryDrawNode) {
            g_trajectoryDrawNode->removeFromParentAndCleanup(true);
            g_trajectoryDrawNode = nullptr;
            g_trajectoryLayer = nullptr;
            g_trajectoryPoints.clear();
        }
        refreshLabels();
        notify(std::string("Position trail ") + (trajectoryEnabled() ? "ON" : "OFF"));
    }
    void onTogglePrediction(CCObject*) {
        setTrajectoryPredictorEnabled(!trajectoryPredictorEnabled());
        if (!trajectoryEnabled() && !trajectoryPredictorEnabled() && g_trajectoryDrawNode) {
            g_trajectoryDrawNode->removeFromParentAndCleanup(true);
            g_trajectoryDrawNode = nullptr;
            g_trajectoryLayer = nullptr;
            g_trajectoryPoints.clear();
        }
        refreshLabels();
        notify(trajectoryPredictorEnabled()
            ? "Trajectory prediction ON (approximate; game triggers can change it)"
            : "Trajectory prediction OFF");
    }
    void onToggleClicks(CCObject*) {
        setShowClicksEnabled(!showClicksEnabled());
        if (!showClicksEnabled() && g_clickLabel) g_clickLabel->setVisible(false);
        refreshLabels();
        notify(std::string("Show Clicks ") + (showClicksEnabled() ? "ON" : "OFF"));
    }
    void onToggleFPSCounter(CCObject*) {
        setFPSCounterEnabled(!fpsCounterEnabled());
        if (!fpsCounterEnabled() && g_fpsCounterLabel) g_fpsCounterLabel->setVisible(false);
        refreshLabels();
        notify(fpsCounterEnabled()
            ? "Game FPS counter ON (measures PlayLayer updates)"
            : "Game FPS counter OFF");
    }
    void onToggleCustomMusic(CCObject*) {
        if (!customMenuMusicEnabled()) {
            std::error_code ec;
            if (!std::filesystem::exists(customMenuMusicFile(), ec) || ec) {
                notify("Copy custom-menu.mp3 into VETER save folder first");
                return;
            }
            setCustomMenuMusicEnabled(true);
        } else {
            setCustomMenuMusicEnabled(false);
        }
        applyCustomMenuMusic();
        refreshLabels();
        notify(std::string("Custom menu music ") + (customMenuMusicEnabled() ? "ON" : "OFF"));
    }
    void onToggleTransparentPause(CCObject*) {
        setTransparentPauseEnabled(!transparentPauseEnabled());
        refreshLabels();
        notify(std::string("Transparent Pause ") + (transparentPauseEnabled() ? "ON (next pause)" : "OFF (next pause)"));
    }
    void onToggleHidePause(CCObject*) {
        setHidePauseButtonEnabled(!hidePauseButtonEnabled());
        refreshLabels();
        notify(std::string("Hide Pause Button ") + (hidePauseButtonEnabled() ? "ON" : "OFF"));
    }
    void onMusicInfo(CCObject*) {
        const auto path = customMenuMusicFile().string();
        FLAlertLayer::create("Custom Menu Music", ("Place your own MP3 file here:\n" + path + "\nThen turn CUSTOM MUSIC ON from the main menu.").c_str(), "OK")->show();
    }
    void onBack(CCObject*) {
        onClose(nullptr);
        if (auto* popup = VETERPopup::create()) popup->show();
    }
};

void VETERPopup::onOpenEditor(CCObject*) {
    onClose(nullptr);
    if (auto* popup = VETEREditorPopup::create()) popup->show();
}

void VETERPopup::onOpenUnlocks(CCObject*) {
    onClose(nullptr);
    if (auto* popup = VETERUnlockPopup::create()) popup->show();
}

void VETERToolsPopup::onEditor(CCObject*) {
    onClose(nullptr);
    if (auto* popup = VETEREditorPopup::create()) popup->show();
}

void VETERToolsPopup::onVisuals(CCObject*) {
    onClose(nullptr);
    if (auto* popup = VETERVisualsPopup::create()) popup->show();
}

class VETEROpenDelegate : public CCNode {
public:
    static VETEROpenDelegate* create() {
        auto* node = new VETEROpenDelegate();
        if (node && node->init()) {
            node->autorelease();
            return node;
        }
        delete node;
        return nullptr;
    }

    void onOpen(CCObject*) {
        if (auto* popup = VETERPopup::create()) popup->show();
    }
};

namespace {
    void attachVETERButton(CCNode* scene) {
        if (!scene || scene->getChildByTag(kVETERButtonTag)) return;

        auto* menu = CCMenu::create();
        auto* delegate = VETEROpenDelegate::create();
        // Use a clear text button rather than a tiny icon: on some Android
        // resolutions the icon was nearly invisible / looked like an arrow.
        CCNode* buttonVisual = ButtonSprite::create(
            "VETER", "bigFont.fnt", "GJ_button_01.png", 0.65f
        );
        if (!menu || !delegate || !buttonVisual) return;

        auto* item = CCMenuItemSpriteExtra::create(
            buttonVisual, delegate, menu_selector(VETEROpenDelegate::onOpen)
        );
        if (!item) return;

        const auto size = CCDirector::sharedDirector()->getWinSize();
        menu->setTag(kVETERButtonTag);
        menu->setPosition({0.f, 0.f});
        item->setPosition({size.width - 76.f, size.height - 76.f});
        item->setScale(0.90f);
        menu->addChild(delegate);
        menu->addChild(item);
        scene->addChild(menu, 9999);
    }
}

// Attach directly to the main-menu layer as soon as it is initialized. This
// avoids relying exclusively on CCDirector::drawScene, which may be too late
// or inconsistent on some Android builds.
class $modify(VETERMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        attachVETERButton(this);
        return true;
    }
};

// Local-only unlock-status overrides. These do not add currency or alter server data.
class $modify(VETERGameStats, GameStatsManager) {
    bool isItemUnlocked(UnlockType type, int id) {
        if (unlockAllEnabled()) return true;
        // The first seven unlock categories in GD 2.2081 are the seven icon forms.
        const int typeIndex = static_cast<int>(type);
        if (iconKitBypassEnabled() && typeIndex >= 0 && typeIndex <= 6) return true;
        return GameStatsManager::isItemUnlocked(type, id);
    }
    bool hasCompletedMainLevel(int levelID) {
        // Local query override only: this does not save completion data.
        if (mainLevelsBypassEnabled() && levelID >= 1 && levelID <= 21) return true;
        return GameStatsManager::hasCompletedMainLevel(levelID);
    }
    bool isStoreItemUnlocked(int id) {
        if (shopUnlockBypassEnabled() || unlockAllEnabled()) return true;
        return GameStatsManager::isStoreItemUnlocked(id);
    }
    bool isSecretChestUnlocked(int id) {
        if (vaultBypassEnabled() || unlockAllEnabled()) return true;
        return GameStatsManager::isSecretChestUnlocked(id);
    }
    bool isGauntletChestUnlocked(int id) {
        if (vaultBypassEnabled() || unlockAllEnabled()) return true;
        return GameStatsManager::isGauntletChestUnlocked(id);
    }
    bool isSpecialChestUnlocked(gd::string id) {
        if (vaultBypassEnabled() || unlockAllEnabled()) return true;
        return GameStatsManager::isSpecialChestUnlocked(id);
    }
};

class $modify(VETERPlayerObject, PlayerObject) {
    void playDeathEffect() {
        if (noDeathEffectEnabled()) return;
        PlayerObject::playDeathEffect();
    }
};

class $modify(VETERPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();
        if (transparentPauseEnabled()) setPauseSpritesAlpha(this, 150);
    }
    void onQuit(CCObject* sender) {
        if (!confirmExitEnabled() || g_confirmExitProceed) {
            PauseLayer::onQuit(sender);
            return;
        }
        auto* pauseLayer = this;
        createQuickPopup("Confirm Exit", "Leave this level? Your current attempt will be lost.", "Stay", "Exit",
            [pauseLayer](FLAlertLayer*, bool confirmed) {
                if (!confirmed) return;
                g_confirmExitProceed = true;
                pauseLayer->onQuit(nullptr);
                g_confirmExitProceed = false;
            });
    }
};

// Preserve the pitch requested by Geometry Dash, while applying the Speedhack
// multiplier to the music channel when Music Speed Sync is enabled.
class $modify(VETERMusicAudioEngine, FMODAudioEngine) {
    void setChannelPitch(int id, AudioTargetType type, float pitch) {
        if (type == AudioTargetType::MusicChannel && std::isfinite(pitch) && pitch > 0.0f) {
            if (!g_settingMusicPitch) {
                g_musicBasePitchByChannel[id] = pitch;
                pitch = std::clamp(pitch * musicPitchScale(), 0.05f, 10.0f);
            }
            g_musicAppliedPitchByChannel[id] = pitch;
        }
        FMODAudioEngine::setChannelPitch(id, type, pitch);
    }
};

// Add the VETER quick-open button to each new scene, not just gameplay.
class $modify(VETERDirector, CCDirector) {
    void drawScene() {
        // Detect level transitions even if no more input arrives. This prevents
        // GD Bot from staying in RECORDING/PLAYING after its PlayLayer disappears.
        if (g_botMode != BotMode::Idle && g_botLayer && PlayLayer::get() != g_botLayer) {
            const auto oldMode = g_botMode;
            if (g_botContinueSession) {
                finalizeContinueSession();
            } else {
                stopBot(false);
            }
            notify(oldMode == BotMode::Recording
                ? "GD Bot: recording stopped because the level changed"
                : "GD Bot: playback stopped because the level changed");
        }

        // Enforce Music OFF in menus as well as in gameplay if the game starts
        // a new track after the toggle has been enabled.
        applyCustomMenuMusic();
        if (musicMuted()) applyMusicMute();
        applyMusicPitch();
        if (auto* scene = this->getRunningScene()) {
            attachVETERButton(scene);
            updateLayoutGrid(scene);
        }
        CCDirector::drawScene();
    }
};

// Capture input at Geometry Dash's common input dispatcher. This is more
// reliable for on-screen/touch controls than relying only on PlayerObject hooks.
class $modify(VETERBaseGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        if (isPlayer1 && !g_injectedInput && g_botMode == BotMode::Continuing) {
            // User gameplay inputs during Continue replay would alter the route before
            // the saved endpoint. UI buttons still work because they bypass this dispatcher.
            return;
        }
        if (isPlayer1 && !g_injectedInput) {
            recordInput(button, down);
            if (down && cpsEnabled()) ++g_cpsInputCount;
            if (down && showClicksEnabled()) g_clickFlashRemaining = 0.22f;
        }
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }
};

class $modify(VETERPlayLayer, PlayLayer) {
    void update(float dt) {
        // In frame-step mode, allow only one PlayLayer update per button press.
        if (frameStepEnabled() && !g_frameStepPending) return;
        if (frameStepEnabled()) g_frameStepPending = false;
        // Restore requested built-in visual/practice modes for each new level instance.
        if (g_debugDrawLayer != this) {
            g_debugDrawLayer = this;
            if (hitboxesEnabled()) this->toggleDebugDraw();
        }
        if (g_practiceMusicLayer != this) {
            g_practiceMusicLayer = this;
            if (practiceMusicEnabled()) this->toggleMusicInPractice();
        }

        applySpeedhack();
        if (musicMuted()) applyMusicMute();
        updateCPSOverlay(this, dt);
        updateFPSCounterOverlay(this);
        updateNoclipStatusOverlay(this);
        if (g_botMode == BotMode::Playing || g_botMode == BotMode::Continuing) processPlayback(this);
        PlayLayer::update(dt);
        const double safeFrameDelta = std::max(0.0, static_cast<double>(dt));
        finishContinuePlayback(this, safeFrameDelta);
        finishRegularPlayback(this);

        // Reset only after the current PlayLayer update has completed. This makes
        // BOT LOOP replay the macro from the actual start rather than applying its
        // first clicks to the prior run's end-state.
        if (g_botLoopRestartPending) {
            g_botLoopRestartPending = false;
            if (g_botMode == BotMode::Playing && g_botLayer == this &&
                PlayLayer::get() == this && m_player1) {
                g_botClock = 0.0;
                g_playIndex = 0;
                g_playbackHeldButtons.clear();
                g_botContinueHandoffPending = false;
                g_botPlaybackFinishPending = false;
                g_botClockAdvancedThisFrame = true;
                this->resetLevel();
            } else if (g_botMode == BotMode::Playing) {
                stopBot(false);
                notify("GD Bot loop stopped because the level ended or changed");
            }
        }

        updateTrajectoryTrail(this);
        updateClickIndicator(this, dt);
        updatePauseButtonVisibility(this);

        if (g_botMode == BotMode::Recording) {
            if (!g_botClockAdvancedThisFrame) g_botClock += safeFrameDelta;
            g_botClockAdvancedThisFrame = false;
        } else if (g_botMode == BotMode::Playing || g_botMode == BotMode::Continuing) {
            if (!g_botClockAdvancedThisFrame) {
                g_botClock += safeFrameDelta * static_cast<double>(botPlaybackRate());
            }
            g_botClockAdvancedThisFrame = false;
        } else {
            g_botClockAdvancedThisFrame = false;
        }
        if (g_autoRetryPending) {
            g_autoRetryPending = false;
            if (autoRetryEnabled() && PlayLayer::get() == this && m_player1) this->resetLevel();
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        const bool primaryPlayer = player && player == m_player1;
        if (primaryPlayer && noclipEnabled()) {
            ++g_noclipDeathCount;
            g_noclipMaxProgress = std::max(g_noclipMaxProgress, std::clamp(getCurrentPercentInt(), 0, 100));
            if (!noclipLimitEnabled() || g_noclipDeathCount <= noclipMaxDeaths()) {
                return;
            }
            setNoclipEnabled(false);
            notify("Noclip limit reached; normal death enabled");
        }
        if (primaryPlayer && (g_botMode == BotMode::Playing || g_botMode == BotMode::Continuing)) {
            if (autoRetryEnabled()) {
                // Auto-Retry and GD Bot must restart together. Previously the bot
                // stopped on death, so Auto-Retry reset the level with no macro running.
                releasePlaybackButtons();
                g_botClock = 0.0;
                g_playIndex = 0;
                g_playbackHeldButtons.clear();
                g_botContinueHandoffPending = false;
                g_botPlaybackFinishPending = false;
                g_botLoopRestartPending = false;
                // The reset triggered below is the start of a fresh timed attempt.
                g_botClockAdvancedThisFrame = true;
                notify("GD Bot: death detected; Auto-Retry will replay the macro from the start");
            } else {
                if (g_botContinueSession) finalizeContinueSession();
                else stopBot(false);
                notify("GD Bot: playback stopped after death (turn on Auto-Retry to retry the macro)");
            }
        } else if (primaryPlayer && g_botMode == BotMode::Recording) {
            // GDBOT1 has no death/reset event. A normal recording is stopped at death.
            // For Continue, discard the unfinishable appended tail and persist the exact
            // original macro, so a later Continue can safely start from that endpoint.
            if (g_botContinueSession) {
                if (g_botContinueOriginalEventCount > 0 &&
                    g_botContinueOriginalEventCount <= g_events.size()) {
                    g_events.resize(g_botContinueOriginalEventCount);
                }
                if (finalizeContinueSession()) {
                    notify("GD Bot Continue: died during appended recording; incomplete tail discarded, original macro preserved");
                }
            } else {
                stopBot(false);
                notify("GD Bot: recording stopped after death; partial macro remains in memory");
            }
        }
        if (primaryPlayer && hitboxesOnDeathEnabled() && !hitboxesEnabled() && !g_noclipDeathDrawActive) {
            toggleDebugDraw();
            g_noclipDeathDrawActive = true;
        }
        PlayLayer::destroyPlayer(player, object);
        if (autoRetryEnabled() && primaryPlayer) {
            g_autoRetryPending = true;
        }
    }

    void resetLevel() {
        g_pauseButtonNode = nullptr;
        g_pauseButtonLayer = nullptr;
        g_noclipDeathCount = 0;
        g_noclipMaxProgress = 0;
        g_checkpointIndex = -1;
        g_autoRetryPending = false;
        if (g_noclipDeathDrawActive && !hitboxesEnabled()) {
            toggleDebugDraw();
        }
        g_noclipDeathDrawActive = false;
        PlayLayer::resetLevel();
    }

    void onQuit() {
        if (g_botContinueSession) finalizeContinueSession();
        else stopBot(false);
        if (auto* director = CCDirector::sharedDirector()) {
            if (auto* scheduler = director->getScheduler()) scheduler->setTimeScale(1.0f);
        }
        PlayLayer::onQuit();
    }
};

$on_mod(Loaded) {
    if (auto* director = CCDirector::sharedDirector()) {
        g_originalAnimationInterval = director->getAnimationInterval();
        if (g_originalAnimationInterval <= 0.0) g_originalAnimationInterval = 1.0 / 60.0;
        if (auto* scheduler = director->getScheduler()) scheduler->setTimeScale(1.0f);
    }
    applyFPSBypass();
    if (musicMuted()) applyMusicMute();
    applyMusicPitch();
    log::info("VETER v1.2.3 loaded: gameplay, GD Bot, FPS/retry/hitboxes, overlays, custom menu music, editor nudge/rotate/scale tools, and local unlock checks");
}
