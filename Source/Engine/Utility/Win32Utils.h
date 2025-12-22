#pragma once

#include <windows.h>

inline LPWSTR hr_trace(HRESULT hr)
{
	LPWSTR msg{ 0 };
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPWSTR>(&msg), 0, NULL);
	return msg;
}

#define U8(x) reinterpret_cast<const char*>(u8##x)
//inline const char* U8(const char8_t* s)
//{
//	return reinterpret_cast<const char*>(s);
//}