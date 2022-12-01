#include <std_include.hpp>
#include <Utilities/nt.hpp>

uintptr_t operator"" _b(const uintptr_t ptr)
{
	static size_t base_address;
	if(!base_address) base_address = size_t(utils::nt::library{}.get_ptr());

	return base_address + ptr;
}