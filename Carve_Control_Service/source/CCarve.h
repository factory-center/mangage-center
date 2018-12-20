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
//宏定义

//类型定义
class  CCarve : public CDevice
{
public:
	CCarve(unsigned short nConn_idx, const string& str_ip);
	int connect(string& str_kernel_err_reason);
	int disconnect(string& str_kernel_err_reason);
	unsigned short Conn_idx() const { return m_nConn_idx; }
	void Conn_idx(unsigned short val) { m_nConn_idx = val; }
protected:
	CCarve();
	//判定连线序号是否合法
	bool is_valid(short nServerIdx)
	{
		//todo
		return true;
	}
private:
	unsigned short m_nConn_idx;//一个雕刻机对应一个连接编号，唯一标识一个控制器，此值必须小于ConnectNum，取值范围[0, ConnectNum-1]
    
};
//函数原型定义
