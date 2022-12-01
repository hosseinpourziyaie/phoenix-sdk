/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|
|- Purpose        : Developer In-game Console
|
|- Developer      : xLabs Devs (Slightly Improved by Huseyin)
|- Timestamp      : 2020-??-??
|
|
|- Phoenix SDK Copyright © Hosseinpourziyaie 2022 <hosseinpourziyaie@gmail.com>
|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <std_include.hpp>
#include <definitions.hpp>
#include <KeyCatcher.hpp>
#include <imgui_core.hpp>

#include <Engine/engine_hq.hpp>
#include <Engine/iw_engine.hpp>

#include <Utilities/concurrency.hpp>
#include <Developer/dev_console.hpp>

using namespace EngineHQ;


namespace game_console
{
	std::vector<suggestion_info> suggestions_list{};
	std::string environment_info_formatted_str{};

	namespace
	{
		vec4_t con_inputBoxColor = { 0.1f, 0.1f, 0.1f, 0.9f };
		vec4_t con_inputHintBoxColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		vec4_t con_outputBarColor = { 0.8f, 0.8f, 0.8f, 0.6f };
		vec4_t con_outputSliderColor = { 0.8f, 0.8f, 0.8f, 1.0f };
		vec4_t con_outputWindowColor = { 0.15f, 0.15f, 0.15f, 0.85f };
		vec4_t con_inputWriteDownColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		vec4_t con_inputDvarMatchColor = { 0.1f, 0.8f, 0.8f, 1.0f };
		vec4_t con_inputDvarInvalidColor = { 0.4f, 0.8f, 0.7f, 1.0f };
		vec4_t con_inputCmdMatchColor = { 0.9f, 0.6f, 0.2f, 1.0f };
		vec4_t con_inputDescriptionColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		vec4_t con_inputExtraInfoColor = { 1.0f, 0.5f, 0.5f, 1.0f }; // 1.0f, 1.0f, 0.6f, 1.0f
		vec4_t con_phoenixSDKLabelsColor = { 0.0f, 1.0f, 1.0f, 1.0f };

		using output_queue = std::deque<std::string>;

		struct ingame_console
		{
			char buffer[256]{};
			int cursor{};
			float font_height{};
			int visible_line_count{};
			int visible_pixel_width{};
			float screen_min[2]{}; //left & top
			float screen_max[2]{}; //right & bottom
			struct {
				float x{}, y{};
			} screen_pointer;
			bool may_auto_complete{};
			char auto_complete_choice[64]{};
			bool output_visible{};
			int display_line_offset{};
			int total_line_count{};
			utils::concurrency::container<output_queue, std::recursive_mutex> output{};
		};

		int miscKeyFlag = 0;
		ingame_console con{};
		IW::font_s console_font{};
		std::once_flag con_isInitialized;

		std::int32_t history_index = -1;
		std::deque<std::string> history{};

		std::string fixed_input{};
		std::vector<suggestion_info> matches{};

		void clear_input()
		{
			strncpy_s(con.buffer, "", sizeof(con.buffer));
			con.cursor = 0;

			fixed_input = "";
			matches.clear();
		}

		void clear_output()
		{
			con.total_line_count = 0;
			con.display_line_offset = 0;
			con.output.access([](output_queue& output)
				{
					output.clear();
				});
			history_index = -1;
			history.clear();
		}

		void print_internal(const std::string& data)
		{
			con.output.access([&](output_queue& output)
				{
					if (con.visible_line_count > 0
						&& con.display_line_offset == (output.size() - con.visible_line_count))
					{
						con.display_line_offset++;
					}
					output.push_back(data);
					if (output.size() > 512)
					{
						output.pop_front();
					}
				});
		}

		void toggle_console()
		{
			clear_input();

			con.output_visible = false;
			miscKeyFlag ^= 1;
		}

		void toggle_console_output()
		{
			con.output_visible = con.output_visible == 0;
		}

		void check_resize()
		{
			con.screen_min[0] = 6.0f;
			con.screen_min[1] = 6.0f;
			con.screen_max[0] = ImGui::GetMainViewport()->Size[0] - 6.0f;
			con.screen_max[1] = ImGui::GetMainViewport()->Size[1] - 6.0f;

			con.font_height = console_font.fontHandle->FontSize;
			con.visible_line_count = static_cast<int>((con.screen_max[1] - con.screen_min[1] - (con.font_height * 2)) - 24.0f) / con.font_height;
			con.visible_pixel_width = static_cast<int>(((con.screen_max[0] - con.screen_min[0]) - 10.0f) - 18.0f);
		}

