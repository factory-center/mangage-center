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
#include <json/json.h>
using std::string;
//�궨��
extern const int MAX_CONNECTIONS_NUM;
//���Ͷ���
//��Ԫ������
class CBaoyuan_Lib 
{
public:
	static CBaoyuan_Lib* instance();
	//�����
	//��ʼ��
	bool init(int nMakerID, const string& str_key, unsigned int nConnectNum, unsigned int  MemSizeR = 100000);
	//���ʼ��
	void fini();
	//��̻����ָ��
	bool create_connection(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool disconnect(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	//TODO::��������������Ҫ������Ϊis_connected
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
	//NC�ļ����
	bool upload_1file(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool delete_1file(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool parse_carve_status_to_description(const int nCarve_status, string& str_carve_status_description, string& str_err_reason_for_debug, string& str_err_reason_for_user);
    //�������
	bool acquire_conn_idx(int& nConn_idx, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	bool release_conn_idx(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user);
private:
	CBaoyuan_Lib();
	//�Ƿ���ռ�е���Դ����������Ϊ˽�У�ʹ����ֻ�ܱ����Լ������������ͷ�
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
	//��ʱ�����
	//������ʱ��
	bool start_timer(string& str_kernel_err_reason);
	bool stop_timer();
	void svc();
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
	boost::thread m_thread_timer; //��ʱ���̶߳���
	bool m_bStop; //��ʱ���߳��Ƿ���ֹ
	std::vector<bool> m_vec_conn_idx_used; //�����豸����/�Ͽ���m_arr_conn_idx_used[i]Ϊfalse����ʾ�������iû�б�ռ�ã������ʾ����i��ռ��
    Uni_Mutex m_mutex_conn_idx;
};
//����ԭ�Ͷ���
