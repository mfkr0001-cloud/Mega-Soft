 
#include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

using namespace geode::prelude;

namespace {
    constexpr int BUTTON_TAG = 0x4D534831;
    constexpr std::size_t MAX_EVENTS = 500000;

    enum class BotMode {
        Idle,
        Recording,
        Playing
    };

    struct BotEvent {
        double time;
        int button;
        bool pressed;
    };

    BotMode botMode = BotMode::Idle;
    std::vector<BotEvent> events;
    std::vector<int> heldButtons;

    double botClock = 0.0;
    std::size_t playIndex = 0;
    int macroLevelID = 0;

    bool injectedInput = false;

    bool noclipEnabled() {
        return Mod::get()->getSettingValue<bool>("noclip-enabled");
    }

    void setNoclip(bool enabled) {
        Mod::get()->setSettingValue("noclip-enabled", enabled);
    }

    bool speedhackEnabled() {
        return Mod::get()->getSavedValue<bool>(
            "speedhack-enabled", false
        );
    }

    void setSpeedhack(bool enabled) {
        Mod::get()->setSavedValue("speedhack-enabled", enabled);
    }

    float speedMultiplier() {
        return std::clamp(
            Mod::get()->getSavedValue<float>(
                "speedhack-multiplier", 1.0f
            ),
            0.5f, 3.0f
        );
    }

    void setSpeedMultiplier(float value) {
        Mod::get()->setSavedValue(
            "speedhack-multiplier",
            std::clamp(value, 0.5f, 3.0f)
        );
    }

    float nextSpeed() {
        static constexpr float values[] = {
            0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 2.0f, 3.0f
        };

        float current = speedMultiplier();

        for (std::size_t i = 0; i < 7; ++i) {
            if (std::fabs(values[i] - current) < 0.01f) {
                return values[(i + 1) % 7];
            }
        }

        return 1.5f;
    }

    void notify(const std::string& message) {
        Notification::create(
            message, NotificationIcon::Info
        )->show();
    }

    int currentLevelID() {
        auto* layer = PlayLayer::get();

        if (!layer || !layer->m_level) {
            return 0;
        }

        return layer->m_level->m_levelID;
    }

    std::filesystem::path macroPath() {
        return Mod::get()->getSaveDir() / "gdbot.gdm";
    }

    void releaseHeldButtons() {
        auto* layer = PlayLayer::get();

        if (!layer || !layer->m_player1) {
            heldButtons.clear();
            return;
        }

        injectedInput = true;

        for (int button : heldButtons) {
            layer->m_player1->releaseButton(
                static_cast<PlayerButton>(button)
            );
        }

        injectedInput = false;
        heldButtons.clear();
    }

    void stopBot(bool message = true) {
        BotMode previous = botMode;

        if (previous == BotMode::Idle) {
            return;
        }

        botMode = BotMode::Idle;

        if (previous == BotMode::Playing) {
            releaseHeldButtons();
        }

        if (message) {
            notify(previous == BotMode::Recording
                ? "GD Bot: recording stopped"
                : "GD Bot: playback stopped");
        }
    }

    void recordInput(
        PlayerObject* player,
        PlayerButton button,
        bool pressed
    ) {
        if (botMode != BotMode::Recording || injectedInput) {
            return;
        }

        auto* layer = PlayLayer::get();

        if (!layer || player != layer->m_player1) {
            return;
        }

        if (events.size() >= MAX_EVENTS) {
            stopBot(false);
            notify("GD Bot: event limit reached");
            return;
        }

        events.push_back({
            botClock,
            static_cast<int>(button),
            pressed
        });
    }

    void saveMacro() {
        if (botMode == BotMode::Recording) {
            stopBot(false);
        }

        if (events.empty()) {
            notify("GD Bot: nothing to save");
            return;
        }

        std::error_code error;
        auto path = macroPath();

        std::filesystem::create_directories(
            path.parent_path(), error
        );

        if (error) {
            notify("GD Bot: cannot create save folder");
            return;
        }

        std::ofstream file(path, std::ios::trunc);

        if (!file) {
            notify("GD Bot: cannot open save file");
            return;
        }

        file << "GDBOT1\n";
        file << macroLevelID << '\n';
        file.precision(10);

        for (const auto& event : events) {
            file << event.time << ' '
                 << (event.pressed ? 1 : 0) << ' '
                 << event.button << '\n';
        }

        file.close();

        if (!file) {
            notify("GD Bot: save failed");
            return;
        }

        notify("GD Bot: macro saved");
    }

