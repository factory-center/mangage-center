/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: CCarve.h
* @brief: ���˵���ļ����ܡ���; (Comment)��
* @author:	minglu2
* @version: 1.0
* @date: 2018/12/14
* 
* @see
* 
* <b>�汾��¼��</b><br>
* <table>
*  <tr> <th>�汾	<th>����		<th>����	<th>��ע </tr>
*  <tr> <td>1.0	    <td>2018/12/14	<td>minglu	<td>Create head file </tr>
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
#include "CDevice.h"
//�궨��

//���Ͷ���
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
	//�ж���������Ƿ�Ϸ�
	bool is_valid(short nServerIdx)
	{
		//todo
		return true;
	}
private:
	unsigned short m_nConn_idx;//һ����̻���Ӧһ�����ӱ�ţ�Ψһ��ʶһ������������ֵ����С��ConnectNum��ȡֵ��Χ[0, ConnectNum-1]
    
};
//����ԭ�Ͷ���
