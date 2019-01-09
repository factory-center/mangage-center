/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: CDevice.h
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
#include <vector>
#include <string>
#include <json/json.h>
using std::string;
using std::vector;
//宏定义

enum EDevice_Type
{
	ECARVE = 0, //雕刻机
	ECLOULD = 1, //公有云
	EHOST = 2, //本地主机
};

//类型定义
class CDevice
{
public:
	CDevice(EDevice_Type eType, const Json::Value& json_params);
	const string& get_ip() const;
	const string& get_id();
	virtual ~CDevice();
protected:
	string m_str_ip; //设备的ip地址列表，可能存在多网卡
	std::string  m_str_id; //设备编号，全局唯一
	EDevice_Type m_eType; //设备类型
};
//函数原型定义