    void loadMacro() {
        stopBot(false);

        std::ifstream file(macroPath());

        if (!file) {
            notify("GD Bot: macro not found");
            return;
        }

        std::string signature;
        int levelID = 0;

        if (!(file >> signature >> levelID) ||
            signature != "GDBOT1") {
            notify("GD Bot: invalid macro file");
            return;
        }

        std::vector<BotEvent> loaded;

        double time = 0.0;
        double previousTime = 0.0;
        int pressed = 0;
        int button = 0;

        while (file >> time >> pressed >> button) {
            if (!std::isfinite(time) ||
                time < previousTime ||
                time < 0.0 ||
                (pressed != 0 && pressed != 1) ||
                button < 0 || button > 32 ||
                loaded.size() >= MAX_EVENTS) {
                notify("GD Bot: invalid macro data");
                return;
            }

            loaded.push_back({
                time, button, pressed != 0
            });

            previousTime = time;
        }

        if (loaded.empty()) {
            notify("GD Bot: macro is empty");
            return;
        }

        events = std::move(loaded);
        macroLevelID = levelID;
        botClock = 0.0;
        playIndex = 0;

        notify("GD Bot: macro loaded");
    }

    void processPlayback(PlayLayer* layer) {
        if (botMode != BotMode::Playing ||
            !layer || !layer->m_player1) {
            return;
        }

        auto* player = layer->m_player1;

        while (playIndex < events.size() &&
               events[playIndex].time <= botClock) {
            const BotEvent event = events[playIndex++];
            auto button = static_cast<PlayerButton>(
                event.button
            );

            injectedInput = true;

            if (event.pressed) {
                player->pushButton(button);

                if (std::find(
                    heldButtons.begin(),
                    heldButtons.end(),
                    event.button
                ) == heldButtons.end()) {
                    heldButtons.push_back(event.button);
                }
            } else {
                player->releaseButton(button);

                std::erase(
                    heldButtons, event.button
                );
            }

            injectedInput = false;
        }

        if (playIndex >= events.size()) {
            releaseHeldButtons();
            botMode = BotMode::Idle;
            notify("GD Bot: playback finished");
        }
    }

    void toggleNoClip() {
        bool enabled = !noclipEnabled();
        setNoclip(enabled);

        notify(enabled
            ? "Mega Soft: NoClip ON"
            : "Mega Soft: NoClip OFF");
    }

    void toggleSpeedhack() {
        bool enabled = !speedhackEnabled();
        setSpeedhack(enabled);

        notify(enabled
            ? "Mega Soft: Speedhack ON"
            : "Mega Soft: Speedhack OFF");
    }
}

