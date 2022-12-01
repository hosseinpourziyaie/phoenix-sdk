#include <std_include.hpp>
#include <definitions.hpp>
#include <imgui_core.hpp>
#include <Input/KeyCatcher.hpp>
#include <Engine/engine_hq.hpp>
#include <Developer/dev_console.hpp>

namespace Template
{
	bool showMenu = true;
	using namespace EngineHQ;
	using namespace KeyCatcher;


	void Imgui_Template()
	{
		if (!showMenu) return;
		ImGui::ShowDemoWindow(&showMenu);
	}

	bool KeyCatcher_Test(int key, bool down)
	{
		if (key == VK_INSERT && down == false)
		{
			showMenu ? showMenu = false : showMenu = true;
		}

		return true; // returning false will prevent the key strike from reaching to actual window
	}

	void phoenix_cmd_test_f(std::string arg)
	{
		game_console::print("Test command executed with following argument: \"%s\"", arg.c_str());
		log_debug_string("Test command executed with following argument: \"%s\"", arg.c_str());
	}

	class component final : public component_interface
	{
	public:
		void initialize() override // initialize event callback
		{
			imgui::add_menu(Imgui_Template); // Add Imgui Instance

			key_event_callbacks.push_back(KeyCatcher_Test); // Register KeyCatcher

			insert_phoenix_cmd("test", "sample for testing phoenix command system", phoenix_cmd_test_f); // Register EngineHQ Command
		}

		void shutdown() override // shutdown event callback
		{

		}

		int load_sequence() override // Initialization priorty 
		{
			return sequence::priority_mid;
		}

		bool is_supported() override // ignore component if certain conditions not met 
		{
			return true;
		}
	};
}

REGISTER_COMPONENT(Template::component)
