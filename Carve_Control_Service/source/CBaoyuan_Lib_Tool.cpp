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
#include "busin_log.h"
#define __CLASS_FUNCTION__ ((std::string("CBaoyuan_Lib::") + std::string(__FUNCTION__)).c_str()) 
const int MAX_CONNECTIONS_NUM = 255;
const int MIN_CONNECTIONS_NUM = 0;
CBaoyuan_Lib* CBaoyuan_Lib::instance()
{
	return ms_pInstance;
}

void CBaoyuan_Lib::test() const
{
	printf("%s | %u\n", __FUNCTION__, __LINE__);
	businlog_crit("%s | %u", __FUNCTION__, __LINE__);
}

bool CBaoyuan_Lib::init(int nMakerID, const string& str_key, unsigned int nConnectNum, unsigned int MemSizeR /*= 100000*/)
{
	businlog_tracer_perf(CBaoyuan_Lib::init);
	//����У��
	businlog_error_return(nConnectNum >= MIN_CONNECTIONS_NUM && nConnectNum <= MAX_CONNECTIONS_NUM
		, ("%s | Invalid ConnectNum:%d, should be [1, %d].", __CLASS_FUNCTION__, nConnectNum, MAX_CONNECTIONS_NUM), false);

	m_sDLL_setting.ConnectNum = nConnectNum; //�B����Ŀ
	m_sDLL_setting.MemSizeR = MemSizeR; //һ������������Ӧ��ע��������ڴ��С
	int nRet_baoyuan = m_sc2_obj.LibraryInitial(&m_sDLL_setting, nMakerID, (char*)str_key.c_str());
	if (0 == nRet_baoyuan)
	{//ʧ��
		businlog_error("%s | scif_Init failed, ret:%d", __CLASS_FUNCTION__, nRet_baoyuan);
		return false;
	} 
	if (10 == nRet_baoyuan)
	{//��ʼ���ɹ����ǽ��ܹ����ִ�ʧ��
		businlog_error("%s | fail to decrypt the key string when init scif, ret:%d, MakerID:%d, key:%s."
			, __CLASS_FUNCTION__, nRet_baoyuan, nMakerID, str_key.c_str());
		return false;
	}
	//���ÿ����־�ȼ�:�д������ʾ
	m_sc2_obj.LibrarySetDebug(2); //todo::for test
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
	businlog_warn("%s | Notice", __CLASS_FUNCTION__);
	{
		Thread_Write_Lock guard(m_rw_mutex_for_available);
		//�ж��������״̬
		if (true == m_bAvailable)
		{//���������ڿ���״̬�����轫������
			m_sc2_obj.LibraryDestroy();
			m_bAvailable = false;
			businlog_crit("%s | release baoyuan lib successfully!", __CLASS_FUNCTION__);
		}
	}
}

