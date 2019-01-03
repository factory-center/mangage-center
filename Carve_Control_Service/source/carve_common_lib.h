/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: carve_common_lib.h
* @brief: 雕刻机设备底层通用库
* @author:	minglu2
* @version: 1.0
* @date: 2018/12/26
* 
* @see
* 
* <b>版本记录：</b><br>
* <table>
*  <tr> <th>版本	<th>日期		<th>作者	<th>备注 </tr>
*  <tr> <td>1.0	    <td>2018/12/26	<td>minglu	<td>Create head file </tr>
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
#include <json/json.h>
using std::string;
//宏定义
enum ECARVE_FACTORY_TYPE
{
	CARVE_FACTORY_TYPE_MIN = 0,
	CARVE_FACTORY_TYPE_BAOYUAN, //宝元千系
	CARVE_FACTORY_TYPE_MAX,
};

enum ECARVE_STATUS_TYPE
{
	CARVE_STATUS_MIN = 0, //最小非法值
	CARVE_STATUS_OFFLINE, //雕刻机离线
	CARVE_STATUS_ONLINE, //雕刻机在线
	CARVE_STATUS_NOT_READY, //雕刻机未就绪
	CARVE_STATUS_READY, //雕刻机就绪
	CARVE_STATUS_ENGRAVING, //正在雕刻中
	CARVE_STATUS_PAUSE, //雕刻机暂停
	CARVE_STATUS_COMPLETED, //雕刻完成
	CARVE_STATUS_ERR, //雕刻机故障中
	CARVE_STATUS_MAX //最大非法值
};

//类型定义
//雕刻机底层通用库，其根据设备信息决定具体使用哪个函数库
class CCarve_Common_Lib_Tool
{
public:
	static CCarve_Common_Lib_Tool* instance()
	{
		return ms_pInstance;
	}
	bool init(string& str_kernel_err_reason);
	bool is_baoyuan(int nfactory_type, const string& str_carve_type_key);
	bool connect(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool disconnect(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool upload_1file(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool set_continue_status(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool reset_carve(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool pause(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool start(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool get_carve_status(const Json::Value& json_conn_value, ECARVE_STATUS_TYPE& eCommon_carver_status, string& str_kernel_err_reason);
    bool get_carve_status_description(const ECARVE_STATUS_TYPE& eCommon_carver_status, string& str_carve_status_description, string& str_kernel_err_reason);
    bool stop_fast(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool cancel_fast_stop(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool delete_1file(const Json::Value& json_conn_value, string& str_kernel_err_reason);
protected:
	bool get_carve_factory_and_type(const Json::Value& json_conn_value, int& nfactory_type, string& str_str_carve_type_key, string& str_kernel_err_reason);
	CCarve_Common_Lib_Tool();
	~CCarve_Common_Lib_Tool();
	
	bool parse_baoyuan_carve_status_to_common(int nBaoyuan_carve_status, ECARVE_STATUS_TYPE& eCarve_common_status, string& str_kernel_err_reason);
private:
	CCarve_Common_Lib_Tool& operator=(const CCarve_Common_Lib_Tool&);
	CCarve_Common_Lib_Tool(const CCarve_Common_Lib_Tool&);
	class CGarbo // 它的唯一工作就是在析构函数中删除CSingleton的实例 
	{
	public:
		~CGarbo();
	};
	// 声明一个静态成员，在程序结束时，系统会调用它的析构函数，注意这里仅仅是声明，还需要在相应的cpp文件中对静态成员进行定义哦。
	static CGarbo Garbo;
	static CCarve_Common_Lib_Tool* ms_pInstance;
};
//函数原型定义
