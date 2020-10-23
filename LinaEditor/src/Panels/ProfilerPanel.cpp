/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-2020] [Inan Evin]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

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

#include "Panels/ProfilerPanel.hpp"
#include "Widgets/WidgetsUtility.hpp"
#include "Core/Application.hpp"
#include "Core/EditorCommon.hpp"
#include "Core/Timer.hpp"
#include "imgui/imgui.h"
#include "imgui/implot/implot.h"

namespace LinaEditor
{
#define MAX_DEQUE_SIZE 100

	struct RollingBuffer {
		float Span;
		ImVector<ImVec2> Data;
		RollingBuffer() {
			Span = 10.0f;
			Data.reserve(2000);
		}
		void AddPoint(float x, float y) {
			float xmod = fmodf(x, Span);
			if (!Data.empty() && xmod < Data.back().x)
				Data.shrink(0);
			Data.push_back(ImVec2(xmod, y));
		}
	};

	void ProfilerPanel::Setup()
	{

	}

	void ProfilerPanel::Draw(float frameTime)
	{
		if (m_show)
		{

			ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
			ImGui::SetNextWindowBgAlpha(1.0f);

			const std::map<std::string, LinaEngine::Timer*>& map = LinaEngine::Timer::GetTimerMap();

			ImGui::Begin(PROFILER_ID, &m_show, flags);

				// Shadow.
			WidgetsUtility::DrawShadowedLine(5);
			WidgetsUtility::IncrementCursorPosY(11);

			for (std::map<std::string, LinaEngine::Timer*>::const_iterator it = map.begin(); it != map.end(); ++it)
			{
				auto a = it->second->GetDuration();
				std::string txt = it->first + " " + std::to_string(a) + " ms";

				WidgetsUtility::IncrementCursorPosX(12);
				ImGui::Text(txt.c_str());
			}

			WidgetsUtility::IncrementCursorPosX(12);
			WidgetsUtility::IncrementCursorPosY(12);

			int fps = LinaEngine::Application::GetApp().GetCurrentFPS();

			static RollingBuffer   rdata1;
			static float t = 0;
			t += ImGui::GetIO().DeltaTime;
			rdata1.AddPoint(t, (float)fps);
			static float history = 10.0f;
			static ImPlotAxisFlags rt_axis = ImPlotAxisFlags_NoTickLabels;

			ImPlot::SetNextPlotLimitsX(0, history, ImGuiCond_Always);
			ImPlot::SetNextPlotLimitsY(0, 500, ImGuiCond_Always);
			ImPlot::PushStyleColor(ImPlotCol_Line, ImGui::GetStyleColorVec4(ImGuiCol_Header));

			if (ImPlot::BeginPlot("##Profiler", NULL, NULL, ImVec2(-1, 115), 0, rt_axis, rt_axis)) {

				std::string fpsLabel = "FPS " + std::to_string(fps);
				ImPlot::PlotLine(fpsLabel.c_str(), &rdata1.Data[0].x, &rdata1.Data[0].y, rdata1.Data.size(), 0, 2 * sizeof(float));
				ImPlot::EndPlot();
			}

			ImPlot::PopStyleColor();


			ImGui::End();

		}
	}
}