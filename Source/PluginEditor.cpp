#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
APAudioProcessorEditor::APAudioProcessorEditor (APAudioProcessor& p)
    : ProcessorEditor (p), wtProc (p)
{
	addAndMakeVisible(meter);
    addAndMakeVisible(tabbed);
    tabbed.addTab("1. Main", APColors::tabBkgd, &tab1, false, 0);
    tabbed.addTab("2. FX", APColors::tabBkgd, &tab2, false, 1);
    tabbed.addTab("3. Rand",   APColors::tabBkgd, &tab3, false, 2);

    tab1.addAndMakeVisible(editor);
	tab2.addAndMakeVisible(fxEditor);
	tab3.addAndMakeVisible(randEditor);
    
    usage.panic.onClick = [this] { wtProc.presetLoaded = true; };
    addAndMakeVisible (usage);
    
	meter.setBounds(1130, 5, 15, 30);
    usage.setBounds (45, 12, 80, 16);
    setSize (1186,725);
}

APAudioProcessorEditor::~APAudioProcessorEditor()
{
}

//==============================================================================
void APAudioProcessorEditor::paint (juce::Graphics& g)
{
    ProcessorEditor::paint (g);

    titleBar.setShowBrowser (true);

    g.fillAll (findColour (gin::PluginLookAndFeel::blackColourId));
}

void APAudioProcessorEditor::resized()
{
    ProcessorEditor::resized ();

    auto rc = getLocalBounds().reduced (1);
    rc.removeFromTop (40);
    tabbed.setBounds (rc);
    auto editorArea = tabbed.getLocalBounds();
    editorArea.removeFromBottom(tabbed.getTabBarDepth());
    editor.setBounds(editorArea);
    patchBrowser.setBounds (rc);
	fxEditor.setBounds(editorArea);
	randEditor.setBounds(editorArea);
}

void APAudioProcessorEditor::addMenuItems (juce::PopupMenu& m)
{
    m.addSeparator();
    m.addItem ("MPE", true, wtProc.globalParams.mpe->getUserValueBool(), [this]
    {
        wtProc.globalParams.mpe->setUserValue (wtProc.globalParams.mpe->getUserValueBool() ? 0.0f : 1.0f);
    });

    auto setSize = [this] (float scale)
    {
        if (auto p = findParentComponentOfClass<gin::ScaledPluginEditor>())
            p->setScale (scale);
    };

    juce::PopupMenu um;
    um.addItem ("50%",  [setSize] { setSize (0.50f); });
    um.addItem ("75%",  [setSize] { setSize (0.75f); });
    um.addItem ("100%", [setSize] { setSize (1.00f); });
    um.addItem ("150%", [setSize] { setSize (1.50f); });
    um.addItem ("200%", [setSize] { setSize (2.00f); });

    m.addSubMenu ("UI Size", um);
}