		void draw_box(const float x, const float y, const float w, const float h, float* color)
		{
			vec4_t dark_color;

			dark_color[0] = color[0] * 0.5f;
			dark_color[1] = color[1] * 0.5f;
			dark_color[2] = color[2] * 0.5f;
			dark_color[3] = color[3];

			IW::R_AddCmdDrawStretchPic(x, y, w, h, 0.0f, 0.0f, 0.0f, 0.0f, color, NULL);
			IW::R_AddCmdDrawStretchPic(x, y, 2.0f, h, 0.0f, 0.0f, 0.0f, 0.0f, dark_color, NULL);
			IW::R_AddCmdDrawStretchPic((x + w) - 2.0f, y, 2.0f, h, 0.0f, 0.0f, 0.0f, 0.0f, dark_color,NULL);
			IW::R_AddCmdDrawStretchPic(x, y, w, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, dark_color, NULL);
			IW::R_AddCmdDrawStretchPic(x, (y + h) - 2.0f, w, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, dark_color,NULL);
		}

		void draw_input_box(const int lines, float* color)
		{
			draw_box( con.screen_pointer.x - 6.0f, con.screen_pointer.y - 6.0f,
			(con.screen_max[0] - con.screen_min[0]) - ((con.screen_pointer.x - 6.0f) - con.screen_min[0]),
				(lines * con.font_height) + 12.0f, color);
		}

		void draw_input_text_and_over(const char* str, float* color)
		{
			IW::R_AddCmdDrawText(str, 0x7FFFFFFF, &console_font, con.screen_pointer.x,
							con.screen_pointer.y, 1.0f, 1.0f, 0.0f, color, 0);

			con.screen_pointer.x = IW::R_TextWidth(str, 0, &console_font) + con.screen_pointer.x + 6.0f;
		}

		float draw_hint_box(const int lines, float* color, [[maybe_unused]] float offset_x = 0.0f,
			[[maybe_unused]] float offset_y = 0.0f)
		{
			const auto _h = lines * con.font_height + 12.0f;
			const auto _y = con.screen_pointer.y - 3.0f + con.font_height + 12.0f + offset_y;
			const auto _w = (con.screen_max[0] - con.screen_min[0]) - ((con.screen_pointer.x - 6.0f) - con.screen_min[0]);

			draw_box(con.screen_pointer.x - 6.0f, _y, _w, _h, color);
			return _h;
		}

		void draw_hint_text(const int line, const char* text, float* color, const float offset_x = 0.0f, const float offset_y = 0.0f)
		{
			const auto _y = con.font_height + con.screen_pointer.y + (con.font_height * line) + 15.0f + offset_y;

			IW::R_AddCmdDrawText(text, 0x7FFFFFFF, &console_font, con.screen_pointer.x + offset_x, _y, 1.0f, 1.0f, 0.0f, color, 0);
		}

		void find_matches(std::string input, std::vector<suggestion_info>& suggestions, const bool exact)
		{
			input = utils::string::to_lower(input);

			for (const auto& entry : suggestions_list)
			{
				auto name = utils::string::to_lower(entry.name);
				if (utils::string::match_compare(input, name, exact))
				{
					suggestions.push_back(entry);
				}

				if (exact && suggestions.size() > 1)
				{
					return;
				}
			}

			if (suggestions.size() == 0 && fetch_dvar_info(input.data()))
			{
				suggestions.push_back({ suggestion_type::ENGINE_DVAR , input, "" });
			}
		}

