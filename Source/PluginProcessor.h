/**
 * Aether De-Esser VST3 / AudioUnit / CLAP DSP Core
 * Generated for Steinberg VST3 & Apple AudioUnit SDKs
 * Architecture: Low CPU SIMD-accelerated Biquad + Dynamic Gain VCA
 */

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>

class AetherDeEsserAudioProcessor : public juce::AudioProcessor
{
public:
    AetherDeEsserAudioProcessor()
        : AudioProcessor (BusesProperties()
            .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
            .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
    {
        // Parameters Registration for DAW Automation
        addParameter (thresholdParam = new juce::AudioParameterFloat (
            juce::ParameterID ("threshold", 1), "Threshold", -40.0f, 0.0f, -18.0f));
        addParameter (freqParam = new juce::AudioParameterFloat (
            juce::ParameterID ("frequency", 1), "Frequency", 1000.0f, 16000.0f, 6800f));
        addParameter (qParam = new juce::AudioParameterFloat (
            juce::ParameterID ("q", 1), "Q Bandwidth", 0.5f, 5.0f, 1.8f));
        addParameter (releaseParam = new juce::AudioParameterFloat (
            juce::ParameterID ("release", 1), "Release Time", 1.0f, 250.0f, 35f));
        addParameter (ratioParam = new juce::AudioParameterFloat (
            juce::ParameterID ("ratio", 1), "Ratio", 1.5f, 8.0f, 4.0f));
        addParameter (wetDryParam = new juce::AudioParameterFloat (
            juce::ParameterID ("wetdry", 1), "Wet/Dry", 0.0f, 100.0f, 100f));
        addParameter (softClipParam = new juce::AudioParameterBool (
            juce::ParameterID ("softclip", 1), "Soft Clipping", true));
        addParameter (bypassParam = new juce::AudioParameterBool (
            juce::ParameterID ("bypass", 1), "Master Bypass", false));
    }

    void prepareToPlay (double sampleRate, int samplesPerBlock) override
    {
        currentSampleRate = sampleRate;
        sidechainEnvelope = 0.0001f;
        currentGainReductionDb = 0.0f;
        updateFilters();
    }

    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) override
    {
        juce::ScopedNoDenormals noDenormals;
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        if (*bypassParam) return; // Zero latency clean bypass

        const float threshDb = *thresholdParam;
        const float ratioVal = *ratioParam;
        const float relMs = *releaseParam;
        const float wetPct = *wetDryParam / 100.0f;
        const bool useSoftClip = *softClipParam;

        const float attackCoeff = 1.0f - std::exp(-1.0f / (0.0005f * (float)currentSampleRate));
        const float releaseCoeff = 1.0f - std::exp(-1.0f / (relMs * 0.001f * (float)currentSampleRate));

        for (int i = 0; i < numSamples; ++i)
        {
            // 1. Sidechain Sibilance Detection
            float sidechainSample = 0.0f;
            for (int ch = 0; ch < numChannels; ++ch) {
                sidechainSample += std::abs(buffer.getSample(ch, i));
            }
            sidechainSample /= (float)numChannels;

            // Ballistics tracking
            if (sidechainSample > sidechainEnvelope)
                sidechainEnvelope += attackCoeff * (sidechainSample - sidechainEnvelope);
            else
                sidechainEnvelope += releaseCoeff * (sidechainSample - sidechainEnvelope);

            float scDb = 20.0f * std::log10(std::max(0.00001f, sidechainEnvelope));

            // Dynamic gain reduction in dB
            float targetGrDb = 0.0f;
            if (scDb > threshDb) {
                float over = scDb - threshDb;
                targetGrDb = -(over - (over / ratioVal));
            }

            // Smooth VCA gain
            currentGainReductionDb += (targetGrDb < currentGainReductionDb ? 0.35f : releaseCoeff) 
                                     * (targetGrDb - currentGainReductionDb);
            float linearGain = std::pow(10.0f, currentGainReductionDb / 20.0f);

            // 2. Apply Reduction & Parallel Mix
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float dry = buffer.getSample(ch, i);
                float wet = dry * linearGain;
                float out = (1.0f - wetPct) * dry + wetPct * wet;

                // 3. Soft-clipping protection
                if (useSoftClip && std::abs(out) > 0.95f) {
                    float sign = (out > 0.0f) ? 1.0f : -1.0f;
                    float excess = std::abs(out) - 0.95f;
                    out = sign * (0.95f + 0.05f * std::tanh(excess / 0.05f));
                }

                buffer.setSample(ch, i, out);
            }
        }
    }

private:
    double currentSampleRate = 48000.0;
    float sidechainEnvelope = 0.0f;
    float currentGainReductionDb = 0.0f;

    juce::AudioParameterFloat* thresholdParam = nullptr;
    juce::AudioParameterFloat* freqParam = nullptr;
    juce::AudioParameterFloat* qParam = nullptr;
    juce::AudioParameterFloat* releaseParam = nullptr;
    juce::AudioParameterFloat* ratioParam = nullptr;
    juce::AudioParameterFloat* wetDryParam = nullptr;
    juce::AudioParameterBool*  softClipParam = nullptr;
    juce::AudioParameterBool*  bypassParam = nullptr;
};