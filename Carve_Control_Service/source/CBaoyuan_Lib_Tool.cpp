/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: CBaoyuan_Lib_Tool.cpp
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
#include "CBaoyuan_Lib_Tool.h"
#include "baoyuan/scif2.h"
#include "utils/msp_errors.h"
#include "../source/CSpdLog.h"
#include "boost/filesystem.hpp"
#include <json/json.h>
#include "CCarve.h"
#ifdef _WINDOWS
#define __CLASS_FUNCTION__ ((std::string(__FUNCTION__)).c_str()) 
#else
#define __CLASS_FUNCTION__ ((std::string("CBaoyuan_Lib::") + std::string(__FUNCTION__)).c_str()) 
#endif

const int MAX_CONNECTIONS_NUM = 255;
const int MIN_CONNECTIONS_NUM = 0;
const int ADDR_CARVE_COMPLETED = 500; //��̽���״̬��ַ��A500
//�ۼƼӹ�ʱ�����
const int ADDR_R_TOTAL_TIME_1th_MS = 82120; //��һ·���ۼƼӹ�ʱ�����ֵ��Ӧ�ĵ�ַ
const int ADDR_R_TOTAL_TIME_2th_MS = ADDR_R_TOTAL_TIME_1th_MS + 1; //�ڶ�·���ۼƼӹ�ʱ�����ֵ��Ӧ��ַ
const int ADDR_R_TOTAL_TIME_1th_HOUR = 82126;//��һ·���ۼƼӹ�ʱ��Сʱֵ��Ӧ��ַ�� R82126
const int ADDR_R_TOTAL_TIME_2th_HOUR = ADDR_R_TOTAL_TIME_1th_HOUR + 1;//�ڶ����ۼ�ʱ��Сʱֵ��Ӧ��ַ
//���μӹ�ʱ��
const int ADDR_R_SINGLE_ENGRAVE_TIME_1th_MS = 82108; //��һ·�����μӹ�ʱ�����ֵ��Ӧ�ĵ�ַ
const int ADDR_R_SINGLE_ENGRAVE_TIME_1th_HOUR = 82114; //��һ·�����μӹ�ʱ��Сʱ��Ӧ�ĵ�ַ
CBaoyuan_Lib* CBaoyuan_Lib::instance()
{
	return ms_pInstance;
}

/************************************
* Method:    init
* Brief:  ����˵��
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: int nMakerID -[in/out]  
*Parameter: const string & str_key -[in/out]  
*Parameter: unsigned int nConnectNum -[in/out]  
*Parameter: unsigned int MemSizeR -[in]  ��ֵ�����˲���Rλֵַ�ķ�Χ��С�˿��ܵ��¶�ȡ������������ռ�úܴ��ڴ�
                                    Rֵռ���ڴ棺MemSizeR*4B
************************************/
bool CBaoyuan_Lib::init(int nMakerID, const string& str_key, unsigned int nConnectNum, unsigned int MemSizeR /*= 4000000*/)
{
	LTrace("CBaoyuan_Lib::init");
	//����У��
	if (!(nConnectNum >= MIN_CONNECTIONS_NUM && nConnectNum <= MAX_CONNECTIONS_NUM))
	{
		LError("Invalid ConnectNum : %d, should be[%d, %d]", nConnectNum, MIN_CONNECTIONS_NUM, MAX_CONNECTIONS_NUM);
		return false;
	}

	m_sDLL_setting.ConnectNum = nConnectNum; //�B����Ŀ
	m_sDLL_setting.MemSizeR = MemSizeR; //һ������������Ӧ��ע��������ڴ��С
	int nRet_baoyuan = m_sc2_obj.LibraryInitial(&m_sDLL_setting, nMakerID, (char*)str_key.c_str());
	if (0 == nRet_baoyuan)
	{//ʧ��
		LError("scif_Init failed, ret:{}", nRet_baoyuan);
		return false;
	} 
	if (10 == nRet_baoyuan)
	{//��ʼ���ɹ����ǽ��ܹ����ִ�ʧ��
		LError("fail to decrypt the key string when init scif, ret:{}, MakerID:{}, key:{}", nRet_baoyuan, nMakerID, str_key);
		return false;
	}
	//���ÿ����־�ȼ�:�д������ʾ
	m_sc2_obj.LibrarySetDebug(2); //todo::for test
	string str_err_reason;
	//������ʱ��
	bool bSuccess = start_timer(str_err_reason);
	if (bSuccess == false)
	{
		LError("faild to start timer, reason:{}", str_err_reason);
		return false;
	}
	
	{
		Thread_Write_Lock guard(m_rw_mutex_for_available);
		m_bAvailable = true;
	}
	
	return true;
}

/************************************
* Method:    fini
* Brief:  ������������ڿ���״̬�����轫���ͷ�
* Access:    public 
* Returns:   void
* Qualifier:
************************************/
void CBaoyuan_Lib::fini()
{
	LWarn("{} | Notice", __CLASS_FUNCTION__);
	{
		//��ֹͣ�̺߳��ͷź�����
		if (false == m_bStop)
		{//�߳���Ȼ��ʼ��
			stop_timer();
		}

		Thread_Write_Lock guard(m_rw_mutex_for_available);
		//�ж��������״̬
		if (true == m_bAvailable)
		{//���������ڿ���״̬�����轫������
			m_sc2_obj.LibraryDestroy();
			m_bAvailable = false;
			LCritical("release baoyuan lib successfully!");
		}
	}
}

