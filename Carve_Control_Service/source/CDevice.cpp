/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: CDevice.cpp
* @brief: 简短说明文件功能、用途 (Comment)。
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
#include "CDevice.h"
#include "CCarve.h"
#include "busin_log.h"


CDevice::CDevice(EDevice_Type eType, const Json::Value& json_params) 
	: m_eType(eType)
{
	std::string str_err_reason;
	if (!json_params.isMember(CCarve::ms_str_ip_key))
	{
		str_err_reason = string("json:") + json_params.toStyledString() + string(" without key:") + CCarve::ms_str_ip_key;
		businlog_error("%s | err reason:%s", __FUNCTION__, str_err_reason.c_str());
		throw std::exception(str_err_reason.c_str());
	}
	if (!json_params.isMember(CCarve::ms_str_carve_id_key))
	{
		str_err_reason = string("json:") + json_params.toStyledString() + string(" without key:") + CCarve::ms_str_carve_id_key;
		businlog_error("%s | err reason:%s", __FUNCTION__, str_err_reason.c_str());
		throw std::exception(str_err_reason.c_str());
	}

	m_str_ip = json_params[CCarve::ms_str_ip_key].asString();
	m_str_id = json_params[CCarve::ms_str_carve_id_key].asString();
}

const string& CDevice::get_ip() const
{
	return m_str_ip;
}

const string& CDevice::get_id()
{
	return m_str_id;
}

CDevice::~CDevice()
{

}
