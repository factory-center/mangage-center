/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: singleton_server.h
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
#pragma once

#ifdef __cplusplus  
extern "C" {  
//包含C语言接口、定义或头文件
#endif  
#ifdef __cplusplus  
}  
#endif  
//引用C++头文件：先是标准库头文件，后是项目头文件
#include <string>
#include <boost/thread.hpp>
#include "server.hpp"
//宏定义

//类型定义
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
//函数原型定义
