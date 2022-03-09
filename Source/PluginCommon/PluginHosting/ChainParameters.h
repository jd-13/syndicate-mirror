#pragma once

#include <functional>
#include <JuceHeader.h>

/**
 * Stores the state of the parameter buttons shown in the header for each chain.
 *
 * Ideally each ChainParameters object would be registered as a parameter (since it needs to trigger
 * updates) but since they vary with the number of bands they can't be
 * registered. Instead a callback is provided for triggering updates, and save/restore state is
 * managed by the splitter itself.
 */
class ChainParameters {
public:
    ChainParameters(std::function<void()> onUpdateCallback) : _isBypassed(false),
                                                              _isMuted(false),
                                                              _isSoloed(false),
                                                              _onUpdateCallback(onUpdateCallback) {}

    void setBypass(bool val) {
        _isBypassed = val;
        _onUpdateCallback();
    }

    void setMute(bool val) {
        _isMuted = val;
        _onUpdateCallback();
    }

    void setSolo(bool val) {
        _isSoloed = val;
        _onUpdateCallback();
    }

    bool getBypass() const { return _isBypassed; }
    bool getMute() const { return _isMuted; }
    bool getSolo() const { return _isSoloed; }

private:
    bool _isBypassed;
    bool _isMuted;
    bool _isSoloed;

    std::function<void()> _onUpdateCallback;
};
