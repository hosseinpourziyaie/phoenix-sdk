#pragma once

namespace diagnostic
{
	void write(const int type, std::string content);
	void write(const int type, const char* fmt, ...);

}