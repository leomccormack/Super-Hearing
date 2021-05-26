/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 6.0.8

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...

//[/Headers]

#include "PluginEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...

//[/MiscUserDefs]

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor* ownerFilter)
    : AudioProcessorEditor(ownerFilter),progressbar(progress)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    CBpitchShift.reset (new juce::ComboBox ("new combo box"));
    addAndMakeVisible (CBpitchShift.get());
    CBpitchShift->setEditableText (false);
    CBpitchShift->setJustificationType (juce::Justification::centredLeft);
    CBpitchShift->setTextWhenNothingSelected (juce::String());
    CBpitchShift->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    CBpitchShift->addListener (this);

    CBpitchShift->setBounds (98, 78, 112, 19);

    s_DoAestimation.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (s_DoAestimation.get());
    s_DoAestimation->setRange (0, 0.99, 0.01);
    s_DoAestimation->setSliderStyle (juce::Slider::LinearHorizontal);
    s_DoAestimation->setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    s_DoAestimation->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff5c5d5e));
    s_DoAestimation->setColour (juce::Slider::trackColourId, juce::Colour (0xff315b6e));
    s_DoAestimation->setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    s_DoAestimation->setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00ffffff));
    s_DoAestimation->addListener (this);

    s_DoAestimation->setBounds (119, 40, 144, 32);

    s_postGain_dB.reset (new juce::Slider ("new slider"));
    addAndMakeVisible (s_postGain_dB.get());
    s_postGain_dB->setRange (-6, 12, 0.01);
    s_postGain_dB->setSliderStyle (juce::Slider::LinearHorizontal);
    s_postGain_dB->setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
    s_postGain_dB->setColour (juce::Slider::backgroundColourId, juce::Colour (0xff5c5d5e));
    s_postGain_dB->setColour (juce::Slider::trackColourId, juce::Colour (0xff315b6e));
    s_postGain_dB->setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    s_postGain_dB->setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00ffffff));
    s_postGain_dB->addListener (this);

    s_postGain_dB->setBounds (336, 72, 144, 32);

    tb_enableDiff.reset (new juce::ToggleButton ("new toggle button"));
    addAndMakeVisible (tb_enableDiff.get());
    tb_enableDiff->setButtonText (juce::String());
    tb_enableDiff->addListener (this);

    tb_enableDiff->setBounds (440, 44, 32, 24);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (500, 112);


    //[Constructor] You can add your own custom stuff here..
	hVst = ownerFilter;
    hUS = hVst->getFXHandle();

    /* add combo box options */
    CBpitchShift->addItem(TRANS("None"),  ULTRASONICLIB_PITCHSHFT_NONE);
    CBpitchShift->addItem(TRANS("Down 1 Oct"), ULTRASONICLIB_PITCHSHFT_DOWN_1_OCT);
    CBpitchShift->addItem(TRANS("Down 2 Oct"), ULTRASONICLIB_PITCHSHFT_DOWN_2_OCT);
    CBpitchShift->addItem(TRANS("Down 3 Oct"), ULTRASONICLIB_PITCHSHFT_DOWN_3_OCT);
    CBpitchShift->addItem(TRANS("Use CH7"), ULTRASONICLIB_PITCHSHFT_USE_CHANNEL_7);

	/* fetch current configuration */
    CBpitchShift->setSelectedId((int)ultrasoniclib_getPitchShiftOption(hUS), dontSendNotification);
    s_DoAestimation->setValue((double)ultrasoniclib_getDoAaveragingCoeff(hUS), dontSendNotification);
    s_postGain_dB->setValue((double)ultrasoniclib_getPostGain_dB(hUS), dontSendNotification);
    tb_enableDiff->setToggleState((bool)ultrasoniclib_getEnableDiffuseness(hUS), dontSendNotification);

    /* tooltips */
    CBpitchShift->setTooltip("Pitch shift");

    /* Specify screen refresh rate */
    startTimer(TIMER_GUI_RELATED, 40);

    /* warnings */
    currentWarning = k_warning_none;

    //[/Constructor]
}

