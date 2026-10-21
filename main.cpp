
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class $modify(MegaHackMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto label = CCLabelBMFont::create(
            "MH",
            "bigFont.fnt"
        );

        if (!label) {
            return true;
        }

        auto button = CCMenuItemSpriteExtra::create(
            label,
            this,
            menu_selector(MegaHackMenuLayer::onMHButton)
        );

        auto menu = CCMenu::create();
        menu->addChild(button);

        menu->setPosition(
            ccp(winSize.width - 35.f, winSize.height * 0.65f)
        );

        button->setScale(0.65f);
        this->addChild(menu, 1000);

        return true;
    }

    void onMHButton(CCObject*) {
        FLAlertLayer::create(
            "Mega Hack Mobile",
            "Your custom Geode mod is running!",
            "Close"
        )->show();
    }
};
