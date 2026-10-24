
#pragma once

// VETER Mobile — фиолетовая тема интерфейса
// Функции мода находятся в main.cpp.
namespace VETERTheme {
    inline ccColor3B popupOuter() { return ccc3(154, 20, 181); }
    inline ccColor3B title() { return ccc3(255, 229, 83); }
    inline ccColor3B text() { return ccc3(246, 242, 255); }
    inline ccColor3B mutedText() { return ccc3(190, 178, 220); }
    inline ccColor3B activeText() { return ccc3(255, 231, 99); }

    inline ccColor3B activeTab() { return ccc3(173, 43, 221); }
    inline ccColor3B idleTab() { return ccc3(37, 23, 68); }
    inline ccColor3B activeTabText() { return ccc3(255, 235, 105); }
    inline ccColor3B idleTabText() { return ccc3(224, 216, 246); }

    inline ccColor4B outerPanel() { return ccc4(177, 20, 192, 255); }
    inline ccColor4B contentPanel() { return ccc4(28, 20, 54, 255); }
    inline ccColor4B sidePanel() { return ccc4(36, 24, 67, 255); }
    inline ccColor4B columnBorder() { return ccc4(183, 94, 226, 255); }
    inline ccColor4B columnFill() { return ccc4(31, 22, 57, 255); }
    inline ccColor4B columnAccent() { return ccc4(194, 50, 228, 255); }

    inline ccColor4B accent() { return ccc4(222, 56, 238, 255); }
    inline ccColor4B accentMuted() { return ccc4(112, 66, 157, 255); }
    inline ccColor4B button() { return ccc4(39, 27, 73, 255); }
    inline ccColor4B toggleOn() { return ccc4(96, 39, 134, 255); }
    inline ccColor4B toggleOff() { return ccc4(31, 23, 54, 255); }
    inline ccColor4B secondaryPopup() { return ccc4(31, 21, 55, 255); }
}
