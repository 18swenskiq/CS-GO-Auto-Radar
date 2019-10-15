#pragma once
#include "CompositorFrame.hpp"

namespace GRAPHS
{
	class OutlineWithGlow {
		TARCF::NodeInstance* nCornerRoundingBlur;
		TARCF::NodeInstance* nCornerRoundingBlurStep;
		TARCF::NodeInstance* nMaskInvert;
		TARCF::NodeInstance* nMaskDist;
		TARCF::NodeInstance* nMaskEdgeStep;
		TARCF::NodeInstance* nMaskInnerSub;
		TARCF::NodeInstance* nGlowPreDist;
		TARCF::NodeInstance* nGlowBlur;
		TARCF::NodeInstance* nGlowPow;
		TARCF::NodeInstance* nBlendTerms;
		TARCF::NodeInstance* nBlendTermsB;

	public:
		float corner_rounding = 0.78f;
		float outline_width = 1.0f;
		int glow_radius = 64;
		bool modulate = false;

		void update_rounding(const float& rounding) {
			this->corner_rounding = rounding;
			nCornerRoundingBlur->setPropertyEx<float>("radius", corner_rounding);
		}

		void update_width(const float& width) {
			this->outline_width = fmin(width, 256.0f);
			nMaskEdgeStep->setPropertyEx<float>("edge", 1.0f - (this->outline_width / 256.0f));
		}

		void update_glow_radius(const float& radius) {
			this->glow_radius = (radius > 128? 128: radius);
			nGlowPreDist->setPropertyEx<int>("maxdist", radius);
		}

		void update_modulate_enable(const bool& enable) {
			this->modulate = enable;
			nBlendTerms->setPropertyEx<int>("maskChannelID",this->modulate? 0:3); // set to alpha (1) or R (mod)
		}

		void imgui_hook(TARCF::NodeInstance* ptrExtColor, float* ptrExtColorMem, const std::string& name) {
			
			ImGui::Text((std::string(name) + " color").c_str());
			ImGui::SameLine();

			ImGui::PushID((std::string(name) + "_color").c_str());
			if (ImGui::ColorEdit4("Color", ptrExtColorMem, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar)) {
				ptrExtColor->setPropertyEx<glm::vec4>("color", glm::vec4(ptrExtColorMem[0], ptrExtColorMem[1], ptrExtColorMem[2], ptrExtColorMem[3]));
			} ImGui::PopID();

			ImGui::PushID((std::string(name) + "_corner_round").c_str());
			if (ImGui::SliderFloat("corner Rounding", &corner_rounding, 0.0f, 5.0f, "%.2f")) {
				nCornerRoundingBlur->setPropertyEx<float>("radius", corner_rounding);
			} ImGui::PopID();

			ImGui::PushID((std::string(name) + "_outline_width").c_str());
			if (ImGui::SliderFloat("outline width", &outline_width, 0.0f, 25.0f, "%.1f")) {
				nMaskEdgeStep->setPropertyEx<float>("edge", 1.0f - (outline_width / 256.0f));
			} ImGui::PopID();

			ImGui::PushID((std::string(name) + "_glow_radius").c_str());
			if (ImGui::SliderInt("glow radius", &glow_radius, 0, 128)) {
				nGlowPreDist->setPropertyEx<int>("maxdist", glow_radius);
			} ImGui::PopID();

			ImGui::PushID((std::string(name) + "_mod").c_str());
			if (ImGui::Checkbox("Modulate Outline", &modulate)) {
				nBlendTerms->setPropertyEx<int>("maskChannelID", modulate? 0: 3); // set to alpha (1) or R (mod)
			} ImGui::PopID();
		}

