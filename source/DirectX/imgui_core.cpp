/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|
|- Purpose        : Implementation of Imgui Library in Phoenix SDK
|
|- Developer      : Huseyin
|- Timestamp      : 2022-10-3
|
|
|- Phoenix SDK Copyright © Hosseinpourziyaie 2022 <hosseinpourziyaie@gmail.com>
|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <std_include.hpp>
#include <definitions.hpp>
#include <dx11_hook.hpp>

#include <imgui_core.hpp>
#include <imgui_font.hpp>
#include <Utilities/io.hpp>

namespace imgui
{
    std::vector<std::function<void()>> imgui_instances;

	bool override_controls = false;
	bool imgui_frame_stopped = false;

	ImFont* m_pFont = nullptr;
	ImFont* phoenix_pFont = nullptr;

    void add_menu(const std::function<void()> callback)
    {
        imgui_instances.push_back(callback);
    }

	bool input_detour(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
			return true;

		if(override_controls) return true;

		return false;
	}

	void draw_text(std::string text, float x, float y, float* color, ImFont* font, float fontSize)
	{
		ImGui::GetForegroundDrawList()->AddText(font, fontSize, ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color[0], color[1], color[2], color[3])), text.c_str(), NULL, 0.0f, NULL);
	}

	void draw_rect(int x, int y, int w, int h, float* color)
	{
		ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color[0], color[1], color[2], color[3])));
	}

	class component final : public component_interface
	{
	public:
		void render_frame() override
		{
			if (imgui_frame_stopped) return;

			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();

			ImGui::NewFrame();

			for (auto itr = imgui_instances.begin(); itr != imgui_instances.end(); ++itr)
				(*itr)();

			ImGui::EndFrame();
			ImGui::Render();

			DirectX11::p_context->OMSetRenderTargets(1, &DirectX11::mainRenderTargetView, NULL);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		void render_start() override
		{
			log_debug_string("Initializing ImGui(v%s) platform backend", ImGui::GetVersion());

			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
			ImGui_ImplWin32_Init(DirectX11::window);
			ImGui_ImplDX11_Init(DirectX11::p_device, DirectX11::p_context);

			if (utils::io::file_exists("C:\\Windows\\Fonts\\Arial.ttf"))
			{
				m_pFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 16.0f);
			}
			else
			{
				m_pFont = io.Fonts->AddFontDefault(); // Fall back to ProggyClean (ImGui Default Font)
			}

			phoenix_pFont = io.Fonts->AddFontFromMemoryCompressedBase85TTF(eurostile_bold_compressed_data_base85, 20.0f);
			
			//ImGui::GetIO().MouseDrawCursor = true;
		}

		void shutdown() override
		{
			imgui_frame_stopped = true;
		}

		int load_sequence() override
		{
			return sequence::priority_first;
		}
	};
}

REGISTER_COMPONENT(imgui::component)