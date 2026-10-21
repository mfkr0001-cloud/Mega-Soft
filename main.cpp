
#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include <algorithm>

using namespace geode::prelude;

namespace MegaSoft {

    static void applyFPS(bool enabled) {
        auto director = CCDirector::sharedDirector();

        if (director) {
            director->setDisplayStats(enabled);
        }

        log::info(
            "MegaSoft: FPS statistics {}",
            enabled ? "ON" : "OFF"
        );
    }

    $on_mod(Loaded) {
        applyFPS(
            Mod::get()->getSettingValue<bool>("show-fps")
        );

        listenForSettingChanges(
            "show-fps",
            [](bool enabled) {
                applyFPS(enabled);
            }
        );
    }
}

class $modify(MegaSoftMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto size = CCDirector::sharedDirector()->getWinSize();

        auto label = CCLabelBMFont::create(
            "MH", "bigFont.fnt"
        );

        if (!label) {
            return true;
        }

        label->setScale(0.65f);

        auto button = CCMenuItemSpriteExtra::create(
            label,
            this,
            menu_selector(MegaSoftMenuLayer::onMegaSoft)
        );

        if (!button) {
            return true;
        }

        auto menu = CCMenu::create();
        menu->addChild(button);

        menu->setPosition(
            ccp(size.width - 38.f, size.height * 0.65f)
        );

        this->addChild(menu, 1000);

        return true;
    }

    void onMegaSoft(CCObject*) {
        bool fps = Mod::get()->getSettingValue<bool>(
            "show-fps"
        );

        bool noclip = Mod::get()->getSettingValue<bool>(
            "noclip"
        );

        bool speed = Mod::get()->getSettingValue<bool>(
            "speed-hack"
        );

        double multiplier =
            Mod::get()->getSettingValue<double>(
                "speed-multiplier"
            );

        FLAlertLayer::create(
            "Mega Soft",
            fmt::format(
                "FPS statistics: {}\n"
                "Noclip: {}\n"
                "Speed Hack: {}\n"
                "Speed multiplier: {:.1f}x\n\n"
                "Change options in Geode mod settings.",
                fps ? "ON" : "OFF",
                noclip ? "ON" : "OFF",
                speed ? "ON" : "OFF",
                multiplier
            ),
            "Close"
        )->show();
    }
};

class $modify(MegaSoftPlayLayer, PlayLayer) {
    void destroyPlayer(
        PlayerObject* player,
        GameObject* object
    ) {
        if (Mod::get()->getSettingValue<bool>("noclip")) {
            return;
        }

        PlayLayer::destroyPlayer(player, object);
    }

    void update(float dt) {
        if (Mod::get()->getSettingValue<bool>("speed-hack")) {
            double multiplier =
                Mod::get()->getSettingValue<double>(
                    "speed-multiplier"
                );

            multiplier = std::clamp(multiplier, 0.5, 3.0);

            dt *= static_cast<float>(multiplier);
        }

        PlayLayer::update(dt);
    }
};
