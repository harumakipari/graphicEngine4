#pragma once
#include <filesystem>
#include <mutex>
#include <queue>
#include <source_location>


class Logger
{
public:
	constexpr static const char* OutputPath = ".\\Data\\Log";
public:
	Logger();
	~Logger();

	static void Initialize();

	static void Log(const char* message);
	static void Warning(const char* message);
	static void Error(const char* message,std::source_location location = std::source_location::current());
	//static void Log(const wchar_t* message);

	static void RenderIMGUI();

private:
	static Logger& Instance() { static Logger instance; return instance; }

	static void LogThreadFunc();
private:
	std::filesystem::path logfilePath;

	std::string log;
	std::queue<std::pair<time_t,std::string>> logQueue;
	std::mutex mtx;

	std::thread logThread_;
	inline static bool logThreadLoop = true;
};