class MegaSoftPopup : public Popup {
protected:
    bool init() {
        if (!Popup::init(340.f, 310.f)) {
            return false;
        }

        setTitle("Mega Soft");

        auto* menu = CCMenu::create();

        if (!menu) {
            return false;
        }

        menu->setPosition({0.f, 0.f});
        addChild(menu);

        auto size = getContentSize();

        auto addButton = [&](
            const std::string& text,
            SEL_MenuHandler selector,
            float x,
            float y,
            const char* frame
        ) {
            auto* sprite = ButtonSprite::create(
                text.c_str(),
                "bigFont.fnt",
                frame,
                0.58f
            );

            if (!sprite) {
                return;
            }

            auto* item = CCMenuItemSpriteExtra::create(
                sprite, this, selector
            );

            if (!item) {
                return;
            }

            item->setPosition({x, y});
            menu->addChild(item);
        };

        float left = size.width * 0.29f;
        float right = size.width * 0.71f;

        float row1 = size.height * 0.78f;
        float row2 = size.height * 0.62f;
        float row3 = size.height * 0.46f;
        float row4 = size.height * 0.30f;

        addButton(
            noclipEnabled() ? "NOCLIP: ON" : "NOCLIP: OFF",
            menu_selector(MegaSoftPopup::onNoClip),
            left, row1,
            noclipEnabled()
                ? "GJ_button_02.png"
                : "GJ_button_04.png"
        );

        addButton(
            speedhackEnabled() ? "SPEED: ON" : "SPEED: OFF",
            menu_selector(MegaSoftPopup::onSpeedhack),
            right, row1,
            speedhackEnabled()
                ? "GJ_button_02.png"
                : "GJ_button_04.png"
        );

        addButton(
            "RATE: " + std::to_string(speedMultiplier()).substr(0, 4) + "x",
            menu_selector(MegaSoftPopup::onNextSpeed),
            left, row2,
            "GJ_button_01.png"
        );

        addButton(
            "BOT: RECORD",
            menu_selector(MegaSoftPopup::onRecord),
            right, row2,
            "GJ_button_01.png"
        );

        addButton(
            "BOT: STOP",
            menu_selector(MegaSoftPopup::onStop),
            left, row3,
            "GJ_button_04.png"
        );

        addButton(
            "BOT: PLAY",
            menu_selector(MegaSoftPopup::onPlay),
            right, row3,
            "GJ_button_01.png"
        );

        addButton(
            "BOT: SAVE",
            menu_selector(MegaSoftPopup::onSave),
            left, row4,
            "GJ_button_02.png"
        );

        addButton(
            "BOT: LOAD",
            menu_selector(MegaSoftPopup::onLoad),
            right, row4,
            "GJ_button_02.png"
        );

        auto* note = CCLabelBMFont::create(
            "GD Bot - record and replay",
            "chatFont.fnt"
        );

        if (note) {
            note->setScale(0.55f);
            note->setPosition({
                size.width * 0.5f,
                size.height * 0.12f
            });
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

    void onNoClip(CCObject*) {
        toggleNoClip();
        onClose(nullptr);
    }

    void onSpeedhack(CCObject*) {
        toggleSpeedhack();
        onClose(nullptr);
    }

    void onNextSpeed(CCObject*) {
        float next = nextSpeed();
        setSpeedMultiplier(next);

        notify(
            "Mega Soft: speed set to " +
            std::to_string(next).substr(0, 4) + "x"
        );

        onClose(nullptr);
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

        events.clear();
        heldButtons.clear();
        botClock = 0.0;
        playIndex = 0;
        macroLevelID = currentLevelID();
        botMode = BotMode::Recording;

        notify("GD Bot: recording started");
        onClose(nullptr);
    }

    void onStop(CCObject*) {
        stopBot();
        onClose(nullptr);
    }

    void onPlay(CCObject*) {
        auto* layer = PlayLayer::get();

        if (!layer || !layer->m_player1) {
            notify("GD Bot: enter a level first");
            onClose(nullptr);
            return;
        }

        if (events.empty()) {
            notify("GD Bot: record or load a macro first");
            onClose(nullptr);
            return;
        }

        int levelID = currentLevelID();

        if (macroLevelID != 0 &&
            levelID != 0 &&
            macroLevelID != levelID) {
            notify("GD Bot: wrong level for this macro");
            onClose(nullptr);
            return;
        }

        stopBot(false);
        layer->resetLevel();

        botClock = 0.0;
        playIndex = 0;
        heldButtons.clear();
        botMode = BotMode::Playing;

        notify("GD Bot: playback started");
        onClose(nullptr);
    }

    void onSave(CCObject*) {
        saveMacro();
        onClose(nullptr);
    }

    void onLoad(CCObject*) {
        loadMacro();
        onClose(nullptr);
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
        if (auto* popup = MegaSoftPopup::create()) {
            popup->show();
        }
    }
};

namespace {
    void attachMegaSoftButton(CCScene* scene) {
        if (!scene ||
            scene->getChildByTag(BUTTON_TAG)) {
            return;
        }

        auto* menu = CCMenu::create();
        auto* delegate = MegaSoftOpenDelegate::create();

        auto* sprite = ButtonSprite::create(
            "MH",
            "bigFont.fnt",
            "GJ_button_01.png",
            0.75f
        );

        if (!menu || !delegate || !sprite) {
            return;
        }

        auto* item = CCMenuItemSpriteExtra::create(
            sprite,
            delegate,
            menu_selector(MegaSoftOpenDelegate::onOpen)
        );

        if (!item) {
            return;
        }

        auto size =
            CCDirector::sharedDirector()->getWinSize();

        menu->setTag(BUTTON_TAG);
        menu->setPosition({0.f, 0.f});

        item->setPosition({
            size.width - 38.f,
            size.height - 35.f
        });

        menu->addChild(delegate);
        menu->addChild(item);

        scene->addChild(menu, 9999);
    }
}

class $modify(MegaSoftDirector, CCDirector) {
    void drawScene() {
        if (auto* scene = getRunningScene()) {
            attachMegaSoftButton(scene);
        }

        CCDirector::drawScene();
    }
};

class $modify(MegaSoftPlayer, PlayerObject) {
    bool pushButton(PlayerButton button) {
        bool result = PlayerObject::pushButton(button);

        recordInput(this, button, true);

        return result;
    }

    bool releaseButton(PlayerButton button) {
        bool result = PlayerObject::releaseButton(button);

        recordInput(this, button, false);

        return result;
    }
};

class $modify(MegaSoftPlayLayer, PlayLayer) {
    void update(float dt) {
        if (botMode == BotMode::Playing) {
            processPlayback(this);
        }

        PlayLayer::update(dt);

        if (botMode == BotMode::Recording ||
            botMode == BotMode::Playing) {
            botClock += std::max(
                0.0, static_cast<double>(dt)
            );
        }
    }

    void destroyPlayer(
        PlayerObject* player,
        GameObject* object
    ) {
        if (noclipEnabled()) {
            return;
        }

        PlayLayer::destroyPlayer(player, object);
    }

    void onQuit() {
        stopBot(false);
        PlayLayer::onQuit();
    }
};

$on_mod(Loaded) {
    log::info(
        "Mega Soft loaded: menu, NoClip and GD Bot"
    );
}
