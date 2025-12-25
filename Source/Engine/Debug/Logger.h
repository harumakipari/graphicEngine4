#pragma once
#include <filesystem>
#include <mutex>
#include <queue>
#include <source_location>



class Logger
{
public:
	constexpr static const char* OutputPath = ".\\Data\\Log";

	enum class LogCategory : uint8_t
	{
		Gameplay,
		Physics,
		UI,
		System,
	};

public:
	Logger();
	~Logger();

	static void Initialize();

	static void Log(const char* message);
	static void Log(const char8_t* message);

	static void Log(LogCategory category, const char* message);
	static void Log(LogCategory category, const char8_t* message);

	static void Warning(const char* message);
	static void Warning(const char8_t* message);

	static void Warning(LogCategory category, const char* message);
	static void Warning(LogCategory category, const char8_t* message);

    static void Error(const char* message,std::source_location location = std::source_location::current());
	static void Error(const char8_t* message,std::source_location location = std::source_location::current());

    static void Error(LogCategory category, const char* message,std::source_location location = std::source_location::current());
	static void Error(LogCategory category, const char8_t* message,std::source_location location = std::source_location::current());

	//static void Log(const wchar_t* message);

	static void DrawImGui();

private:
	static Logger& Instance() { static Logger instance; return instance; }

	static void LogThreadFunc();
private:
	std::filesystem::path logfilePath;

	std::string log;

	struct LogItem
	{
		time_t time;
		LogCategory category;
		std::string message;
        std::string timeString;

	};
	std::queue<LogItem> logQueue;
	std::mutex mtx;

	std::thread logThread_;
	inline static bool logThreadLoop = true;

    std::vector<LogItem> logItems; // Logger メンバ
	inline static bool autoScroll = true;
public:
	inline static bool showGameplay = true;
	inline static bool showPhysics = true;
	inline static bool showUI = true;
	inline static bool showSystem = true;
};
