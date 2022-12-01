/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|
|- Purpose        : Maximize the capabilities of phoenix sdk on call of duty games
|                     by Integration of IW Engine Functions those interact with console
|
|- Developer      : Huseyin
|- Timestamp      : 2022-10-5  
|
|
|- Phoenix SDK Copyright © Hosseinpourziyaie 2022 <hosseinpourziyaie@gmail.com>
|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <std_include.hpp>
#include <Utilities/pattern.hpp>
#include <Utilities/hook.hpp>
#include <Engine/engine_hq.hpp>
#include <Engine/iw_engine.hpp>
#include <Engine/dvar_helper.hpp>
#include <Developer/dev_console.hpp>

using namespace EngineHQ;
using namespace iw_dvar_helper;

namespace IW
{
	const std::map<std::string, std::string> s2_dvar_corresponders =
	{
		{ "cg_draw2d" , "2562" },
		{ "cg_drawgun", "1762" },
		{ "cg_drawfps", "2377" },
		{ "timescale" , "5401" },
		{ "mapname"   , "1673" },
		{ "snd_musicdisabled", "2757" }
	};

	typedef char* (__fastcall* UI_FormattedBuildNumber_t)();
	UI_FormattedBuildNumber_t UI_FormattedBuildNumber = (UI_FormattedBuildNumber_t)0x000000000;  // UN-INITIALIZED

	typedef void(__fastcall* Sys_EnterCriticalSection_t)(int critSect);
	Sys_EnterCriticalSection_t Sys_EnterCriticalSection = (Sys_EnterCriticalSection_t)0x000000000;  // UN-INITIALIZED

	typedef void(__fastcall* Sys_LeaveCriticalSection_t)(int critSect);
	Sys_LeaveCriticalSection_t Sys_LeaveCriticalSection = (Sys_LeaveCriticalSection_t)0x000000000;  // UN-INITIALIZED

	typedef void(__fastcall* Cbuf_AddText_t)(int localClientNum, const char* text);
	Cbuf_AddText_t Cbuf_AddText = (Cbuf_AddText_t)0x000000000;  // UN-INITIALIZED

	typedef iw_dvar_t*(__cdecl* Dvar_FindVar_t)(const char* name);
	Dvar_FindVar_t Dvar_FindVar = (Dvar_FindVar_t)0x000000000;  // UN-INITIALIZED

	typedef const char*(__cdecl* Dvar_ValueToString_t)(iw_dvar_t* dvar, bool is_hashed, iw_dvar_value value);
	Dvar_ValueToString_t Dvar_ValueToString = (Dvar_ValueToString_t)0x000000000;  // UN-INITIALIZED

	int CRITSECT_CBUF_NUMBER = 0x000000000;   // UN-INITIALIZED
	CmdText* cmd_textArray = (CmdText*)0x000000000;   // UN-INITIALIZED
	CmdFunc** cmd_functions = (CmdFunc**)0x000000000;   // UN-INITIALIZED

	//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

	void Cbuf_AddText_Recreation(int localClientNum, const char* text)
	{
		Sys_EnterCriticalSection(CRITSECT_CBUF_NUMBER);

		CmdText* cmd_text = &cmd_textArray[localClientNum];

		int v4 = strlen(text);
		int v5 = cmd_text->cmdsize;
		if ((int)v5 + v4 < cmd_text->maxsize)
		{
			memcpy(&cmd_text->data[v5], text, v4 + 1);
			cmd_text->cmdsize += v4;
		}

		Sys_LeaveCriticalSection(CRITSECT_CBUF_NUMBER);
	}

	void Cmd_ExecuteText(int localClientNum, std::string text)
	{
		if (loaded_engine_handler.engine_type == ENGINE_S1)
		{
			Cbuf_AddText_Recreation(localClientNum, utils::string::va("%s \n", text.data()));
		}
		else if (loaded_engine_handler.engine_type == ENGINE_S2)
		{
			for (const auto& elem : IW::s2_dvar_corresponders)
			{
				text = utils::string::replace(utils::string::to_lower(text), elem.first, elem.second);
			}

			Cbuf_AddText_Recreation(localClientNum, utils::string::va("%s \n", text.data()));
		}
		else
		{
			log_error_string("Cmd_ExecuteText unhandled engine type %i", loaded_engine_handler.engine_type);
		}
	}

