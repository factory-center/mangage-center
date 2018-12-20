/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: CBaoyuan_Lib_Tool.cpp
* @brief: 简短说明文件功能、用途 (Comment)。
* @author:	minglu2
* @version: 1.0
* @date: 2018/12/17
* 
* @see
* 
* <b>版本记录：</b><br>
* <table>
*  <tr> <th>版本	<th>日期		<th>作者	<th>备注 </tr>
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
	//参数校验
	businlog_error_return(nConnectNum >= MIN_CONNECTIONS_NUM && nConnectNum <= MAX_CONNECTIONS_NUM
		, ("%s | Invalid ConnectNum:%d, should be [1, %d].", __CLASS_FUNCTION__, nConnectNum, MAX_CONNECTIONS_NUM), false);

	m_sDLL_setting.ConnectNum = nConnectNum; //連線數目
	m_sDLL_setting.MemSizeR = MemSizeR; //一个控制器所对应的注册区域的内存大小
	int nRet_baoyuan = m_sc2_obj.LibraryInitial(&m_sDLL_setting, nMakerID, (char*)str_key.c_str());
	if (0 == nRet_baoyuan)
	{//失败
		businlog_error("%s | scif_Init failed, ret:%d", __CLASS_FUNCTION__, nRet_baoyuan);
		return false;
	} 
	if (10 == nRet_baoyuan)
	{//初始化成功但是解密功能字串失敗
		businlog_error("%s | fail to decrypt the key string when init scif, ret:%d, MakerID:%d, key:%s."
			, __CLASS_FUNCTION__, nRet_baoyuan, nMakerID, str_key.c_str());
		return false;
	}
	//设置库的日志等级:有错误才显示
	m_sc2_obj.LibrarySetDebug(2); //todo::for test
	{
		Thread_Write_Lock guard(m_rw_mutex_for_available);
		m_bAvailable = true;
	}
	return true;
}

/************************************
* Method:    fini
* Brief:  函数库如果处于可用状态，则需将其释放
* Access:    public 
* Returns:   void
* Qualifier:
************************************/
void CBaoyuan_Lib::fini()
{
	businlog_warn("%s | Notice", __CLASS_FUNCTION__);
	{
		Thread_Write_Lock guard(m_rw_mutex_for_available);
		//判定函数库的状态
		if (true == m_bAvailable)
		{//函数库属于可用状态，则需将其销毁
			m_sc2_obj.LibraryDestroy();
			m_bAvailable = false;
			businlog_crit("%s | release baoyuan lib successfully!", __CLASS_FUNCTION__);
		}
	}
}

