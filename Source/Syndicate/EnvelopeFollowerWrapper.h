#pragma once

#include <memory>
#include "WEFilters/AREnvelopeFollowerSquareLaw.h"

struct EnvelopeFollowerWrapper {
    std::shared_ptr<WECore::AREnv::AREnvelopeFollowerSquareLaw> envelope;
    float amount;
    bool useSidechainInput;
};
