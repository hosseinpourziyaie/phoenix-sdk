/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|
|- Purpose        : Component Handling system; super neat design
|
|- Developer      : xLabs Devs (Slightly Modified by Huseyin)
|- Timestamp      : 2020-??-??
|
|
|- Phoenix SDK Copyright © Hosseinpourziyaie 2022 <hosseinpourziyaie@gmail.com>
|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <std_include.hpp>

std::once_flag renderer_first_time;

void component_loader::register_component(std::unique_ptr<component_interface>&& component_)
{
	auto& components = get_components();
	components.push_back(std::move(component_));

	std::ranges::stable_sort(components, [](const std::unique_ptr<component_interface>& a,
	                                        const std::unique_ptr<component_interface>& b)
	{
		return a->load_sequence() < b->load_sequence();
	});
}

bool component_loader::initialize()
{
	clean();

	try
	{
		for (const auto& component_ : get_components())
		{
			component_->initialize();
		}
	}
	catch (premature_shutdown_trigger&)
	{
		return false;
	}
	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR);
		return false;
	}

	return true;
}

bool component_loader::render_frame()
{
	try
	{
		std::call_once(renderer_first_time, [&]()
			{
				for (const auto& component_ : get_components())
				{
					component_->render_start();
				}
			});

		for (const auto& component_ : get_components())
		{
			component_->render_frame();
		}
	}
	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR);
		return false;
	}

	return true;
}

void component_loader::shutdown()
{
	static auto res = []
	{
		try
		{
			for (const auto& component_ : get_components())
			{
				component_->shutdown();
			}
		}
		catch (const std::exception& e)
		{
			MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR);
			return false;
		}

		return true;
	}();

	if (!res)
	{
		TerminateProcess(GetCurrentProcess(), 1);
	}
}

void component_loader::clean()
{
	auto& components = get_components();
	for (auto i = components.begin(); i != components.end();)
	{
		if (!(*i)->is_supported())
		{
			(*i)->shutdown();
			i = components.erase(i);
		}
		else
		{
			++i;
		}
	}
}

void component_loader::trigger_premature_shutdown()
{
	throw premature_shutdown_trigger();
}

std::vector<std::unique_ptr<component_interface>>& component_loader::get_components()
{
	using component_vector = std::vector<std::unique_ptr<component_interface>>;
	using component_vector_container = std::unique_ptr<component_vector, std::function<void(component_vector*)>>;

	static component_vector_container components(new component_vector, [](const component_vector* component_vector)
	{
		shutdown();
		delete component_vector;
	});

	return *components;
}