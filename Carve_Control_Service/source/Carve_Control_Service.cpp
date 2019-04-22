#ifdef WIN32
#include <SDKDDKVer.h>
#endif

#include "../source/CSpdLog.h"
#include "../source/CCarve.h"
#include "utils/msp_errors.h"
#include "../source/carve_common_lib.h"
#include "../source/http_server/server.hpp"
#include "../source/http_server/singleton_server.h"
#include <boost_common.h>
#include <json/json.h>
#include "../source/carve_manager.h"
#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/ini_parser.hpp>
#if (defined WIN32) || (defined WIN64)
#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")
#endif
int main()
{
	//默认日志配置，具体参见类型Log_Cfg_T
	int ret = 0;
	string str_log_path = "../log/emcs.log";
	string str_cfg_file_path = "./emcs.cfg";
	
	//初始化库
	string str_err_reason;
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->init(str_err_reason);
	if (bSuccess == false)
	{
		LError("fail to init CCarve_Common_Lib_Tool, reason:{}", str_err_reason);
		return false;
	}
	string str_err_reason_for_debug, str_err_reason_for_user;
#ifdef SERVER_WITH_CONTROL_LOGIC
	ret = CCarve_Manager::instance()->start_poll_carve_status(str_err_reason_for_debug, str_err_reason_for_user);
	if (0!=ret)
	{
		LError("start_poll_carve_status failed, reason:{}", str_err_reason_for_debug);
		return false;
	}

#endif
	//启动网络模块：创建线程以监听端口
	//从配置文件中读取本地ip和port
	string str_local_ip;	//本机ip
	string str_port;		//本机端口
	size_t nThread_num = 5;
	boost::property_tree::ptree pt_config, pt_section;
	try
	{
		read_ini(str_cfg_file_path, pt_config);
		string str_section_name = "network";
		pt_section = pt_config.get_child(str_section_name); //失败时会抛出异常
		str_local_ip = pt_section.get<string>("ip");
		str_port = pt_section.get<string>("port");
		nThread_num = pt_section.get<int>("receiver_num", 5);
	}
	catch (std::exception& e)
	{
		LError("open config file:{} error, err reason:{}", str_cfg_file_path, e.what());
		return false;
	}
	if (str_local_ip.empty())
	{
		LError("fail to read ip from config file:{}", str_cfg_file_path);
		return false;
	}
	if (str_port.empty())
	{
		LError("fail to read port from config file:{}", str_cfg_file_path);
		return false;
	}

	LInfo("start:ip[{}],port[{}]", str_local_ip, str_port);
	ret = singleton_default<CSingleton_Server>::instance().start(
		str_local_ip, str_port, nThread_num, str_err_reason);
	if (0!=ret)
	{
		LError("fail to start Server, reason:{}", str_err_reason);
		return false;
	}

	singleton_default<CSingleton_Server>::instance().thread_join();

	return true;
}
