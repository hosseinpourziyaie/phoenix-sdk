#pragma once

struct dvar_t
{
	std::string value;
	std::string reset;
	std::string range;
	uint64_t address;
};

namespace EngineHQ
{
	void execute_cmd(std::string cmd);
	bool fetch_dvar_info(std::string name, dvar_t* output = nullptr);

	void insert_phoenix_cmd(std::string name, std::string desc, std::function<void(std::string)> func);

	enum engine_list
	{
		ENGINE_UNK = -1,
		ENGINE_IW = 0,
		ENGINE_S1 = 1,
		ENGINE_H1 = 2,
		ENGINE_S2 = 3,
		ENGINE_T8 = 4
	};

	struct module_information
	{
		std::string name;
		uint64_t    base;
		uint64_t    size;
	};

	struct engine_handler
	{
		uint32_t engine_type;
		std::string build_info;
		std::function<void(int, std::string)> cmd_exec;
		std::function<bool(std::string, dvar_t*)> dvar_info;
	};

	extern module_information main_module_info;
	extern std::map<std::string, std::function<void()>> supported_games;
	extern engine_handler loaded_engine_handler;
}