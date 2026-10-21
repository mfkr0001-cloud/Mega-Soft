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
        item->setPosition({size.width - 38.f, size.height - 35.f});
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
        float scaledDt = dt;
        if (speedhackEnabled()) scaledDt *= speedMultiplier();

        if (g_botMode == BotMode::Playing) processPlayback(this);
        PlayLayer::update(scaledDt);

        if (g_botMode == BotMode::Recording || g_botMode == BotMode::Playing) {
            g_botClock += std::max(0.0, static_cast<double>(scaledDt));
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (noclipEnabled()) return;
        PlayLayer::destroyPlayer(player, object);
    }

    void onQuit() {
        stopBot(false);
        PlayLayer::onQuit();
    }
};

$on_mod(Loaded) {
    log::info("Mega Soft loaded: global menu, basic NoClip, Speedhack and GD Bot");
}

