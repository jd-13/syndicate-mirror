#pragma once

namespace UIUtils {
    // Chains/plugin slots
    constexpr int CHAIN_WIDTH {200};
    constexpr int PLUGIN_SLOT_HEIGHT {30};
    constexpr int PLUGIN_SLOT_CORNER_RADIUS {PLUGIN_SLOT_HEIGHT / 2};
    constexpr int SLOT_DRAG_HANDLE_WIDTH {PLUGIN_SLOT_HEIGHT};

    // Modulation tray
    constexpr int PLUGIN_SLOT_MOD_TRAY_HEIGHT {PLUGIN_SLOT_HEIGHT * 3};
    constexpr int PLUGIN_MOD_TARGET_SLIDER_HEIGHT {static_cast<int>(UIUtils::PLUGIN_SLOT_MOD_TRAY_HEIGHT * 0.25)};
    constexpr int PLUGIN_MOD_TARGET_SLIDER_WIDTH {PLUGIN_MOD_TARGET_SLIDER_HEIGHT};

    int getChainXPos(int chainIndex, int numChains, int graphViewWidth);

    // Macros
    constexpr int NUM_MACROS {4};
    constexpr int MACRO_WIDTH {64};
    constexpr int MACRO_HEIGHT {104};
    constexpr int MACRO_YPAD {10};

    // Modulation sources
    constexpr int MODULATION_BAR_WIDTH {572};
    constexpr int MODULATION_BAR_HEIGHT {130};
    constexpr int MODULATION_LIST_WIDTH {160};
    constexpr int MODULATION_LIST_COLUMN_WIDTH {MODULATION_LIST_WIDTH / 2};
    constexpr int MODULATION_LIST_BUTTON_HEIGHT {24};
}
