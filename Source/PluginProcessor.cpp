#include "PluginProcessor.h"

// Creates the JUCE plugin filter instance
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AetherDeEsserAudioProcessor();
}