bool CBaoyuan_Lib::create_connection(unsigned short nConn_idx, const string& str_carve_ip, string& str_kernel_err_reason)
{
	businlog_tracer_perf(CBaoyuan_Lib::create_connection);
	//判定参数合法性
	businlog_error_return_err_reason(nConn_idx >= 0 && nConn_idx < m_sDLL_setting.ConnectNum
		, __CLASS_FUNCTION__ << " | Invalid Connection index:" << nConn_idx << ", must be [0," << m_sDLL_setting.ConnectNum - 1
		<< "]", str_kernel_err_reason, false);

	int talktime=0;
	//直接輸入控制器IP進行連線
	//執行此函式成功只代表連線設定成功，有無真正建立起連線，必須呼叫 GetConnectionMsg 函式來檢查連線狀態
	int nRet_baoyuan = m_sc2_obj.ConnectLocalIP(nConn_idx, (char*)str_carve_ip.c_str());
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to connect ip:" << str_carve_ip
		<< " with connection index:" << nConn_idx, str_kernel_err_reason, false);
	
	int nStatus = SC_CONN_STATE_DISCONNECT;
	//在一定时间内循环检测连接状态，如果超时且还未连接成功，则报错退出
	while (nStatus != SC_CONN_STATE_OK)
	{
		nStatus = m_sc2_obj.GetConnectionMsg(nConn_idx, SCIF_CONNECT_STATE);
		if (SC_CONN_STATE_OK == nStatus)
		{//连接成功
			break;
		}
		//此时链接失败
		if (talktime >= 10)
		{//重复次数达到指定次数
			str_kernel_err_reason = "Connect time out";
			businlog_error("%s | connection index:%d, err reason:%s, Status:%d"
				, __CLASS_FUNCTION__, nConn_idx, str_kernel_err_reason.c_str(), nStatus);
			return false;
		}
		else
		{//链接失败，未达到指定次数
			//休眠一会儿
			Sleep(100);
			talktime++;
		}
	}
	//此时链接成功
	//设定循环命令：让命令被循环执行
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
* Brief:  将0或者1作为R值写入位址nAddr的nBitIdx位元位址
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: unsigned short nConn_idx -[in]  连接编号，其值小于总的连接数目，为-1时表示对所有连接进行操作
*Parameter: unsigned int nAddr -[in]  要寫入 R 值的位址
*Parameter: unsigned char nBitIdx -[in] 要寫入 R 值的位元位址  
*Parameter: unsigned char nBitValue -[in] 設定值，0 或 1
*Parameter: size_t nMax_wait_time -[in] 执行命令的最大等待时间（ms）
*Parameter: string & str_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::set_RBit(unsigned short nConn_idx, unsigned int nAddr, unsigned char nBitIdx, unsigned char nBitValue
							, size_t nMax_wait_time, string& str_err_reason)
{
	businlog_tracer_perf(CBaoyuan_Lib::set_RBit);
	//判定参数合法性
	//nServerIdx值必須小於 scif_Init 函式初始化時，struct DLL_USE_SETTING 中 TalkInfoNum 所設定的連線數目。
	businlog_error_return(is_valid_conn_idx(nConn_idx, str_err_reason)
		, ("%s | err reason:%s", __CLASS_FUNCTION__, str_err_reason.c_str()), false);
	//設定值只能为 0 或 1
	businlog_error_return_err_reason(0 == nBitValue || 1 == nBitValue, __CLASS_FUNCTION__ << " | Invalid BitValue:"
		<< nBitValue << ", must 0 or 1.", str_err_reason, false);
	
	//判定函数库是否可用
	{
		Thread_Read_Lock guard(m_rw_mutex_for_available);
		businlog_error_return_err_reason(m_bAvailable, __CLASS_FUNCTION__ << " | The Baoyuan Lib is unavailable"
			, str_err_reason, false);
	}

	//連續資料讀取設定
	unsigned int nAddr_for_ReadR = 0;
	unsigned int nNum_for_ReadR = 32; 
//	int nRet_baoyuan =  scif_cmd_ReadR(SC_POLLING_CMD, nConn_idx, nAddr_for_ReadR, nNum_for_ReadR);
	int nRet_baoyuan =  m_sc2_obj.LReadNR(nConn_idx, nAddr_for_ReadR, nNum_for_ReadR);
	//返回值为0，则表示设定命令失败
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to read, Conn index:" 
		<< nConn_idx << ", Addr:" << nAddr_for_ReadR << ", num:"
		<< nNum_for_ReadR << ", ret:" << nRet_baoyuan, str_err_reason, false);
	if(1 == nRet_baoyuan)
	{//該設定會被重新組合
		businlog_warn("%s | the cmd has been regrouped, conn index:%d, Addr:%d, Num:%d"
			, __CLASS_FUNCTION__, nConn_idx, nAddr_for_ReadR, nNum_for_ReadR);
	}
	
	//鏡射記憶體設定，必须在对记忆体读写前进行操作
// 	nRet_baoyuan = scif_SetMirror(nConn_idx);
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, ServerIdx:"
// 		<< nConn_idx, str_err_reason, false);
	
	//执行 R 值單個 bit 位址資料的寫入
	nRet_baoyuan = m_sc2_obj.DWrite1RBit(nConn_idx, nAddr, nBitIdx, nBitValue);
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to WriteRBit, Conn idx:" 
		<< nConn_idx << ", Addr:" << nAddr << ", BitIdx:" << nBitIdx << ", BitValue:" << nBitValue, str_err_reason, false);
	
	//讓等待先前所設定的直接命令完成後，再繼續執行下去
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time/*1000*/);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | Time out to do cmd, conn idx:" 
		<< nConn_idx, str_err_reason, false);
	return true;
}

