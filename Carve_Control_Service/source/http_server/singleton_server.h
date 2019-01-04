/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: singleton_server.h
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
#pragma once

#ifdef __cplusplus  
extern "C" {  
//����C���Խӿڡ������ͷ�ļ�
#endif  
#ifdef __cplusplus  
}  
#endif  
//����C++ͷ�ļ������Ǳ�׼��ͷ�ļ���������Ŀͷ�ļ�
#include <string>
#include <boost/thread.hpp>
#include "server.hpp"
//�궨��

//���Ͷ���
class CSingleton_Server
{
public:
	CSingleton_Server()
	{

	}
	void do_something()
	{

	};
	~CSingleton_Server()
	{

	}
	int start(const std::string& str_ip, const std::string& str_port, std::size_t nThreads_num, std::string& str_err_reason);
	void stop()
	{
		if (m_thread_server.joinable())
		{

			m_thread_server.interrupt();
		}
	}
protected:
private:
	boost::thread m_thread_server;
	boost::shared_ptr<http::server3::server> m_server_ptr;
};
//����ԭ�Ͷ���
