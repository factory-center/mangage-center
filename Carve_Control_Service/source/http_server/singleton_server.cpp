/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: singleton_server.cpp
* @brief: ���˵���ļ����ܡ���; (Comment)��
* @author:	minglu2
* @version: 1.0
* @date: 2019/01/03
* 
* @see
* 
* <b>�汾��¼��</b><br>
* <table>
*  <tr> <th>�汾	<th>����		<th>����	<th>��ע </tr>
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
* Brief:  �����������������ص�ַ�Ͷ˿�
* Access:    public 
* Returns:   int 0:success; other:error code
* Qualifier:
*Parameter: const std::string & str_ip -[in] ���ص�ַ 
*Parameter: const std::string & str_port -[in] �����������˿�  
*Parameter: std::size_t nThreads_num -[in]  �Ӷ˿ڶ�д���ݵ��߳��� 
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
		//����һ���߳�
		m_thread_server = boost::thread(boost::bind(&http::server3::server::run, m_server_ptr));
		//�������˳�״̬��������̷߳���
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
