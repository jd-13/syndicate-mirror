#include "PluginScanStatusMessage.h"

#include "PluginScanStatusBar.h"

PluginScanStatusBar::PluginScanStatusBar(PluginScanClient& pluginScanClient,
                                         const SelectorComponentStyle& style) :
        _pluginScanClient(pluginScanClient) {

    statusLbl.reset(new juce::Label("Status Label", juce::String()));
    addAndMakeVisible(statusLbl.get());
    statusLbl->setFont(juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Regular"));
    statusLbl->setJustificationType(juce::Justification::centredLeft);
    statusLbl->setEditable(false, false, false);
    statusLbl->setColour(juce::Label::backgroundColourId, style.controlColour);
    statusLbl->setColour(juce::Label::textColourId, style.neutralColour);

    auto styleButton = [&style](std::unique_ptr<juce::TextButton>& button) {
        button->setLookAndFeel(style.scanButtonLookAndFeel.get());
        button->setColour(juce::TextButton::buttonOnColourId, style.controlColour);
        button->setColour(juce::TextButton::buttonColourId, style.neutralColour);
        button->setColour(juce::TextButton::textColourOnId, style.controlColour);
        button->setColour(juce::TextButton::textColourOffId, style.neutralColour);
    };

    startScanBtn.reset(new juce::TextButton("Start Scan Button"));
    addAndMakeVisible(startScanBtn.get());
    startScanBtn->setButtonText(TRANS("Start Scan"));
    startScanBtn->addListener(this);
    styleButton(startScanBtn);

    stopScanBtn.reset(new juce::TextButton("Stop Scan Button"));
    addAndMakeVisible(stopScanBtn.get());
    stopScanBtn->setButtonText(TRANS("Stop Scan"));
    stopScanBtn->addListener(this);
    styleButton(stopScanBtn);

    rescanAllBtn.reset(new juce::TextButton("Rescan All Button"));
    addAndMakeVisible(rescanAllBtn.get());
    rescanAllBtn->setButtonText(TRANS("Rescan All Plugins"));
    rescanAllBtn->addListener(this);
    styleButton(rescanAllBtn);

    rescanCrashedBtn.reset(new juce::TextButton("Rescan Crashed Button"));
    addAndMakeVisible(rescanCrashedBtn.get());
    rescanCrashedBtn->setButtonText(TRANS("Rescan Crashed Plugins"));
    rescanCrashedBtn->addListener(this);
    styleButton(rescanCrashedBtn);

    viewCrashedBtn.reset(new juce::TextButton("View Crashed Button"));
    addAndMakeVisible(viewCrashedBtn.get());
    viewCrashedBtn->setButtonText(TRANS("View Crashed Plugins"));
    viewCrashedBtn->addListener(this);
    styleButton(viewCrashedBtn);

    _pluginScanClient.addListener(this);
}

PluginScanStatusBar::~PluginScanStatusBar() {
    _pluginScanClient.stopScan();
    _pluginScanClient.removeListener(this);

    statusLbl = nullptr;
    startScanBtn = nullptr;
    stopScanBtn = nullptr;
    rescanAllBtn = nullptr;
    rescanCrashedBtn = nullptr;
    viewCrashedBtn = nullptr;
    crashedPluginsPopover = nullptr;
}

void PluginScanStatusBar::resized() {
    constexpr int STATUS_WIDTH {200};
    constexpr int SPACER_WIDTH {10};
    constexpr int NARROW_BUTTON_WIDTH {80};
    constexpr int WIDE_BUTTON_WIDTH {130};

    if (crashedPluginsPopover != nullptr) {
        crashedPluginsPopover->setBounds(getLocalBounds());
    }

    juce::Rectangle<int> availableArea = getLocalBounds();

    statusLbl->setBounds(availableArea.removeFromLeft(STATUS_WIDTH));
    availableArea.removeFromLeft(SPACER_WIDTH);
    startScanBtn->setBounds(availableArea.removeFromLeft(NARROW_BUTTON_WIDTH));
    availableArea.removeFromLeft(SPACER_WIDTH);
    stopScanBtn->setBounds(availableArea.removeFromLeft(NARROW_BUTTON_WIDTH));
    availableArea.removeFromLeft(SPACER_WIDTH);
    rescanAllBtn->setBounds(availableArea.removeFromLeft(WIDE_BUTTON_WIDTH));
    availableArea.removeFromLeft(SPACER_WIDTH);
    rescanCrashedBtn->setBounds(availableArea.removeFromLeft(WIDE_BUTTON_WIDTH));
    availableArea.removeFromLeft(SPACER_WIDTH);
    viewCrashedBtn->setBounds(availableArea.removeFromLeft(WIDE_BUTTON_WIDTH));
}

void PluginScanStatusBar::buttonClicked(juce::Button* buttonThatWasClicked) {
    if (buttonThatWasClicked == startScanBtn.get()) {
        _pluginScanClient.startScan();
    } else if (buttonThatWasClicked == stopScanBtn.get()) {
        _pluginScanClient.stopScan();
    } else if (buttonThatWasClicked == rescanAllBtn.get()) {
        _pluginScanClient.rescanAllPlugins();
    } else if (buttonThatWasClicked == rescanCrashedBtn.get()) {
        _pluginScanClient.rescanCrashedPlugins();
    } else if (buttonThatWasClicked == viewCrashedBtn.get()) {
        _createCrashedPluginsDialogue();
    }
}

void PluginScanStatusBar::handleMessage(const juce::Message& message) {
    const PluginScanStatusMessage* statusMessage {
            dynamic_cast<const PluginScanStatusMessage*>(&message)
    };

    if (statusMessage != nullptr) {

        // Set the status text
        juce::String statusString;
        const juce::String numPlugins(statusMessage->numPluginsScanned);

        if (statusMessage->isScanRunning) {
            // Scan currently running
            statusString = "Found " + numPlugins + " plugins, scanning...";
        } else if (statusMessage->scanStartedByAnotherInstance) {
            // Another instance started a scan
            statusString = "Found " + numPlugins + " plugins, scanning (alt)...";
        } else if (statusMessage->numPluginsScanned == 0 || !statusMessage->hasPreviousScan) {
            // Couldn't restore any plugins, no scan currently running
            statusString = "No plugins available - click start scan to begin";
        } else {
            // Successfully restored plugins, no scan currently running
            statusString = "Found " + numPlugins + " plugins";
        }

        _updateButtonState(statusMessage->isScanRunning, statusMessage->scanStartedByAnotherInstance);
        statusLbl->setText(statusString, juce::dontSendNotification);
    }
}

void PluginScanStatusBar::_updateButtonState(bool isScanRunning, bool scanStartedByAnotherInstance) {
    if (isScanRunning || scanStartedByAnotherInstance) {
        startScanBtn->setEnabled(false);
        stopScanBtn->setEnabled(!scanStartedByAnotherInstance);
        rescanAllBtn->setEnabled(false);
        rescanCrashedBtn->setEnabled(false);
        viewCrashedBtn->setEnabled(false);
    } else {
        startScanBtn->setEnabled(true);
        stopScanBtn->setEnabled(false);
        rescanAllBtn->setEnabled(true);
        rescanCrashedBtn->setEnabled(true);
        viewCrashedBtn->setEnabled(true);
    }
}

void PluginScanStatusBar::_createCrashedPluginsDialogue() {
    // Read the plugins from the crashed file
    juce::String bodyText;

    juce::File crashedPluginsFile = Utils::DataDirectory.getChildFile(Utils::CRASHED_PLUGINS_FILE_NAME);
    if (crashedPluginsFile.existsAsFile()) {
        const juce::String crashedPluginsStr = crashedPluginsFile.loadFileAsString();

        if (!crashedPluginsStr.isEmpty()) {
            bodyText += "The following plugins crashed during validation:\n\n";
            bodyText += crashedPluginsStr;
        }
    }

    if (bodyText.isEmpty()) {
        bodyText += "No plugins failed validation";
    }

    crashedPluginsPopover.reset(new UIUtils::PopoverComponent("Crashed plugins", bodyText, [&]() {crashedPluginsPopover.reset(); }));
    getParentComponent()->addAndMakeVisible(crashedPluginsPopover.get());
    crashedPluginsPopover->setBounds(getParentComponent()->getLocalBounds());
}
