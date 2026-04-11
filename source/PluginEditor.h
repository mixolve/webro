#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class SquareButtonLook : public juce::LookAndFeel_V4
{
public:
    juce::Colour accent = juce::Colour::fromRGB(153,153,255);
    juce::Typeface::Ptr typeface;
    float fontHeight = 20.0f;

    void drawButtonBackground (juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour&,
                               bool isMouseOverButton,
                               bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();

        auto fill = juce::Colours::transparentBlack;

        if (isMouseOverButton)
            fill = accent.withAlpha (0.10f);

        if (isButtonDown)
            fill = accent.withAlpha (0.20f);

        g.setColour (fill);
        g.fillRect (bounds);

        g.setColour (accent);
        g.drawRect (bounds, 1.0f);
    }

    juce::Font getTextButtonFont (juce::TextButton&, int) override
    {
        auto options = juce::FontOptions (fontHeight);

        if (typeface != nullptr)
            options = options.withTypeface (typeface);

        return options;
    }
};

class UrlEditor : public juce::TextEditor
{
public:
    std::function<void()> onEnterEdit;
    std::function<void()> onExitEdit;

    void mouseDown (const juce::MouseEvent& event) override
    {
        if (onEnterEdit != nullptr)
            onEnterEdit();

        juce::TextEditor::mouseDown (event);
    }

    void focusLost (juce::Component::FocusChangeType cause) override
    {
        juce::TextEditor::focusLost (cause);

        if (onExitEdit != nullptr)
            onExitEdit();
    }
};

class InvisibleResizableCorner final : public juce::ResizableCornerComponent
{
public:
    using juce::ResizableCornerComponent::ResizableCornerComponent;

private:
    void paint (juce::Graphics&) override {}
    bool hitTest (int, int) override { return true; }
};

class InvisibleResizableBorder final : public juce::ResizableBorderComponent
{
public:
    using juce::ResizableBorderComponent::ResizableBorderComponent;

private:
    void paint (juce::Graphics&) override {}
};

class WbeAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                   public juce::ChangeListener,
                                   private juce::Button::Listener,
                                   private juce::TextEditor::Listener
{
public:
    explicit WbeAudioProcessorEditor (WbeAudioProcessor&);
    ~WbeAudioProcessorEditor() override;

    void resized() override;
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;

private:
    WbeAudioProcessor& audioProcessor;

    juce::WebBrowserComponent browser;
    UrlEditor urlEditor;
    juce::TextButton backButton    { "<" };
    juce::TextButton homeButton    { "H" };
    juce::TextButton forwardButton { ">" };

    SquareButtonLook squareLook;
    juce::Typeface::Ptr guiTypeface;
    std::unique_ptr<InvisibleResizableBorder> rightResizeHandle;
    std::unique_ptr<InvisibleResizableBorder> bottomResizeHandle;

    juce::String lastLoadedURL;
    bool showingHomeScreen = true;

    void buttonClicked (juce::Button* button) override;
    void textEditorFocusLost (juce::TextEditor& editor) override;
    void textEditorReturnKeyPressed (juce::TextEditor& editor) override;
    void goHome();
    void refreshFromProcessor();
    void updateBarVisibility();
    void submitURL (const juce::String& urlText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WbeAudioProcessorEditor)
};
