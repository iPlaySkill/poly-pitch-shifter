#include "Parameters.h"
#include <cassert>
#include <iostream>
#include <set>
#include <string>

int main()
{
    std::cout << "[RUNNING] ParametersTest..." << std::endl;

    auto layout = pps::params::createParameterLayout();
    std::set<std::string> paramIds;
    int count = 0;

    for (auto it = layout.begin(); it != layout.end(); ++it)
    {
        const auto *param = dynamic_cast<const juce::RangedAudioParameter *>(*it);
        assert(param != nullptr);

        const std::string id = param->getParameterID().toStdString();
        assert(!id.empty());
        assert(paramIds.find(id) == paramIds.end()); // Verify uniqueness
        paramIds.insert(id);
        ++count;
    }

    assert(count == 7);
    assert(paramIds.count(pps::params::pidBypass) == 1);
    assert(paramIds.count(pps::params::pidShift) == 1);
    assert(paramIds.count(pps::params::pidFine) == 1);
    assert(paramIds.count(pps::params::pidMix) == 1);
    assert(paramIds.count(pps::params::pidGain) == 1);
    assert(paramIds.count(pps::params::pidTransientBypass) == 1);
    assert(paramIds.count(pps::params::pidTransientSensitivity) == 1);

    std::cout << "  Passed: All 7 plugin parameters uniquely registered in layout." << std::endl;
    std::cout << "[PASSED] ParametersTest succeeded." << std::endl;
    return 0;
}
