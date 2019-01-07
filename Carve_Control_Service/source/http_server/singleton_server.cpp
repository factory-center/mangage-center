/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: singleton_server.cpp
* @brief: 简短说明文件功能、用途 (Comment)。
* @author:	minglu2
* @version: 1.0
* @date: 2019/01/03
* 
* @see
* 
* <b>版本记录：</b><br>
* <table>
*  <tr> <th>版本	<th>日期		<th>作者	<th>备注 </tr>
*  <tr> <td>1.0	    <td>2019/01/03	<td>minglu	<td>Create head file </tr>
* </table>
*****************************************************************/
#include "singleton_server.h"
#include "server.hpp"
#include <boost/thread.hpp>
#include "../busin_log.h"
#include "utils/msp_errors.h"
#ifdef _WINDOWS
#define __CLASS_FUNCTION__ ((std::string(__FUNCTION__)).c_str()) 
#else
#define __CLASS_FUNCTION__ ((std::string("CSingleton_Server::") + std::string(__FUNCTION__)).c_str()) 
#endif


/************************************
* Method:    start
* Brief:  启动服务来监听本地地址和端口
* Access:    public 
* Returns:   int 0:success; other:error code
* Qualifier:
*Parameter: const std::string & str_ip -[in] 本地地址 
*Parameter: const std::string & str_port -[in] 服务所监听端口  
*Parameter: std::size_t nThreads_num -[in]  从端口读写数据的线程数 
*Parameter: std::string & str_err_reason -[in/out]  
************************************/
int CSingleton_Server::start(const std::string& str_ip, const std::string& str_port, std::size_t nThreads_num, std::string& str_err_reason)
{
	try
	{
		// Initialise the server.
		m_server_ptr.reset(new http::server3::server(str_ip, str_port, "./doc", nThreads_num));
		int ret = m_server_ptr->init(str_err_reason);
		businlog_error_return(!ret, ("%s | fail to init server, ip:%s, port:%s, thread num:%d, reason:%s"
			, __CLASS_FUNCTION__, str_ip.c_str(), str_port.c_str(), nThreads_num, str_err_reason.c_str()), ret);
		//创建一个线程
		m_thread_server = boost::thread(boost::bind(&http::server3::server::run, m_server_ptr));
		//不关心退出状态，则可以线程分离
		//	m_thread_server.detach();
		businlog_crit("%s | Start Server successfully, ip:%s, port:%s", __CLASS_FUNCTION__, str_ip.c_str(), str_port.c_str());
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason = "Has exception:" + std::string(e.what());
		businlog_error("%s | has exception:%s", __CLASS_FUNCTION__, e.what());
		return MSP_ERROR_EXCEPTION;
	}
}

void CSingleton_Server::stop()
{
	if (m_thread_server.joinable())
	{

		m_thread_server.interrupt();
	}
}

const std::string CSingleton_Server::get_ip() const
{
	if (m_server_ptr)
	{
		return m_server_ptr->get_ip();
	}
	else
	{
		return std::string();
	}
}

const std::string CSingleton_Server::get_port() const
{
	if (m_server_ptr)
	{
		return m_server_ptr->get_port();
	}
	else
	{
		return std::string();
	}
}