bool CBaoyuan_Lib::create_connection(unsigned short nConn_idx, const string& str_carve_ip, string& str_kernel_err_reason)
{
	businlog_tracer_perf(CBaoyuan_Lib::create_connection);
	//�ж������Ϸ���
	businlog_error_return_err_reason(nConn_idx >= 0 && nConn_idx < m_sDLL_setting.ConnectNum
		, __CLASS_FUNCTION__ << " | Invalid Connection index:" << nConn_idx << ", must be [0," << m_sDLL_setting.ConnectNum - 1
		<< "]", str_kernel_err_reason, false);

	int talktime=0;
	//ֱ��ݔ�������IP�M���B��
	//���д˺�ʽ�ɹ�ֻ�����B���O���ɹ����Пo�����������B������횺��� GetConnectionMsg ��ʽ��z���B����B
	int nRet_baoyuan = m_sc2_obj.ConnectLocalIP(nConn_idx, (char*)str_carve_ip.c_str());
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to connect ip:" << str_carve_ip
		<< " with connection index:" << nConn_idx, str_kernel_err_reason, false);
	
	int nStatus = SC_CONN_STATE_DISCONNECT;
	//��һ��ʱ����ѭ���������״̬�������ʱ�һ�δ���ӳɹ����򱨴��˳�
	while (nStatus != SC_CONN_STATE_OK)
	{
		nStatus = m_sc2_obj.GetConnectionMsg(nConn_idx, SCIF_CONNECT_STATE);
		if (SC_CONN_STATE_OK == nStatus)
		{//���ӳɹ�
			break;
		}
		//��ʱ����ʧ��
		if (talktime >= 10)
		{//�ظ������ﵽָ������
			str_kernel_err_reason = "Connect time out";
			businlog_error("%s | connection index:%d, err reason:%s, Status:%d"
				, __CLASS_FUNCTION__, nConn_idx, str_kernel_err_reason.c_str(), nStatus);
			return false;
		}
		else
		{//����ʧ�ܣ�δ�ﵽָ������
			//����һ���
			Sleep(100);
			talktime++;
		}
	}
	//��ʱ���ӳɹ�
	//�趨ѭ����������ѭ��ִ��
	m_sc2_obj.LClearQueue(nConn_idx);
	m_sc2_obj.LReadBegin(nConn_idx);
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 17003, 1);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | fail to read, line:" << __LINE__
		, str_kernel_err_reason, false);
	m_sc2_obj.LReadNR(nConn_idx, 17034, 1);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | fail to read, line:" << __LINE__
		, str_kernel_err_reason, false);
	m_sc2_obj.LReadNR(nConn_idx, 29000, 1);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | fail to read, line:" << __LINE__
		, str_kernel_err_reason, false);
	m_sc2_obj.LReadNR(nConn_idx, 29001, 1);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | fail to read, line:" << __LINE__
		, str_kernel_err_reason, false);
	m_sc2_obj.LReadNR(nConn_idx, 29002, 1);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | fail to read, line:" << __LINE__
		, str_kernel_err_reason, false);
	m_sc2_obj.LReadNR(nConn_idx, 29003, 1);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | fail to read, line:" << __LINE__
		, str_kernel_err_reason, false);
	m_sc2_obj.LReadNR(nConn_idx, 1040000, 1);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | fail to read, line:" << __LINE__
		, str_kernel_err_reason, false);
	m_sc2_obj.LReadNR(nConn_idx, 3006072, 1);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | fail to read, line:" << __LINE__
		, str_kernel_err_reason, false);
	m_sc2_obj.LReadEnd(nConn_idx);
	return true;
}


bool CBaoyuan_Lib::get_status(unsigned short nConn_idx, int& nStatus, string& str_kernel_err_reason)
{
	businlog_error_return(is_valid_conn_idx(nConn_idx, str_kernel_err_reason)
		, ("%s | err reason:%s", __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	nStatus = m_sc2_obj.GetConnectionMsg(nConn_idx, SCIF_CONNECT_STATE);
	return true;
}

bool CBaoyuan_Lib::confirm_task(unsigned short nServer_idx, size_t nMax_wait_time, string& str_kernel_err_reason)
{
	size_t nAddr = 20000;
	size_t nBit_idx = 10;
	int nBit_value = 0;
	return set_RBit(nServer_idx, nAddr, nBit_idx, nBit_value, nMax_wait_time, str_kernel_err_reason);
}

/************************************
* Method:    set_RBit
* Brief:  ��0����1��ΪRֵд��λַnAddr��nBitIdxλԪλַ
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: unsigned short nConn_idx -[in]  ���ӱ�ţ���ֵС���ܵ�������Ŀ��Ϊ-1ʱ��ʾ���������ӽ��в���
*Parameter: unsigned int nAddr -[in]  Ҫ���� R ֵ��λַ
*Parameter: unsigned char nBitIdx -[in] Ҫ���� R ֵ��λԪλַ  
*Parameter: unsigned char nBitValue -[in] �O��ֵ��0 �� 1
*Parameter: size_t nMax_wait_time -[in] ִ����������ȴ�ʱ�䣨ms��
*Parameter: string & str_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::set_RBit(unsigned short nConn_idx, unsigned int nAddr, unsigned char nBitIdx, unsigned char nBitValue
							, size_t nMax_wait_time, string& str_err_reason)
{
	businlog_tracer_perf(CBaoyuan_Lib::set_RBit);
	//�ж������Ϸ���
	//nServerIdxֵ���С� scif_Init ��ʽ��ʼ���r��struct DLL_USE_SETTING �� TalkInfoNum ���O�����B����Ŀ��
	businlog_error_return(is_valid_conn_idx(nConn_idx, str_err_reason)
		, ("%s | err reason:%s", __CLASS_FUNCTION__, str_err_reason.c_str()), false);
	//�O��ֵֻ��Ϊ 0 �� 1
	businlog_error_return_err_reason(0 == nBitValue || 1 == nBitValue, __CLASS_FUNCTION__ << " | Invalid BitValue:"
		<< nBitValue << ", must 0 or 1.", str_err_reason, false);
	
	//�ж��������Ƿ����
	{
		Thread_Read_Lock guard(m_rw_mutex_for_available);
		businlog_error_return_err_reason(m_bAvailable, __CLASS_FUNCTION__ << " | The Baoyuan Lib is unavailable"
			, str_err_reason, false);
	}

	//�B�m�Y���xȡ�O��
	unsigned int nAddr_for_ReadR = 0;
	unsigned int nNum_for_ReadR = 32; 
//	int nRet_baoyuan =  scif_cmd_ReadR(SC_POLLING_CMD, nConn_idx, nAddr_for_ReadR, nNum_for_ReadR);
	int nRet_baoyuan =  m_sc2_obj.LReadNR(nConn_idx, nAddr_for_ReadR, nNum_for_ReadR);
	//����ֵΪ0�����ʾ�趨����ʧ��
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to read, Conn index:" 
		<< nConn_idx << ", Addr:" << nAddr_for_ReadR << ", num:"
		<< nNum_for_ReadR << ", ret:" << nRet_baoyuan, str_err_reason, false);
	if(1 == nRet_baoyuan)
	{//ԓ�O���������½M��
		businlog_warn("%s | the cmd has been regrouped, conn index:%d, Addr:%d, Num:%d"
			, __CLASS_FUNCTION__, nConn_idx, nAddr_for_ReadR, nNum_for_ReadR);
	}
	
	//�R��ӛ���w�O���������ڶԼ������дǰ���в���
// 	nRet_baoyuan = scif_SetMirror(nConn_idx);
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, ServerIdx:"
// 		<< nConn_idx, str_err_reason, false);
	
	//ִ�� R ֵ�΂� bit λַ�Y�ϵČ���
	nRet_baoyuan = m_sc2_obj.DWrite1RBit(nConn_idx, nAddr, nBitIdx, nBitValue);
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to WriteRBit, Conn idx:" 
		<< nConn_idx << ", Addr:" << nAddr << ", BitIdx:" << nBitIdx << ", BitValue:" << nBitValue, str_err_reason, false);
	
	//׌�ȴ���ǰ���O����ֱ����������ᣬ���^�m������ȥ
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time/*1000*/);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | Time out to do cmd, conn idx:" 
		<< nConn_idx, str_err_reason, false);
	return true;
}

