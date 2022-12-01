/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|
|- Purpose        : Main Handler for Engine Integration
|
|- Developer      : Huseyin
|- Timestamp      : 2022-10-6
|
|
|- Phoenix SDK Copyright © Hosseinpourziyaie 2022 <hosseinpourziyaie@gmail.com>
|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <std_include.hpp>
#include <Engine/engine_hq.hpp>
#include <Engine/iw_engine.hpp>
#include <DirectX/dx11_hook.hpp>
#include <Developer/dev_console.hpp>

namespace EngineHQ
{
	engine_handler loaded_engine_handler{};
	module_information main_module_info{};
	std::map<std::string, std::function<void()>> supported_games;
	std::map<std::string, std::function<void(std::string)>> phoenix_cmd_list;

	void execute_cmd(std::string command)
	{
		if (command.empty()) return; // early return if text input's size is zero

		std::string cmd_argv_zero = utils::string::split(command, ' ')[0];

		std::map<std::string, std::function<void(std::string)>>::iterator i = phoenix_cmd_list.find(cmd_argv_zero);

		if (i != phoenix_cmd_list.end())
		{
			i->second(command.substr(cmd_argv_zero.length() + 1, std::string::npos));
		}
		else if (loaded_engine_handler.cmd_exec != nullptr) // divert to engine handler if its set
		{
			loaded_engine_handler.cmd_exec(0, command);
		}
		else
		{
			game_console::print("Engine Command Handler Not set");
		}
	}

	bool fetch_dvar_info(std::string dvarName, dvar_t* output)
	{
		if (dvarName.empty() || !loaded_engine_handler.dvar_info) return false;

		return loaded_engine_handler.dvar_info(dvarName, output);
	}

	void insert_phoenix_cmd(std::string name, std::string desc, std::function<void(std::string)> func)
	{
		EngineHQ::phoenix_cmd_list.insert({ name, func }); // cmd handler
		game_console::suggestions_list.push_back({ game_console::suggestion_type::PHOENIX_CMD, name, desc }); // console suggestion
	}

	void initialize_engine()
	{
		if (supported_games.find(main_module_info.name) != supported_games.end())
		{
			log_debug_string("Initializing Engine Handler for %s", main_module_info.name.c_str()); supported_games[main_module_info.name]();
		}
	}

	class component final : public component_interface
	{
	public:
		void initialize() override
		{
			main_module_info.name = utils::common::getModuleFileName();
			main_module_info.base = utils::common::getModuleBaseAddr();
			main_module_info.size = utils::common::getModuleImgSize();

			log_debug_string("Main module info: %s --- baseAddr: 0x%llX --- imgSize: 0x%llX", main_module_info.name.c_str(), main_module_info.base, main_module_info.size);
			game_console::environment_info_formatted_str = utils::string::va("Phoenix v%s --- %s", PHOENIX_VERSION_STR, utils::string::to_upper(main_module_info.name).c_str()); // utils::common::GetWindowTitleString(DirectX11::window)

			initialize_engine();
		}

		void shutdown() override
		{

		}

		int load_sequence() override
		{
			return sequence::priority_last;
		}
	};
}

REGISTER_COMPONENT(EngineHQ::component)