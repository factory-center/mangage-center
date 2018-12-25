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
#include "boost/filesystem.hpp"
#ifdef _WINDOWS
#define __CLASS_FUNCTION__ ((std::string(__FUNCTION__)).c_str()) 
#else
#define __CLASS_FUNCTION__ ((std::string("CBaoyuan_Lib::") + std::string(__FUNCTION__)).c_str()) 
#endif

const int MAX_CONNECTIONS_NUM = 255;
const int MIN_CONNECTIONS_NUM = 0;
CBaoyuan_Lib* CBaoyuan_Lib::instance()
{
	return ms_pInstance;
}

int CBaoyuan_Lib::test()
{
	printf("%s | %u\n", __FUNCTION__, __LINE__);
	businlog_crit("%s | %u", __FUNCTION__, __LINE__);
	return MSP_SUCCESS;
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
	string str_err_reason;
	//������ʱ��
	bool bSuccess = start_timer(str_err_reason);
	businlog_error_return(bSuccess, ("%s | faild to start timer, reason:%s", __CLASS_FUNCTION__, str_err_reason.c_str()), false);
	
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

	
	//ֱ��ݔ�������IP�M���B��
	//���д˺�ʽ�ɹ�ֻ�����B���O���ɹ����Пo�����������B������횺��� GetConnectionMsg ��ʽ��z���B����B
	int nRet_baoyuan = m_sc2_obj.ConnectLocalIP(nConn_idx, (char*)str_carve_ip.c_str());
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to connect ip:" << str_carve_ip
		<< " with connection index:" << nConn_idx, str_kernel_err_reason, false);
	
	//------�O��Ҫ݆ԃ�ă���
	m_sc2_obj.LReadBegin(nConn_idx);
	m_sc2_obj.LReadNR(nConn_idx, 23004, 2);
	m_sc2_obj.LReadEnd(nConn_idx);

	int talktime = 0;
	int nStatus = SC_CONN_STATE_DISCONNECT;
	//��һ��ʱ����ѭ���������״̬�������ʱ�һ�δ���ӳɹ����򱨴��˳�
	while (nStatus != SC_CONN_STATE_OK)
	{
		m_sc2_obj.MainProcess();
		nStatus = m_sc2_obj.GetConnectionMsg(nConn_idx, SCIF_CONNECT_STATE);
		if (SC_CONN_STATE_OK == nStatus)
		{//���ӳɹ�
			break;
		}
		//��ʱ����ʧ��
		if (talktime >= 10)
		{//�ظ������ﵽָ������
			str_kernel_err_reason = "Connect time out";
			businlog_error("%s | connection index:%d, ip:%s, err reason:%s, Status:%d"
				, __CLASS_FUNCTION__, nConn_idx, str_carve_ip.c_str(), str_kernel_err_reason.c_str(), nStatus);
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


bool CBaoyuan_Lib::disconnect(unsigned short nConn_idx, string& str_kernel_err_reason)
{
	//�ж������Ϸ���
	businlog_error_return(is_valid_conn_idx(nConn_idx, str_kernel_err_reason)
		, ("%s | Invalid conn index:%d, reason:%s", __CLASS_FUNCTION__, nConn_idx, str_kernel_err_reason.c_str()), false);
	int nRet_baoyuan = m_sc2_obj.Disconnect(nConn_idx);
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to disconnect, conn index:" 
		<< nConn_idx, str_kernel_err_reason, false);
	return true;
}

bool CBaoyuan_Lib::get_status(unsigned short nConn_idx, int& nStatus, string& str_kernel_err_reason)
{
	businlog_error_return(is_valid_conn_idx(nConn_idx, str_kernel_err_reason)
		, ("%s | err reason:%s", __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	//------�O��Ҫ݆ԃ�ă���
	m_sc2_obj.LReadBegin(nConn_idx);
	m_sc2_obj.LReadNR(nConn_idx, 23004, 2);
	m_sc2_obj.LReadEnd(nConn_idx);
	
	m_sc2_obj.MainProcess();
	nStatus = m_sc2_obj.GetConnectionMsg(nConn_idx, SCIF_CONNECT_STATE);
	businlog_error_return_err_reason(SC_CONN_STATE_OK == nStatus, __CLASS_FUNCTION__ << " | Connect is over, conn idx:" 
		<< nConn_idx << ", status now:" << nStatus, str_kernel_err_reason, false);
	return true;
}

bool CBaoyuan_Lib::confirm_task(unsigned short nConn_idx, size_t nMax_wait_time, string& str_kernel_err_reason)
{
	size_t nAddr = 20000;
	unsigned char nBit_idx = 10;
	unsigned char nBit_value = 0;
	return set_RBit(nConn_idx, nAddr, nBit_idx, nBit_value, nMax_wait_time, str_kernel_err_reason);
}

bool CBaoyuan_Lib::set_continue_status(unsigned short nConn_idx, unsigned char nStatus, unsigned short nMax_wait_time, string& str_kernel_err_reason)
{

	//		ResetBaoyuanRBit(1040000, 0);
	return set_RBit(nConn_idx, 1040000, 0, nStatus, nMax_wait_time, str_kernel_err_reason);
}

bool CBaoyuan_Lib::reset_carve(unsigned short nConn_idx, unsigned short nMax_wait_time, string& str_kernel_err_reason)
{

	// 		ret=SetBaoyuanRBit(20000, 0);
	// 		Sleep(100);
	// 		ResetBaoyuanRBit(20000, 0);
	bool bSuccess = set_RBit(nConn_idx, 20000, 0, 1, nMax_wait_time, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to set RBit, reason:%s", __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	boost::this_thread::sleep(boost::posix_time::millisec(100));
	bSuccess = set_RBit(nConn_idx, 20000, 0, 0, nMax_wait_time, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to set RBit, reason:%s", __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	return true;
}

bool CBaoyuan_Lib::pause(unsigned short nConn_idx, unsigned short nMax_wait_time, string& str_kernel_err_reason)
{
	// 		SetBaoyuanRBit(20000, 11);
	// 		Sleep(200);
	// 		ResetBaoyuanRBit(20000, 11);
	bool bSuccess = set_RBit(nConn_idx, 20000, 11, 1, nMax_wait_time, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to set RBit, reason:%s", __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	boost::this_thread::sleep(boost::posix_time::millisec(100));
	bSuccess = set_RBit(nConn_idx, 20000, 11, 0, nMax_wait_time, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to set RBit, reason:%s", __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	return true;
}

bool CBaoyuan_Lib::start(unsigned short nConn_idx, const string& str_nc_file_path, unsigned short nMax_wait_time, string& str_kernel_err_reason)
{
	// 		SetBaoyuanRString(17022,filename, 8*4);
	// 		SetBaoyuanRValue(17002, 1);
	// 		SetBaoyuanRBit(20000, 10);
	//��ȡ�ļ�������������չ����
	boost::filesystem::path path_nc_file(str_nc_file_path);
	//�ж��ļ��Ƿ����
	businlog_error_return_err_reason(boost::filesystem::exists(path_nc_file), __CLASS_FUNCTION__ 
		<< " | Can not find file:" << str_nc_file_path, str_kernel_err_reason, false);

	string str_filename_without_ext = path_nc_file.stem().string();
	bool bSuccess = set_RString(nConn_idx, 17022, str_filename_without_ext.size()
		, str_filename_without_ext.c_str(), nMax_wait_time * 3, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to set RString, file name withou extend:%s, reason:%s"
		, __CLASS_FUNCTION__, str_filename_without_ext.c_str(), str_kernel_err_reason.c_str()), false);
	bSuccess = set_RValue(nConn_idx,17002, 1, nMax_wait_time * 3, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to set RValue, reason:%s"
		, __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	bSuccess = set_RBit(nConn_idx, 20000, 10, 1, nMax_wait_time, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to set RBit, reason:%s"
		, __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	return true;
}

bool CBaoyuan_Lib::upload_1file(unsigned short nConn_idx, const string& str_file_path, string& str_kernel_err_reason)
{
	businlog_tracer_perf(CBaoyuan_Lib::upload_1file);
	//�ж������Ϸ���
	//nConn_idxֵ���С� scif_Init ��ʽ��ʼ���r��struct DLL_USE_SETTING �� ConnectNum ���O�����B����Ŀ��
	businlog_error_return(is_valid_conn_idx(nConn_idx, str_kernel_err_reason)
		, ("%s | err reason:%s", __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);

	//�����B������ͬһ���n����ݔ���ܣ���춂�ݔǰ�� FtpSetConnection ��ʽ�O�����������B��
	//�O�� FTP ����
	int nRet_baoyuan = m_sc2_obj.FtpSetConnection(nConn_idx);
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set ftp connection, conn idx:"
		<< nConn_idx, str_kernel_err_reason, false);
	//�ς�һ���n��
	boost::filesystem::path file_boost_path(str_file_path);
	//�ж��ļ��Ƿ����
	businlog_error_return_err_reason(boost::filesystem::exists(file_boost_path), __CLASS_FUNCTION__ << " | Can not find file:"
		<< str_file_path << ", conn idx:" << nConn_idx, str_kernel_err_reason, false);
	businlog_crit("%s | conn idx:%d, file name:%s, file path:%s"
		, __CLASS_FUNCTION__, nConn_idx, file_boost_path.filename().string().c_str(), str_file_path.c_str());
//	m_sc2_obj.MainProcess();
	nRet_baoyuan = m_sc2_obj.FtpUpload1File(FTP_FOLDER_NCFILES, ""
		, (char*)file_boost_path.filename().string().c_str(), (char*)str_file_path.c_str());

	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to upload file:" << str_file_path
		<< ", conn idx:" << nConn_idx, str_kernel_err_reason, false);
	//ȡ�È��нY��  ---  һ�����нY��ֻ���؂�һ��,Ȼ��ͱ����
	size_t nCost_time_ms = 0; //�ķѵ�ʱ��
	size_t nThreshold_time_ms = 2 * 60 * 60 * 1000; //ʱ����ֵ
	size_t nWait_time_ms = 50; //ÿ������ʱ��

	//�ж������ļ�ʱ����û�г�ʱ��������ѭ��
	while (true)
	{
		//����һ��
//		Sleep(50);
//		businlog_crit("%s | start to wait", __CLASS_FUNCTION__);
		boost::this_thread::sleep(boost::posix_time::millisec(nWait_time_ms));
//		businlog_crit("%s | finish to wait", __CLASS_FUNCTION__);
//		m_sc2_obj.MainProcess();
		//���ftp�����Ƿ����
		nRet_baoyuan = m_sc2_obj.FtpCheckDone();
		if ( 1 ==  nRet_baoyuan)
		{//�����Ѿ����
			//��ȡftp�������
			nRet_baoyuan = m_sc2_obj.GetLibraryMsg(SCIF_FTP_RESULT);
			//����Ƿ��ϴ��ɹ�������ϴ�ʧ�ܣ���ֱ�ӱ�����
			businlog_error_return_err_reason(FTP_RESULT_SUCCESS == nRet_baoyuan, __CLASS_FUNCTION__ << " | ftp failed, conn idx:" 
				<< nConn_idx << ", file path:" << str_file_path << ", ret:"<< nRet_baoyuan 
				<< ", ftp result note:" << strerror_ftp(nRet_baoyuan), str_kernel_err_reason, false);
			//��ʱ�����ϴ��ɹ�
			return true;
		} 
		else
		{//������δ��ɣ����ۼӺķ�ʱ��
			nCost_time_ms += nWait_time_ms;
			//�ж��Ƿ�ʱ
			businlog_error_return_err_reason(nCost_time_ms < nThreshold_time_ms, __CLASS_FUNCTION__
				<< " | timeout to upload file:" << str_file_path << ", cost time:" << nCost_time_ms 
				<< " ms, conn idx:" << nConn_idx, str_kernel_err_reason, false);
		}
	}
	//��ʱ�Ѿ��ɹ��ϴ����
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
		businlog_error("%s | timer has been already stop", __CLASS_FUNCTION__);
		return false;
	}
	m_bStop = true;
	m_thread_timer.join();
	businlog_crit("%s | stop timer successfully", __CLASS_FUNCTION__);
	return true;
	
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
							, unsigned short nMax_wait_time, string& str_err_reason)
{
	businlog_tracer_perf(CBaoyuan_Lib::set_RBit);
	//�ж������Ϸ���
	//nConn_idxֵ���С� scif_Init ��ʽ��ʼ���r��struct DLL_USE_SETTING �� ConnectNum���O�����B����Ŀ��
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

bool CBaoyuan_Lib::set_RValue(unsigned short nConn_idx, unsigned int nAddr, unsigned int nVal, unsigned short nMax_wait_time, string& str_kernel_err_reason)
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
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | Time out to do cmd, nConn_idx:" 
		<< nConn_idx, str_kernel_err_reason, false);
	return true;
}

/************************************
* Method:    set_RString
* Brief:  �����ִ����������е� R ֵ
* Access:    private 
* Returns:   bool true���ɹ���false��ʧ��
* Qualifier:
*Parameter: unsigned short nConn_idx -[in]  ��������
*Parameter: size_t nAddr -[in]  Ҫ������Y��λַ
*Parameter: size_t nBuff_size -[in]  Ҫд�����������λ�ֽ�
*Parameter: const char * pBuff -[in]  Ҫд����ַ���
*Parameter: unsigned short nMax_wait_time -[in]  ��ʱʱ�䣬��λms
*Parameter: string & str_kernel_err_reason -[out] ����ԭ�� 
************************************/
bool CBaoyuan_Lib::set_RString(unsigned short nConn_idx, size_t nAddr, size_t nBuff_size, const char* pBuff, unsigned short nMax_wait_time, string& str_kernel_err_reason)
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

bool CBaoyuan_Lib::set_CValue(unsigned short nConn_idx, int nAddr, int nValue, unsigned short nMax_wait_time, string& str_kernel_err_reason)
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
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | Time out to do cmd, nConn_idx:" 
		<< nConn_idx, str_kernel_err_reason, false);
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
}

CBaoyuan_Lib::~CBaoyuan_Lib()
{
	businlog_warn("%s | to release source", __CLASS_FUNCTION__);
	fini();
}

bool CBaoyuan_Lib::is_valid_conn_idx(unsigned short nConn_idx, string& str_kernel_err_reason)
{
	//nConn_idxֵ���С� scif_Init ��ʽ��ʼ���r��struct DLL_USE_SETTING �� ConnectNum ���O�����B����Ŀ��
	businlog_error_return_err_reason(nConn_idx >= 0 && nConn_idx < m_sDLL_setting.ConnectNum
		, __CLASS_FUNCTION__ << " | Invalid connection index:" << nConn_idx << ", must be [0," 
		<< m_sDLL_setting.ConnectNum - 1 << "]", str_kernel_err_reason, false);
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
	businlog_crit("%s | state of ftp:%d", __FUNCTION__, nState);
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
		businlog_error_return_err_reason(false, __CLASS_FUNCTION__ << " | Don't support ftp result:" << nResult_ftp
			, str_kernel_err_reason, str_kernel_err_reason);
		break;
	}
	return str_kernel_err_reason;
}

void CBaoyuan_Lib::svc()
{
	businlog_crit("%s | start thread successfully", __CLASS_FUNCTION__);
	size_t nWait_time_ms = 20; //ÿ������ʱ��
	while (false == m_bStop)
	{
		boost::this_thread::sleep(boost::posix_time::millisec(nWait_time_ms));
		businlog_crit("%s | start do MainProcess", __CLASS_FUNCTION__);
		int ret = m_sc2_obj.MainProcess();
		businlog_crit("%s | finish do MainProcess", __CLASS_FUNCTION__);
		if (false/*ret == 0*/)
		{
			businlog_error("%s | fail to MainProcess", __CLASS_FUNCTION__);
			break;
		}
	}
	businlog_warn("%s | ++++++++++++++++timer is over+++++++++++++", __CLASS_FUNCTION__);
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
