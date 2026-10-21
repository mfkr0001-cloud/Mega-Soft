#include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace geode::prelude;

namespace {
    constexpr int kMegaSoftButtonTag = 0x4D534831;
    constexpr std::size_t kMaxMacroEvents = 500000;

    enum class BotMode { Idle, Recording, Playing };

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
    bool g_injectedInput = false;

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
        notify(muted ? "Mega Soft: Music OFF" : "Mega Soft: Music ON");
    }

    void applySpeedhack() {
        auto* director = CCDirector::sharedDirector();
        auto* scheduler = director ? director->getScheduler() : nullptr;
        if (scheduler) {
            scheduler->setTimeScale((PlayLayer::get() && speedhackEnabled()) ? speedMultiplier() : 1.0f);
        }
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

    std::filesystem::path macroPath() {
        return Mod::get()->getSaveDir() / "gdbot.gdm";
    }

    void releasePlaybackButtons() {
        auto* layer = PlayLayer::get();
        if (!layer || !layer->m_player1) {
            g_playbackHeldButtons.clear();
            return;
        }

        g_injectedInput = true;
        for (const int value : g_playbackHeldButtons) {
            layer->m_player1->releaseButton(static_cast<PlayerButton>(value));
        }
        g_injectedInput = false;
        g_playbackHeldButtons.clear();
    }

    void stopBot(bool showNotification = true) {
        const auto oldMode = g_botMode;
        if (oldMode == BotMode::Idle) return;

        g_botMode = BotMode::Idle;
        if (oldMode == BotMode::Playing) releasePlaybackButtons();

        if (showNotification) {
            if (oldMode == BotMode::Recording) {
                notify("GD Bot: recording stopped");
            } else {
                notify("GD Bot: playback stopped");
            }
        }
    }

    void recordInput(int button, bool pressed) {
        if (g_botMode != BotMode::Recording || g_injectedInput) return;
        auto* layer = PlayLayer::get();
        if (!layer || !layer->m_player1) return;
        if (button < 0 || button > 32) return;
        if (g_events.size() >= kMaxMacroEvents) {
            stopBot(false);
            notify("GD Bot: event limit reached; recording stopped");
            return;
        }
        g_events.push_back({g_botClock, button, pressed});
    }

    void saveMacro() {
        if (g_botMode == BotMode::Recording) stopBot(false);
        if (g_events.empty()) {
            notify("GD Bot: nothing recorded yet");
            return;
        }

        const auto path = macroPath();
        std::error_code error;
        std::filesystem::create_directories(path.parent_path(), error);
        if (error) {
            log::error("GD Bot: could not create save folder: {}", error.message());
            notify("GD Bot: could not create save folder");
            return;
        }

        std::ofstream file(path, std::ios::trunc);
        if (!file) {
            notify("GD Bot: could not open the save file");
            return;
        }

        file << "GDBOT1\n" << g_macroLevelID << '\n';
        file.precision(8);
        for (const auto& event : g_events) {
            file << event.time << ' ' << (event.pressed ? 1 : 0) << ' ' << event.button << '\n';
        }
        file.close();

        if (!file) {
            notify("GD Bot: saving failed");
            return;
        }
        log::info("GD Bot: saved {} input events", g_events.size());
        notify("GD Bot: macro saved");
    }

    void loadMacro() {
        stopBot(false);
        std::ifstream file(macroPath());
        if (!file) {
            notify("GD Bot: saved macro not found");
            return;
        }

        std::string signature;
        int levelID = 0;
        if (!(file >> signature >> levelID) || signature != "GDBOT1") {
            notify("GD Bot: invalid macro file");
            return;
        }

        std::vector<BotEvent> loaded;
        double timestamp = 0.0;
        double previousTimestamp = 0.0;
        int pressed = 0;
        int button = 0;

        while (file >> timestamp >> pressed >> button) {
            if (!std::isfinite(timestamp) || timestamp < 0.0 || timestamp < previousTimestamp ||
                (pressed != 0 && pressed != 1) || button < 0 || button > 32 ||
                loaded.size() >= kMaxMacroEvents) {
                notify("GD Bot: invalid or oversized macro");
                return;
            }
            loaded.push_back({timestamp, button, pressed != 0});
            previousTimestamp = timestamp;
        }

        if (loaded.empty()) {
            notify("GD Bot: macro contains no inputs");
            return;
        }

        g_events = std::move(loaded);
        g_macroLevelID = levelID;
        g_botClock = 0.0;
        g_playIndex = 0;
        log::info("GD Bot: loaded {} input events", g_events.size());
        notify("GD Bot: macro loaded");
    }

    void processPlayback(PlayLayer* layer) {
        if (g_botMode != BotMode::Playing || !layer || !layer->m_player1) return;

        auto* player = layer->m_player1;
        while (g_playIndex < g_events.size() && g_events[g_playIndex].time <= g_botClock) {
            const auto event = g_events[g_playIndex++];
            const auto button = static_cast<PlayerButton>(event.button);

            g_injectedInput = true;
            if (event.pressed) {
                player->pushButton(button);
                if (std::find(g_playbackHeldButtons.begin(), g_playbackHeldButtons.end(), event.button) == g_playbackHeldButtons.end()) {
                    g_playbackHeldButtons.push_back(event.button);
                }
            } else {
                player->releaseButton(button);
                std::erase(g_playbackHeldButtons, event.button);
            }
            g_injectedInput = false;
        }

        if (g_playIndex >= g_events.size()) {
            releasePlaybackButtons();
            g_botMode = BotMode::Idle;
            notify("GD Bot: playback finished");
        }
    }

    void toggleNoClip() {
        const bool enabled = !noclipEnabled();
        setNoclipEnabled(enabled);
        log::info("Mega Soft: NoClip {}", enabled ? "ON" : "OFF");
        notify(enabled ? "Mega Soft: NoClip ON" : "Mega Soft: NoClip OFF");
    }

    void toggleSpeedhack() {
        const bool enabled = !speedhackEnabled();
        setSpeedhackEnabled(enabled);
        applySpeedhack();
        log::info("Mega Soft: Speedhack {} at {}x", enabled ? "ON" : "OFF", speedMultiplier());
        notify(enabled ? "Mega Soft: Speedhack ON" : "Mega Soft: Speedhack OFF");
    }
}

class MegaSoftPopup : public Popup {
private:
    ButtonSprite* m_noClipSprite = nullptr;
    ButtonSprite* m_speedhackSprite = nullptr;
    ButtonSprite* m_musicSprite = nullptr;
    ButtonSprite* m_speedValueSprite = nullptr;

protected:
    bool init() {
        // Compact grid; button labels are deliberately short so they stay inside
        // the popup on narrow Android screens as well as on desktop.
        if (!Popup::init(340.f, 320.f)) return false;
        setTitle("Mega Soft");

        auto* menu = CCMenu::create();
        if (!menu) return false;
        menu->setPosition({0.f, 0.f});
        addChild(menu);

        const auto size = getContentSize();
        auto addButton = [&](const std::string& text, SEL_MenuHandler selector,
                             float x, float y, const char* frame) -> ButtonSprite* {
            auto* sprite = ButtonSprite::create(text.c_str(), "bigFont.fnt", frame, 0.36f);
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
        const float rowToggles = size.height - 62.f;
        const float rowSpeed = size.height - 108.f;
        const float rowBot1 = size.height - 158.f;
        const float rowBot2 = size.height - 204.f;
        const float rowBot3 = size.height - 250.f;

        m_noClipSprite = addButton(noclipEnabled() ? "NOCLIP ON" : "NOCLIP OFF",
            menu_selector(MegaSoftPopup::onToggleNoClip), left, rowToggles,
            noclipEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_speedhackSprite = addButton(speedhackEnabled() ? "SPEED ON" : "SPEED OFF",
            menu_selector(MegaSoftPopup::onToggleSpeedhack), center, rowToggles,
            speedhackEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_musicSprite = addButton(musicMuted() ? "MUSIC OFF" : "MUSIC ON",
            menu_selector(MegaSoftPopup::onToggleMusic), right, rowToggles,
            musicMuted() ? "GJ_button_04.png" : "GJ_button_02.png");

        addButton("-0.1x", menu_selector(MegaSoftPopup::onSpeedDown), left, rowSpeed, "GJ_button_01.png");
        m_speedValueSprite = addButton(speedLabel(), menu_selector(MegaSoftPopup::onSpeedLabel), center, rowSpeed, "GJ_button_02.png");
        addButton("+0.1x", menu_selector(MegaSoftPopup::onSpeedUp), right, rowSpeed, "GJ_button_01.png");

        addButton("BOT REC", menu_selector(MegaSoftPopup::onRecord), left, rowBot1, "GJ_button_01.png");
        addButton("BOT STOP", menu_selector(MegaSoftPopup::onStopBot), right, rowBot1, "GJ_button_04.png");
        addButton("BOT PLAY", menu_selector(MegaSoftPopup::onPlay), left, rowBot2, "GJ_button_01.png");
        addButton("BOT SAVE", menu_selector(MegaSoftPopup::onSave), right, rowBot2, "GJ_button_02.png");
        addButton("BOT LOAD", menu_selector(MegaSoftPopup::onLoad), left, rowBot3, "GJ_button_02.png");
        addButton("BOT STATUS", menu_selector(MegaSoftPopup::onBotStatus), right, rowBot3, "GJ_button_01.png");

        auto* note = CCLabelBMFont::create("Enter a level first | Record > Save > Load > Play", "chatFont.fnt");
        if (note) {
            note->setScale(0.28f);
            note->setPosition({size.width * 0.5f, 23.f});
            addChild(note);
        }
        return true;
    }

public:
    static MegaSoftPopup* create() {
        auto* popup = new MegaSoftPopup();
        if (popup && popup->init()) {
            popup->autorelease();
            return popup;
        }
        delete popup;
        return nullptr;
    }

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

    void onSpeedLabel(CCObject*) {
        notify("Tap -0.1x or +0.1x to set speed");
    }

    void onSpeedDown(CCObject*) {
        setSpeedMultiplier(speedMultiplier() - 0.1f);
        if (speedhackEnabled()) applySpeedhack();
        if (m_speedValueSprite) m_speedValueSprite->setString(speedLabel().c_str());
        notify("Mega Soft: speed set to " + speedLabel().substr(6));
    }

    void onSpeedUp(CCObject*) {
        setSpeedMultiplier(speedMultiplier() + 0.1f);
        if (speedhackEnabled()) applySpeedhack();
        if (m_speedValueSprite) m_speedValueSprite->setString(speedLabel().c_str());
        notify("Mega Soft: speed set to " + speedLabel().substr(6));
    }

    void onRecord(CCObject*) {
        auto* layer = PlayLayer::get();
        if (!layer || !layer->m_player1) {
            notify("GD Bot: enter a level first");
            onClose(nullptr);
            return;
        }
        stopBot(false);
        g_events.clear();
        g_playbackHeldButtons.clear();
        g_botClock = 0.0;
        g_playIndex = 0;
        layer->resetLevel();
        g_macroLevelID = currentLevelID();
        g_botMode = BotMode::Recording;
        notify("GD Bot: recording started; play the level now");
        onClose(nullptr);
    }

    void onStopBot(CCObject*) {
        stopBot();
        onClose(nullptr);
    }

    void onPlay(CCObject*) {
        auto* layer = PlayLayer::get();
        if (!layer || !layer->m_player1) {
            notify("GD Bot: enter the recorded level first");
            onClose(nullptr);
            return;
        }
        if (g_events.empty()) {
            notify("GD Bot: record or load a macro first");
            onClose(nullptr);
            return;
        }
        const int levelID = currentLevelID();
        if (g_macroLevelID != 0 && levelID != 0 && g_macroLevelID != levelID) {
            notify("GD Bot: this macro belongs to another level");
            onClose(nullptr);
            return;
        }
        stopBot(false);
        g_botClock = 0.0;
        g_playIndex = 0;
        g_playbackHeldButtons.clear();
        layer->resetLevel();
        g_botMode = BotMode::Playing;
        notify("GD Bot: playback started");
        onClose(nullptr);
    }

    void onSave(CCObject*) { saveMacro(); onClose(nullptr); }
    void onLoad(CCObject*) { loadMacro(); onClose(nullptr); }

    void onBotStatus(CCObject*) {
        const char* mode = g_botMode == BotMode::Recording ? "RECORDING" :
                           g_botMode == BotMode::Playing ? "PLAYING" : "IDLE";
        notify(std::string("GD Bot: ") + mode + " | events: " + std::to_string(g_events.size()));
    }
};

class MegaSoftOpenDelegate : public CCNode {
public:
    static MegaSoftOpenDelegate* create() {
        auto* node = new MegaSoftOpenDelegate();
        if (node && node->init()) {
            node->autorelease();
            return node;
        }
        delete node;
        return nullptr;
    }

    void onOpen(CCObject*) {
        if (auto* popup = MegaSoftPopup::create()) popup->show();
    }
};

namespace {
    void attachMegaSoftButton(CCScene* scene) {
        if (!scene || scene->getChildByTag(kMegaSoftButtonTag)) return;

        auto* menu = CCMenu::create();
        auto* delegate = MegaSoftOpenDelegate::create();
        auto* sprite = ButtonSprite::create("MH", "bigFont.fnt", "GJ_button_01.png", 0.75f);
        if (!menu || !delegate || !sprite) return;

        auto* item = CCMenuItemSpriteExtra::create(
            sprite, delegate, menu_selector(MegaSoftOpenDelegate::onOpen)
        );
        if (!item) return;

        const auto size = CCDirector::sharedDirector()->getWinSize();
        menu->setTag(kMegaSoftButtonTag);
        menu->setPosition({0.f, 0.f});
        item->setPosition({size.width - 52.f, size.height - 52.f});
        item->setScale(0.68f);
        menu->addChild(delegate);
        menu->addChild(item);
        scene->addChild(menu, 9999);
    }
}

// Add the MH button to each new scene, not just the gameplay layer.
class $modify(MegaSoftDirector, CCDirector) {
    void drawScene() {
        // Enforce Music OFF in menus as well as in gameplay if the game starts
        // a new track after the toggle has been enabled.
        if (musicMuted()) applyMusicMute();
        if (auto* scene = this->getRunningScene()) {
            attachMegaSoftButton(scene);
        }
        CCDirector::drawScene();
    }
};

// Capture input at Geometry Dash's common input dispatcher. This is more
// reliable for on-screen/touch controls than relying only on PlayerObject hooks.
class $modify(MegaSoftBaseGameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        if (isPlayer1 && !g_injectedInput) {
            recordInput(button, down);
        }
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }
};

class $modify(MegaSoftPlayLayer, PlayLayer) {
    void update(float dt) {
        // Reapply toggles as levels/scenes change. Music is only touched while
        // muted, so normal playback is not repeatedly resumed every frame.
        applySpeedhack();
        if (musicMuted()) applyMusicMute();

        if (g_botMode == BotMode::Playing) processPlayback(this);
        PlayLayer::update(dt);

        if (g_botMode == BotMode::Recording || g_botMode == BotMode::Playing) {
            g_botClock += std::max(0.0, static_cast<double>(dt));
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (noclipEnabled()) return;
        PlayLayer::destroyPlayer(player, object);
    }

    void onQuit() {
        stopBot(false);
        if (auto* director = CCDirector::sharedDirector()) {
            if (auto* scheduler = director->getScheduler()) scheduler->setTimeScale(1.0f);
        }
        PlayLayer::onQuit();
    }
};

$on_mod(Loaded) {
    if (auto* director = CCDirector::sharedDirector()) {
        if (auto* scheduler = director->getScheduler()) scheduler->setTimeScale(1.0f);
    }
    if (musicMuted()) applyMusicMute();
    log::info("Mega Soft loaded: compact menu, NoClip, Speedhack, Music toggle and GD Bot");
}
