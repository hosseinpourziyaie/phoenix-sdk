#pragma once

namespace iw_dvar_helper
{
	enum iw_dvar_type : std::int8_t
	{
		boolean = 0,
		boolean_hashed = 10,
		value = 1,
		value_hashed = 11,
		vec2 = 2,
		vec3 = 3,
		vec4 = 4,
		integer = 5,
		integer_hashed = 12,
		enumeration = 6,
		string = 7,
		color = 8,
		rgb = 9 // Color without alpha
	};

	union iw_dvar_value
	{
		bool enabled;
		int integer;
		unsigned int unsignedInt;
		float value;
		float vector[4];
		const char* string;
		char color[4];
	};

	union iw_dvar_limits
	{
		struct
		{
			int stringCount;
			const char* const* strings;
		} enumeration;

		struct
		{
			int min;
			int max;
		} integer;

		struct
		{
			float min;
			float max;
		} value;

		struct
		{
			float min;
			float max;
		} vector;
	};

	struct iw_dvar_t
	{
		const char* name;
		unsigned int flags;
		iw_dvar_type type;
		bool modified;
		iw_dvar_value current;
		iw_dvar_value latched;
		iw_dvar_value reset;
		iw_dvar_limits domain;
	};


	std::string dvar_domain_to_string(const iw_dvar_type type, const iw_dvar_limits& domain);
	std::string dvar_value_to_string(const iw_dvar_type type, const iw_dvar_value value);
}