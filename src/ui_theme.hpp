#pragma once
#include <Geode/Geode.hpp>

namespace VETERTheme {
    inline cocos2d::ccColor3B popupOuter() { return cocos2d::ccc3(154, 20, 181); }
    inline cocos2d::ccColor3B title() { return cocos2d::ccc3(255, 229, 83); }
    inline cocos2d::ccColor3B text() { return cocos2d::ccc3(246, 242, 255); }
    inline cocos2d::ccColor3B mutedText() { return cocos2d::ccc3(190, 178, 220); }
    inline cocos2d::ccColor3B activeText() { return cocos2d::ccc3(255, 231, 99); }
    inline cocos2d::ccColor3B activeTab() { return cocos2d::ccc3(173, 43, 221); }
    inline cocos2d::ccColor3B idleTab() { return cocos2d::ccc3(37, 23, 68); }
    inline cocos2d::ccColor3B activeTabText() { return cocos2d::ccc3(255, 235, 105); }
    inline cocos2d::ccColor3B idleTabText() { return mutedText(); }
    inline cocos2d::ccColor3B accent() { return cocos2d::ccc3(173, 43, 221); }
    inline cocos2d::ccColor3B accentMuted() { return cocos2d::ccc3(93, 42, 126); }
    inline cocos2d::ccColor3B secondaryPopup() { return cocos2d::ccc3(37, 23, 68); }
    inline cocos2d::ccColor3B outerPanel() { return cocos2d::ccc3(24, 15, 45); }
    inline cocos2d::ccColor3B contentPanel() { return cocos2d::ccc3(37, 23, 68); }
    inline cocos2d::ccColor3B sidePanel() { return cocos2d::ccc3(30, 18, 55); }
    inline cocos2d::ccColor3B columnBorder() { return cocos2d::ccc3(100, 39, 135); }
    inline cocos2d::ccColor3B columnFill() { return cocos2d::ccc3(37, 23, 68); }
    inline cocos2d::ccColor3B columnAccent() { return accent(); }
    inline cocos2d::ccColor3B button() { return cocos2d::ccc3(70, 35, 105); }
    inline cocos2d::ccColor3B toggleOn() { return cocos2d::ccc3(35, 145, 75); }
    inline cocos2d::ccColor3B toggleOff() { return cocos2d::ccc3(120, 45, 55); }
}
