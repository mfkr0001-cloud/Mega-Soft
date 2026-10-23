#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
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
