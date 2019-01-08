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
using std::string;
//宏定义
extern const int MAX_CONNECTIONS_NUM;
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
	bool create_connection(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool disconnect(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	//TODO::下面两个函数需要重命名为is_connected
	bool get_connect_status(const Json::Value& json_conn_value, int& nStatus, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool get_connect_status(int nConn_idx, int& nStatus, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool confirm_task(unsigned short nConn_idx, size_t nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool set_continue_status(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool reset_carve(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool pause(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool start(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool get_carve_status(const Json::Value& json_conn_value, int& nCarve_status, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool stop_fast(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool cancel_fast_stop(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool is_connected(int nConn_idx, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool is_connected(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	//NC文件相关
	bool upload_1file(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool delete_1file(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool parse_carve_status_to_description(const int nCarve_status, string& str_carve_status_description, string& str_err_reason_for_debug, string& str_err_reason_for_user);
    //索引相关
	bool acquire_conn_idx(int& nConn_idx, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool release_conn_idx(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
private:
	CBaoyuan_Lib();
	//是否其占有的资源。析构函数为私有，使得其只能被其自己的垃圾工程释放
	~CBaoyuan_Lib(); 
	CBaoyuan_Lib& operator=(const CBaoyuan_Lib&);
	CBaoyuan_Lib(const CBaoyuan_Lib&);
	bool set_RBit(int nConn_idx, unsigned int nAddr, unsigned char nBitIdx, unsigned char nBitValue, unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool set_RValue(int nConn_idx, unsigned int  nAddr, unsigned int nVal, unsigned short nMax_wait_time,  string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool set_RString(int nConn_idx, size_t nAddr,  size_t nBuff_size, const char* pBuff, unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool set_CValue(int nConn_idx, int nAddr, int nValue, unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool get_RValue(int Conn_idx, int nAddr, int& nValue, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool get_RBit(int nConn_idx, int nAddr, int nBitIdx, int& nValue, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool is_valid_conn_idx(int nConn_idx, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool is_valid_addr(int nAddr, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool is_valid_bit_idx(int nBitIdx, string& str_err_reason_for_debug, string& str_err_reason_for_user);
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
	std::vector<bool> m_vec_conn_idx_used; //用于设备连接/断开。m_arr_conn_idx_used[i]为false，表示索引编号i没有被占用；否则表示索引i被占用
    Uni_Mutex m_mutex_conn_idx;
};
//函数原型定义
