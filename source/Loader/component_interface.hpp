#pragma once

class component_interface
{
public:
	virtual ~component_interface() = default;

	virtual void initialize()
	{
	}

	virtual void shutdown()
	{
	}

	virtual void render_start()
	{
	}

	virtual void render_frame()
	{
	}
	
	virtual bool is_supported()
	{
		return true;
	}

	virtual int load_sequence()
	{
		return sequence::priority_mid;
	}

	enum sequence
	{
		priority_first = 1,
		priority_mid = 2,
		priority_last = 3
	};

	enum MyEnum
	{

	};
};