		void draw_input()
		{
			con.screen_pointer.x = con.screen_min[0] + 6.0f;
			con.screen_pointer.y = con.screen_min[1] + 6.0f;

			draw_input_box(1, con_inputBoxColor);
			draw_input_text_and_over("PHOENIX SDK>", con_phoenixSDKLabelsColor);

			con.auto_complete_choice[0] = 0;

			IW::R_AddCmdDrawTextWithCursor(con.buffer, 0x7FFFFFFF, &console_font, con.screen_pointer.x, con.screen_pointer.y, 1.0f, 1.0f, 0, con_inputWriteDownColor, 0, con.cursor, '|');

			// check if using a prefixed '/' or not
			const auto input = con.buffer[1] && (con.buffer[0] == '/' || con.buffer[0] == '\\')
				? std::string(con.buffer).substr(1)
				: std::string(con.buffer);

			if (!input.length())
			{
				return;
			}

			if (input != fixed_input)
			{
				matches.clear();

				if (input.find(" ") != std::string::npos)
				{
					find_matches(input.substr(0, input.find(" ")), matches, true);
				}
				else
				{
					find_matches(input, matches, false);
				}

				fixed_input = input;
			}

			con.may_auto_complete = false;

			if (matches.size() > 24)
			{
				draw_hint_box(1, con_inputHintBoxColor);
				draw_hint_text(0, utils::string::va("%i matches (too many to show here)", matches.size()), con_inputDvarMatchColor);
			}
			else if (matches.size() == 1)
			{
				const auto offset_x = (con.screen_max[0] - con.screen_pointer.x) / 4.f;

				if (matches[0].type == suggestion_type::ENGINE_CMD || matches[0].type == suggestion_type::PHOENIX_CMD)
				{
					draw_hint_box(1, con_inputHintBoxColor);
					draw_hint_text(0, matches[0].name.data(), con_inputCmdMatchColor);
					draw_hint_text(0, matches[0].desc.data(), con_inputCmdMatchColor, offset_x * 1.5f);
				}
				else
				{
					dvar_t matched_dvar;
					bool found = fetch_dvar_info(matches[0].name.data(), &matched_dvar);

					const auto line_count = found ? 3 : 1;

					const auto height = draw_hint_box(line_count, con_inputHintBoxColor);
					draw_hint_text(0, matches[0].name.data(), found ? con_inputDvarMatchColor : con_inputDvarInvalidColor);

					if (found)
					{
						draw_hint_text(0, matched_dvar.value.data(), con_inputDvarMatchColor, offset_x);
						draw_hint_text(1, "  default", con_inputDvarInvalidColor);
						draw_hint_text(1, matched_dvar.reset.data(), con_inputDvarInvalidColor, offset_x);
						draw_hint_text(2, matches[0].desc.data(), con_inputDescriptionColor, 0);

						if (matched_dvar.range.length())
						{
							const auto offset_y = height + 3.f;

							draw_hint_box(1, con_inputHintBoxColor, 0, offset_y);
							draw_hint_text(0, matched_dvar.range.data(), con_inputExtraInfoColor, 0, offset_y);
						}
					}
					else
					{
						draw_hint_text(0, matches[0].desc.data(), con_inputDvarInvalidColor, offset_x * 1.5f);
					}
				}

				strncpy_s(con.auto_complete_choice, matches[0].name.data(), 64);
				con.may_auto_complete = true;
			}
			else if (matches.size() > 1)
			{
				draw_hint_box(static_cast<int>(matches.size()), con_inputHintBoxColor);

				const auto offset_x = (con.screen_max[0] - con.screen_pointer.x) / 4.f;

				for (size_t i = 0; i < matches.size(); i++)
				{
					if (matches[i].type == suggestion_type::ENGINE_CMD || matches[i].type == suggestion_type::PHOENIX_CMD)
					{
						draw_hint_text(static_cast<int>(i), matches[i].name.data(), con_inputCmdMatchColor);
						draw_hint_text(static_cast<int>(i), matches[i].desc.data(), con_inputCmdMatchColor, offset_x * 1.5f);
					}
					else
					{
						dvar_t matched_dvar;
						bool found = fetch_dvar_info(matches[i].name.data(), &matched_dvar);

						draw_hint_text(static_cast<int>(i), matches[i].name.data(), found ? con_inputDvarMatchColor : con_inputDvarInvalidColor);

						if (found)
						{
							draw_hint_text(static_cast<int>(i), matched_dvar.value.data(), con_inputDvarMatchColor, offset_x);
						}

						draw_hint_text(static_cast<int>(i), matches[i].desc.data(), found ? con_inputDvarMatchColor : con_inputDvarInvalidColor, offset_x * 1.5f);
					}
				}

				strncpy_s(con.auto_complete_choice, matches[0].name.data(), 64);
				con.may_auto_complete = true;
			}
		}

		void draw_output_scrollbar(const float x, float y, const float width, const float height, output_queue& output)
		{
			const auto _x = (x + width) - 10.0f;
			draw_box(_x, y, 10.0f, height, con_outputBarColor);

			auto _height = height;
			if (output.size() > con.visible_line_count)
			{
				const auto percentage = static_cast<float>(con.visible_line_count) / output.size();
				_height *= percentage;

				const auto remainingSpace = height - _height;
				const auto percentageAbove = static_cast<float>(con.display_line_offset) / (output.size() - con.visible_line_count);

				y = y + (remainingSpace * percentageAbove);
			}

			draw_box(_x, y, 10.0f, _height, con_outputSliderColor);
		}

