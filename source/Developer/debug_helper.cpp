/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|
|- Purpose        : Diagnosting Toolkit
|
|- Developer      : Huseyin
|- Timestamp      : 2022-10-16
|
|
|- Phoenix SDK Copyright © Hosseinpourziyaie 2022 <hosseinpourziyaie@gmail.com>
|
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include <std_include.hpp>
#include <Developer/debug_helper.hpp>

#define TIMESTAMP

namespace diagnostic
{
	std::string get_type_str(const int type)
	{
		switch (type)
		{
		case 1:
			return "INFO";
		case 2:
			return "WARN";
		case 3:
			return "ERROR";
		default:
			return "DEBUG";
		}
	}

	void write(const int type, std::string str)
	{
		std::ofstream stream;
		stream.open("phoenix.log", std::ios_base::app);

#ifdef TIMESTAMP
		time_t now = time(0);
		std::tm* t = std::localtime(&now);
		stream << "" << std::put_time(t, "%Y-%m-%d %H:%M:%S") << "\t";
#endif // TIMESTAMP

		stream << "[ " << get_type_str(type) << " ] " << str << std::endl;
	}

	void write(const int type, const char* fmt, ...)
	{
		char va_buffer[0x200] = { 0 };

		va_list ap;
		va_start(ap, fmt);
		vsprintf_s(va_buffer, fmt, ap);
		va_end(ap);

		const auto formatted = std::string(va_buffer);
		write(type, formatted);
	}
}