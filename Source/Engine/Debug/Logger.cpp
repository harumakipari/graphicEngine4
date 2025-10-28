#include "Logger.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "imgui.h"
#endif


#include <filesystem>
#include <fstream>
#include <chrono>
#include <format>


static constexpr const char* fmt = "%Y-%m-%d__%H-%M-%S";

using namespace std::chrono;

Logger::Logger() {
	logThread_ = std::thread(&Logger::LogThreadFunc);
}

Logger::~Logger() {
	logThreadLoop = false;
	if (logThread_.joinable()) {
		logThread_.join();
	}
}

void Logger::Initialize() {

	auto& instance = Instance();

	//現在時刻を取得
	auto now = std::time(nullptr);
	//auto* localTime = std::localtime(&now);
	std::tm localTime{};
	localtime_s(&localTime, &now);

	char timeStr[80];
	//std::strftime(timeStr, sizeof(timeStr), fmt, localTime);
	std::strftime(timeStr, sizeof(timeStr), fmt, &localTime);
    std::string filename;
	filename = "log_";
	filename += timeStr;
	filename += ".txt";

	instance.logfilePath = OutputPath;
	instance.logfilePath /= filename;

	//絶対パス化
	instance.logfilePath = std::filesystem::absolute(instance.logfilePath);

	//ディレクトリがなければ作成
	std::filesystem::create_directories(instance.logfilePath.parent_path());

	instance.log.clear();
	//instance.log.reserve(SIZE_MAX);//無限にログを貯められるようにする 大きすぎるので小さくする
	instance.log.reserve(INT_MAX);//無限にログを貯められるようにする

}

void Logger::Log(const char* message) {
	std::lock_guard lock(Instance().mtx);
	Instance().logQueue.emplace(std::time(nullptr), message);
}

void Logger::Warning(const char* message) {
	std::lock_guard lock(Instance().mtx);
	Instance().logQueue.emplace(std::time(nullptr), std::format("[WARNING] {}", message));
}

void Logger::Error(const char* message,std::source_location location) {
	std::lock_guard lock(Instance().mtx);
	Instance().logQueue.emplace(std::time(nullptr), std::format("[ERROR]\n\n\n ErrorLocation: \n\n\n\t File : {}\n\n\n\t Function : {}\n\n\n\t Line : {}\n\n\n Message :\n\n\n\t {}",location.file_name(),location.function_name(),location.line(), message));
}

void Logger::RenderIMGUI() {
	if (ImGui::TreeNode(reinterpret_cast<const char*>(u8"ログ情報")))
	{
		auto& instance = Instance();
		ImGui::BeginChild("", ImVec2(), true, ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar);

		//1行ずつ取得する
		size_t start = 0,end = 0;
		while ((end = instance.log.find('\n',start))!=std::string::npos)
		{
			std::string_view str(instance.log.c_str() + start, end - start);
			bool cmd = false;
			if (str.size()==0)
			{
				if (*str.data() =='\n'&&*(str.data()+1)=='\n')//Error
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
					start = start + 2;//2文字分進める
					end = instance.log.find('\n', start);
					str = std::string_view(instance.log.c_str() + start, end - start);
					cmd = true;
				}
				else if (*str.data() == '\n')//Warning
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
					start = start + 1;//1文字分進める
					end = instance.log.find('\n', start);
					str = std::string_view(instance.log.c_str() + start, end - start);
					cmd = true;
				}
			}

			ImGui::TextUnformatted(str.data(),str.data() + (end - start));

			if (cmd)ImGui::PopStyleColor();
			start = end + 1;
		}

		ImGui::EndChild();
		ImGui::TreePop();
	}
}

//void Logger::Log(const wchar_t* message) {
//	std::ofstream ofs(Instance().logfilePath, std::ios::out | std::ios::app);
//	if (!ofs.is_open())
//	{
//		//ファイルを新規作成
//		ofs.open(Instance().logfilePath, std::ios::out);
//		if (!ofs.is_open())throw std::runtime_error("Failed to create log file.");
//	}
//
//	auto now = std::time(nullptr);
//	auto* localTime = std::localtime(&now);
//	char timeStr[80];
//	std::strftime(timeStr, sizeof(timeStr), fmt, localTime);
//
//	std::lock_guard lock(Instance().mtx);
//	ofs << timeStr << " : " << std::string(message, message + wcslen(message)) << std::endl;
//
//	ofs.close();
//}

void Logger::LogThreadFunc() {
	while (logThreadLoop)
	{
		if (Instance().logQueue.empty())continue;

		auto& instance = Instance();
		std::lock_guard lock(instance.mtx);
		std::ofstream ofs(instance.logfilePath, std::ios::out | std::ios::app);
		if (!ofs.is_open())return;

		time_t currentTime = 0;
		std::string timeStr;

		while (!instance.logQueue.empty())
		{
			auto [time, message] = instance.logQueue.front();
			if (currentTime != time) {
				currentTime = time;
				//const auto* localTime = std::localtime(&currentTime);
				std::tm localTime{};
				localtime_s(&localTime, &currentTime);
			    char buf[80];
				//std::strftime(buf, sizeof(buf), fmt, localTime);
				std::strftime(buf, sizeof(buf), fmt, &localTime);
				timeStr = buf;
			}

			std::string str = std::format("{} : {}\n", timeStr, message);
			if (message[0] == '[')//エラーメッセージや警告は特別扱い
			{
				if (message.find("[WARNING]")!=std::string::npos)
				{
					str = "\n" + str;
				}
				else if (message.find("[ERROR]")!=std::string::npos)
				{
					str = "\n\n" + str;
				}
			}

			ofs << str;
			instance.log += str;
			instance.logQueue.pop();
		}
		ofs.close();
	}
}
