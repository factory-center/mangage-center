#pragma once
#if (defined _WIN32) || (defined _WIN64)
#include <io.h>
#include <direct.h>  
#endif  
#ifdef linux   
#include <unistd.h> 
#include <sys/types.h>  
#include <sys/stat.h> 
#endif 


#if (defined _WIN32) || (defined _WIN64)
#define __FILENAME__ (strrchr(__FILE__, '\\') ? (strrchr(__FILE__, '\\') + 1):__FILE__)
#else
#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#endif

//定义一个在日志后添加 文件名 函数名 行号 的宏定义
#ifndef suffix
#define suffix(msg)  std::string().append("<")\
        .append(__FILENAME__).append("><").append(__func__)\
        .append("><").append(std::to_string(__LINE__))\
        .append("> ").append(msg).c_str()
#endif

//在  spdlog.h   之前定义，才有效
#ifndef SPDLOG_TRACE_ON
#define SPDLOG_TRACE_ON
#endif

#ifndef SPDLOG_DEBUG_ON
#define SPDLOG_DEBUG_ON
#endif

#include <spdlog\spdlog.h>
#include <spdlog\common.h>
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <spdlog\sinks\daily_file_sink.h>


#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/ini_parser.hpp>

class Logger
{

public:
	static Logger& GetInstance() 
	{
		static Logger m_instance;
		return m_instance;
	}

	auto GetLogger() 
	{ 
		return nml_logger; 
	}

private:
	Logger() 
	{
		//读取配置文件
		boost::property_tree::ptree pt_config, pt_section;
		try
		{
			read_ini("./emcs.cfg", pt_config);
			std::string str_section_name = "logger";
			pt_section = pt_config.get_child(str_section_name); //失败时会抛出异常
			nlog_level = (spdlog::level::level_enum)pt_section.get<int>("level");
			nlog_flush = (spdlog::level::level_enum)pt_section.get<int>("flush");
			s_log_file_path = pt_section.get<std::string>("file");
		}
		catch (std::exception& e)
		{
			printf(e.what());
			nlog_level = spdlog::level::err;
			s_log_file_path = "./log/emcs.log";
		}

		//确保目录存在
		int nPos = s_log_file_path.find_last_of('/');
		std::string s_file_path = s_log_file_path.substr(0, nPos);
		MakeSureDirExist(s_file_path.c_str(), s_file_path.length());

		//设置为异步日志
		//spdlog::set_async_mode(32768);  // 必须为 2 的幂
		std::vector<spdlog::sink_ptr> vecSink;
#ifdef _CONSOLE
		vecSink.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
#endif
		vecSink.push_back(std::make_shared<spdlog::sinks::daily_file_sink_mt>(s_log_file_path, 0, 0));
		nml_logger = std::make_shared<spdlog::logger>("both", begin(vecSink), end(vecSink));
		spdlog::register_logger(nml_logger);

		//设置日志记录级别
		nml_logger->set_level(nlog_level);
		nml_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%5l%$]  %v");
		//设置当出发 err 或更严重的错误时立刻刷新日志到  disk 
		nml_logger->flush_on(nlog_flush);

		//spdlog::flush_every(std::chrono::seconds(3));
	}

	~Logger() 
	{
		spdlog::drop_all();
	}

	bool MakeSureDirExist(const char *dir,int len)
	{
		if (_access(dir, 0) == -1)
		{
#if (defined _WIN32) || (defined _WIN64) 
			int flag = _mkdir(dir);
#endif  
#ifdef linux   
			int flag = mkdir(dir.c_str(), 0777);
#endif  
			return (flag == 0);
		}
		return true;
	}
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

private:
	std::shared_ptr<spdlog::logger> nml_logger;
	spdlog::level::level_enum nlog_level;
	spdlog::level::level_enum nlog_flush;
	std::string s_log_file_path;
};

#define LTrace(msg,...) Logger::GetInstance().GetLogger()->trace(suffix(msg),__VA_ARGS__)
#define LDebug(msg,...) Logger::GetInstance().GetLogger()->debug(suffix(msg),__VA_ARGS__)
#define LInfo(msg,...) Logger::GetInstance().GetLogger()->info(suffix(msg),__VA_ARGS__)
#define LWarn(msg,...) Logger::GetInstance().GetLogger()->warn(suffix(msg),__VA_ARGS__)
#define LError(msg,...) Logger::GetInstance().GetLogger()->error(suffix(msg),__VA_ARGS__)
#define LCritical(msg,...) Logger::GetInstance().GetLogger()->critical(suffix(msg),__VA_ARGS__)

#if (defined _WIN32) || (defined _WIN64)
#define errcode WSAGetLastError()
#endif