bool CBaoyuan_Lib::set_RValue(unsigned short nConn_idx, unsigned int nAddr, unsigned int nVal, size_t nMax_wait_time, string& str_kernel_err_reason)
{
	businlog_tracer_perf(CBaoyuan_Lib::set_RValue);
	//�������Ϸ���
	businlog_error_return(is_valid_conn_idx(nConn_idx, str_kernel_err_reason)
		, ("%s | error reason:%s", __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	//�B�m�Y���xȡ�O��
//	int nRet_baoyuan = scif_cmd_ReadR(SC_POLLING_CMD, nServer_idx, 0, 32);
	int nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 0, 32);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | fail to ReadR, server index:" << nConn_idx
		, str_kernel_err_reason, false);
	if (1 == nRet_baoyuan)
	{
		businlog_warn("%s | the cmd has been regrouped, conn index:%d"
			, __CLASS_FUNCTION__, nConn_idx);
	}
// 	nRet_baoyuan = scif_SetMirror(nConn_idx);		//�R��ӛ���w�O��
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, server index:" << nConn_idx
// 		, str_kernel_err_reason, false);
	//ֱ��д��һ�����ֽڵ�Rֵ
	nRet_baoyuan = m_sc2_obj.DWrite1R(nConn_idx, nAddr, nVal);
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to WriteR, conn index:" << nConn_idx
		, str_kernel_err_reason, false);
	//׌�ȴ���ǰ���O����ֱ����������ᣬ���^�m������ȥ
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | Time out to do cmd, serverIdx:" 
		<< nConn_idx, str_kernel_err_reason, false);
	return true;
}

