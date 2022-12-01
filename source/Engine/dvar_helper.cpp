/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|
|- Purpose        : IW Engine Dvar Helper Functions and definitions
|
|- Developer      : Huseyin
|- Timestamp      : 2022-10-6
|
|
|- Phoenix SDK Copyright © Hosseinpourziyaie 2022 <hosseinpourziyaie@gmail.com>
|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <std_include.hpp>
#include <dvar_helper.hpp>

namespace iw_dvar_helper
{
	std::string dvar_get_vector_domain(const int components, const iw_dvar_limits& domain)
	{
		if (domain.vector.min == -FLT_MAX)
		{
			if (domain.vector.max == FLT_MAX)
			{
				return utils::string::va("Domain is any %iD vector", components);
			}
			else
			{
				return utils::string::va("Domain is any %iD vector with components %g or smaller", components,
					domain.vector.max);
			}
		}
		else if (domain.vector.max == FLT_MAX)
		{
			return utils::string::va("Domain is any %iD vector with components %g or bigger", components,
				domain.vector.min);
		}
		else
		{
			return utils::string::va("Domain is any %iD vector with components from %g to %g", components,
				domain.vector.min, domain.vector.max);
		}
	}

	std::string dvar_domain_to_string(const iw_dvar_type type, const iw_dvar_limits& domain)
	{
		std::string str;

		switch (type)
		{
		case iw_dvar_type::boolean:
		case iw_dvar_type::boolean_hashed:
			return "Domain is 0 or 1"s;

		case iw_dvar_type::value:
		case iw_dvar_type::value_hashed:
			if (domain.value.min == -FLT_MAX)
			{
				if (domain.value.max == FLT_MAX)
				{
					return "Domain is any number"s;
				}
				else
				{
					return utils::string::va("Domain is any number %g or smaller", domain.value.max);
				}
			}
			else if (domain.value.max == FLT_MAX)
			{
				return utils::string::va("Domain is any number %g or bigger", domain.value.min);
			}
			else
			{
				return utils::string::va("Domain is any number from %g to %g", domain.value.min, domain.value.max);
			}

		case iw_dvar_type::vec2:
			return dvar_get_vector_domain(2, domain);

		case iw_dvar_type::rgb:
		case iw_dvar_type::vec3:
			return dvar_get_vector_domain(3, domain);

		case iw_dvar_type::vec4:
			return dvar_get_vector_domain(4, domain);

		case iw_dvar_type::integer:
		case iw_dvar_type::integer_hashed:
			if (domain.enumeration.stringCount == INT_MIN)
			{
				if (domain.integer.max == INT_MAX)
				{
					return "Domain is any integer"s;
				}
				else
				{
					return utils::string::va("Domain is any integer %i or smaller", domain.integer.max);
				}
			}
			else if (domain.integer.max == INT_MAX)
			{
				return utils::string::va("Domain is any integer %i or bigger", domain.integer.min);
			}
			else
			{
				return utils::string::va("Domain is any integer from %i to %i", domain.integer.min, domain.integer.max);
			}

		case iw_dvar_type::color:
			return "Domain is any 4-component color, in RGBA format"s;

		case iw_dvar_type::enumeration:
			str = "Domain is one of the following:"s;

			for (auto string_index = 0; string_index < domain.enumeration.stringCount; ++string_index)
			{
				str += utils::string::va("\n  %2i: %s", string_index, domain.enumeration.strings[string_index]);
			}

			return str;

		case iw_dvar_type::string:
			return "Domain is any text"s;

		default:
			return utils::string::va("unhandled dvar type '%i'", type);
		}
	}

	std::string dvar_value_to_string(const iw_dvar_type type, const iw_dvar_value value)
	{
		switch (type)
		{
		case iw_dvar_type::boolean:
			if (value.enabled) return "1"; else return "0";

		case iw_dvar_type::value:
			return std::to_string(value.value);

		case iw_dvar_type::integer:
			return std::to_string(value.unsignedInt);

		case iw_dvar_type::string:
			return value.string;

		default:
			return "N/A";
		}
	}
}