	bool Dvar_FetchInformation(std::string dvarName, dvar_t* output)
	{
		if (loaded_engine_handler.engine_type == ENGINE_S1)
		{
			auto* const dvar = Dvar_FindVar(dvarName.c_str());

			if (!dvar) return false;
			if (!output) return true;

			iw_dvar_type type = *(iw_dvar_type*)((INT64)dvar + 0xC);
			iw_dvar_value current = *(iw_dvar_value*)((INT64)dvar + 0x10);
			iw_dvar_value reset = *(iw_dvar_value*)((INT64)dvar + 0x30);
			iw_dvar_limits domain = *(iw_dvar_limits*)((INT64)dvar + 0x40);

			output->address = reinterpret_cast<uint64_t>(dvar);
			output->value = iw_dvar_helper::dvar_value_to_string(type, current);
			output->reset = iw_dvar_helper::dvar_value_to_string(type, reset);
			output->range = iw_dvar_helper::dvar_domain_to_string(type, domain);

			return true;
		}
		else if (loaded_engine_handler.engine_type == ENGINE_S2)
		{
			auto it = s2_dvar_corresponders.find(utils::string::to_lower(dvarName));
			if (it != s2_dvar_corresponders.end()) dvarName = it->second.data(); // Replace with Known Hash names for S2

			auto* const dvar = Dvar_FindVar(dvarName.c_str());

			if (!dvar) return false;
			if (!output) return true;

			output->address = reinterpret_cast<uint64_t>(dvar);
			output->value = Dvar_ValueToString(dvar, 1, dvar->current);
			output->reset = Dvar_ValueToString(dvar, 1, dvar->reset);
			output->range = dvar_domain_to_string(dvar->type, dvar->domain);

			return true;
		}
		else 
		{
			return false;
		}
	}

	//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

	void dump_cmdlist_f(std::string args)
	{
		std::vector<std::string> cmd_list;

		CmdFunc* cmd = *cmd_functions;
		int i = 0;

		while (cmd)
		{
			if (cmd->name)
			{
				cmd_list.push_back(cmd->name);
				i++;
			}

			cmd = cmd->next;
		}

		game_console::print("============================    ENGINE COMMANDS LIST    ============================\n");
		for (auto iter : cmd_list)
		{
			std::string name = utils::string::to_lower(iter);
			game_console::print(name);
		}
		game_console::print("============================     TOTAL %03i COMMANDS     ============================\n", i);
	}

	//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