bool CBaoyuan_Lib::set_RString(unsigned short nConn_idx, size_t nAddr, size_t nBuff_size, const char* pBuff, size_t nMax_wait_time, string& str_kernel_err_reason)
{
	businlog_tracer_perf(CBaoyuan_Lib::set_RString);
	//�������Ϸ���
	businlog_error_return(is_valid_conn_idx(nConn_idx, str_kernel_err_reason)
		, ("%s | err reason:%s", __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	businlog_error_return_err_reason(NULL != pBuff, __CLASS_FUNCTION__ << " | p is NULL", str_kernel_err_reason, false);
	
	//�B�m�Y���xȡ�O��	
	int nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 0, nBuff_size);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | fail to ReadR, conn index:" << nConn_idx
		, str_kernel_err_reason, false);
	if (1 == nRet_baoyuan)
	{
		businlog_warn("%s | the cmd has been regrouped, conn index:%d", __CLASS_FUNCTION__, nConn_idx);
	}
	
	//�R��ӛ���w�O��
// 	nRet_baoyuan = scif_SetMirror(nConn_idx);		
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, server index:" << nConn_idx
// 		, str_kernel_err_reason, false);
	//�����ִ��� R
	nRet_baoyuan = m_sc2_obj.DWriteRString(nConn_idx, nAddr, nBuff_size, (char*)pBuff);
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to WriteRString, conn index:" << nConn_idx
		 << ", addr:" << nAddr << ", buff size:" << nBuff_size, str_kernel_err_reason, false);
	
	//׌�ȴ���ǰ���O����ֱ����������ᣬ���^�m������ȥ
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time);
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | Time out to do cmd, conn idx:" 
		<< nConn_idx, str_kernel_err_reason, false);
	return true;
}

bool CBaoyuan_Lib::set_CValue(unsigned short nConn_idx, int nAddr, int nValue, size_t nMax_wait_time, string& str_kernel_err_reason)
{
	//�����Ϸ����ж�
	businlog_error_return(is_valid_conn_idx(nConn_idx, str_kernel_err_reason), ("%s | err reason:%s", __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	//�B�m�Y���xȡ�O��
	int nRet_baoyuan = m_sc2_obj.LReadNC(nConn_idx, 0, 32);
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to ReadC, server index:" << nConn_idx
		, str_kernel_err_reason, false);
	if (1 == nRet_baoyuan)
	{
		businlog_warn("%s | the cmd has been regrouped, conn index:%d", __CLASS_FUNCTION__, nConn_idx);
	}
	//�R��ӛ���w�O��
// 	nRet_baoyuan = scif_SetMirror(nServer_idx);
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, server index:" << nServer_idx
// 		, str_kernel_err_reason, false);

	//д��ιP�Y�ϵ�command
	nRet_baoyuan =  m_sc2_obj.DWrite1C(nConn_idx, nAddr, nValue);		
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to WriteC, conn index:" << nConn_idx
		<< ", addr:" << nAddr << ", value:" << nValue, str_kernel_err_reason, false);
    //׌�ȴ���ǰ���O����ֱ����������ᣬ���^�m������ȥ
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time);
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | Time out to do cmd, serverIdx:" 
		<< nConn_idx, str_kernel_err_reason, false);
	return true;
}

CBaoyuan_Lib::CBaoyuan_Lib() : m_bAvailable(false)
{
	memset(&m_sDLL_setting, 0, sizeof(DLL_USE_SETTING));
	m_sDLL_setting.SoftwareType = 4;		//ܛ�w�N�   
	/*	m_sDLL_setting.TalkInfoNum = 200;			//�B����Ŀ*/
	m_sDLL_setting.MemSizeI = I_NUM;
	m_sDLL_setting.MemSizeO = O_NUM;
	m_sDLL_setting.MemSizeC = C_NUM;
	m_sDLL_setting.MemSizeS = S_NUM;
	m_sDLL_setting.MemSizeA = A_NUM;
	/*	m_sDLL_setting.MemSizeR = 4000000;*/
	m_sDLL_setting.MemSizeTimer = 0;
	m_sDLL_setting.MemSizeF = F_NUM;
}

CBaoyuan_Lib::~CBaoyuan_Lib()
{
	businlog_warn("%s | to release source", __CLASS_FUNCTION__);
	fini();
}

bool CBaoyuan_Lib::is_valid_conn_idx(unsigned short nConn_idx, string& str_kernel_err_reason)
{
	//nServerIdxֵ���С� scif_Init ��ʽ��ʼ���r��struct DLL_USE_SETTING �� TalkInfoNum ���O�����B����Ŀ��
	businlog_error_return_err_reason(nConn_idx >= 0 && nConn_idx < m_sDLL_setting.ConnectNum
		, __CLASS_FUNCTION__ << " | Invalid connection index:" << nConn_idx << ", must be [0," 
		<< m_sDLL_setting.ConnectNum - 1 << "]", str_kernel_err_reason, false);
	return true;
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