		void draw_output_text(const float x, float y, output_queue& output)
		{
			const auto offset = output.size() >= con.visible_line_count
				? 0.0f
				: (con.font_height * (con.visible_line_count - output.size()));

			for (auto i = 0; i < con.visible_line_count; i++)
			{
				const auto index = i + con.display_line_offset;
				if (index >= output.size())
				{
					break;
				}

				IW::R_AddCmdDrawText(output.at(index).data(), 0x7FFF, &console_font, x, y + offset + (i * con.font_height), 1.0f, 1.0f, 0.0f, con_inputWriteDownColor, 0);
			}
		}

		void draw_output_window()
		{
			con.output.access([](output_queue& output)
				{
					draw_box(con.screen_min[0], con.screen_min[1] + ((1 * con.font_height) + 14.0f), con.screen_max[0] - con.screen_min[0],
						(con.screen_max[1] - con.screen_min[1]) - ((1 * con.font_height) + 14.0f), con_outputWindowColor);

					const auto x = con.screen_min[0] + 6.0f;
					const auto y = (con.screen_min[1] + ((1 * con.font_height) + 14.0f)) + 6.0f;
					const auto width = (con.screen_max[0] - con.screen_min[0]) - 12.0f;
					const auto height = ((con.screen_max[1] - con.screen_min[1]) - ((1 * con.font_height) + 14.0f)) - 12.0f;

					IW::R_AddCmdDrawText(environment_info_formatted_str.c_str(), 0x7FFFFFFF, &console_font, x, y + (height - 16.0f), 1.0f, 1.0f, 0.0f, con_phoenixSDKLabelsColor, 0);

					draw_output_scrollbar(x, y, width, height, output);
					draw_output_text(x, y, output);
				});
		}

		void draw_console()
		{
			std::call_once(con_isInitialized, [&]() {
				console_font.fontName = "phoenix_pFont";
				console_font.fontHandle = imgui::phoenix_pFont;
				console_font.pixelHeight = imgui::phoenix_pFont->FontSize;
			});

			check_resize();

			if (miscKeyFlag & 1)
			{
				if (!(miscKeyFlag & 1))
				{
					con.output_visible = false;
				}

				if (con.output_visible)
				{
					draw_output_window();
				}

				draw_input();
			}
		}
	}

	void print(const char* fmt, ...)
	{
		char va_buffer[0x200] = { 0 };

		va_list ap;
		va_start(ap, fmt);
		vsprintf_s(va_buffer, fmt, ap);
		va_end(ap);

		const auto formatted = std::string(va_buffer);
		const auto lines = utils::string::split(formatted, '\n');

		for (const auto& line : lines)
		{
			print_internal(line);
		}
	}

	void print(const std::string& data)
	{
		const auto lines = utils::string::split(data, '\n');
		for (const auto& line : lines)
		{
			print_internal(line);
		}
	}

	bool console_char_event(const int local_client_num, const int key)
	{
		if (key == '`' ||
			key == '~' ||
			key == '|' ||
			key == '\\')
		{
			return false;
		}

		if (key > 127)
		{
			return true;
		}

		if (miscKeyFlag & 1)
		{
			if (key == KeyCatcher::keyNum_t::K_TAB) // tab (auto complete) 
			{
				if (con.may_auto_complete)
				{
					const auto first_char = con.buffer[0];

					clear_input();

					if (first_char == '\\' || first_char == '/')
					{
						con.buffer[0] = first_char;
						con.buffer[1] = '\0';
					}

					strncat_s(con.buffer, con.auto_complete_choice, 64);
					con.cursor = static_cast<int>(std::string(con.buffer).length());

					if (con.cursor != 254)
					{
						con.buffer[con.cursor++] = ' ';
						con.buffer[con.cursor] = '\0';
					}
				}
			}

			if (key == 'v' - 'a' + 1) // paste
			{
				const auto clipboard = utils::string::get_clipboard_data();
				if (clipboard.empty())
				{
					return false;
				}

				for (size_t i = 0; i < clipboard.length(); i++)
				{
					console_char_event(local_client_num, clipboard[i]);
				}

				return false;
			}

			if (key == 'c' - 'a' + 1) // clear
			{
				clear_input();
				con.total_line_count = 0;
				con.display_line_offset = 0;
				con.output.access([](output_queue& output)
					{
						output.clear();
					});
				history_index = -1;
				history.clear();

				return false;
			}

			if (key == 'h' - 'a' + 1) // backspace
			{
				if (con.cursor > 0)
				{
					memmove(con.buffer + con.cursor - 1, con.buffer + con.cursor,
						strlen(con.buffer) + 1 - con.cursor);
					con.cursor--;
				}

				return false;
			}

			if (key < 32)
			{
				return false;
			}

			if (con.cursor == 256 - 1)
			{
				return false;
			}

			memmove(con.buffer + con.cursor + 1, con.buffer + con.cursor, strlen(con.buffer) + 1 - con.cursor);
			con.buffer[con.cursor] = static_cast<char>(key);
			con.cursor++;

			if (con.cursor == strlen(con.buffer) + 1)
			{
				con.buffer[con.cursor] = 0;
			}

			return false; // intercept all inputs if console is open
		}

		return true;
	}

