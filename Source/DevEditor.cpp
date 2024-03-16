#include "DevEditor.h"

DevEditor::DevEditor(APAudioProcessor& proc_) : proc(proc_), msegComponent(msegData)
{
	addMouseListener(this, false);
	msegData.reset();
	msegComponent.setParams(proc.mseg1Params.wave, proc.mseg1Params.sync, proc.mseg1Params.rate, proc.mseg1Params.beat, proc.mseg1Params.depth, proc.mseg1Params.offset,
		proc.mseg1Params.phase, proc.mseg1Params.enable, proc.mseg1Params.xgrid, proc.mseg1Params.ygrid, proc.mseg1Params.loop);
	msegComponent.setEditable(true);
	msegComponent.setDrawMode(false, gin::MSEGComponent::DrawMode::up);
	addAndMakeVisible(msegComponent);
	//msegComponent.repaint();
	msegLabel.setText("MSEG1", juce::dontSendNotification);
	addAndMakeVisible(msegLabel);
}

void DevEditor::resized()
{
	msegLabel.setBounds(75, 40, 400, 35);
	msegComponent.setBounds(75, 75, 400, 200);

}

void DevEditor::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) {
	float xgrid = proc.mseg1Params.xgrid->getUserValue();
	float ygrid = proc.mseg1Params.ygrid->getUserValue();
	if (!juce::ModifierKeys::currentModifiers.isAnyModifierKeyDown()) {
		if (wheel.deltaY > 0) {
			if (!wheel.isReversed)
			{
				proc.mseg1Params.xgrid->setUserValue(xgrid + 1.f);
			}
			else
			{
				proc.mseg1Params.xgrid->setUserValue(xgrid - 1.f);
			}
		}
		if (wheel.deltaY <= 0) {
			if (!wheel.isReversed)
			{
				proc.mseg1Params.xgrid->setUserValue(xgrid - 1.f);
			}
			else
			{
				proc.mseg1Params.xgrid->setUserValue(xgrid + 1.f);
			}
		}
	}
	else {
		if (wheel.deltaY > 0) {
			if (!wheel.isReversed)
			{
				proc.mseg1Params.ygrid->setUserValue(ygrid + 1.f);
			}
			else
			{
				proc.mseg1Params.ygrid->setUserValue(ygrid - 1.f);
			}
		}
		if (wheel.deltaY <= 0) {
			if (!wheel.isReversed)
			{
				proc.mseg1Params.ygrid->setUserValue(ygrid - 1.f);
			}
			else
			{
				proc.mseg1Params.ygrid->setUserValue(ygrid + 1.f);
			}
		}
	}
	repaint();
}


void DevEditor::valueUpdated(gin::Parameter* param)
{
}
