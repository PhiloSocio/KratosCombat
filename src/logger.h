#pragma once

#ifdef DEBUG_MODE
	#include <spdlog/sinks/basic_file_sink.h>
#else
	#include <spdlog/sinks/msvc_sink.h>
#endif

void SetupLog() {

#ifdef DEBUG_MODE
	auto logsFolder = SKSE::log::log_directory();
	if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
	auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
	auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
	auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
	auto level = spdlog::level::trace;
#else
	auto fileLoggerPtr = std::make_shared<spdlog::sinks::msvc_sink_mt>();
	auto level = spdlog::level::info;
#endif
	auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
	spdlog::set_default_logger(std::move(loggerPtr));
	spdlog::set_level(level);
	spdlog::flush_on(level);
}
