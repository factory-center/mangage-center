/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: CCarve.h
* @brief: 简短说明文件功能、用途 (Comment)。
* @author:	minglu2
* @version: 1.0
* @date: 2018/12/14
* 
* @see
* 
* <b>版本记录：</b><br>
* <table>
*  <tr> <th>版本	<th>日期		<th>作者	<th>备注 </tr>
*  <tr> <td>1.0	    <td>2018/12/14	<td>minglu	<td>Create head file </tr>
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
#include "CDevice.h"
#include "boost_common.h"
#include "carve_common_lib.h"
//宏定义

//类型定义
class  CCarve : public CDevice
{
public:
	CCarve(const Json::Value& json_params);
	int connect(string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int disconnect(string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int set_continue_status(unsigned char nStatus, unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int reset(unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int start(const string& str_nc_file_path, unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int pause(unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int upload_1_file(const string& str_file_path, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int get_carve_status(ECARVE_STATUS_TYPE& eCarve_common_status, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int stop_fast(unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int cancel_fast_stop(unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int delete_1_file(const string& str_file_path, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int get_current_line_num(int& nCurrent_line_num, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int acquire_resource(string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int release_resource(string& str_err_reason_for_debug, string& str_err_reason_for_user);
	unsigned short Conn_idx() const { return m_nConn_idx; }
	void Conn_idx(unsigned short val) { m_nConn_idx = val; }
	static const string ms_str_factory_type_key; 
	static const string ms_str_carve_type_key;
	static const string ms_str_conn_idx_key;
	static const string ms_str_ip_key;
	static const string ms_str_file_path_key;
	static const string ms_str_status_key;
	static const string ms_str_max_wait_time_key;
	static const string ms_str_carve_id_key;
	virtual ~CCarve();
protected:
	CCarve();
	CCarve(const CCarve&);
	CCarve& operator=(const CCarve&);
private:
	int m_nConn_idx;//一个雕刻机对应一个连接编号，唯一标识一个控制器，此值必须小于ConnectNum，取值范围[0, ConnectNum-1]
    Uni_Mutex m_mutex_for_cmd; //同一时刻只能执行一个命令
	bool m_bConnected; //雕刻机当前是否连接正常。由于每次只能执行一个命令，故无需给其增加锁
	bool m_bAcq_Res_Success; //申请资源是否成功
	ECARVE_FACTORY_TYPE m_eFactory_type; //雕刻机厂商类型
	string m_str_carve_type; //设备型号
};
//函数原型定义
