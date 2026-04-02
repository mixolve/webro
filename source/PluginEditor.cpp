#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "WebroBinaryData.h"

namespace
{
constexpr float guiFontSize = 16.5f;
constexpr int bottomBarHeight = 30;
constexpr int outerMargin = 3;
constexpr int buttonSize = 24;
constexpr int buttonGap = 4;
constexpr int textFieldHeight = 24;
constexpr int resizeStrip = 3;
constexpr int topResizeStrip = 3;
constexpr int leftResizeStrip = 3;
constexpr int bottomResizeStrip = 8;
}

WebroAudioProcessorEditor::WebroAudioProcessorEditor (WebroAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p)
{
    audioProcessor.addChangeListener (this);

    setOpaque (true);
    setResizable (true, false);
    setResizeLimits (1, 1, 10000, 10000);
    setSize (1000, 700);

    guiTypeface = juce::Typeface::createSystemTypefaceFor (WebroBinaryData::SometypeMonoRegular_ttf,
                                                           WebroBinaryData::SometypeMonoRegular_ttfSize);
    squareLook.typeface = guiTypeface;
    squareLook.fontHeight = guiFontSize;

    addAndMakeVisible (browser);
    addAndMakeVisible (urlEditor);
    addAndMakeVisible (backButton);
    addAndMakeVisible (homeButton);
    addAndMakeVisible (forwardButton);
    rightResizeHandle = std::make_unique<InvisibleResizableBorder> (this, getConstrainer());
    rightResizeHandle->setBorderThickness ({ 0, 0, 0, resizeStrip });
    addAndMakeVisible (*rightResizeHandle);
    rightResizeHandle->setAlwaysOnTop (true);

    bottomResizeHandle = std::make_unique<InvisibleResizableBorder> (this, getConstrainer());
    bottomResizeHandle->setBorderThickness ({ 0, 0, bottomResizeStrip, 0 });
    addAndMakeVisible (*bottomResizeHandle);
    bottomResizeHandle->setAlwaysOnTop (true);

    auto urlFontOptions = juce::FontOptions (guiFontSize);

    if (guiTypeface != nullptr)
        urlFontOptions = urlFontOptions.withTypeface (guiTypeface);

    urlEditor.setFont (urlFontOptions);
    urlEditor.setJustification (juce::Justification::centredLeft);
    urlEditor.setIndents (8, 0);
    urlEditor.setBorder ({ 0, 1, 2, 1 });
    urlEditor.setPopupMenuEnabled (true);
    urlEditor.setScrollToShowCursor (true);
    urlEditor.setReadOnly (true);
    urlEditor.setCaretVisible (false);
    urlEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colours::transparentBlack);
    urlEditor.setColour (juce::TextEditor::outlineColourId, juce::Colour::fromRGB (153, 153, 255));
    urlEditor.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colour::fromRGB (153, 153, 255));
    urlEditor.setColour (juce::CaretComponent::caretColourId, juce::Colours::white);
    urlEditor.setColour (juce::TextEditor::textColourId, juce::Colours::white);
    urlEditor.onEnterEdit = [this]
    {
        urlEditor.setReadOnly (false);
        urlEditor.setCaretVisible (true);
    };
    urlEditor.onExitEdit = [this]
    {
        urlEditor.setReadOnly (true);
        urlEditor.setCaretVisible (false);
        urlEditor.moveCaretToEnd (false);
        urlEditor.setHighlightedRegion ({});
    };
    urlEditor.addListener (this);

    for (auto* button : { &backButton, &homeButton, &forwardButton })
    {
        button->addListener (this);
        button->setLookAndFeel (&squareLook);
        button->setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        button->setColour (juce::TextButton::textColourOffId, juce::Colour::fromRGB (153, 153, 255));
        button->setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    }

    auto url = audioProcessor.getCurrentURL();
    urlEditor.setText (url, juce::dontSendNotification);

    if (url.isNotEmpty())
    {
        lastLoadedURL = url;
        showingHomeScreen = false;
        browser.goToURL (url);
    }
    else
    {
        showingHomeScreen = true;
        browser.goToURL ("about:blank");
    }

    updateBarVisibility();
}

