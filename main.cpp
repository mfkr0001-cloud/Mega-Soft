
#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

using namespace geode::prelude;

class $modify(MegaSoftMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto menu = CCMenu::create();
        if (!menu) {
            return true;
        }

        menu->setPosition({0.0f, 0.0f});
        menu->setID("megasoft-main-menu");
        addChild(menu, 1000);

        auto buttonSprite = ButtonSprite::create(
            "MH",
            40,
            true,
            "goldFont.fnt",
            "GJ_button_01.png",
            30.0f,
            0.8f
        );

        if (!buttonSprite) {
            log::error("Mega Soft: failed to create button sprite");
            return true;
        }

        auto button = CCMenuItemSpriteExtra::create(
            buttonSprite,
            this,
            menu_selector(MegaSoftMenuLayer::onMegaSoftClick)
        );

        if (!button) {
            log::error("Mega Soft: failed to create menu button");
            return true;
        }

        button->setID("megasoft-open-button");
        button->setPosition({
            winSize.width - 35.0f,
            winSize.height - 35.0f
        });

        menu->addChild(button);

        log::info("Mega Soft: main menu button created");
        return true;
    }

    void onMegaSoftClick(CCObject*) {
        FLAlertLayer::create(
            "Mega Soft",
            "Mega Soft is working!\n\n"
            "The menu button is connected correctly.",
            "OK"
        )->show();
    }
};