	void R_AddCmdDrawTextWithCursor(std::string text, int maxChars, font_s* font, float x, float y, float xScale, float yScale, float rotation, float* color, int style, int cursor_pos, char cursor_char)
	{
		ImGui::GetForegroundDrawList()->AddText(font->fontHandle, xScale * font->fontHandle->FontSize, ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color[0], color[1], color[2], color[3])), text.c_str(), NULL, 0.0f, NULL);

		auto now = std::chrono::high_resolution_clock::now();
		auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

		if (timestamp - ((timestamp / 1000) * 1000) < 500)
		{
			x = x + static_cast<float>(R_TextWidth(text.substr(0, cursor_pos).c_str(), 0, font));
			ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + 2, y + font->fontHandle->FontSize), ImGui::ColorConvertFloat4ToU32(ImVec4(color[0], color[1], color[2], color[3])));
		}
	}

	int R_TextWidth(const char* text, int maxChars, font_s* font)
	{
		return static_cast<int>(font->fontHandle->CalcTextSizeA(font->fontHandle->FontSize, FLT_MAX, 0.0f, text).x);
	}

	void R_AddCmdDrawText(const char* text, int maxChars, font_s* font, float x, float y, float xScale, float yScale, float rotation, float* color, int style)
	{
		ImGui::GetForegroundDrawList()->AddText(font->fontHandle, xScale * font->fontHandle->FontSize, ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color[0], color[1], color[2], color[3])), text, NULL, 0.0f, NULL);
	}

	void R_AddCmdDrawStretchPic(float x, float y, float w, float h, float s0, float t0, float s1, float t1, float* color, void* material)
	{
		ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color[0], color[1], color[2], color[3])));
	}
	
	//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

	void insert_common_iw_console_suggestions()
	{
		game_console::suggestions_list.push_back({ game_console::suggestion_type::ENGINE_DVAR, "cg_fov", "The field of view angle in degrees" });
		game_console::suggestions_list.push_back({ game_console::suggestion_type::ENGINE_DVAR, "cg_fovScale", "Scale applied to the field of view" });
		game_console::suggestions_list.push_back({ game_console::suggestion_type::ENGINE_DVAR, "cg_draw2D", "Draw 2D screen elements" });
		game_console::suggestions_list.push_back({ game_console::suggestion_type::ENGINE_DVAR, "cg_drawGun", "Draw the view model" });
		game_console::suggestions_list.push_back({ game_console::suggestion_type::ENGINE_DVAR, "timescale", "Set the game speed" });
		game_console::suggestions_list.push_back({ game_console::suggestion_type::ENGINE_DVAR, "ui_mapname", "Current map name" });
		
		game_console::suggestions_list.push_back({ game_console::suggestion_type::ENGINE_CMD, "connect", "direct connect to a lobby session" });
		game_console::suggestions_list.push_back({ game_console::suggestion_type::ENGINE_CMD, "disconnect", "disconnect from current session" });
		game_console::suggestions_list.push_back({ game_console::suggestion_type::ENGINE_CMD, "quit", "close the game" });
	}

	void initialize_s1()
	{
		auto build_info_func = utils::pattern::ps_find_idastyle("40 57 48 83 EC 30 48 8B 05 ? ? ? ? 48 85", main_module_info.base, main_module_info.size);
		if (build_info_func)
		{
			UI_FormattedBuildNumber = (UI_FormattedBuildNumber_t)build_info_func;
		}

		if (*(DWORD*)(0x3A1C20_b + 1) == 0x218214) // [S1_MP64_SHIP 1.22.2195988]
		{
			Sys_EnterCriticalSection = (Sys_EnterCriticalSection_t)0x1404CCFE0;   // [S1MP64 1.22.2195988]
			Sys_LeaveCriticalSection = (Sys_LeaveCriticalSection_t)0x1404CD050;   // [S1MP64 1.22.2195988]

			cmd_textArray = (CmdText*)0x1479F6CB8;  CRITSECT_CBUF_NUMBER = 71;    // [S1MP64 1.22.2195988]
			cmd_functions = (CmdFunc**)0x1479ECC58;                               // [S1MP64 1.22.2195988]

			Dvar_FindVar = (Dvar_FindVar_t)0x1404BF8B0;                           // [S1MP64 1.22.2195988]

			loaded_engine_handler.engine_type = ENGINE_S1;
			loaded_engine_handler.cmd_exec = Cmd_ExecuteText;
			loaded_engine_handler.dvar_info = Dvar_FetchInformation;
		}

		insert_common_iw_console_suggestions();
	}

	void initialize_s2()
	{
		auto build_info_func = utils::pattern::ps_find_idastyle("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 40 33 C9", main_module_info.base, main_module_info.size);
		if (build_info_func)
		{
			UI_FormattedBuildNumber = (UI_FormattedBuildNumber_t)build_info_func;
		}

		if (*(DWORD*)(0x6A1EE0_b + 1) == 0x224149) // [S2_MP64_SHIP 1.25.2244937]
		{		
			Sys_EnterCriticalSection = (Sys_EnterCriticalSection_t)0x7D7B50_b;   // [S2MP64 1.25.2244937]
			Sys_LeaveCriticalSection = (Sys_LeaveCriticalSection_t)0x7D7BC0_b;   // [S2MP64 1.25.2244937]

			cmd_textArray = (CmdText*)0xAC674B8_b; CRITSECT_CBUF_NUMBER = 193;   // [S2MP64 1.25.2244937]
			cmd_functions = (CmdFunc**)0xAC672D8_b;                              // [S2MP64 1.25.2244937]

			Dvar_FindVar = (Dvar_FindVar_t)0x109C30_b;                           // [S2MP64 1.25.2244937]
			Dvar_ValueToString = (Dvar_ValueToString_t)0x1194D0_b;               // [S2MP64 1.25.2244937]

			loaded_engine_handler.engine_type = ENGINE_S2;
			loaded_engine_handler.cmd_exec = Cmd_ExecuteText;
			loaded_engine_handler.dvar_info = Dvar_FetchInformation;
		}

		insert_common_iw_console_suggestions();
	}

	class component final : public component_interface
	{
	public:
		void initialize() override
		{
			supported_games.insert({ "s1_mp64_ship.exe", initialize_s1 });
			supported_games.insert({ "s1_sp64_ship.exe", initialize_s1 });

			supported_games.insert({ "s2_mp64_ship.exe", initialize_s2 });
			supported_games.insert({ "s2_sp64_ship.exe", initialize_s2 });


			insert_phoenix_cmd("cmdlist", "dump list of engine commands using itteration of cmd_functions", dump_cmdlist_f);
		}

		void shutdown() override
		{

		}
	};
}

REGISTER_COMPONENT(IW::component)