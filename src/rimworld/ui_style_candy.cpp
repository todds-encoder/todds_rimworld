/*
https://github.com/Raais/ImguiCandy

MIT License

Copyright (c) 2024 José Ángel Soler Ortiz
Copyright (c) 2021 Raais N.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "todds/ui_style_candy.hpp"

namespace imcandy {

void style_grey() {
	// 'Blender Dark' theme from v3.0.0 [Improvised]
	// Colors grabbed using X11 Soft/xcolor
	ImGui::StyleColorsDark(nullptr); // Reset to base/dark theme
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.84F, 0.84F, 0.84F, 1.00F);
	colors[ImGuiCol_WindowBg] = ImVec4(0.22F, 0.22F, 0.22F, 1.00F);
	colors[ImGuiCol_ChildBg] = ImVec4(0.19F, 0.19F, 0.19F, 1.00F);
	colors[ImGuiCol_PopupBg] = ImVec4(0.09F, 0.09F, 0.09F, 1.00F);
	colors[ImGuiCol_Border] = ImVec4(0.17F, 0.17F, 0.17F, 1.00F);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.10F, 0.10F, 0.10F, 0.00F);
	colors[ImGuiCol_FrameBg] = ImVec4(0.33F, 0.33F, 0.33F, 1.00F);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.47F, 0.47F, 0.47F, 1.00F);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.16F, 0.16F, 0.16F, 1.00F);
	colors[ImGuiCol_TitleBg] = ImVec4(0.11F, 0.11F, 0.11F, 1.00F);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.28F, 0.45F, 0.70F, 1.00F);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.11F, 0.11F, 0.11F, 1.00F);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.33F, 0.33F, 0.33F, 1.00F);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.33F, 0.33F, 0.33F, 1.00F);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35F, 0.35F, 0.35F, 1.00F);
	colors[ImGuiCol_CheckMark] = ImVec4(0.28F, 0.45F, 0.70F, 1.00F);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.28F, 0.45F, 0.70F, 1.00F);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.28F, 0.45F, 0.70F, 1.00F);
	colors[ImGuiCol_Button] = ImVec4(0.33F, 0.33F, 0.33F, 1.00F);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.40F, 0.40F, 0.40F, 1.00F);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.28F, 0.45F, 0.70F, 1.00F);
	colors[ImGuiCol_Header] = ImVec4(0.27F, 0.27F, 0.27F, 1.00F);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.28F, 0.45F, 0.70F, 1.00F);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.27F, 0.27F, 0.27F, 1.00F);
	colors[ImGuiCol_Separator] = ImVec4(0.18F, 0.18F, 0.18F, 1.00F);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.28F, 0.45F, 0.70F, 1.00F);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.28F, 0.45F, 0.70F, 1.00F);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.54F, 0.54F, 0.54F, 1.00F);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.28F, 0.45F, 0.70F, 1.00F);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.19F, 0.39F, 0.69F, 1.00F);
	colors[ImGuiCol_Tab] = ImVec4(0.11F, 0.11F, 0.11F, 1.00F);
	colors[ImGuiCol_TabHovered] = ImVec4(0.14F, 0.14F, 0.14F, 1.00F);
	colors[ImGuiCol_TabActive] = ImVec4(0.19F, 0.19F, 0.19F, 1.00F);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.28F, 0.45F, 0.70F, 1.00F);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.20F, 0.39F, 0.69F, 1.00F);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.28F, 0.45F, 0.70F, 1.00F);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.28F, 0.45F, 0.70F, 1.00F);
}

void style_nord() {
	// Nord/Nordic GTK [Improvised]
	// https://github.com/EliverLara/Nordic
	ImGui::StyleColorsDark(nullptr); // Reset to base/dark theme
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.85F, 0.87F, 0.91F, 0.88F);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.49F, 0.50F, 0.53F, 1.00F);
	colors[ImGuiCol_WindowBg] = ImVec4(0.18F, 0.20F, 0.25F, 1.00F);
	colors[ImGuiCol_ChildBg] = ImVec4(0.16F, 0.17F, 0.20F, 1.00F);
	colors[ImGuiCol_PopupBg] = ImVec4(0.23F, 0.26F, 0.32F, 1.00F);
	colors[ImGuiCol_Border] = ImVec4(0.14F, 0.16F, 0.19F, 1.00F);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.09F, 0.09F, 0.09F, 0.00F);
	colors[ImGuiCol_FrameBg] = ImVec4(0.23F, 0.26F, 0.32F, 1.00F);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.56F, 0.74F, 0.73F, 1.00F);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.53F, 0.75F, 0.82F, 1.00F);
	colors[ImGuiCol_TitleBg] = ImVec4(0.16F, 0.16F, 0.20F, 1.00F);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.16F, 0.16F, 0.20F, 1.00F);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16F, 0.16F, 0.20F, 1.00F);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.16F, 0.16F, 0.20F, 1.00F);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.18F, 0.20F, 0.25F, 1.00F);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.23F, 0.26F, 0.32F, 0.60F);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.23F, 0.26F, 0.32F, 1.00F);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.23F, 0.26F, 0.32F, 1.00F);
	colors[ImGuiCol_CheckMark] = ImVec4(0.37F, 0.51F, 0.67F, 1.00F);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.51F, 0.63F, 0.76F, 1.00F);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37F, 0.51F, 0.67F, 1.00F);
	colors[ImGuiCol_Button] = ImVec4(0.18F, 0.20F, 0.25F, 1.00F);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.51F, 0.63F, 0.76F, 1.00F);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.37F, 0.51F, 0.67F, 1.00F);
	colors[ImGuiCol_Header] = ImVec4(0.51F, 0.63F, 0.76F, 1.00F);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.53F, 0.75F, 0.82F, 1.00F);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.37F, 0.51F, 0.67F, 1.00F);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.56F, 0.74F, 0.73F, 1.00F);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.53F, 0.75F, 0.82F, 1.00F);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.53F, 0.75F, 0.82F, 0.86F);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.61F, 0.74F, 0.87F, 1.00F);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.37F, 0.51F, 0.67F, 1.00F);
	colors[ImGuiCol_Tab] = ImVec4(0.18F, 0.20F, 0.25F, 1.00F);
	colors[ImGuiCol_TabHovered] = ImVec4(0.22F, 0.24F, 0.31F, 1.00F);
	colors[ImGuiCol_TabActive] = ImVec4(0.23F, 0.26F, 0.32F, 1.00F);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.13F, 0.15F, 0.18F, 1.00F);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.17F, 0.19F, 0.23F, 1.00F);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.56F, 0.74F, 0.73F, 1.00F);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.53F, 0.75F, 0.82F, 1.00F);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.37F, 0.51F, 0.67F, 1.00F);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.53F, 0.75F, 0.82F, 0.86F);
}

} // namespace imcandy