/************************************
* Method:    create_connection
* Brief:  ����ĳ����̻�
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: const Json::Value & json_conn_value -[in]  ���м������Ӻź�ip��ַ
*Parameter: string & str_kernel_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::create_connection(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{

	LTrace("CBaoyuan_Lib::create_connection");
	//�ж������Ϸ���
	if (json_conn_value.isMember(CCarve::ms_str_conn_idx_key) == false)
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ",without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "�����в����м�:" + CCarve::ms_str_conn_idx_key;
		return false;
	}
	if (json_conn_value.isMember(CCarve::ms_str_ip_key) == false)
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ",without key:" + CCarve::ms_str_ip_key;
		str_err_reason_for_user = "�����в����м�:" + CCarve::ms_str_ip_key;
		return false;
	}


	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	const string str_carve_ip = json_conn_value[CCarve::ms_str_ip_key].asString(); 

	if (!(nConn_idx >= 0 && nConn_idx < (int)m_sDLL_setting.ConnectNum))
	{
		LError("Invalid Connection index:{}, must be [0,{}]", nConn_idx, m_sDLL_setting.ConnectNum - 1);
		str_err_reason_for_debug = "Invalid Connection index:" + std::to_string(nConn_idx) + ",must be [0," + std::to_string(m_sDLL_setting.ConnectNum - 1) + "]";
		str_err_reason_for_user = "���������Ƿ�����������[0," + CCarve::ms_str_ip_key + "]";
		return false;
	}

	
	//ֱ��ݔ�������IP�M���B��
	//���д˺�ʽ�ɹ�ֻ�����B���O���ɹ����Пo�����������B������횺��� GetConnectionMsg ��ʽ��z���B����B
	int nRet_baoyuan = m_sc2_obj.ConnectLocalIP(nConn_idx, (char*)str_carve_ip.c_str());
	if (0 == nRet_baoyuan)
	{
		LError("fail to connect ip:{}", str_carve_ip);
		str_err_reason_for_debug = "fail to connect ip:" + str_carve_ip + ",with connection index:" + std::to_string(nConn_idx);
		str_err_reason_for_user = "����ʧ�ܣ������豸����";
		return false;
	}
	
	//------�O��Ҫ݆ԃ�ă���
	m_sc2_obj.LReadBegin(nConn_idx);
	m_sc2_obj.LReadNR(nConn_idx, 23004, 2);
	m_sc2_obj.LReadEnd(nConn_idx);

	int talktime = 0;
	int nStatus = SC_CONN_STATE_DISCONNECT;
	//��һ��ʱ����ѭ���������״̬�������ʱ�һ�δ���ӳɹ����򱨴��˳�
	while (nStatus != SC_CONN_STATE_OK)
	{
//		m_sc2_obj.MainProcess(); //�Ƿ���Ҫ���Σ�
		nStatus = m_sc2_obj.GetConnectionMsg(nConn_idx, SCIF_CONNECT_STATE);
		if (SC_CONN_STATE_OK == nStatus)
		{//���ӳɹ�
			break;
		}
		//��ʱ����ʧ��
		if (talktime >= 10)
		{//�ظ������ﵽָ������
			LError("Connect time out, ip:{}, conn idx:{}, carve status:{}", str_carve_ip, nConn_idx, nStatus);
			str_err_reason_for_debug = "Connect time out, ip:" + str_carve_ip + ", conn idx:" + std::to_string(nConn_idx) + "carve status :" + std::to_string(nStatus);
			str_err_reason_for_user = "�����豸��ʱ";
			return false;
		}
		else
		{//����ʧ�ܣ�δ�ﵽָ������
			//����һ���
//			Sleep(50);
			boost::this_thread::sleep(boost::posix_time::millisec(50));
			talktime++;
		}
	}
	//��ʱ���ӳɹ�
	//�趨ѭ����������ѭ��ִ��
	m_sc2_obj.LClearQueue(nConn_idx);
	m_sc2_obj.LReadBegin(nConn_idx);
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 17003, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "���õ�̻�����ʧ��";
		return false;
	}
	
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 17034, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "���õ�̻�����ʧ��";
		return false;
	}
	
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 29000, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "���õ�̻�����ʧ��";
		return false;
	}
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 29001, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "���õ�̻�����ʧ��";
		return false;
	}
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 29002, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "���õ�̻�����ʧ��";
		return false;
	}
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 29003, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "���õ�̻�����ʧ��";
		return false;
	}
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 1040000, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "���õ�̻�����ʧ��";
		return false;
	}
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 3006072, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "���õ�̻�����ʧ��";
		return false;
	}
	//����ѭ�������ȡ��һ·���ۼƼӹ�ʱ���Ӧ�ĺ���
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, ADDR_R_TOTAL_TIME_1th_MS, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "���õ�̻�����ʧ��";
		return false;
	}
	//����ѭ�������ȡ��һ·���ۼƼӹ�ʱ���Ӧ��Сʱ
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, ADDR_R_TOTAL_TIME_1th_HOUR, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "���õ�̻�����ʧ��";
		return false;
	}
	//����ѭ�������ȡ����Ƿ����
	nRet_baoyuan = m_sc2_obj.LReadNA(nConn_idx, ADDR_CARVE_COMPLETED, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "���õ�̻�����ʧ��";
		return false;
	}
	//����ѭ�������ȡ���μӹ�ʱ��
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, ADDR_R_SINGLE_ENGRAVE_TIME_1th_MS, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "���õ�̻�����ʧ��";
		return false;
	}
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, ADDR_R_SINGLE_ENGRAVE_TIME_1th_HOUR, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "���õ�̻�����ʧ��";
		return false;
	}

	m_sc2_obj.LReadEnd(nConn_idx);
	return true;
}


/************************************
* Method:    disconnect
* Brief:  �Ͽ�ĳ����̻�
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: const Json::Value & json_conn_value -[in]  ���м������ߺ�
*Parameter: string & str_kernel_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::disconnect(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//�ж������Ϸ���
	if (!(json_conn_value.isMember(CCarve::ms_str_conn_idx_key)))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "�����в����м�"+ CCarve::ms_str_conn_idx_key;
		return false;
	}

	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	if (is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user) == false)
	{
		LError("Invalid conn index:{}, reason:{}", nConn_idx, str_err_reason_for_debug);
		return false;
	}

	int nRet_baoyuan = m_sc2_obj.Disconnect(nConn_idx);
	if (nRet_baoyuan == 0)
	{
		LError("fail to disconnect, conn index:{}", nConn_idx);
		str_err_reason_for_debug = "fail to disconnect, conn index:" + std::to_string(nConn_idx);
		str_err_reason_for_user = "�Ͽ���̻�ʧ��";
		return false;
	}
	return true;
}

/************************************
* Method:    get_connect_status
* Brief:  ��ȡĳ����̻���״̬
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: const Json::Value & json_conn_value -[in]  ĳ����̻���Ӧ�����ߺ�
*Parameter: int & nStatus -[in/out]  
*Parameter: string & str_kernel_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::get_connect_status(const Json::Value& json_conn_value, int& nStatus, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//�ж������Ϸ���
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}



	//��ȡ��������
	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}
	//��ȡ����״̬
	nStatus = m_sc2_obj.GetConnectionMsg(nConn_idx, SCIF_CONNECT_STATE);
	return true;
}

/************************************
* Method:    get_connect_status
* Brief:  ��ȡĳ����̻�״̬
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: int nConn_idx -[in] ��������������Ϊ-1
*Parameter: int & nStatus -[out]  ����״̬
*Parameter: string & str_err_reason_for_debug -[out]  
*Parameter: string & str_err_reason_for_user -[out]  
************************************/
bool CBaoyuan_Lib::get_connect_status(int nConn_idx, int& nStatus, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//�ж������Ϸ���
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}
	//��ȡ����״̬
	nStatus = m_sc2_obj.GetConnectionMsg(nConn_idx, SCIF_CONNECT_STATE);
	return true;
}

bool CBaoyuan_Lib::confirm_task(unsigned short nConn_idx, size_t nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	size_t nAddr = 20000;
	unsigned char nBit_idx = 10;
	unsigned char nBit_value = 0;
	return set_RBit(nConn_idx, nAddr, nBit_idx, nBit_value, (unsigned short)nMax_wait_time, str_err_reason_for_debug, str_err_reason_for_user);
}

