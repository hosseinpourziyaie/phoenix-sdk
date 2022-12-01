#pragma once

namespace game_console
{
	enum suggestion_type
	{
		PHOENIX_DVAR, PHOENIX_CMD, ENGINE_DVAR, ENGINE_CMD
	};

	struct suggestion_info
	{
		suggestion_type type;
		std::string name;
		std::string desc;
	};

	void print(const char* fmt, ...);
	void print(const std::string& data);

	extern std::vector<suggestion_info> suggestions_list;
	extern std::string environment_info_formatted_str;
}