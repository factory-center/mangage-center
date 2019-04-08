#ifdef WIN32
#include <SDKDDKVer.h>
#endif

#include "../source/busin_log.h"
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


int main()
{
	//默认日志配置，具体参见类型Log_Cfg_T
	string str_log_path = "../log/emcs.log";
	businlog_cfg default_cfg(str_log_path.c_str(), "Network Engraving Machine Logging");
	default_cfg.level(7);
	string str_cfg_file_path = "./emcs.cfg";
	//打开配置文件，如果失败，则使用默认日志配置
	int ret = businlog_open(default_cfg, str_cfg_file_path.c_str());
	if (ret)
	{
		businlog_error("%s | open log file:%s error, ret:%d", __FUNCTION__, str_log_path.c_str(), ret);
		return false;
	}
	//初始化库
	string str_err_reason;
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->init(str_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to init CCarve_Common_Lib_Tool, reason:%s", __FUNCTION__, str_err_reason.c_str()), false);
	string str_err_reason_for_debug, str_err_reason_for_user;
#ifdef SERVER_WITH_CONTROL_LOGIC
	ret = CCarve_Manager::instance()->start_poll_carve_status(str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(!ret, ("%s | start_poll_carve_status failed, reason:%s", __FUNCTION__, str_err_reason_for_debug.c_str()), false);
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
		businlog_error("%s | open config file:%s error, err reason:%s", __FUNCTION__, str_cfg_file_path.c_str(), e.what());
		return false;
	}
	if (str_local_ip.empty())
	{
		businlog_error("%s | fail to read ip from config file:%s.", __FUNCTION__, str_cfg_file_path.c_str());
		return false;
	}
	if (str_port.empty())
	{
		businlog_error("%s | fail to read port from config file:%s.", __FUNCTION__, str_cfg_file_path.c_str());
		return false;
	}

	businlog_info("%s | start:ip[%s],port[%s]", __FUNCTION__, str_local_ip.c_str(), str_port.c_str());
	ret = singleton_default<CSingleton_Server>::instance().start(
		str_local_ip, str_port, nThread_num, str_err_reason);

	businlog_error_return(!ret, ("%s | fail to start Server, reason:%s", __FUNCTION__, str_err_reason.c_str()), false);
	singleton_default<CSingleton_Server>::instance().thread_join();

	return true;
}
