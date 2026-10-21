
#include <Geode/Geode.hpp>

using namespace geode::prelude;

static bool g_menuOpen = false;
static bool g_noclip = false;
static bool g_jumpHack = false;
static bool g_autoClicker = false;
static bool g_showHitboxes = false;

class MegaSoftPopup : public FLAlertLayer {
protected:
    bool init() {
        if (!FLAlertLayer::init(
            360.f, 260.f, 0.8f,
            "MegaSoft", "Close", nullptr
        )) {
            return false;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto title = CCLabelBMFont::create(
            "MEGASOFT MENU", "bigFont.fnt"
        );
        title->setScale(0.65f);
        title->setPosition(
            ccp(winSize.width / 2.f, winSize.height / 2.f + 75.f)
        );
        this->addChild(title);

        auto menu = CCMenu::create();
        menu->setPosition(CCPointZero);
        this->addChild(menu);

        addToggle(menu, "Noclip", g_noclip, 0);
        addToggle(menu, "Jump Hack", g_jumpHack, 1);
        addToggle(menu, "Auto Clicker", g_autoClicker, 2);
        addToggle(menu, "Hitboxes", g_showHitboxes, 3);

        return true;
    }

    void addToggle(CCMenu* menu, const char* name, bool& state, int row) {
        auto label = CCLabelBMFont::create(name, "bigFont.fnt");
        label->setScale(0.45f);

        auto item = CCMenuItemToggler::createWithStandardSprites(
            this,
            menu_selector(MegaSoftPopup::onToggle),
            0.7f
        );

        if (!item) {
            return;
        }

        item->setTag(row);
        item->toggle(!state);

        auto entry = CCMenuItemSpriteExtra::create(
            label, this,
            menu_selector(MegaSoftPopup::onLabel)
        );
        entry->setTag(row);

        auto y = 48.f - row * 32.f;

        item->setPosition(ccp(-55.f, y));
        entry->setPosition(ccp(35.f, y));

        menu->addChild(item);
        menu->addChild(entry);
    }

    void onToggle(CCObject* sender) {
        auto item = static_cast<CCMenuItemToggler*>(sender);

        switch (item->getTag()) {
            case 0: g_noclip = item->isToggled(); break;
            case 1: g_jumpHack = item->isToggled(); break;
            case 2: g_autoClicker = item->isToggled(); break;
            case 3: g_showHitboxes = item->isToggled(); break;
        }
    }

    void onLabel(CCObject* sender) {
        auto item = static_cast<CCMenuItem*>(sender);
        auto menu = static_cast<CCMenu*>(item->getParent());

        if (!menu) return;

        auto toggler = static_cast<CCMenuItemToggler*>(
            menu->getChildByTag(item->getTag())
        );

        if (toggler) {
            toggler->activate();
        }
    }

public:
    static MegaSoftPopup* create() {
        auto ret = new MegaSoftPopup();

        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }

        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class $modify(MegaHackMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto label = CCLabelBMFont::create(
            "MH", "bigFont.fnt"
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
        auto popup = MegaSoftPopup::create();

        if (popup) {
            popup->show();
        }
    }
};

