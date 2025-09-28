/*
 ==============================================================================

 This file is part of UltrasonicSuperHearing.
 Copyright (c) 2020 - Leo McCormack.

 UltrasonicSuperHearing is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 UltrasonicSuperHearing is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with UltrasonicSuperHearing.  If not, see <http://www.gnu.org/licenses/>.

 ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "PluginProcessor.h"

typedef enum _WARNINGS{
    k_warning_none,
    k_warning_supported_fs,
    k_warning_frameSize,
    k_warning_NInputCH,
    k_warning_NOutputCH
}WARNINGS;

class PluginEditor  : public AudioProcessorEditor,
                      public Timer,
                      public juce::ComboBox::Listener,
                      public juce::Slider::Listener,
                      public juce::Button::Listener
{
public:
    PluginEditor (PluginProcessor& p);
    ~PluginEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;
    void sliderValueChanged (juce::Slider* sliderThatWasMoved) override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;

private:
    PluginProcessor& processor;
    void* hUS;
    void timerCallback() override;
    double progress = 0.0;
    ProgressBar progressbar;

    /* warnings */
    WARNINGS currentWarning;

    /* tooltips */
    SharedResourcePointer<TooltipWindow> tipWindow;
    HyperlinkButton publicationLink { "(Related Publication)", { "https://leomccormack.github.io/sparta-site/docs/help/related-publications/pulkki2021superhuman.pdf" } };

    std::unique_ptr<ComboBoxWithAttachment> CBpitchShift;
    std::unique_ptr<juce::Slider> s_DoAestimation;
    std::unique_ptr<juce::Slider> s_postGain_dB;
    std::unique_ptr<juce::ToggleButton> tb_enableDiff;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