	bool console_key_event(const int local_client_num, const int key, const int down)
	{
		if (key == KeyCatcher::keyNum_t::K_GRAVE /*|| key == KeyCatcher::keyNum_t::K_TILDE*/)
		{
			if (!down)
			{
				return false;
			}

			const auto shift_down = KeyCatcher::playerKeys.keys[KeyCatcher::keyNum_t::K_SHIFT].down;
			if (shift_down)
			{
				if (!(miscKeyFlag & 1))
				{
					toggle_console();
				}

				toggle_console_output();
				return false;
			}

			toggle_console();

			return false;
		}

		if (miscKeyFlag & 1)
		{
			if (down)
			{
				if (key == KeyCatcher::keyNum_t::K_UPARROW)
				{
					if (++history_index >= history.size())
					{
						history_index = static_cast<int>(history.size()) - 1;
					}

					clear_input();

					if (history_index != -1)
					{
						strncpy_s(con.buffer, history.at(history_index).c_str(), 0x100);
						con.cursor = static_cast<int>(strlen(con.buffer));
					}
				}
				else if (key == KeyCatcher::keyNum_t::K_DOWNARROW)
				{
					if (--history_index < -1)
					{
						history_index = -1;
					}

					clear_input();

					if (history_index != -1)
					{
						strncpy_s(con.buffer, history.at(history_index).c_str(), 0x100);
						con.cursor = static_cast<int>(strlen(con.buffer));
					}
				}
				else if (key == KeyCatcher::keyNum_t::K_RIGHTARROW)
				{
					if (con.cursor < strlen(con.buffer))
					{
						con.cursor++;
					}
				}
				else if (key == KeyCatcher::keyNum_t::K_LEFTARROW)
				{
					if (con.cursor > 0)
					{
						con.cursor--;
					}
				}

				//scroll through output
				if (key == KeyCatcher::keyNum_t::K_MWHEELUP || key == KeyCatcher::keyNum_t::K_PGUP)
				{
					con.output.access([](output_queue& output)
						{
							if (output.size() > con.visible_line_count && con.display_line_offset > 0)
							{
								con.display_line_offset--;
							}
						});
				}
				else if (key == KeyCatcher::keyNum_t::K_MWHEELDOWN || key == KeyCatcher::keyNum_t::K_PGDN)
				{
					con.output.access([](output_queue& output)
						{
							if (output.size() > con.visible_line_count
								&& con.display_line_offset < (output.size() - con.visible_line_count))
							{
								con.display_line_offset++;
							}
						});
				}

				if (key == KeyCatcher::keyNum_t::K_ENTER)
				{
					if (history_index != -1)
					{
						const auto itr = history.begin() + history_index;

						if (*itr == con.buffer)
						{
							history.erase(history.begin() + history_index);
						}
					}

					history.push_front(con.buffer);

					print("]%s\n", con.buffer);
					execute_cmd(fixed_input);

					if (history.size() > 10)
					{
						history.erase(history.begin() + 10);
					}

					history_index = -1;

					clear_input();
				}

			}

			return false; // intercept all inputs if console is open
		}

		return true;
	}

	bool DeveloperConsole_KeyEvent(int key, bool down)
	{
		return console_key_event(0,key,down);
	}

	bool DeveloperConsole_CharEvent(int key)
	{
		return console_char_event(0, key);
	}

	class component final : public component_interface
	{
	public:
		void initialize() override
		{
			imgui::add_menu(draw_console);

			con.cursor = 0;
			con.visible_line_count = 0;
			con.output_visible = false;
			con.display_line_offset = 0;
			con.total_line_count = 0;
			strncpy_s(con.buffer, "", 256);

			con.screen_pointer.x = 0.0f;
			con.screen_pointer.y = 0.0f;
			con.may_auto_complete = false;
			strncpy_s(con.auto_complete_choice, "", 64);

			//// add clear command
			//std::function<void(std::string)> clear_screen_f = [](std::string) { clear_input(); clear_output(); };
			//insert_phoenix_cmd("cls", "clear console output log", clear_screen_f);

			KeyCatcher::key_event_callbacks.push_back(DeveloperConsole_KeyEvent);
			KeyCatcher::char_event_callbacks.push_back(DeveloperConsole_CharEvent);
		}
	};
}

REGISTER_COMPONENT(game_console::component)