WebroAudioProcessorEditor::~WebroAudioProcessorEditor()
{
    audioProcessor.removeChangeListener (this);

    urlEditor.removeListener (this);

    for (auto* button : { &backButton, &homeButton, &forwardButton })
    {
        button->removeListener (this);
        button->setLookAndFeel (nullptr);
    }
}

void WebroAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    auto bottomBar = area.removeFromBottom (bottomBarHeight);

    browser.setBounds (area.withTrimmedLeft (leftResizeStrip)
                           .withTrimmedTop (topResizeStrip)
                           .withTrimmedRight (resizeStrip));

    auto controls = bottomBar.reduced (outerMargin, outerMargin);
    const int buttonsWidth = buttonSize * 3 + buttonGap * 2;
    auto buttonsArea = controls.removeFromLeft (buttonsWidth);
    controls.removeFromLeft (buttonGap);

    const int fieldY = controls.getY() + (controls.getHeight() - textFieldHeight) / 2;
    urlEditor.setBounds (controls.getX(), fieldY, controls.getWidth(), textFieldHeight);

    backButton.setBounds (buttonsArea.removeFromLeft (buttonSize));
    buttonsArea.removeFromLeft (buttonGap);
    homeButton.setBounds (buttonsArea.removeFromLeft (buttonSize));
    buttonsArea.removeFromLeft (buttonGap);
    forwardButton.setBounds (buttonsArea.removeFromLeft (buttonSize));

    if (rightResizeHandle != nullptr)
        rightResizeHandle->setBounds (getWidth() - resizeStrip, 0, resizeStrip, getHeight());

    if (bottomResizeHandle != nullptr)
        bottomResizeHandle->setBounds (0, getHeight() - bottomResizeStrip, getWidth(), bottomResizeStrip);
}

void WebroAudioProcessorEditor::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if (source == &audioProcessor)
        refreshFromProcessor();
}

void WebroAudioProcessorEditor::refreshFromProcessor()
{
    auto url = audioProcessor.getCurrentURL();
    urlEditor.setText (url, juce::dontSendNotification);

    if (url.isEmpty())
    {
        showingHomeScreen = true;
        lastLoadedURL.clear();
        browser.goToURL ("about:blank");
        updateBarVisibility();
        return;
    }

    showingHomeScreen = false;
    updateBarVisibility();

    if (url != lastLoadedURL)
    {
        lastLoadedURL = url;
        browser.goToURL (url);
    }
}

void WebroAudioProcessorEditor::updateBarVisibility()
{
    urlEditor.setVisible (true);
    backButton.setVisible (true);
    homeButton.setVisible (true);
    forwardButton.setVisible (true);

    if (showingHomeScreen)
        urlEditor.grabKeyboardFocus();

    resized();
}

void WebroAudioProcessorEditor::buttonClicked (juce::Button* button)
{
    urlEditor.giveAwayKeyboardFocus();

    if (button == &backButton)
        browser.goBack();
    else if (button == &homeButton)
        goHome();
    else if (button == &forwardButton)
        browser.goForward();
}

void WebroAudioProcessorEditor::textEditorFocusLost (juce::TextEditor& editor)
{
    if (&editor != &urlEditor)
        return;

    urlEditor.onExitEdit();
}

void WebroAudioProcessorEditor::textEditorReturnKeyPressed (juce::TextEditor& editor)
{
    if (&editor == &urlEditor)
        submitURL (urlEditor.getText());
}

void WebroAudioProcessorEditor::submitURL (const juce::String& enteredText)
{
    auto urlText = enteredText.trim();

    if (urlText.isEmpty())
        return;

    if (! urlText.startsWithIgnoreCase ("http://")
        && ! urlText.startsWithIgnoreCase ("https://")
        && ! urlText.startsWithIgnoreCase ("file://"))
    {
        urlText = "https://" + urlText;
    }

    showingHomeScreen = false;
    urlEditor.giveAwayKeyboardFocus();
    audioProcessor.setCurrentURL (urlText);
}

void WebroAudioProcessorEditor::goHome()
{
    showingHomeScreen = true;
    lastLoadedURL.clear();

    urlEditor.clear();
    browser.goToURL ("about:blank");
    audioProcessor.setCurrentURL ({});

    updateBarVisibility();
}
