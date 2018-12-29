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
#include "boost_common.h"
#include "carve_common_lib.h"
//�궨��

//���Ͷ���
class  CCarve : public CDevice
{
public:
	CCarve(unsigned short nConn_idx, const string& str_ip);
	int connect(string& str_kernel_err_reason);
	int disconnect(string& str_kernel_err_reason);
	int set_continue_status(unsigned char nStatus, unsigned short nMax_wait_time, string& str_kernel_err_reason);
	int reset(unsigned short nMax_wait_time, string& str_kernel_err_reason);
	int start(const string& str_nc_file_path, unsigned short nMax_wait_time, string& str_kernel_err_reason);
	int pause(unsigned short nMax_wait_time, string& str_kernel_err_reason);
	int upload_1_file(const string& str_file_path, string& str_kernel_err_reason);
	int get_carve_status(ECARVE_STATUS_TYPE& eCarve_common_status, string& str_kernel_err_reason);
	int stop_fast(unsigned short nMax_wait_time, string& str_kernel_err_reason);
	int delete_1_file(const string& str_file_path, string& str_kernel_err_reason);
	unsigned short Conn_idx() const { return m_nConn_idx; }
	void Conn_idx(unsigned short val) { m_nConn_idx = val; }
	static const string ms_str_factory_type_key;
	static const string ms_str_carve_type_key;
	static const string ms_str_conn_idx_key;
	static const string ms_str_ip_key;
	static const string ms_str_file_path_key;
	static const string ms_str_status_key;
	static const string ms_str_max_wait_time_key;
protected:
	CCarve();
private:
	int m_nConn_idx;//һ����̻���Ӧһ�����ӱ�ţ�Ψһ��ʶһ������������ֵ����С��ConnectNum��ȡֵ��Χ[0, ConnectNum-1]
    Uni_Mutex m_mutex_for_cmd; //ͬһʱ��ֻ��ִ��һ������
	bool m_bConnected; //��̻���ǰ�Ƿ���������������ÿ��ֻ��ִ��һ��������������������
	ECARVE_FACTORY_TYPE m_eFactory_type; //��̻���������
	string m_str_carve_type; //�豸�ͺ�
};
//����ԭ�Ͷ���
