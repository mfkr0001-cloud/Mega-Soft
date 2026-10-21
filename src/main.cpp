#include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>
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

    void recordInput(PlayerObject* player, PlayerButton button, bool pressed) {
        if (g_botMode != BotMode::Recording || g_injectedInput) return;

        auto* layer = PlayLayer::get();
        if (!layer || !layer->m_player1 || player != layer->m_player1) return;
        if (g_events.size() >= kMaxMacroEvents) {
            stopBot(false);
            notify("GD Bot: event limit reached; recording stopped");
            return;
        }

        g_events.push_back({g_botClock, static_cast<int>(button), pressed});
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
    ButtonSprite* m_speedValueSprite = nullptr;

protected:
    bool init() {
        if (!Popup::init(340.f, 310.f)) return false;
        setTitle("Mega Soft");

        auto* menu = CCMenu::create();
        if (!menu) return false;
        menu->setPosition({0.f, 0.f});
        addChild(menu);

        const auto size = getContentSize();
        auto addButton = [&](const std::string& text, SEL_MenuHandler selector,
                             float x, float y, const char* frame) -> ButtonSprite* {
            auto* sprite = ButtonSprite::create(text.c_str(), "bigFont.fnt", frame, 0.48f);
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
        const float row1 = size.height * 0.75f;
        const float row2 = size.height * 0.61f;
        const float row3 = size.height * 0.47f;
        const float row4 = size.height * 0.33f;
        const float row5 = size.height * 0.20f;

        m_noClipSprite = addButton(noclipEnabled() ? "NOCLIP: ON" : "NOCLIP: OFF",
                                   menu_selector(MegaSoftPopup::onToggleNoClip), left, row1,
                                   noclipEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");
        m_speedhackSprite = addButton(speedhackEnabled() ? "SPEED: ON" : "SPEED: OFF",
                                      menu_selector(MegaSoftPopup::onToggleSpeedhack), right, row1,
                                      speedhackEnabled() ? "GJ_button_02.png" : "GJ_button_04.png");

        addButton("-0.1x", menu_selector(MegaSoftPopup::onSpeedDown), left, row2, "GJ_button_01.png");
        m_speedValueSprite = addButton(speedLabel(), menu_selector(MegaSoftPopup::onSpeedLabel), center, row2, "GJ_button_02.png");
        addButton("+0.1x", menu_selector(MegaSoftPopup::onSpeedUp), right, row2, "GJ_button_01.png");

        addButton("BOT RECORD", menu_selector(MegaSoftPopup::onRecord), left, row3, "GJ_button_01.png");
        addButton("BOT STOP", menu_selector(MegaSoftPopup::onStopBot), right, row3, "GJ_button_04.png");
        addButton("BOT PLAY", menu_selector(MegaSoftPopup::onPlay), left, row4, "GJ_button_01.png");
        addButton("BOT SAVE", menu_selector(MegaSoftPopup::onSave), right, row4, "GJ_button_02.png");
        addButton("BOT LOAD", menu_selector(MegaSoftPopup::onLoad), center, row5, "GJ_button_02.png");

        auto* note = CCLabelBMFont::create("Speed: 0.1x-5.0x | Bot: same level", "chatFont.fnt");
        if (note) {
            note->setScale(0.40f);
            note->setPosition({size.width * 0.5f, size.height * 0.07f});
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
        if (m_noClipSprite) m_noClipSprite->setString(noclipEnabled() ? "NOCLIP: ON" : "NOCLIP: OFF");
    }

    void onToggleSpeedhack(CCObject*) {
        toggleSpeedhack();
        if (m_speedhackSprite) m_speedhackSprite->setString(speedhackEnabled() ? "SPEED: ON" : "SPEED: OFF");
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
        layer->resetLevel();
        g_events.clear();
        g_playbackHeldButtons.clear();
        g_botClock = 0.0;
        g_playIndex = 0;
        g_macroLevelID = currentLevelID();
        g_botMode = BotMode::Recording;
        notify("GD Bot: recording started; play from the beginning");
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
        layer->resetLevel();
        g_botClock = 0.0;
        g_playIndex = 0;
        g_playbackHeldButtons.clear();
        g_botMode = BotMode::Playing;
        notify("GD Bot: playback started; practice checkpoints are not recommended");
        onClose(nullptr);
    }

    void onSave(CCObject*) { saveMacro(); onClose(nullptr); }
    void onLoad(CCObject*) { loadMacro(); onClose(nullptr); }
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
        if (auto* scene = this->getRunningScene()) {
            attachMegaSoftButton(scene);
        }
        CCDirector::drawScene();
    }
};

class $modify(MegaSoftPlayer, PlayerObject) {
    bool pushButton(PlayerButton button) {
        const bool result = PlayerObject::pushButton(button);
        recordInput(this, button, true);
        return result;
    }

    bool releaseButton(PlayerButton button) {
        const bool result = PlayerObject::releaseButton(button);
        recordInput(this, button, false);
        return result;
    }
};

class $modify(MegaSoftPlayLayer, PlayLayer) {
    void update(float dt) {
        // Apply the selected scheduler scale every frame, so it is re-applied
        // after changing levels and can be switched off immediately.
        applySpeedhack();

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
    log::info("Mega Soft loaded: fixed controls, NoClip, scheduler Speedhack and GD Bot");
}
