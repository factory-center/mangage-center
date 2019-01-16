/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: carve_manger.h
* @brief: 雕刻机管理类，用于管理所有雕刻机
* @author:	minglu2
* @version: 1.0
* @date: 2019/01/07
* 
* @see
* 
* <b>版本记录：</b><br>
* <table>
*  <tr> <th>版本	<th>日期		<th>作者	<th>备注 </tr>
*  <tr> <td>1.0	    <td>2019/01/07	<td>minglu	<td>Create head file </tr>
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
#include <boost_common.h>
#include <string>
#include <json/json.h>
#include "carve_common_lib.h"
#include "CCarve.h"
using std::string;
//宏定义
//类型定义
class CCarve_Manager
{
public:
	static CCarve_Manager* instance();
	int connect_carve(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int disconnect_carve(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int get_carve_status(const Json::Value& json_params, ECARVE_STATUS_TYPE& eCarve_common_status, string& str_err_reason_for_debug, string& str_err_reason_for_user);
    int get_carve_info(const Json::Value& json_params, SCarve_Info& carve_info, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int get_all_carves_info(const Json::Value& json_params, Json::Value& json_result
		, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int start_poll_carve_status(string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int stop_poll_carve_status(string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int start_engraving(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
    int upload_1_file(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int emergency_stop_one(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int emergency_stop_all(const Json::Value& json_params,Json::Value& json_result,string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int adjust_speed(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	
protected:
	CCarve_Manager();
	~CCarve_Manager();
	void svc();
private:
	CCarve_Manager& operator=(const CCarve_Manager&);
	CCarve_Manager(const CCarve_Manager&);
	class CGarbo // 它的唯一工作就是在析构函数中删除CSingleton的实例 
	{
	public:
		~CGarbo();
	};
	typedef std::map<string, boost::shared_ptr<CCarve>>::iterator TYPE_MAP_ITER;
	// 声明一个静态成员，在程序结束时，系统会调用它的析构函数，注意这里仅仅是声明，还需要在相应的cpp文件中对静态成员进行定义哦。
	static CGarbo Garbo;
	static CCarve_Manager* ms_pInstance;
	std::map<string, boost::shared_ptr<CCarve>> m_map_carveId_carvePtr; //键为设备编号，值为CCarve的智能指针
	Thread_WR_Mutex m_rw_carveId_carvePtr;
	boost::thread m_thread_poll_status;
	bool m_bStop_poll_status; //状态轮询开关
};
//函数原型定义
