#include "pch.h"
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


const char* CategoryToString(Logger::LogCategory c)
{
    switch (c)
    {
    case Logger::LogCategory::Gameplay: return "[Gameplay]";
    case Logger::LogCategory::Physics:  return "[Physics ]";
    case Logger::LogCategory::UI:       return "[UI      ]";
    case Logger::LogCategory::System:   return "[System  ]";
    }
    return "[Unknown ]";
}

bool IsCategoryVisible(Logger::LogCategory category)
{
    switch (category)
    {
    case Logger::LogCategory::Gameplay: return Logger::showGameplay;
    case Logger::LogCategory::Physics:  return Logger::showPhysics;
    case Logger::LogCategory::UI:       return Logger::showUI;
    case Logger::LogCategory::System:   return Logger::showSystem;
    }
    return true;
}

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

    instance.logItems.clear();
    instance.log.clear();
    //instance.log.reserve(SIZE_MAX);//無限にログを貯められるようにする 大きすぎるので小さくする
    instance.log.reserve(INT_MAX);//無限にログを貯められるようにする

}

void Logger::Log(LogCategory category, const char* message)
{
    std::lock_guard lock(Instance().mtx);
    Instance().logQueue.emplace(std::time(nullptr), category, message);
}

void Logger::Log(LogCategory category, const char8_t* message)
{
    Log(category, reinterpret_cast<const char*>(message));
}

void Logger::Log(const char* message)
{
    Log(LogCategory::Gameplay, message);
}

void Logger::Log(const char8_t* message)
{
    Log(LogCategory::Gameplay, reinterpret_cast<const char*>(message));
}

void Logger::Warning(LogCategory category, const char* message)
{
    std::lock_guard lock(Instance().mtx);
    Instance().logQueue.emplace(std::time(nullptr), category, std::format("[WARNING] {}", message));
}

void Logger::Warning(const char* message)
{
    Warning(LogCategory::Gameplay, message);
}

void Logger::Warning(const char8_t* message)
{
    Warning(reinterpret_cast<const char*>(message));
}

void Logger::Warning(LogCategory category, const char8_t* message)
{
    Warning(category, reinterpret_cast<const char*>(message));
}

void Logger::Error(LogCategory category, const char* message, std::source_location location)
{
    std::lock_guard lock(Instance().mtx);
    Instance().logQueue.emplace(std::time(nullptr), category, std::format("[ERROR]\n\n\n ErrorLocation: \n\n\n\t File : {}\n\n\n\t Function : {}\n\n\n\t Line : {}\n\n\n Message :\n\n\n\t {}", location.file_name(), location.function_name(), location.line(), message));
}

void Logger::Error(const char8_t* message, std::source_location location)
{
    Error(reinterpret_cast<const char*>(message), location);
}

void Logger::Error(const char* message, std::source_location location)
{
    Error(LogCategory::Gameplay, message, location);
}

void Logger::Error(LogCategory category, const char8_t* message, std::source_location location)
{
    Error(category, reinterpret_cast<const char*>(message), location);
}

void Logger::DrawImGui()
{
#ifdef USE_IMGUI

    if (ImGui::TreeNode(reinterpret_cast<const char*>(u8"ログ情報")))
    {
        ImGui::Checkbox("Gameplay", &showGameplay);
        ImGui::SameLine();
        ImGui::Checkbox("Physics", &showPhysics);
        ImGui::SameLine();
        ImGui::Checkbox("UI", &showUI);
        ImGui::SameLine();
        ImGui::Checkbox("System", &showSystem);
        ImGui::SameLine();
        if (ImGui::Button("Clear"))
        {
            auto& instance = Instance();
            instance.logItems.clear();
            instance.log.clear();
        }
        ImGui::SameLine();
        ImGui::Checkbox("AutoScroll", &autoScroll);
        ImGui::Separator();

        auto& instance = Instance();
        ImGui::BeginChild("", ImVec2(), true, ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& item : instance.logItems)
        {
            // カテゴリフィルタ
            if (!IsCategoryVisible(item.category))
                continue;

            bool colored = false;

            // 色分け
            if (item.message.find("[ERROR]") != std::string::npos)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.2f, 0.2f, 1));
                colored = true;
            }
            else if (item.message.find("[WARNING]") != std::string::npos)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.8f, 0.2f, 1));
                colored = true;
            }

            // ★ 1行として表示（ここ重要）
            std::string line = std::format(
                "{} {} : {}",
                CategoryToString(item.category),
                item.timeString,
                item.message
            );

            ImGui::TextUnformatted(line.c_str());

            if (colored)
                ImGui::PopStyleColor();
        }

        if (autoScroll)
        {
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            {
                ImGui::SetScrollHereY(1.0f);
            }
        }

        ImGui::EndChild();
#if 0
        //1行ずつ取得する
        size_t start = 0, end = 0;
        while ((end = instance.log.find('\n', start)) != std::string::npos)
        {
            std::string_view str(instance.log.c_str() + start, end - start);
            bool cmd = false;
            if (str.size() == 0)
            {
                if (*str.data() == '\n' && *(str.data() + 1) == '\n')//Error
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

            ImGui::TextUnformatted(str.data(), str.data() + (end - start));

            if (cmd)ImGui::PopStyleColor();
            start = end + 1;
        }

        ImGui::EndChild();

#endif // 0
        ImGui::TreePop();
    }
#endif
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
            LogItem item = instance.logQueue.front(); 
            instance.logQueue.pop();

            if (currentTime != item.time)
            {
                currentTime = item.time;
                std::tm localTime{};
                localtime_s(&localTime, &currentTime);
                char buf[80];
                std::strftime(buf, sizeof(buf), fmt, &localTime);
                timeStr = buf;
            }

            item.timeString = timeStr;

            std::string str = std::format(
                "{} {} : {}\n",
                CategoryToString(item.category),
                timeStr,
                item.message
            );

            ofs << str;
            instance.log += str;
            instance.logItems.push_back(std::move(item));
        }
        ofs.close();
    }
}