PluginEditor::~PluginEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    CBpitchShift = nullptr;
    s_DoAestimation = nullptr;
    s_postGain_dB = nullptr;
    tb_enableDiff = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void PluginEditor::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colours::white);

    {
        int x = 0, y = 70, width = 498, height = 42;
        juce::Colour fillColour1 = juce::Colour (0xff1c3949), fillColour2 = juce::Colour (0xff071e22);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setGradientFill (juce::ColourGradient (fillColour1,
                                             8.0f - 0.0f + x,
                                             120.0f - 70.0f + y,
                                             fillColour2,
                                             8.0f - 0.0f + x,
                                             96.0f - 70.0f + y,
                                             false));
        g.fillRect (x, y, width, height);
    }

    {
        int x = 10, y = 71, width = 476, height = 33;
        juce::Colour fillColour = juce::Colour (0x10c7c7c7);
        juce::Colour strokeColour = juce::Colour (0x1fffffff);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 0, y = 30, width = 530, height = 40;
        juce::Colour fillColour1 = juce::Colour (0xff1c3949), fillColour2 = juce::Colour (0xff071e22);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setGradientFill (juce::ColourGradient (fillColour1,
                                             8.0f - 0.0f + x,
                                             32.0f - 30.0f + y,
                                             fillColour2,
                                             8.0f - 0.0f + x,
                                             56.0f - 30.0f + y,
                                             false));
        g.fillRect (x, y, width, height);
    }

    {
        float x = 1.0f, y = 2.0f, width = 498.0f, height = 31.0f;
        juce::Colour fillColour1 = juce::Colour (0xff061c20), fillColour2 = juce::Colour (0xff1c3949);
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setGradientFill (juce::ColourGradient (fillColour1,
                                             0.0f - 1.0f + x,
                                             32.0f - 2.0f + y,
                                             fillColour2,
                                             528.0f - 1.0f + x,
                                             32.0f - 2.0f + y,
                                             false));
        g.fillRoundedRectangle (x, y, width, height, 5.000f);
        g.setColour (strokeColour);
        g.drawRoundedRectangle (x, y, width, height, 5.000f, 2.000f);
    }

    {
        int x = 10, y = 40, width = 476, height = 32;
        juce::Colour fillColour = juce::Colour (0x10c7c7c7);
        juce::Colour strokeColour = juce::Colour (0x1fffffff);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 1);

    }

    {
        int x = 16, y = 1, width = 272, height = 32;
        juce::String text (TRANS("UltrasonicSuperHearing"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (18.80f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 0, y = 0, width = 532, height = 2;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 0, y = 0, width = 2, height = 120;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 498, y = 3, width = 6, height = 117;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 0, y = 110, width = 512, height = 2;
        juce::Colour strokeColour = juce::Colour (0xffb9b9b9);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (strokeColour);
        g.drawRect (x, y, width, height, 2);

    }

    {
        int x = 15, y = 71, width = 96, height = 35;
        juce::String text (TRANS("Pitch Shift:"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 16, y = 40, width = 144, height = 30;
        juce::String text (TRANS("DoA Averaging:"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 232, y = 72, width = 144, height = 30;
        juce::String text (TRANS("Post Gain (dB)"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    {
        int x = 319, y = 40, width = 144, height = 30;
        juce::String text (TRANS("Enable Diff:"));
        juce::Colour fillColour = juce::Colours::white;
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.setFont (juce::Font (15.00f, juce::Font::plain).withTypefaceStyle ("Bold"));
        g.drawText (text, x, y, width, height,
                    juce::Justification::centredLeft, true);
    }

    //[UserPaint] Add your own custom painting code here..

    /* display version/date built */
	g.setColour(Colours::white);
	g.setFont(Font(11.00f, Font::plain));
	g.drawText(TRANS("Ver ") + JucePlugin_VersionString + BUILD_VER_SUFFIX + TRANS(", Build Date ") + __DATE__ + TRANS(" "),
		230, 16, 530, 11,
		Justification::centredLeft, true);

    /* display warning message */
    g.setColour(Colours::red);
    g.setFont(Font(11.00f, Font::plain));
    switch (currentWarning){
        case k_warning_none:
            break;
        case k_warning_supported_fs:
            g.drawText(TRANS("Set sample rate to 192kHz"),
                       getBounds().getWidth()-225, 6, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_frameSize:
            g.drawText(TRANS("Set frame size to multiple of ") + String(ultrasoniclib_getFrameSize()),
                       getBounds().getWidth()-225, 6, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_NInputCH:
            g.drawText(TRANS("Insufficient number of input channels (") + String(hVst->getTotalNumInputChannels()) +
                       TRANS("/") + String(ultrasoniclib_getNInputCHrequired(hUS)) + TRANS(")"),
                       getBounds().getWidth()-225, 6, 530, 11,
                       Justification::centredLeft, true);
            break;
        case k_warning_NOutputCH:
            g.drawText(TRANS("Insufficient number of output channels (") + String(hVst->getTotalNumOutputChannels()) +
                       TRANS("/") + String(ultrasoniclib_getNOutputCHrequired(hUS)) + TRANS(")"),
                       getBounds().getWidth()-225, 6, 530, 11,
                       Justification::centredLeft, true);
            break;
    }


    //[/UserPaint]
}

void PluginEditor::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..

	repaint();
    //[/UserResized]
}

void PluginEditor::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == CBpitchShift.get())
    {
        //[UserComboBoxCode_CBpitchShift] -- add your combo box handling code here..
        ultrasoniclib_setPitchShiftOption(hUS, (ULTRASONICLIB_PITCHSHFT_OPTIONS)CBpitchShift->getSelectedId());
        //[/UserComboBoxCode_CBpitchShift]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}

void PluginEditor::sliderValueChanged (juce::Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == s_DoAestimation.get())
    {
        //[UserSliderCode_s_DoAestimation] -- add your slider handling code here..
        ultrasoniclib_setDoAaveragingCoeff(hUS, (float)s_DoAestimation->getValue());
        //[/UserSliderCode_s_DoAestimation]
    }
    else if (sliderThatWasMoved == s_postGain_dB.get())
    {
        //[UserSliderCode_s_postGain_dB] -- add your slider handling code here..
        ultrasoniclib_setPostGain_dB(hUS, (float)s_postGain_dB->getValue());
        //[/UserSliderCode_s_postGain_dB]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}

void PluginEditor::buttonClicked (juce::Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == tb_enableDiff.get())
    {
        //[UserButtonCode_tb_enableDiff] -- add your button handler code here..
        ultrasoniclib_setEnableDiffuseness(hUS, (int)tb_enableDiff->getToggleState());
        //[/UserButtonCode_tb_enableDiff]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void PluginEditor::timerCallback(int timerID)
{
    switch(timerID){
        case TIMER_PROCESSING_RELATED:
            /* handled in PluginProcessor */
            break;

        case TIMER_GUI_RELATED:
            /* parameters whos values can change internally should be periodically refreshed */

            /* Progress bar */
            if(ultrasoniclib_getCodecStatus(hUS)==CODEC_STATUS_INITIALISING){
                addAndMakeVisible(progressbar);
                progress = (double)ultrasoniclib_getProgressBar0_1(hUS);
                char text[ULTRASONICLIB_PROGRESSBARTEXT_CHAR_LENGTH];
                ultrasoniclib_getProgressBarText(hUS, (char*)text);
                progressbar.setTextToDisplay(String(text));
            }
            else
                removeChildComponent(&progressbar);

            /* display warning message, if needed */
            if ( ((ultrasoniclib_getDAWsamplerate(hUS) < 192e3)) ){
                currentWarning = k_warning_supported_fs;
                repaint(0,0,getWidth(),32);
            }
            else if ((hVst->getCurrentBlockSize() % ultrasoniclib_getFrameSize()) != 0){
                currentWarning = k_warning_frameSize;
                repaint(0,0,getWidth(),32);
            }
            else if ((hVst->getCurrentNumInputs() < ultrasoniclib_getNInputCHrequired(hUS))){
                currentWarning = k_warning_NInputCH;
                repaint(0,0,getWidth(),32);
            }
            else if ((hVst->getCurrentNumOutputs() < ultrasoniclib_getNOutputCHrequired(hUS))){
                currentWarning = k_warning_NOutputCH;
                repaint(0,0,getWidth(),32);
            }
    }
}


//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="PluginEditor" componentName=""
                 parentClasses="public AudioProcessorEditor, public MultiTimer"
                 constructorParams="PluginProcessor* ownerFilter" variableInitialisers="AudioProcessorEditor(ownerFilter),progressbar(progress)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="500" initialHeight="112">
  <BACKGROUND backgroundColour="ffffffff">
    <RECT pos="0 70 498 42" fill="linear: 8 120, 8 96, 0=ff1c3949, 1=ff071e22"
          hasStroke="0"/>
    <RECT pos="10 71 476 33" fill="solid: 10c7c7c7" hasStroke="1" stroke="1.1, mitered, butt"
          strokeColour="solid: 1fffffff"/>
    <RECT pos="0 30 530 40" fill="linear: 8 32, 8 56, 0=ff1c3949, 1=ff071e22"
          hasStroke="0"/>
    <ROUNDRECT pos="1 2 498 31" cornerSize="5.0" fill="linear: 0 32, 528 32, 0=ff061c20, 1=ff1c3949"
               hasStroke="1" stroke="2, mitered, butt" strokeColour="solid: ffb9b9b9"/>
    <RECT pos="10 40 476 32" fill="solid: 10c7c7c7" hasStroke="1" stroke="1.1, mitered, butt"
          strokeColour="solid: 1fffffff"/>
    <TEXT pos="16 1 272 32" fill="solid: ffffffff" hasStroke="0" text="UltrasonicSuperHearing"
          fontname="Default font" fontsize="18.8" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <RECT pos="0 0 532 2" fill="solid: 61a52a" hasStroke="1" stroke="2, mitered, butt"
          strokeColour="solid: ffb9b9b9"/>
    <RECT pos="0 0 2 120" fill="solid: 61a52a" hasStroke="1" stroke="2, mitered, butt"
          strokeColour="solid: ffb9b9b9"/>
    <RECT pos="498 3 6 117" fill="solid: 61a52a" hasStroke="1" stroke="2, mitered, butt"
          strokeColour="solid: ffb9b9b9"/>
    <RECT pos="0 110 512 2" fill="solid: 61a52a" hasStroke="1" stroke="2, mitered, butt"
          strokeColour="solid: ffb9b9b9"/>
    <TEXT pos="15 71 96 35" fill="solid: ffffffff" hasStroke="0" text="Pitch Shift:"
          fontname="Default font" fontsize="15.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="16 40 144 30" fill="solid: ffffffff" hasStroke="0" text="DoA Averaging:"
          fontname="Default font" fontsize="15.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="232 72 144 30" fill="solid: ffffffff" hasStroke="0" text="Post Gain (dB)"
          fontname="Default font" fontsize="15.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
    <TEXT pos="319 40 144 30" fill="solid: ffffffff" hasStroke="0" text="Enable Diff:"
          fontname="Default font" fontsize="15.0" kerning="0.0" bold="1"
          italic="0" justification="33" typefaceStyle="Bold"/>
  </BACKGROUND>
  <COMBOBOX name="new combo box" id="aeb0b2f644784061" memberName="CBpitchShift"
            virtualName="" explicitFocusOrder="0" pos="98 78 112 19" editable="0"
            layout="33" items="" textWhenNonSelected="" textWhenNoItems="(no choices)"/>
  <SLIDER name="new slider" id="ace036a85eec9703" memberName="s_DoAestimation"
          virtualName="" explicitFocusOrder="0" pos="119 40 144 32" bkgcol="ff5c5d5e"
          trackcol="ff315b6e" textboxtext="ffffffff" textboxbkgd="ffffff"
          min="0.0" max="0.99" int="0.01" style="LinearHorizontal" textBoxPos="TextBoxRight"
          textBoxEditable="1" textBoxWidth="60" textBoxHeight="20" skewFactor="1.0"
          needsCallback="1"/>
  <SLIDER name="new slider" id="e76ef92f02fe0bab" memberName="s_postGain_dB"
          virtualName="" explicitFocusOrder="0" pos="336 72 144 32" bkgcol="ff5c5d5e"
          trackcol="ff315b6e" textboxtext="ffffffff" textboxbkgd="ffffff"
          min="-6.0" max="12.0" int="0.01" style="LinearHorizontal" textBoxPos="TextBoxRight"
          textBoxEditable="1" textBoxWidth="60" textBoxHeight="20" skewFactor="1.0"
          needsCallback="1"/>
  <TOGGLEBUTTON name="new toggle button" id="4e84428f87dc3d5d" memberName="tb_enableDiff"
                virtualName="" explicitFocusOrder="0" pos="440 44 32 24" buttonText=""
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

