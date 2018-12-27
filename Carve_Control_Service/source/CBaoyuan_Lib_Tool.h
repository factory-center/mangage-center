/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: CBaoyuan_Lib_Tool.h
* @brief: 简短说明文件功能、用途 (Comment)。
* @author:	minglu2
* @version: 1.0
* @date: 2018/12/17
* 
* @see
* 
* <b>版本记录：</b><br>
* <table>
*  <tr> <th>版本	<th>日期		<th>作者	<th>备注 </tr>
*  <tr> <td>1.0	    <td>2018/12/17	<td>minglu	<td>Create head file </tr>
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
#include "boost_common.h"
#include "baoyuan/scif2_define.h"
#include <string>
#include "baoyuan/scif2.h"
#include <json/json.h>
//宏定义
using std::string;
//类型定义
//宝元函数库
class CBaoyuan_Lib 
{
public:
	static CBaoyuan_Lib* instance();
	//库相关
	//初始化
	bool init(int nMakerID, const string& str_key, unsigned int nConnectNum, unsigned int  MemSizeR = 100000);
	//逆初始化
	void fini();
	//雕刻机相关指令
	bool create_connection(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool disconnect(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool get_connect_status(const Json::Value& json_conn_value, int& nStatus, string& str_kernel_err_reason);

	bool confirm_task(unsigned short nConn_idx, size_t nMax_wait_time, string& str_kernel_err_reason);
	bool set_continue_status(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool reset_carve(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool pause(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool start(const Json::Value& json_conn_value, string& str_kernel_err_reason);
    //NC文件相关
	bool upload_1file(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	
private:
	CBaoyuan_Lib();
	//是否其占有的资源。析构函数为私有，使得其只能被其自己的垃圾工程释放
	~CBaoyuan_Lib(); 
	CBaoyuan_Lib& operator=(const CBaoyuan_Lib&);
	CBaoyuan_Lib(const CBaoyuan_Lib&);
	bool set_RBit(unsigned short nConn_idx, unsigned int nAddr, unsigned char nBitIdx, unsigned char nBitValue, unsigned short nMax_wait_time, string& str_err_reason);
	bool set_RValue(unsigned short nConn_idx, unsigned int  nAddr, unsigned int nVal, unsigned short nMax_wait_time, string& str_kernel_err_reason);
	bool set_RString(unsigned short nConn_idx, size_t nAddr,  size_t nBuff_size, const char* pBuff, unsigned short nMax_wait_time, string& str_kernel_err_reason);
	bool set_CValue(unsigned short nConn_idx, int nAddr, int nValue, unsigned short nMax_wait_time, string& str_kernel_err_reason);
	bool is_valid_conn_idx(unsigned short nConn_idx, string& str_kernel_err_reason);
	string strerror_ftp(int nResult_ftp);
	//定时器相关
	//启动定时器
	bool start_timer(string& str_kernel_err_reason);
	bool stop_timer();
	void svc();
	class CGarbo // 它的唯一工作就是在析构函数中删除CSingleton的实例 
	{
	public:
		~CGarbo();
	};
	// 声明一个静态成员，在程序结束时，系统会调用它的析构函数，注意这里仅仅是声明，还需要在相应的cpp文件中对静态成员进行定义哦。
	static CGarbo Garbo;

	bool m_bAvailable; //函数库是否可用
	Thread_WR_Mutex m_rw_mutex_for_available; //函数库是否可用对应的读写锁
	DLL_USE_SETTING m_sDLL_setting; //动态库对应的配置
    static CBaoyuan_Lib* ms_pInstance;
	SC2 m_sc2_obj; //宝元库类对象，需要在编译器的预处理命令中定义：__CLASS
	boost::thread m_thread_timer; //定时器线程对象
	bool m_bStop; //定时器线程是否终止
};
//函数原型定义