bool CBaoyuan_Lib::set_RValue(unsigned short nConn_idx, unsigned int nAddr, unsigned int nVal, size_t nMax_wait_time, string& str_kernel_err_reason)
{
	businlog_tracer_perf(CBaoyuan_Lib::set_RValue);
	//检查参数合法性
	businlog_error_return(is_valid_conn_idx(nConn_idx, str_kernel_err_reason)
		, ("%s | error reason:%s", __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	//連續資料讀取設定
//	int nRet_baoyuan = scif_cmd_ReadR(SC_POLLING_CMD, nServer_idx, 0, 32);
	int nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 0, 32);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | fail to ReadR, server index:" << nConn_idx
		, str_kernel_err_reason, false);
	if (1 == nRet_baoyuan)
	{
		businlog_warn("%s | the cmd has been regrouped, conn index:%d"
			, __CLASS_FUNCTION__, nConn_idx);
	}
// 	nRet_baoyuan = scif_SetMirror(nConn_idx);		//鏡射記憶體設定
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, server index:" << nConn_idx
// 		, str_kernel_err_reason, false);
	//直接写入一个四字节到R值
	nRet_baoyuan = m_sc2_obj.DWrite1R(nConn_idx, nAddr, nVal);
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to WriteR, conn index:" << nConn_idx
		, str_kernel_err_reason, false);
	//讓等待先前所設定的直接命令完成後，再繼續執行下去
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | Time out to do cmd, serverIdx:" 
		<< nConn_idx, str_kernel_err_reason, false);
	return true;
}

bool CBaoyuan_Lib::set_RString(unsigned short nConn_idx, size_t nAddr, size_t nBuff_size, const char* pBuff, size_t nMax_wait_time, string& str_kernel_err_reason)
{
	businlog_tracer_perf(CBaoyuan_Lib::set_RString);
	//检查参数合法性
	businlog_error_return(is_valid_conn_idx(nConn_idx, str_kernel_err_reason)
		, ("%s | err reason:%s", __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	businlog_error_return_err_reason(NULL != pBuff, __CLASS_FUNCTION__ << " | p is NULL", str_kernel_err_reason, false);
	
	//連續資料讀取設定	
	int nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 0, nBuff_size);
	businlog_error_return_err_reason(nRet_baoyuan != 0, __CLASS_FUNCTION__ << " | fail to ReadR, conn index:" << nConn_idx
		, str_kernel_err_reason, false);
	if (1 == nRet_baoyuan)
	{
		businlog_warn("%s | the cmd has been regrouped, conn index:%d", __CLASS_FUNCTION__, nConn_idx);
	}
	
	//鏡射記憶體設定
// 	nRet_baoyuan = scif_SetMirror(nConn_idx);		
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, server index:" << nConn_idx
// 		, str_kernel_err_reason, false);
	//寫入字串到 R
	nRet_baoyuan = m_sc2_obj.DWriteRString(nConn_idx, nAddr, nBuff_size, (char*)pBuff);
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to WriteRString, conn index:" << nConn_idx
		 << ", addr:" << nAddr << ", buff size:" << nBuff_size, str_kernel_err_reason, false);
	
	//讓等待先前所設定的直接命令完成後，再繼續執行下去
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time);
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | Time out to do cmd, conn idx:" 
		<< nConn_idx, str_kernel_err_reason, false);
	return true;
}

bool CBaoyuan_Lib::set_CValue(unsigned short nConn_idx, int nAddr, int nValue, size_t nMax_wait_time, string& str_kernel_err_reason)
{
	//参数合法性判定
	businlog_error_return(is_valid_conn_idx(nConn_idx, str_kernel_err_reason), ("%s | err reason:%s", __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	//連續資料讀取設定
	int nRet_baoyuan = m_sc2_obj.LReadNC(nConn_idx, 0, 32);
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to ReadC, server index:" << nConn_idx
		, str_kernel_err_reason, false);
	if (1 == nRet_baoyuan)
	{
		businlog_warn("%s | the cmd has been regrouped, conn index:%d", __CLASS_FUNCTION__, nConn_idx);
	}
	//鏡射記憶體設定
// 	nRet_baoyuan = scif_SetMirror(nServer_idx);
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, server index:" << nServer_idx
// 		, str_kernel_err_reason, false);

	//写入單筆資料到command
	nRet_baoyuan =  m_sc2_obj.DWrite1C(nConn_idx, nAddr, nValue);		
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to WriteC, conn index:" << nConn_idx
		<< ", addr:" << nAddr << ", value:" << nValue, str_kernel_err_reason, false);
    //讓等待先前所設定的直接命令完成後，再繼續執行下去
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time);
	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | Time out to do cmd, serverIdx:" 
		<< nConn_idx, str_kernel_err_reason, false);
	return true;
}

CBaoyuan_Lib::CBaoyuan_Lib() : m_bAvailable(false)
{
	memset(&m_sDLL_setting, 0, sizeof(DLL_USE_SETTING));
	m_sDLL_setting.SoftwareType = 4;		//軟體種類   
	/*	m_sDLL_setting.TalkInfoNum = 200;			//連線數目*/
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
	//nServerIdx值必須小於 scif_Init 函式初始化時，struct DLL_USE_SETTING 中 TalkInfoNum 所設定的連線數目。
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