		// Glow: 
		//	takes:  - A mask input
		//			- A modulation texture
		//  outputs:
		//		Glowy outline of the input shape
		OutlineWithGlow(
			TARCF::NodeInstance* src,	TARCF::NodeInstance* nSrcMod, 
			unsigned int srcConID = 0U, unsigned int nSrcModConID = 0U, 
			
			unsigned int rX = 1024u, unsigned int rY = 1024u) {

			nCornerRoundingBlur = new TARCF::NodeInstance(rX, rY, "guassian");
			nCornerRoundingBlur->setPropertyEx<float>("radius", 0.78f);

			TARCF::NodeInstance::connect(src, nCornerRoundingBlur, 0, 0);

			nCornerRoundingBlurStep = new TARCF::NodeInstance(rX, rY, "step");

			TARCF::NodeInstance::connect(nCornerRoundingBlur, nCornerRoundingBlurStep, 0, 0);

			nMaskInvert = new TARCF::NodeInstance(rX, rY, "invert");

			TARCF::NodeInstance::connect(nCornerRoundingBlurStep, nMaskInvert, 0, 0);

			nMaskDist = new TARCF::NodeInstance(rX, rY, "distance");
			nMaskDist->setPropertyEx<int>("maxdist", 64);

			TARCF::NodeInstance::connect(nMaskInvert, nMaskDist, 0, 0);

			nMaskEdgeStep = new TARCF::NodeInstance(rX, rY, "step");
			nMaskEdgeStep->setPropertyEx<float>("edge", 1.0f - (1.0f / 256.0f));

			TARCF::NodeInstance::connect(nMaskDist, nMaskEdgeStep, 0, 0);

			nMaskInnerSub = new TARCF::NodeInstance(rX, rY, "blend");
			nMaskInnerSub->setPropertyEx<signed int>("mode", TARCF::Atomic::Blend::BlendMode::BLEND_SUB);

			TARCF::NodeInstance::connect(nMaskEdgeStep, nMaskInnerSub, 0, 0);
			TARCF::NodeInstance::connect(nMaskInvert, nMaskInnerSub, 0, 1);

			nGlowPreDist = new TARCF::NodeInstance(rX, rY, "distance");
			nGlowPreDist->setPropertyEx<int>("maxdist", 64);

			TARCF::NodeInstance::connect(nMaskInnerSub, nGlowPreDist, 0, 0);

			nGlowBlur = new TARCF::NodeInstance(rX, rY, "guassian");
			nGlowBlur->setPropertyEx<float>("radius", 2.0f);

			nGlowPow = new TARCF::NodeInstance(rX, rY, "pow");
			nGlowPow->setPropertyEx<float>("value", 16.0f);

			TARCF::NodeInstance::connect(nGlowPreDist, nGlowBlur, 0, 0);

			TARCF::NodeInstance::connect(nGlowBlur, nGlowPow, 0, 0);

			nBlendTerms = new TARCF::NodeInstance(rX, rY, "blend");
			nBlendTerms->setPropertyEx<signed int>("mode", TARCF::Atomic::Blend::BlendMode::BLEND_ADD);
			nBlendTerms->setPropertyEx<float>("factor", 0.99f);
			nBlendTerms->setPropertyEx<int>("maskChannelID", 3); // set to alpha (1)

			TARCF::NodeInstance::connect(nMaskInnerSub, nBlendTerms, 0, 1);
			TARCF::NodeInstance::connect(nGlowPow, nBlendTerms, 0, 0);
			TARCF::NodeInstance::connect(nSrcMod, nBlendTerms, nSrcModConID, 2);

			nBlendTermsB = new TARCF::NodeInstance(rX, rY, "blend");
			nBlendTermsB->setPropertyEx<signed int>("mode", TARCF::Atomic::Blend::BlendMode::BLEND_ADD);
			nBlendTermsB->setPropertyEx<float>("factor", 0.1f);

			TARCF::NodeInstance::connect(nBlendTerms, nBlendTermsB, 0, 0);
			TARCF::NodeInstance::connect(nCornerRoundingBlurStep, nBlendTermsB, 0, 1);
		}

		void connect_output(TARCF::NodeInstance* output, unsigned int dstConID = 0U) {
			TARCF::NodeInstance::connect(nBlendTermsB, output, 0, dstConID);
		}

		TARCF::NodeInstance* get_final() {
			return nBlendTermsB;
		}
	};
}