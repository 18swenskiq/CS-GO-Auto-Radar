#pragma once
#include "imgui_color_gradient.h"

namespace gradpreset {
	void load_dust_2(ImGradient* grad) {
		grad->getMarks().clear(); // clear gradient

		grad->addMark(0.0f,		ImColor(43,  56,  60));
		grad->addMark(0.5f,		ImColor(60,  70,  73));
		grad->addMark(1.0f,		ImColor(84,  88,  74));
	}

	void load_mirage(ImGradient* grad) {
		grad->getMarks().clear();

		grad->addMark(0.0f,		ImColor(42,  74,  77));
		grad->addMark(0.45f,	ImColor(68,  75,  80));
		grad->addMark(1.0f,		ImColor(115, 96,  85));
	}

	void load_overpass(ImGradient* grad) {
		grad->getMarks().clear();

		grad->addMark(0.0f,		ImColor(42,  74,  77));
		grad->addMark(0.40f,	ImColor(57,  70,  80));
		grad->addMark(1.0f,		ImColor(115, 96,  85));
	}

	void load_cache(ImGradient* grad) {
		grad->getMarks().clear();

		grad->addMark(0.0f,		ImColor(56,  71,  76));
		grad->addMark(0.5f,		ImColor(70,  70,  80));
		grad->addMark(1.0f,		ImColor(104, 92,  82));
	}

	void load_inferno(ImGradient* grad) {
		grad->getMarks().clear();

		grad->addMark(0.0f,		ImColor(41, 74, 76));
		grad->addMark(0.5f,		ImColor(62, 74, 80));
		grad->addMark(1.0f,		ImColor(98, 99, 98));
	}

	void load_train(ImGradient* grad) {
		grad->getMarks().clear();

		grad->addMark(0.0f,		ImColor(40, 68, 76));
		grad->addMark(0.5f,		ImColor(72, 79, 90));
		grad->addMark(1.0f,		ImColor(112, 90, 62));
	}

	void load_nuke(ImGradient* grad) {
		grad->getMarks().clear();

		grad->addMark(0.0f,		ImColor(57, 56, 71));
		grad->addMark(0.25f,	ImColor(29, 56, 65));
		grad->addMark(0.75f,	ImColor(69, 72, 69));
		grad->addMark(1.0f,		ImColor(110, 90, 80));
	}

	void load_vertigo(ImGradient* grad) {
		grad->getMarks().clear();

		grad->addMark(0.0f,		ImColor(52, 31, 46));
		grad->addMark(0.5f,		ImColor(88, 58, 49));
		grad->addMark(1.0f,		ImColor(92, 89, 60));
	}
}