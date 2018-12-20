/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: CBaoyuan_Lib_Tool.h
* @brief: ���˵���ļ����ܡ���; (Comment)��
* @author:	minglu2
* @version: 1.0
* @date: 2018/12/17
* 
* @see
* 
* <b>�汾��¼��</b><br>
* <table>
*  <tr> <th>�汾	<th>����		<th>����	<th>��ע </tr>
*  <tr> <td>1.0	    <td>2018/12/17	<td>minglu	<td>Create head file </tr>
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
#include "boost_common.h"
#include "baoyuan/scif2_define.h"
#include <string>
#include "baoyuan/scif2.h"
//�궨��
using std::string;
//���Ͷ���

class CBaoyuan_Lib 
{
public:
	static CBaoyuan_Lib* instance();
	int test();
	//��ʼ��
	bool init(int nMakerID, const string& str_key, unsigned int nConnectNum, unsigned int  MemSizeR = 100000);
	//���ʼ��
	void fini();
	bool create_connection(unsigned short nConn_idx, const string& str_carve_ip, string& str_kernel_err_reason);
	bool disconnect(unsigned short nConn_idx, string& str_kernel_err_reason);
	bool get_status(unsigned short nConn_idx, int& nStatus, string& str_kernel_err_reason);

	bool confirm_task(unsigned short nServer_idx, size_t nMax_wait_time, string& str_kernel_err_reason);

private:
	CBaoyuan_Lib();
	//��������Ϊ˽�У�ʹ����ֻ�ܱ����Լ������������ͷ�
	~CBaoyuan_Lib(); 
	CBaoyuan_Lib& operator=(const CBaoyuan_Lib&);
	CBaoyuan_Lib(const CBaoyuan_Lib&);
	bool set_RBit(unsigned short nConn_idx, unsigned int nAddr, unsigned char nBitIdx, unsigned char nBitValue, unsigned short nMax_wait_time, string& str_err_reason);
	bool set_RValue(unsigned short nConn_idx, unsigned int  nAddr, unsigned int nVal, unsigned short nMax_wait_time, string& str_kernel_err_reason);
	bool set_RString(unsigned short nConn_idx, size_t nAddr,  size_t nBuff_size, const char* pBuff, unsigned short nMax_wait_time, string& str_kernel_err_reason);
	bool set_CValue(unsigned short nConn_idx, int nAddr, int nValue, unsigned short nMax_wait_time, string& str_kernel_err_reason);
	bool is_valid_conn_idx(unsigned short nConn_idx, string& str_kernel_err_reason);
	class CGarbo // ����Ψһ��������������������ɾ��CSingleton��ʵ�� 
	{
	public:
		~CGarbo();
	};
	// ����һ����̬��Ա���ڳ������ʱ��ϵͳ�������������������ע���������������������Ҫ����Ӧ��cpp�ļ��жԾ�̬��Ա���ж���Ŷ��
	static CGarbo Garbo;

	bool m_bAvailable; //�������Ƿ����
	Thread_WR_Mutex m_rw_mutex_for_available; //�������Ƿ���ö�Ӧ�Ķ�д��
	DLL_USE_SETTING m_sDLL_setting; //��̬���Ӧ������
    static CBaoyuan_Lib* ms_pInstance;
	SC2 m_sc2_obj; //��Ԫ���������Ҫ�ڱ�������Ԥ���������ж��壺__CLASS
};
//����ԭ�Ͷ���