#include "RandEditor.h"

RandEditor::RandEditor(APAudioProcessor& proc_) : proc(proc_)
{
	addAndMakeVisible(matrix);
	addAndMakeVisible(randomizeButton);
	randomizeButton.onClick = [this] { randomize(); };
	//addAndMakeVisible(hello);
}

void RandEditor::randomize()
{
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(-1.f, 1.f);
	std::uniform_int_distribution<> envDis(0, 3);
	auto& params = proc.getPluginParameters();
	for (auto param : params) {
		if (!param->isInternal()) {
			auto& mm = *param->getModMatrix();
			if (dis(gen)>0.95) {
				for (int i = 1; i < proc.modMatrix.getNumModSources(); i++) {
					if (dis(gen) > 0.5) {
						auto modSource = gin::ModSrcId(i);
						mm.setModDepth(modSource, gin::ModDstId(param->getModIndex()), dis(gen));
					}
				}
			}
		}
	}
}

void RandEditor::resized()
{
	matrix.setBounds(5, 5, 5*56, 5*70+23);
	randomizeButton.setBounds(5, 378, 3*56, 20);
	//hello.setBounds(5, 335, 150, 20);
}