bool CBaoyuan_Lib::set_continue_status(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//�ж������Ϸ���
	//�ж��Ƿ���conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	//�ж��Ƿ���״̬
	if (!json_conn_value.isMember(CCarve::ms_str_status_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_status_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	//�ж��Ƿ������ʱʱ��
	if (!json_conn_value.isMember(CCarve::ms_str_max_wait_time_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_max_wait_time_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_max_wait_time_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	int nStatus = json_conn_value[CCarve::ms_str_status_key].asInt();
	unsigned short nMax_wait_time = json_conn_value[CCarve::ms_str_max_wait_time_key].asInt();
	//		ResetBaoyuanRBit(1040000, 0);
	return set_RBit(nConn_idx, 1040000, 0, nStatus, nMax_wait_time, str_err_reason_for_debug, str_err_reason_for_user);
}

bool CBaoyuan_Lib::reset_carve(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{

	// 		ret=SetBaoyuanRBit(20000, 0);
	// 		Sleep(100);
	// 		ResetBaoyuanRBit(20000, 0);
	//�ж��Ƿ���conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	//�ж��Ƿ������ʱʱ��
	if (!json_conn_value.isMember(CCarve::ms_str_max_wait_time_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_max_wait_time_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_max_wait_time_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	unsigned short nMax_wait_time = json_conn_value[CCarve::ms_str_max_wait_time_key].asInt();

	bool bSuccess = set_RBit(nConn_idx, 20000, 0, 1, nMax_wait_time, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{ 
		LError("fail to set RBit, reason:{}", str_err_reason_for_debug);
		return false;
	}

	boost::this_thread::sleep(boost::posix_time::millisec(100));
	bSuccess = set_RBit(nConn_idx, 20000, 0, 0, nMax_wait_time, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to set RBit, reason:{}", str_err_reason_for_debug);
		return false;
	}
	return true;
}

bool CBaoyuan_Lib::pause(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	// 		SetBaoyuanRBit(20000, 11);
	// 		Sleep(200);
	// 		ResetBaoyuanRBit(20000, 11);

	//�ж��Ƿ���conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}


	//�ж��Ƿ������ʱʱ��
	if (!json_conn_value.isMember(CCarve::ms_str_max_wait_time_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_max_wait_time_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_max_wait_time_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	unsigned short nMax_wait_time = json_conn_value[CCarve::ms_str_max_wait_time_key].asInt();

 	bool bSuccess = set_RBit(nConn_idx, 20000, 11, 1, nMax_wait_time, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to set RBit, reason:{}", str_err_reason_for_debug);
		return false;
	}
 	boost::this_thread::sleep(boost::posix_time::millisec(100));
	bSuccess = set_RBit(nConn_idx, 20000, 11, 0, nMax_wait_time, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to set RBit, reason:{}", str_err_reason_for_debug);
		return false;
	}
	return true;
}

bool CBaoyuan_Lib::start(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	// 		SetBaoyuanRString(17022,filename, 8*4);
	// 		SetBaoyuanRValue(17002, 1);
	// 		SetBaoyuanRBit(20000, 10);
	//�ж��Ƿ���conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	//�ж��Ƿ����ļ�·��
	if (!json_conn_value.isMember(CCarve::ms_str_file_path_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_file_path_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_file_path_key;
		str_err_reason_for_user = "��������";
		return false;
	}


	//�ж��Ƿ������ʱʱ��
	if (!json_conn_value.isMember(CCarve::ms_str_max_wait_time_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_max_wait_time_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_max_wait_time_key;
		str_err_reason_for_user = "��������";
		return false;
	}


	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	const string& str_nc_file_path = json_conn_value[CCarve::ms_str_file_path_key].asString();
	unsigned short nMax_wait_time = json_conn_value[CCarve::ms_str_max_wait_time_key].asInt();

	//��ȡ�ļ�������������չ����
	boost::filesystem::path path_nc_file(str_nc_file_path);
	//�ж��ļ��Ƿ����
	if (!boost::filesystem::exists(path_nc_file))
	{
		LError("Can not find file:{}", str_nc_file_path);
		str_err_reason_for_debug = "Can not find file:" + str_nc_file_path;
		str_err_reason_for_user = "û�ҵ�ָ���ļ�";
		return false;
	}

	string str_filename = path_nc_file.filename().string();
	//���ô�����ļ���
	bool bSuccess = set_RString(nConn_idx, 17022, str_filename.size()
		, str_filename.c_str(), nMax_wait_time * 3, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to set RString, file name:{}, reason:{}", str_filename, str_err_reason_for_debug);
		return false;
	}

	//����ģʽ��0Ϊ�ֶ�ģʽ��1Ϊ�Զ�ģʽ
	int nCarve_status = 0;
	bSuccess = get_RValue(nConn_idx, 17034, nCarve_status, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to set RValue, reason:{}", str_err_reason_for_debug);
		return false;
	}

	if (nCarve_status == 0)
	{
		bSuccess = set_RValue(nConn_idx, 17002, 1, nMax_wait_time * 3, str_err_reason_for_debug, str_err_reason_for_user);
		if (bSuccess == false)
		{
			LError("fail to set RValue, reason:{}", str_err_reason_for_debug);
			return false;
		}
	}

	//�����ӹ�
	bSuccess = set_RBit(nConn_idx, 20000, 10, 1, nMax_wait_time, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to set RBit, reason:{}", str_err_reason_for_debug);
		return false;
	}
	return true;
}

bool CBaoyuan_Lib::get_carve_status(const Json::Value& json_conn_value, int& nCarve_status, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//�ж��Ƿ���conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();

	//nConn_idxֵ���С� scif_Init ��ʽ��ʼ���r��struct DLL_USE_SETTING �� ConnectNum ���O�����B����Ŀ��
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}

	bool bSuccess = get_RValue(nConn_idx, 17003, nCarve_status, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to get R Value, reason:{}", str_err_reason_for_debug);
		return false;
	}
	const int nCarve_ready = 1;
	//��״̬ΪReadyʱ����Ҫ�ٴ��ж���ϸ��Ϊ����̫���ǵ�����̬
	if (nCarve_ready == nCarve_status)
	{
		
		//��ȡA500��ֵ���ж����Ƿ�Ϊ1��Ϊ1����Ϊ������
		int nValue = m_sc2_obj.memA(nConn_idx, ADDR_CARVE_COMPLETED);
		if (1 == nValue)
	    {//Ϊ������̬
			nCarve_status =  5;
			//��ADDR_CARVE_COMPLETED״̬����Ϊ0
			m_sc2_obj.DWrite1A(nConn_idx, ADDR_CARVE_COMPLETED, 0);
	    }
	}
	return true;
}

/************************************
* Method:    stop_fast
* Brief:  ��ͣ
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: const Json::Value & json_conn_value -[in/out]  
*Parameter: string & str_kernel_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::stop_fast(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//�ж��Ƿ���conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	if (!json_conn_value.isMember(CCarve::ms_str_max_wait_time_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_max_wait_time_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_max_wait_time_key;
		str_err_reason_for_user = "��������";
		return false;
	}


	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	unsigned short nMax_wait_time = json_conn_value[CCarve::ms_str_max_wait_time_key].asInt();

	bool bSuccess = set_RBit(nConn_idx, 20000, 31, 1, nMax_wait_time, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to set RBit, reason:{}", str_err_reason_for_debug);
		return false;
	}

	boost::this_thread::sleep(boost::posix_time::millisec(100));
//	bSuccess = set_RBit(nConn_idx, 20000, 31, 0, nMax_wait_time, str_kernel_err_reason);

	return true;
}

bool CBaoyuan_Lib::cancel_fast_stop(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::cancel_fast_stop");
	//�ж��Ƿ���conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}
	if (!json_conn_value.isMember(CCarve::ms_str_max_wait_time_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_max_wait_time_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_max_wait_time_key;
		str_err_reason_for_user = "��������";
		return false;
	}


	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	unsigned short nMax_wait_time = json_conn_value[CCarve::ms_str_max_wait_time_key].asInt();

	bool bSuccess = set_RBit(nConn_idx, 20000, 31, 0, nMax_wait_time, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to set RBit, reason:{}", str_err_reason_for_debug);
		return false;
	}

	boost::this_thread::sleep(boost::posix_time::millisec(100));
	return true;
}

bool CBaoyuan_Lib::is_connected(int nConn_idx, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//�ж������Ϸ���
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}

	int nStatus = m_sc2_obj.GetConnectionMsg(nConn_idx, SCIF_CONNECT_STATE);
	if (!(SC_CONN_STATE_OK == nStatus || SC_CONN_STATE_CONNECTING == nStatus))
	{
		LError("Connect is over, conn idx:{}, status now:{}", nConn_idx, nStatus);
		str_err_reason_for_debug = "Connect is over, conn idx:" + std::to_string(nConn_idx) + ", status now:" + std::to_string(nStatus);
		str_err_reason_for_user = "�豸�����쳣";
		return false;
	}
	return true;
}

bool CBaoyuan_Lib::is_connected(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//�ж������Ϸ���
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError(" err reason:{}", str_err_reason_for_debug);
		return false;
	}

	int nStatus = m_sc2_obj.GetConnectionMsg(nConn_idx, SCIF_CONNECT_STATE);
	if (!(SC_CONN_STATE_OK == nStatus || SC_CONN_STATE_CONNECTING == nStatus))
	{
		LError("Connect is over, conn idx:{}, status now:{}", std::to_string(nConn_idx), std::to_string(nStatus));
		str_err_reason_for_debug = "Connect is over, conn idx:"+ std::to_string(nConn_idx)+ ",status now:" + std::to_string(nStatus);
		str_err_reason_for_user = "�豸�����쳣";
		return false;
	}
	return true;
}

bool CBaoyuan_Lib::get_current_line_num(const Json::Value& json_conn_value, int& nCurrent_line_num, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::get_current_line_num");
	//�ж��Ƿ���conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	//��ȡ��������
	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}

	//��ȡָ��λ�������Ի�ȡ��ǰ�ļ����к�
	return get_RValue(nConn_idx, 3006072, nCurrent_line_num, str_err_reason_for_debug, str_err_reason_for_user);
}

/************************************
* Method:    get_engraving_time
* Brief:  �ӵ�̻��л�ȡ�ܵļӹ�ʱ��͵��μӹ�ʱ��
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: const Json::Value & json_conn_value -[in]  
*Parameter: size_t & nTotal_engraving_time_minute -[out]  
*Parameter: size_t & nSingle_engraving_time_minute -[out]  
*Parameter: string & str_err_reason_for_debug -[out]  
*Parameter: string & str_err_reason_for_user -[out]  
************************************/
bool CBaoyuan_Lib::get_engraving_time(const Json::Value& json_conn_value, size_t& nTotal_engraving_time_minute, size_t& nSingle_engraving_time_minute, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::get_engraving_time");
	//�ж��Ƿ���conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	//��ȡ��������
	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	//�ж������Ϸ���
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}

	//��ȡ�ۼƼӹ���Ӧ�ĺ���
	int nTime_ms = 0;
	bool bSuccess  = get_RValue(nConn_idx, ADDR_R_TOTAL_TIME_1th_MS, nTime_ms, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("ail to get RValue, reason:{}", str_err_reason_for_debug);
		return false;
	}

	//��ȡ�ۼƼӹ�ʱ���Ӧ��Сʱ
	int nTime_hour = 0;
	bSuccess = get_RValue(nConn_idx, ADDR_R_TOTAL_TIME_1th_HOUR, nTime_hour, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("ail to get RValue, reason:{}", str_err_reason_for_debug);
		return false;
	}
	//�������Сʱ����Ϊ����
	nTotal_engraving_time_minute = int(nTime_ms * 1.0 / (1000 * 60) + nTime_hour * 60);
	nTime_ms = 0;
	nTime_hour = 0;
	//��ȡ���μӹ�ʱ���Ӧ�ĺ���
	bSuccess = get_RValue(nConn_idx, ADDR_R_SINGLE_ENGRAVE_TIME_1th_MS, nTime_ms, str_err_reason_for_debug, str_err_reason_for_debug);
	if (bSuccess == false)
	{
		LError("ail to get RValue, reason:{}", str_err_reason_for_debug);
		return false;
	}

	//��ȡ���μӹ�ʱ���Ӧ��Сʱ
	bSuccess = get_RValue(nConn_idx, ADDR_R_SINGLE_ENGRAVE_TIME_1th_HOUR, nTime_hour, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("ail to get RValue, reason:{}", str_err_reason_for_debug);
		return false;
	}

	//��Сʱ������ת��Ϊ����
	nSingle_engraving_time_minute = int(nTime_ms * 1.0 / (1000 * 60) + nTime_hour * 60);
	return true;
}

bool CBaoyuan_Lib::upload_1file(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::upload_1file");
	//�ж������Ϸ���
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}
	if (!json_conn_value.isMember(CCarve::ms_str_file_path_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_file_path_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_file_path_key;
		str_err_reason_for_user = "��������";
		return false;
	}


	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	const string& str_file_path = json_conn_value[CCarve::ms_str_file_path_key].asString();

	//nConn_idxֵ���С� scif_Init ��ʽ��ʼ���r��struct DLL_USE_SETTING �� ConnectNum ���O�����B����Ŀ��
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason::{}", str_err_reason_for_debug);
		return false;
	}


	//�ж�����״̬
	bool bSuccess = is_connected(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("Connection is over, json info:{}, reason:{}", json_conn_value.toStyledString(), str_err_reason_for_debug);
		return false;
	}


	//�����B������ͬһ���n����ݔ���ܣ���춂�ݔǰ�� FtpSetConnection ��ʽ�O�����������B��
	//�O�� FTP ����
	int nRet_baoyuan = m_sc2_obj.FtpSetConnection(nConn_idx);
	if (nRet_baoyuan == 0)
	{
		LError("fail to set ftp connection, conn idx:{}", nConn_idx);
		str_err_reason_for_debug = "fail to set ftp connection, conn idx:" + std::to_string(nConn_idx);
		str_err_reason_for_user = "�����豸ʧ��";
		return false;
	}

	//�ς�һ���n��
	boost::filesystem::path file_boost_path(str_file_path);
	//�ж��ļ��Ƿ����
	if (!boost::filesystem::exists(file_boost_path))
	{
		LError("Can not find file::{}, conn idx:{}", str_file_path, nConn_idx);
		str_err_reason_for_debug = "Can not find file:" + str_file_path + ",conn idx :" + std::to_string(nConn_idx);
		str_err_reason_for_user = "�޷��ҵ�ָ���ļ�";
		return false;
	}

	nRet_baoyuan = m_sc2_obj.FtpUpload1File(FTP_FOLDER_RUN_NCFILES, ""
		, (char*)file_boost_path.filename().string().c_str(), (char*)str_file_path.c_str());

	if (nRet_baoyuan == 0)
	{
		LError("fail to upload file:{}, conn idx:{}", str_file_path, nConn_idx);
		str_err_reason_for_debug = "fail to upload file:" + str_file_path + ",conn idx :" + std::to_string(nConn_idx);
		str_err_reason_for_user = "�ϴ��ļ�ʧ��";
		return false;
	}
	//ȡ�È��нY��  ---  һ�����нY��ֻ���؂�һ��,Ȼ��ͱ����
	size_t nCost_time_ms = 0; //�ķѵ�ʱ��
	size_t nThreshold_time_ms = 2 * 60 * 60 * 1000; //ʱ����ֵ
	size_t nWait_time_ms = 50; //ÿ������ʱ��

	//�ж������ļ�ʱ����û�г�ʱ��������ѭ��
	while (true)
	{
		//����һ��
		boost::this_thread::sleep(boost::posix_time::millisec(nWait_time_ms));
		//���ftp�����Ƿ����
		nRet_baoyuan = m_sc2_obj.FtpCheckDone();
		if ( 1 ==  nRet_baoyuan)
		{//�����Ѿ����
			//��ȡftp�������
			nRet_baoyuan = m_sc2_obj.GetLibraryMsg(SCIF_FTP_RESULT);
			//����Ƿ��ϴ��ɹ�������ϴ�ʧ�ܣ���ֱ�ӱ�����
			if (!(FTP_RESULT_SUCCESS == nRet_baoyuan))
			{
				LError("ftp failed ,file:{}, conn idx:{},ret:{},ftp result note:{}",
					str_file_path, nConn_idx, nRet_baoyuan, strerror_ftp(nRet_baoyuan));
				str_err_reason_for_debug = "ftp failed, file:" + str_file_path + ", conn idx :" + std::to_string(nConn_idx) +
					"ret :" + std::to_string(nRet_baoyuan) + " ftp result note :" + strerror_ftp(nRet_baoyuan);
				str_err_reason_for_user = "ͨ��ftp�ϴ��ļ����豸ʧ��";
				return false;
			}
			//��ʱ�����ϴ��ɹ�
			return true;
		} 
		else
		{//������δ��ɣ����ۼӺķ�ʱ��
			nCost_time_ms += nWait_time_ms;
			//�ж��Ƿ�ʱ
			if (!(nCost_time_ms < nThreshold_time_ms))
			{
				LError("timeout to upload file:{}, cost time:{}ms, conn idx:{}",str_file_path, nCost_time_ms, nConn_idx);
				str_err_reason_for_debug = "timeout to upload file:" + str_file_path + ", cost time :" + 
					std::to_string(nCost_time_ms) + ",conn idx :" + std::to_string(nConn_idx);
				str_err_reason_for_user = "�ϴ��ļ���ʱ";
				return false;
			}
		}
	}
	//��ʱ�Ѿ��ɹ��ϴ����
	return true;
}

bool CBaoyuan_Lib::delete_1file(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::delete_1file");
	//�ж������Ϸ���
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key :" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	if (!json_conn_value.isMember(CCarve::ms_str_file_path_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_file_path_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key :" + CCarve::ms_str_file_path_key;
		str_err_reason_for_user = "��������";
		return false;
	}


	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	const string& str_file_path = json_conn_value[CCarve::ms_str_file_path_key].asString();

	//nConn_idxֵ���С� scif_Init ��ʽ��ʼ���r��struct DLL_USE_SETTING �� ConnectNum ���O�����B����Ŀ��
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:", str_err_reason_for_debug);
		return false;
	}

	//��ѯ����״̬
	bool bSuccess = is_connected(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("Connection is over, json info:{}, reason:{}", json_conn_value.toStyledString(), str_err_reason_for_debug);
		return false;
	}

	//�����B������ͬһ���n����ݔ���ܣ���춂�ݔǰ�� FtpSetConnection ��ʽ�O��������������
	//�O�� FTP ����
	int nRet_baoyuan = m_sc2_obj.FtpSetConnection(nConn_idx);
	if (nRet_baoyuan == 0)
	{
		LError("fail to set ftp connection, conn idx:{}", nConn_idx);
		str_err_reason_for_debug = "fail to set ftp connection, conn idx:" + std::to_string(nConn_idx);
		str_err_reason_for_user = "�����豸ʧ��";
		return false;
	}
	//�ς�һ���n��
	boost::filesystem::path file_boost_path(str_file_path);
	LInfo("conn idx:{}, file name:{}, file path:{}", nConn_idx, file_boost_path.filename().string(), str_file_path);
	
	nRet_baoyuan = m_sc2_obj.FtpDelete1File(FTP_FOLDER_RUN_NCFILES, "", (char*)file_boost_path.filename().string().c_str());
	if (nRet_baoyuan == 0)
	{
		LError("fail to delete file:{}, conn idx:{}", str_file_path, nConn_idx);
		str_err_reason_for_debug = "fail to delete file:" + str_file_path + ", conn idx:" + std::to_string(nConn_idx);
		str_err_reason_for_user = "ɾ���豸�е��ļ�ʧ��";
		return false;
	}

	//ȡ�È��нY��  ---  һ�����нY��ֻ���؂�һ��,Ȼ��ͱ����
	size_t nCost_time_ms = 0; //�ķѵ�ʱ��
	size_t nThreshold_time_ms = 2 * 60 * 60 * 1000; //ʱ����ֵ
	size_t nWait_time_ms = 50; //ÿ������ʱ��

	//�ж������ļ�ʱ����û�г�ʱ��������ѭ��
	while (true)
	{
		//����һ��
		boost::this_thread::sleep(boost::posix_time::millisec(nWait_time_ms));
		//���ftp�����Ƿ����
		nRet_baoyuan = m_sc2_obj.FtpCheckDone();
		if ( 1 ==  nRet_baoyuan)
		{//�����Ѿ����
			//��ȡftp�������
			nRet_baoyuan = m_sc2_obj.GetLibraryMsg(SCIF_FTP_RESULT);
			//����Ƿ�����ɹ����������ʧ�ܣ���ֱ�ӱ�����
			if (nRet_baoyuan == 0)
			{
				LError("ftp failed,conn idx:{},file name:{}, ret:{}, ftp result note:{}", nConn_idx, file_boost_path.filename().string(), nRet_baoyuan, strerror_ftp(nRet_baoyuan));
				str_err_reason_for_debug = "ftp failed,conn idx:" + std::to_string(nConn_idx) + ", file name:" + file_boost_path.filename().string()
					+ ",ret:" + std::to_string(nRet_baoyuan) + ",ftp result note:" + strerror_ftp(nRet_baoyuan);
				str_err_reason_for_user = "ɾ���豸�е��ļ�ʧ��";
				return false;
			}

			//��ʱ���������ɹ�
			return true;
		} 
		else
		{//������δ��ɣ����ۼӺķ�ʱ��
			nCost_time_ms += nWait_time_ms;
			//�ж��Ƿ�ʱ
			if (!(nCost_time_ms < nThreshold_time_ms))
			{
				LError("timeout to delete file:{}, cost time:{}ms, conn idx:{}", file_boost_path.filename().string(), nCost_time_ms, nConn_idx);
				str_err_reason_for_debug = "timeout to delete file:" + file_boost_path.filename().string() + ", cost time:"
					+ std::to_string(nCost_time_ms) + "ms, conn idx:" + std::to_string(nConn_idx);
				str_err_reason_for_user = "ɾ���豸�е��ļ���ʱ";
				return false;
			}
		}
	}
	//��ʱ�Ѿ������ɹ�
	return true;
}

bool CBaoyuan_Lib::adjust_speed(const Json::Value& json_conn_value,string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::adjust_speed");
	//�ж������Ϸ���
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key :" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	const string& str_key = "speed_percent";	
	if (!json_conn_value.isMember(str_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), str_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key :" + str_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();

	unsigned int n_speed_percent = json_conn_value[str_key].asInt();
	unsigned short nMax_wait_time = json_conn_value[CCarve::ms_str_max_wait_time_key].asInt();

	//nConn_idxֵ���С� scif_Init ��ʽ��ʼ���r��struct DLL_USE_SETTING �� ConnectNum ���O�����B����Ŀ��
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}

	//��ѯ����״̬
	bool bSuccess = is_connected(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("Connection is over, json info:{}, reason:{}", json_conn_value.toStyledString(),str_err_reason_for_debug);
		return false;
	}

	//�����ٶ�
	bSuccess = set_RValue(nConn_idx,17000, n_speed_percent*10000, nMax_wait_time * 3, str_err_reason_for_debug, str_err_reason_for_user);
	return bSuccess;
}
	


//��ʽ������
bool CBaoyuan_Lib::program_restart(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//�ж��Ƿ���conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key :" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	//�ж��Ƿ������ʱʱ��
	if (!json_conn_value.isMember(CCarve::ms_str_max_wait_time_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_max_wait_time_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key :" + CCarve::ms_str_max_wait_time_key;
		str_err_reason_for_user = "��������";
		return false;
	}
	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	unsigned short nMax_wait_time = json_conn_value[CCarve::ms_str_max_wait_time_key].asInt();
	//TODO
	LInfo("TODO.");
	return true;
}

bool CBaoyuan_Lib::parse_carve_status_to_description(const int nCarve_status, string& str_carve_status_description, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	str_carve_status_description = "Carve ";
	switch (nCarve_status)
	{
	case 0://δ����
		str_carve_status_description += "Not Ready";
		break;
	case 1: //�Ѿ�����
		str_carve_status_description += "Ready";
		break;
	case 2: //Cycle Start:�����
		str_carve_status_description += "Engraving";
		break;
	case 3: //Feed Hold : �ӹ���ͣ
		str_carve_status_description += "Pause";
		break;
	case 4: //Block Stop:����ֹͣ���\��һ�룬�l���e�`�r�������@����B
		str_carve_status_description += "Error";
		break;
	case 5: //completed�������ɣ��Զ���ģ�
		str_carve_status_description += "Completed";
		break;
	default:
		LError("jnvalid status value:{}, must be [0, 5]", nCarve_status);
		str_err_reason_for_debug = "jnvalid status value:" + std::to_string(nCarve_status) + ", must be [0, 5]";
		str_err_reason_for_user = "��������";
		return false;
	}
	return true;
}

/************************************
* Method:    acquire_conn_idx
* Brief:  ����һ�����п��õ���������
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: int & nConn_idx -[out]  
*Parameter: string & str_err_reason_for_debug -[out]  
*Parameter: string & str_err_reason_for_user -[out]  
************************************/
bool CBaoyuan_Lib::acquire_conn_idx(int& nConn_idx, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	boost::mutex::scoped_lock guard(m_mutex_conn_idx);
	for (int i = 0; i != MAX_CONNECTIONS_NUM; ++i)
	{
		if (false == m_vec_conn_idx_used[i])
		{//�ҵ�һ�����е�����
			nConn_idx = i;
			m_vec_conn_idx_used[i] = true;
			return true;
		}
	}
	//��ʱ����һȦ��û���ҵ����еģ���������豸���࣬�����
	LError("The num of carves is lager than {}", MAX_CONNECTIONS_NUM);
	str_err_reason_for_debug = "The num of carves is lager than " + std::to_string(MAX_CONNECTIONS_NUM);
	str_err_reason_for_user = "������Ԫ��֧�ֵ��豸��Ŀ����Ԫ���֧���豸��ĿΪ:"+ std::to_string(MAX_CONNECTIONS_NUM);
	return false;
}

/************************************
* Method:    release_conn_idx
* Brief:  �黹֮ǰ�������������
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: int nConn_idx -[in/out]  
*Parameter: string & str_kernel_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::release_conn_idx(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//�ж��Ƿ���conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key :" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "��������";
		return false;
	}

	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	//�ж������Ϸ���
	if (!(nConn_idx > -1 && nConn_idx < MAX_CONNECTIONS_NUM))
	{
		LError("Invalid conn idx:{}, should be [0,{}]", nConn_idx, MAX_CONNECTIONS_NUM);
		str_err_reason_for_debug = "Invalid conn idx:" + std::to_string(nConn_idx) + ", should be [0," + std::to_string(MAX_CONNECTIONS_NUM) + "]";
		str_err_reason_for_user = "��������";
		return false;
	}


	boost::mutex::scoped_lock guard(m_mutex_conn_idx);
	//�ж�������ǰ�Ƿ���æµ״̬
	if (!m_vec_conn_idx_used[nConn_idx])
	{
		LError("conn idx:{} is already idle", nConn_idx);
		str_err_reason_for_debug = "conn idx:{}" + std::to_string(nConn_idx) + " is already idle";
		str_err_reason_for_user = "��������";
		return false;
	}
	m_vec_conn_idx_used[nConn_idx] = false;
	return true;
}

bool CBaoyuan_Lib::start_timer(string& str_kernel_err_reason)
{
	m_bStop = false;
	m_thread_timer = boost::thread(boost::bind(&CBaoyuan_Lib::svc, this));
	//�������˳�״̬��������̷߳���
//	m_thread_timer.detach();
	return true;
}

bool CBaoyuan_Lib::stop_timer()
{
	if (true == m_bStop)
	{//֮ǰ�Ѿ�ֹͣ��
		LError("timer has been already stop");
		return false;
	}
	m_bStop = true;
	m_thread_timer.join();
	LTrace("stop timer successfully");
	return true;
	
}

/************************************
* Method:    set_RBit
* Brief:  �ж������Ϸ��ԣ�����Ϸ�����0����1��ΪRֵд��λַnAddr��nBitIdxλԪλַ
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: int nConn_idx -[in]  ���ӱ�ţ���ֵС���ܵ�������Ŀ��Ϊ-1ʱ��ʾ���������ӽ��в���
*Parameter: unsigned int nAddr -[in]  Ҫ���� R ֵ��λַ 
*Parameter: unsigned char nBitIdx -[in] Ҫ���� R ֵ��λԪλַ  [0, 31]
*Parameter: unsigned char nBitValue -[in] �O��ֵ��0 �� 1
*Parameter: size_t nMax_wait_time -[in] ִ����������ȴ�ʱ�䣨ms��
*Parameter: string & str_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::set_RBit(int nConn_idx, unsigned int nAddr, unsigned char nBitIdx, unsigned char nBitValue
							, unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::set_RBit");
	//�ж������Ϸ���
	//nConn_idxֵ���С� scif_Init ��ʽ��ʼ���r��struct DLL_USE_SETTING �� ConnectNum���O�����B����Ŀ��
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}
	//�O��ֵֻ��Ϊ 0 �� 1
	if (!(0 == nBitValue || 1 == nBitValue))
	{
		LError("Invalid BitValue:{}, must 0 or 1.", nBitValue);
		str_err_reason_for_debug = "Invalid BitValue:" + std::to_string(nBitValue) + ", must 0 or 1.";
		str_err_reason_for_user = "�������󣬲����豸ʧ��";
		return false;
	}

	if (!is_valid_addr(nAddr, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}

	if (!is_valid_bit_idx(nBitIdx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("invalid bitIdx,reason:{}", str_err_reason_for_debug);
		return false;
	}

	//�ж��������Ƿ����
	{
		Thread_Read_Lock guard(m_rw_mutex_for_available);
		if (!m_bAvailable)
		{
			LError("The Baoyuan Lib is unavailable:{}", str_err_reason_for_debug);
			str_err_reason_for_debug = "The Baoyuan Lib is unavailable:" + str_err_reason_for_debug;
			str_err_reason_for_user = "�������󣬲����豸ʧ��";
			return false;
		}
	}

	//�B�m�Y���xȡ�O��
	unsigned int nAddr_for_ReadR = 0;
	unsigned int nNum_for_ReadR = 32; 
//	int nRet_baoyuan =  scif_cmd_ReadR(SC_POLLING_CMD, nConn_idx, nAddr_for_ReadR, nNum_for_ReadR);
	int nRet_baoyuan =  m_sc2_obj.LReadNR(nConn_idx, nAddr_for_ReadR, nNum_for_ReadR);
	//����ֵΪ0�����ʾ�趨����ʧ��
	if (nRet_baoyuan == 0)
	{
		LError("fail to read, Conn index:{}, Addr:{}, num:{}, ret:{}", nConn_idx, nAddr_for_ReadR, nNum_for_ReadR, nRet_baoyuan);
		str_err_reason_for_debug = "fail to read, Conn index:" + std::to_string(nConn_idx) + ", Addr:"
			+ std::to_string(nAddr_for_ReadR) + ", num:" + std::to_string(nNum_for_ReadR) + ", ret:" + std::to_string(nRet_baoyuan);
		str_err_reason_for_user = "�����豸ʧ��";
		return false;
	}

	if(1 == nRet_baoyuan)
	{//ԓ�O���������½M��
		LWarn("the cmd has been regrouped, conn index:{}, Addr:{}, Num:{}", nConn_idx, nAddr_for_ReadR, nNum_for_ReadR);
	}
	
	//�R��ӛ���w�O���������ڶԼ������дǰ���в���
// 	nRet_baoyuan = scif_SetMirror(nConn_idx);
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, ServerIdx:"
// 		<< nConn_idx, str_err_reason, false);
	
	//ִ�� R ֵ�΂� bit λַ�Y�ϵČ���
	nRet_baoyuan = m_sc2_obj.DWrite1RBit(nConn_idx, nAddr, nBitIdx, nBitValue);
	if (nRet_baoyuan == 0)
	{
		LError("fail to WriteRBit, Conn index:{}, Addr:{}, BitIdx:{}, BitValue:{}", nConn_idx, nAddr, nBitIdx, nBitValue);
		str_err_reason_for_debug = "fail to WriteRBit, Conn index:" + std::to_string(nConn_idx) + ", Addr:"
			+ std::to_string(nAddr) + ", BitIdx:" + std::to_string(nBitIdx) + ", BitValue:" + std::to_string(nBitValue);
		str_err_reason_for_user = "�����豸ʧ��";
		return false;
	}
	//׌�ȴ���ǰ���O����ֱ����������ᣬ���^�m������ȥ
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time/*1000*/);
	if (nRet_baoyuan == 0)
	{
		LError("Time out to do cmd, conn idx:{}, Max wait time(ms):{}", nConn_idx, nMax_wait_time);
		str_err_reason_for_debug = "Time out to do cmd, conn idx:" + std::to_string(nConn_idx) + ", Max wait time(ms):" + std::to_string(nMax_wait_time);
		str_err_reason_for_user = "�����豸��ʱ";
		return false;
	}
	return true;
}

/************************************
* Method:    set_RValue
* Brief:  д��һ�޷���������ָ����Rֵ��
* Access:    private 
* Returns:   bool ture:�ɹ���false:ʧ��
* Qualifier:
*Parameter: int nConn_idx -[in] �������� 
*Parameter: unsigned int nAddr -[in]  Ҫд��ĵ�ַ
*Parameter: unsigned int nVal -[in]  Ҫд���ֵ
*Parameter: unsigned short nMax_wait_time -[in] ���ȴ�ʱ�䣬��λmm 
*Parameter: string & str_kernel_err_reason -[out]  
************************************/
bool CBaoyuan_Lib::set_RValue(int nConn_idx, unsigned int nAddr, unsigned int nVal, unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::set_RValue");
	//�������Ϸ���
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("error reason:{}", str_err_reason_for_debug);
		return false;
	}
	if (!is_valid_addr(nAddr, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("error reason:{}", str_err_reason_for_debug);
		return false;
	}

	//�B�m�Y���xȡ�O��
//	int nRet_baoyuan = scif_cmd_ReadR(SC_POLLING_CMD, nServer_idx, 0, 32);
	int nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 0, 32);

	if (nRet_baoyuan==0)
	{
		LError("fail to ReadR, server index:{}", nConn_idx);
		str_err_reason_for_debug = "fail to ReadR, server index:" + std::to_string(nConn_idx);
		str_err_reason_for_user = "�����豸ʧ��";
		return false;
	}
	if (1 == nRet_baoyuan)
	{
		LWarn("the cmd has been regrouped, conn index:{}", nConn_idx);
	}
// 	nRet_baoyuan = scif_SetMirror(nConn_idx);		//�R��ӛ���w�O��
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, server index:" << nConn_idx
// 		, str_kernel_err_reason, false);
	//ֱ��д��һ�����ֽڵ�Rֵ
	nRet_baoyuan = m_sc2_obj.DWrite1R(nConn_idx, nAddr, nVal);
	if (nRet_baoyuan == 0)
	{
		LError("fail to WriteR, conn index:{}", nConn_idx);
		str_err_reason_for_debug = "fail to WriteR, conn index:" + std::to_string(nConn_idx);
		str_err_reason_for_user = "�����豸ʧ��";
		return false;
	}


	//׌�ȴ���ǰ���O����ֱ����������ᣬ���^�m������ȥ
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time);
	if (nRet_baoyuan == 0)
	{
		LError("Time out to do cmd, nConn_idx:{}, max wait time(ms):{}", nConn_idx, nMax_wait_time);
		str_err_reason_for_debug = "Time out to do cmd, nConn_idx:" + std::to_string(nConn_idx)+",max wait time(ms):"+ std::to_string(nMax_wait_time);
		str_err_reason_for_user = "�����豸��ʱ";
		return false;
	}
	return true;
}

/************************************
* Method:    set_RString
* Brief:  �����ִ����������е� R ֵ
* Access:    private 
* Returns:   bool true���ɹ���false��ʧ��
* Qualifier:
*Parameter: int nConn_idx -[in]  ��������, -1��ʾ��������
*Parameter: size_t nAddr -[in]  Ҫ������Y��λַ
*Parameter: size_t nBuff_size -[in]  Ҫд�����������λ�ֽ�
*Parameter: const char * pBuff -[in]  Ҫд����ַ���
*Parameter: unsigned short nMax_wait_time -[in]  ��ʱʱ�䣬��λms
*Parameter: string & str_kernel_err_reason -[out] ����ԭ�� 
************************************/
bool CBaoyuan_Lib::set_RString(int nConn_idx, size_t nAddr, size_t nBuff_size, const char* pBuff, unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::set_RString");
	//�������Ϸ���
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}

	if (!(NULL != pBuff))
	{
		LError("p is NULL");
		str_err_reason_for_debug = "p is NULL";
		str_err_reason_for_user = "��������";
		return false;
	}

	if (!is_valid_addr(nAddr, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}

	//�B�m�Y���xȡ�O��	
	int nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 0, nBuff_size);
	if (nRet_baoyuan == 0)
	{
		LError("fail to ReadR, conn index:{}", nConn_idx);
		str_err_reason_for_debug = "fail to ReadR, conn index:"+ std::to_string(nConn_idx);
		str_err_reason_for_user = "�����豸ʧ��";
		return false;
	}

	if (1 == nRet_baoyuan)
	{
		LWarn("the cmd has been regrouped, conn index:{}", nConn_idx);
	}
	
	//�R��ӛ���w�O��
// 	nRet_baoyuan = scif_SetMirror(nConn_idx);		
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, server index:" << nConn_idx
// 		, str_kernel_err_reason, false);
	//�����ִ��� R
	nRet_baoyuan = m_sc2_obj.DWriteRString(nConn_idx, nAddr, nBuff_size, (char*)pBuff);
	if (0 == nRet_baoyuan)
	{//ʧ��
		ERROR_MSG err_msg;
		m_sc2_obj.GetConnectionError(nConn_idx, &err_msg);
		LError("conn idx:{}, Error:{}", nConn_idx, err_msg.Error);
		if (nRet_baoyuan == 0)
		{
			LError("fail to WriteRString, conn index:{}, addr:{}, buff size:{}", nConn_idx, nAddr, nBuff_size);
			str_err_reason_for_debug = "fail to WriteRString, conn index:" + std::to_string(nConn_idx)+",addr:"+ std::to_string(nAddr)
				+",buff size:"+ std::to_string(nBuff_size);
			str_err_reason_for_user = "�����豸ʧ��";
			return false;
		}

	}
	//׌�ȴ���ǰ���O����ֱ����������ᣬ���^�m������ȥ
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time);
	if (nRet_baoyuan == 0)
	{
		LError("Time out to do cmd, conn idx:{}, max wait time(ms):{}", nConn_idx, nMax_wait_time);
		str_err_reason_for_debug = "Time out to do cmd, conn idx:" + std::to_string(nConn_idx) + ", max wait time(ms):" + std::to_string(nMax_wait_time);
		str_err_reason_for_user = "�����豸��ʱ";
		return false;
	}
	return true;
}

/************************************
* Method:    set_CValue
* Brief:  д��һ��������Command��
* Access:    private 
* Returns:   bool
* Qualifier:
*Parameter: int nConn_idx -[in] �������� 
*Parameter: int nAddr -[in]  Ҫд��ĵ�ַ
*Parameter: int nValue -[in] Ҫд���ֵ 
*Parameter: unsigned short nMax_wait_time -[in] ��λms 
*Parameter: string & str_kernel_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::set_CValue(int nConn_idx, int nAddr, int nValue, unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//�����Ϸ����ж�
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}
	if (!is_valid_addr(nAddr, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("invalid addr, reason:{}", str_err_reason_for_debug);
		return false;
	}


	//�B�m�Y���xȡ�O��
	int nRet_baoyuan = m_sc2_obj.LReadNC(nConn_idx, 0, 32);
	if (nRet_baoyuan == 0)
	{
		LError("Time out to do cmd, conn idx:{}, max wait time(ms):{}", nConn_idx, nMax_wait_time);
		str_err_reason_for_debug = "Time out to do cmd, conn idx:" + std::to_string(nConn_idx) + ", max wait time(ms):" + std::to_string(nMax_wait_time);
		str_err_reason_for_user = "�����豸��ʱ";
		return false;
	}


	if (1 == nRet_baoyuan)
	{
		LWarn("the cmd has been regrouped, conn index:{}", nConn_idx);
	}
	//�R��ӛ���w�O��
// 	nRet_baoyuan = scif_SetMirror(nServer_idx);
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, server index:" << nServer_idx
// 		, str_kernel_err_reason, false);

	//д��ιP�Y�ϵ�command
	nRet_baoyuan =  m_sc2_obj.DWrite1C(nConn_idx, nAddr, nValue);		
	if (nRet_baoyuan == 0)
	{
		LError("fail to WriteC, conn idx:{}, addr:{}, value:{}", nConn_idx, nAddr, nValue);
		str_err_reason_for_debug = "fail to WriteC, conn idx:"+ std::to_string(nConn_idx) +", addr:"+ std::to_string(nAddr) +", value:" + std::to_string(nValue);
		str_err_reason_for_user = "�����豸ʧ��";
		return false;
	}
    //׌�ȴ���ǰ���O����ֱ����������ᣬ���^�m������ȥ
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time);
	if (nRet_baoyuan == 0)
	{
		LError("Time out to do cmd, nConn_idx:{}", nConn_idx);
		str_err_reason_for_debug = "Time out to do cmd, nConn_idx:{}" + std::to_string(nConn_idx);
		str_err_reason_for_user = "�����豸��ʱ";
		return false;
	}
	return true;
}

/************************************
* Method:    get_RValue
* Brief:  ���ж���̻��Ƿ����ӳɹ���������ӳɹ������ȡRֵ����
* Access:    private 
* Returns:   bool
* Qualifier:
*Parameter: int Conn_idx -[in/out]  
*Parameter: int nAddr -[in/out]  
*Parameter: int & nValue -[in/out]  
*Parameter: string & str_kernel_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::get_RValue(int Conn_idx, int nAddr, int& nValue, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//�����Ϸ����ж�
	if (!is_valid_conn_idx(Conn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}
	if (!is_valid_addr(nAddr, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}

	//�ж�����״̬
	bool bSuccess = is_connected(Conn_idx, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("Connection is over, reason:{}", str_err_reason_for_debug);
		return false;
	}

	//��ȡ����
	nValue = m_sc2_obj.memR(Conn_idx, nAddr);
	return true;
}

bool CBaoyuan_Lib::get_RBit(int nConn_idx, int nAddr, int nBitIdx, int& nValue, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//�����Ϸ����ж�
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("Invalid conn idx : {}, err reason : {}", nConn_idx,str_err_reason_for_debug);
		return false;
	}
	if (!is_valid_addr(nAddr, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("invalid addr : {}, err reason : {}", nAddr, str_err_reason_for_debug);
		return false;
	}
	if (!is_valid_bit_idx(nBitIdx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("iinvalid bitIdx : {}, err reason : {}", nBitIdx, str_err_reason_for_debug);
		return false;
	}

	//�ж������Ƿ�����
	bool bSuccess = is_connected(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("Connection is over, err reason : {}", str_err_reason_for_debug);
		return false;
	}
	//��ȡ����
	nValue = m_sc2_obj.memRBit(nConn_idx, nAddr, nBitIdx);
	return true;
}

CBaoyuan_Lib::CBaoyuan_Lib() 
	: m_bAvailable(false)
	, m_bStop(true)
{
	memset(&m_sDLL_setting, 0, sizeof(DLL_USE_SETTING));
	m_sDLL_setting.SoftwareType = 4;		//ܛ�w�N�   
	/*	m_sDLL_setting.ConnectNum = 200;			//�B����Ŀ*/
	m_sDLL_setting.MemSizeI = I_NUM;
	m_sDLL_setting.MemSizeO = O_NUM;
	m_sDLL_setting.MemSizeC = C_NUM;
	m_sDLL_setting.MemSizeS = S_NUM;
	m_sDLL_setting.MemSizeA = A_NUM;
	/*	m_sDLL_setting.MemSizeR = 4000000;*/
	m_sDLL_setting.MemSizeTimer = 0;
	m_sDLL_setting.MemSizeF = F_NUM;
	//������������δʹ��
	for (int i = 0; i != MAX_CONNECTIONS_NUM; ++i)
	{
		m_vec_conn_idx_used.push_back(false);
	}
}

CBaoyuan_Lib::~CBaoyuan_Lib()
{
	LWarn("to release source");
	fini();
}

bool CBaoyuan_Lib::is_valid_conn_idx(int nConn_idx, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//nConn_idxֵ���С� scif_Init ��ʽ��ʼ���r��struct DLL_USE_SETTING �� ConnectNum ���O�����B����Ŀ��
	if (!(nConn_idx >= -1 && nConn_idx < (int)m_sDLL_setting.ConnectNum))
	{
		LError("Invalid connection index:{}, must be [-1,{}]", nConn_idx, m_sDLL_setting.ConnectNum - 1);
		str_err_reason_for_debug = "Invalid connection index:" + std::to_string(nConn_idx)
			+ ", must be [-1," + std::to_string(m_sDLL_setting.ConnectNum - 1) + "]";
		str_err_reason_for_user = "��������";
		return false;
	}
	return true;
}

bool CBaoyuan_Lib::is_valid_addr(int nAddr, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	if (!(nAddr >= 0 && nAddr < (int)m_sDLL_setting.MemSizeR))
	{
		LError("Iinvalid addr:{}, should be between 0 and MemSizeR:{}", nAddr, m_sDLL_setting.MemSizeR);
		str_err_reason_for_debug = "Iinvalid addr:{}:" + std::to_string(nAddr)
			+ ",should be between 0 and MemSizeR:" + std::to_string(m_sDLL_setting.MemSizeR);
		str_err_reason_for_user = "��������";
		return false;
	}
	return true;
}

bool CBaoyuan_Lib::is_valid_bit_idx(int nBitIdx, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	if (!(nBitIdx >= 0 && nBitIdx < 32))
	{
		LError("invalid bitIdx:{}, should be [0, 31]", nBitIdx);
		str_err_reason_for_debug = "nvalid bitIdx:" + std::to_string(nBitIdx) + ",should be [0, 31]";
		str_err_reason_for_user = "��������";
		return false;
	}
	return true;
}

/************************************
* Method:    strerror_ftp
* Brief:  ��FTP�Ĵ�����ת��Ϊ��Ӧ����ʾ��Ϣ
* Access:    private 
* Returns:   string
* Qualifier:
*Parameter: int nResult_ftp -[in]  GetLibraryMsg(SCIF_FTP_RESULT) 
************************************/
string CBaoyuan_Lib::strerror_ftp(int nResult_ftp)
{
	string str_kernel_err_reason;
	int nState = m_sc2_obj.GetLibraryMsg(SCIF_FTP_STATE);
	LCritical("state of ftp:{}", nState);
	switch (nResult_ftp)
	{
	case FTP_RESULT_PROCESSING:
		str_kernel_err_reason = "PROCESSING";
		break;
	case FTP_RESULT_SUCCESS:
		str_kernel_err_reason = "SUCCESS";
		break;
	case FTP_RESULT_FAIL_TO_READ_LOCAL_FILE:
		str_kernel_err_reason = "FAIL_TO_READ_LOCAL_FILE";
		break;
	case FTP_RESULT_FAIL_TO_WRITE_LOCAL_FILE:
		str_kernel_err_reason = "FAIL_TO_WRITE_LOCAL_FILE";
		break;
	case FTP_RESULT_FAIL_TO_READ_REMOTE_FILE:
		str_kernel_err_reason = "FAIL_TO_READ_REMOTE_FILE";
		break;
	case FTP_RESULT_FAIL_TO_WRITE_REMOTE_FILE:
		str_kernel_err_reason = "FAIL_TO_WRITE_REMOTE_FILE";
		break;
	case FTP_RESULT_FAIL_TO_SET_COMMAND:
		str_kernel_err_reason = "FAIL_TO_SET_COMMAND";
		break;
	case FTP_RESULT_FAIL_TO_COMMUNICATION:
		str_kernel_err_reason = "FAIL_TO_COMMUNICATION";
		break;
	case FTP_RESULT_FILE_MISMATCH:
		str_kernel_err_reason = "FILE_MISMATCH";
		break;
	default:
		LError("Don't support ftp result:{}", nResult_ftp);
		str_kernel_err_reason = "Don't support ftp result:" + std::to_string(nResult_ftp);
		return str_kernel_err_reason;

		break;
	}
	return str_kernel_err_reason;
}

void CBaoyuan_Lib::svc()
{
	LCritical("start thread successfully");
	size_t nWait_time_ms = 100; //ÿ������ʱ�䣬���ݴ������������ݡ�ͨѶ��ʱ�������ã���Χ20-1000ms
	while (false == m_bStop)
	{
		boost::this_thread::sleep(boost::posix_time::millisec(nWait_time_ms));
		int ret = m_sc2_obj.MainProcess();
	}
	LWarn("++++++++++++++++timer is over+++++++++++++");
}

CBaoyuan_Lib::CGarbo CBaoyuan_Lib::Garbo;

CBaoyuan_Lib* CBaoyuan_Lib::ms_pInstance = new CBaoyuan_Lib();

CBaoyuan_Lib::CGarbo::~CGarbo()
{
	if (NULL != CBaoyuan_Lib::ms_pInstance)
	{
		delete CBaoyuan_Lib::ms_pInstance;
		CBaoyuan_Lib::ms_pInstance = NULL;
	}
}
