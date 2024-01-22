#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <melatonin_blur/melatonin_blur.h>


class OrbitViz : public juce::Component {
public:
	struct Position { float x; float y; };
	Position epicycleCenter{ 1,0 };

	void setScale(float input) { scale = input; }

	void addCircle(juce::Path& path, juce::Point<float>& center, float size) {
		path.addEllipse(center.getX() - size*0.5f, center.getY() -size*0.5f, size, size);
	}

	juce::Rectangle<float> getBody(juce::Point<float>& center, float size) {
		return juce::Rectangle<float>(center.getX() - size * 0.5f, center.getY() - size * 0.5f, size, size);
	}

	void paint(juce::Graphics& g) override {
		// redefine variables
		juce::Rectangle<float> bounds = getLocalBounds().toFloat();
		auto width = bounds.getWidth() * scale;
		auto equantPos = juce::Point<float>(bounds.getCentreX(), bounds.getCentreY() + equant * 0.5f * (width / 2.95f) );
		auto r1 = defRad *  (width / 6.f);
		auto r2 = epi1Rad * (width / 6.f);
		auto r3 = epi2Rad * (width / 6.f);
		auto r4 = epi3Rad * (width / 6.f);
		auto p1 = defPhase;
		auto p2 = epi1Phase;
		auto p3 = epi2Phase;
		auto p4 = epi3Phase;
		auto center = bounds.getCentre();


		juce::Colour orbitColor = juce::Colours::white.darker(0.2f); // for deferent and epicycles
		auto stroketype = juce::PathStrokeType(1.0f, juce::PathStrokeType::JointStyle::mitered, juce::PathStrokeType::EndCapStyle::butt);

		// osc 1 -------------------------
		juce::Point<float> osc1;
		osc1 = juce::Point<float>(center.getX() + r1 * std::cos(p1), center.getY() + r1 * std::sin(p1));
	
		juce::Path circ1;
		addCircle(circ1, center, r1*2.f);
		g.setColour(orbitColor);

		melatonin::DropShadow shadow; // = { juce::Colours::black, 8 };
		shadow.setColor(juce::Colours::white);
		shadow.setRadius(8);
		shadow.setOffset(juce::Point<int>(0, 0));
		shadow.render(g, circ1, stroketype);

		g.strokePath(circ1, stroketype, {}); // osc1 orbit

		
		// osc 2 -------------------------
		juce::Point<float> osc2;
		osc2 = juce::Point<float>(osc1.x + r2 * std::cos(p2), osc1.y + r2 * std::sin(p2));

		juce::Path circ2;
		addCircle(circ2, osc1, r2 * 2.f);
		g.setColour(orbitColor);
		shadow.render(g, circ2, stroketype);
		g.strokePath(circ2, stroketype, {}); // osc2 orbit


		// osc 3 -------------------------
		juce::Point<float> osc3;
		juce::Path circ3;
		if (algo == 0 || algo == 1) {
			osc3 = juce::Point<float>(osc2.x + r3 * std::cos(p3), osc2.y + r3 * std::sin(p3));
			addCircle(circ3, osc2, r3 * 2.f);
		}
		if (algo == 2 || algo == 3) {
			osc3 = juce::Point<float>(osc1.x + r3 * std::cos(p3), osc1.y + r3 * std::sin(p3));
			addCircle(circ3, osc1, r3 * 2.f);
		}

		g.setColour(orbitColor);
		shadow.render(g, circ3, stroketype);
		g.strokePath(circ3, stroketype, {}); // osc3 orbit



		// osc 4 -------------------------
		juce::Point<float> osc4;
		juce::Path circ4;
		if (algo == 0 || algo == 2) {
			osc4 = juce::Point<float>(osc3.x + r4 * std::cos(p4), osc3.y + r4 * std::sin(p4));
			addCircle(circ4, osc3, r4 * 2.f);
		}
		if (algo == 1) {
			osc4 = juce::Point<float>(osc2.x + r4 * std::cos(p4), osc2.y + r4 * std::sin(p4));
			addCircle(circ4, osc2, r4 * 2.f);
		}
		if (algo == 3) {
			osc4 = juce::Point<float>(osc1.x + r4 * std::cos(p4), osc1.y + r4 * std::sin(p4));
			addCircle(circ4, osc1, r4 * 2.f);
		}

		g.setColour(orbitColor);
		shadow.render(g, circ4, stroketype);
		g.strokePath(circ4, stroketype, {}); // osc4 orbit

		// -------------------------------

		// body positions / rects
		auto equantBody = getBody(equantPos, 9.f);
		auto equantShadow = getBody(equantPos, 14.f);
		auto body1 = getBody(osc1, 9.0f);
		auto body2 = getBody(osc2, 9.0f);
		auto body3 = getBody(osc3, 9.0f);
		auto body4 = getBody(osc4, 9.0f);

		// lines to audible planets
		g.setColour(juce::Colours::grey);
		g.drawLine(equantBody.getCentreX(), equantBody.getCentreY(), body4.getCentreX(), body4.getCentreY(), 2.0f);
		if (algo == 2 || algo == 3)
		{
			g.drawLine(equantBody.getCentreX(), equantBody.getCentreY(), body2.getCentreX(), body2.getCentreY(), 2.0f);
		}
		if (algo == 1 || algo == 3)
		{
			g.drawLine(equantBody.getCentreX(), equantBody.getCentreY(), body3.getCentreX(), body3.getCentreY(), 2.0f);
		}

		// drawing bodies in reverse order
		g.setColour(juce::Colours::blue);
		juce::Path body4Outline;
		auto body4Center = body4.getCentre();
		addCircle(body4Outline, body4Center, 7.0f);
		shadow.setColor(juce::Colours::white); // blue.brighter(0.4f));
		shadow.setRadius(6);
		shadow.render(g, body4Outline, stroketype);
		g.fillEllipse(body4);

		g.setColour(juce::Colours::green);
		juce::Path body3Outline;
		auto body3Center = body3.getCentre();
		addCircle(body3Outline, body3Center, 7.0f);
		shadow.render(g, body3Outline, stroketype);
		g.fillEllipse(body3);

		g.setColour(juce::Colours::yellow);
		juce::Path body2Outline;
		auto body2Center = body2.getCentre();
		addCircle(body2Outline, body2Center, 7.0f);
		shadow.render(g, body2Outline, stroketype);
		g.fillEllipse(body2);

		g.setColour(juce::Colours::red);
		juce::Path body1Outline;
		auto body1Center = body1.getCentre();
		addCircle(body1Outline, body1Center, 7.0f);
		shadow.render(g, body1Outline, stroketype);
		g.fillEllipse(body1);


		g.setColour(juce::Colours::black);
		juce::Path equantBodyOutline;
		equantBodyOutline.addEllipse(equantShadow);
		shadow.setColor(juce::Colours::red);
		shadow.setRadius(10);
		shadow.render(g, equantBodyOutline, stroketype);
		g.fillEllipse(equantBody);
	}

	void setEquant(float input) { equant = std::clamp(input, -.5f, .5f); }
	void setDefPhase(float phase) { defPhase = phase; }
	void setEpi1Phase(float phase) { epi1Phase = phase; }
	void setEpi2Phase(float phase) { epi2Phase = phase; }
	void setEpi3Phase(float phase) { epi3Phase = phase; }
	void setDefRad(float input) { defRad = validate(input); }
	void setEpi1Rad(float input) { epi1Rad = validate(input); }
	void setEpi2Rad(float input) { epi2Rad = validate(input); }
	void setEpi3Rad(float input) { epi3Rad = validate(input); }
	void setAlgo(int input) { algo = input; }
	float validate(float x) {
		return std::clamp(x, 0.f, 1.f);
	}
private:
	float equant{ 0.f }, defPhase{ 0.f }, epi1Phase{ 0.f }, epi2Phase{ 0.f }, epi3Phase{ 0.f }, defRad{ 1.f }, epi1Rad{ 0.5f }, epi2Rad{ 0.25f }, epi3Rad{ 0.2f };
	int algo{ 0 };
	float scale{ 1.f };

};
