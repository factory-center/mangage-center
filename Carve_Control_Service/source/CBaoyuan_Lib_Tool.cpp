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
const int ADDR_CARVE_COMPLETED = 500; //雕刻结束状态地址，A500
//累计加工时间相关
const int ADDR_R_TOTAL_TIME_1th_MS = 82120; //第一路径累计加工时间毫秒值对应的地址
const int ADDR_R_TOTAL_TIME_2th_MS = ADDR_R_TOTAL_TIME_1th_MS + 1; //第二路径累计加工时间毫秒值对应地址
const int ADDR_R_TOTAL_TIME_1th_HOUR = 82126;//第一路径累计加工时间小时值对应地址， R82126
const int ADDR_R_TOTAL_TIME_2th_HOUR = ADDR_R_TOTAL_TIME_1th_HOUR + 1;//第二组累计时间小时值对应地址
//单次加工时间
const int ADDR_R_SINGLE_ENGRAVE_TIME_1th_MS = 82108; //第一路径单次加工时间毫秒值对应的地址
const int ADDR_R_SINGLE_ENGRAVE_TIME_1th_HOUR = 82114; //第一路径单次加工时间小时对应的地址
CBaoyuan_Lib* CBaoyuan_Lib::instance()
{
	return ms_pInstance;
}

/************************************
* Method:    init
* Brief:  函数说明
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: int nMakerID -[in/out]  
*Parameter: const string & str_key -[in/out]  
*Parameter: unsigned int nConnectNum -[in/out]  
*Parameter: unsigned int MemSizeR -[in]  此值决定了操作R位址值的范围，小了可能导致读取不到，大了又占用很大内存
                                    R值占用内存：MemSizeR*4B
************************************/
bool CBaoyuan_Lib::init(int nMakerID, const string& str_key, unsigned int nConnectNum, unsigned int MemSizeR /*= 4000000*/)
{
	LTrace("CBaoyuan_Lib::init");
	//参数校验
	if (!(nConnectNum >= MIN_CONNECTIONS_NUM && nConnectNum <= MAX_CONNECTIONS_NUM))
	{
		LError("Invalid ConnectNum : %d, should be[%d, %d]", nConnectNum, MIN_CONNECTIONS_NUM, MAX_CONNECTIONS_NUM);
		return false;
	}

	m_sDLL_setting.ConnectNum = nConnectNum; //連線數目
	m_sDLL_setting.MemSizeR = MemSizeR; //一个控制器所对应的注册区域的内存大小
	int nRet_baoyuan = m_sc2_obj.LibraryInitial(&m_sDLL_setting, nMakerID, (char*)str_key.c_str());
	if (0 == nRet_baoyuan)
	{//失败
		LError("scif_Init failed, ret:{}", nRet_baoyuan);
		return false;
	} 
	if (10 == nRet_baoyuan)
	{//初始化成功但是解密功能字串失敗
		LError("fail to decrypt the key string when init scif, ret:{}, MakerID:{}, key:{}", nRet_baoyuan, nMakerID, str_key);
		return false;
	}
	//设置库的日志等级:有错误才显示
	m_sc2_obj.LibrarySetDebug(2); //todo::for test
	string str_err_reason;
	//启动定时器
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
* Brief:  函数库如果处于可用状态，则需将其释放
* Access:    public 
* Returns:   void
* Qualifier:
************************************/
void CBaoyuan_Lib::fini()
{
	LWarn("{} | Notice", __CLASS_FUNCTION__);
	{
		//先停止线程后释放函数库
		if (false == m_bStop)
		{//线程仍然开始着
			stop_timer();
		}

		Thread_Write_Lock guard(m_rw_mutex_for_available);
		//判定函数库的状态
		if (true == m_bAvailable)
		{//函数库属于可用状态，则需将其销毁
			m_sc2_obj.LibraryDestroy();
			m_bAvailable = false;
			LCritical("release baoyuan lib successfully!");
		}
	}
}

/************************************
* Method:    create_connection
* Brief:  连接某个雕刻机
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: const Json::Value & json_conn_value -[in]  含有键：连接号和ip地址
*Parameter: string & str_kernel_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::create_connection(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{

	LTrace("CBaoyuan_Lib::create_connection");
	//判定参数合法性
	if (json_conn_value.isMember(CCarve::ms_str_conn_idx_key) == false)
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ",without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数中不含有键:" + CCarve::ms_str_conn_idx_key;
		return false;
	}
	if (json_conn_value.isMember(CCarve::ms_str_ip_key) == false)
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ",without key:" + CCarve::ms_str_ip_key;
		str_err_reason_for_user = "参数中不含有键:" + CCarve::ms_str_ip_key;
		return false;
	}


	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	const string str_carve_ip = json_conn_value[CCarve::ms_str_ip_key].asString(); 

	if (!(nConn_idx >= 0 && nConn_idx < (int)m_sDLL_setting.ConnectNum))
	{
		LError("Invalid Connection index:{}, must be [0,{}]", nConn_idx, m_sDLL_setting.ConnectNum - 1);
		str_err_reason_for_debug = "Invalid Connection index:" + std::to_string(nConn_idx) + ",must be [0," + std::to_string(m_sDLL_setting.ConnectNum - 1) + "]";
		str_err_reason_for_user = "连接索引非法，必须属于[0," + CCarve::ms_str_ip_key + "]";
		return false;
	}

	
	//直接輸入控制器IP進行連線
	//執行此函式成功只代表連線設定成功，有無真正建立起連線，必須呼叫 GetConnectionMsg 函式來檢查連線狀態
	int nRet_baoyuan = m_sc2_obj.ConnectLocalIP(nConn_idx, (char*)str_carve_ip.c_str());
	if (0 == nRet_baoyuan)
	{
		LError("fail to connect ip:{}", str_carve_ip);
		str_err_reason_for_debug = "fail to connect ip:" + str_carve_ip + ",with connection index:" + std::to_string(nConn_idx);
		str_err_reason_for_user = "连接失败，请检查设备网络";
		return false;
	}
	
	//------設定要輪詢的內容
	m_sc2_obj.LReadBegin(nConn_idx);
	m_sc2_obj.LReadNR(nConn_idx, 23004, 2);
	m_sc2_obj.LReadEnd(nConn_idx);

	int talktime = 0;
	int nStatus = SC_CONN_STATE_DISCONNECT;
	//在一定时间内循环检测连接状态，如果超时且还未连接成功，则报错退出
	while (nStatus != SC_CONN_STATE_OK)
	{
//		m_sc2_obj.MainProcess(); //是否需要屏蔽？
		nStatus = m_sc2_obj.GetConnectionMsg(nConn_idx, SCIF_CONNECT_STATE);
		if (SC_CONN_STATE_OK == nStatus)
		{//连接成功
			break;
		}
		//此时链接失败
		if (talktime >= 10)
		{//重复次数达到指定次数
			LError("Connect time out, ip:{}, conn idx:{}, carve status:{}", str_carve_ip, nConn_idx, nStatus);
			str_err_reason_for_debug = "Connect time out, ip:" + str_carve_ip + ", conn idx:" + std::to_string(nConn_idx) + "carve status :" + std::to_string(nStatus);
			str_err_reason_for_user = "连接设备超时";
			return false;
		}
		else
		{//链接失败，未达到指定次数
			//休眠一会儿
//			Sleep(50);
			boost::this_thread::sleep(boost::posix_time::millisec(50));
			talktime++;
		}
	}
	//此时链接成功
	//设定循环命令：让命令被循环执行
	m_sc2_obj.LClearQueue(nConn_idx);
	m_sc2_obj.LReadBegin(nConn_idx);
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 17003, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "设置雕刻机命令失败";
		return false;
	}
	
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 17034, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "设置雕刻机命令失败";
		return false;
	}
	
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 29000, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "设置雕刻机命令失败";
		return false;
	}
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 29001, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "设置雕刻机命令失败";
		return false;
	}
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 29002, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "设置雕刻机命令失败";
		return false;
	}
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 29003, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "设置雕刻机命令失败";
		return false;
	}
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 1040000, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "设置雕刻机命令失败";
		return false;
	}
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 3006072, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "设置雕刻机命令失败";
		return false;
	}
	//设置循环命令读取第一路径累计加工时间对应的毫秒
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, ADDR_R_TOTAL_TIME_1th_MS, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "设置雕刻机命令失败";
		return false;
	}
	//设置循环命令读取第一路径累计加工时间对应的小时
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, ADDR_R_TOTAL_TIME_1th_HOUR, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "设置雕刻机命令失败";
		return false;
	}
	//设置循环命令读取雕刻是否完成
	nRet_baoyuan = m_sc2_obj.LReadNA(nConn_idx, ADDR_CARVE_COMPLETED, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "设置雕刻机命令失败";
		return false;
	}
	//设置循环命令读取单次加工时间
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, ADDR_R_SINGLE_ENGRAVE_TIME_1th_MS, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "设置雕刻机命令失败";
		return false;
	}
	nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, ADDR_R_SINGLE_ENGRAVE_TIME_1th_HOUR, 1);
	if (0 == nRet_baoyuan)
	{
		LError("fail to read");
		str_err_reason_for_debug = "fail to read, line:" + std::to_string(__LINE__);
		str_err_reason_for_user = "设置雕刻机命令失败";
		return false;
	}

	m_sc2_obj.LReadEnd(nConn_idx);
	return true;
}


/************************************
* Method:    disconnect
* Brief:  断开某个雕刻机
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: const Json::Value & json_conn_value -[in]  含有键：连线号
*Parameter: string & str_kernel_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::disconnect(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//判定参数合法性
	if (!(json_conn_value.isMember(CCarve::ms_str_conn_idx_key)))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数中不含有键"+ CCarve::ms_str_conn_idx_key;
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
		str_err_reason_for_user = "断开雕刻机失败";
		return false;
	}
	return true;
}

/************************************
* Method:    get_connect_status
* Brief:  获取某个雕刻机的状态
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: const Json::Value & json_conn_value -[in]  某个雕刻机对应的连线号
*Parameter: int & nStatus -[in/out]  
*Parameter: string & str_kernel_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::get_connect_status(const Json::Value& json_conn_value, int& nStatus, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//判定参数合法性
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}



	//获取连线索引
	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}
	//获取连接状态
	nStatus = m_sc2_obj.GetConnectionMsg(nConn_idx, SCIF_CONNECT_STATE);
	return true;
}

/************************************
* Method:    get_connect_status
* Brief:  获取某个雕刻机状态
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: int nConn_idx -[in] 连接索引，不能为-1
*Parameter: int & nStatus -[out]  连接状态
*Parameter: string & str_err_reason_for_debug -[out]  
*Parameter: string & str_err_reason_for_user -[out]  
************************************/
bool CBaoyuan_Lib::get_connect_status(int nConn_idx, int& nStatus, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//判定参数合法性
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}
	//获取连接状态
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
	//判定参数合法性
	//判定是否含有conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}

	//判定是否含有状态
	if (!json_conn_value.isMember(CCarve::ms_str_status_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_status_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}

	//判定是否含有最大超时时间
	if (!json_conn_value.isMember(CCarve::ms_str_max_wait_time_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_max_wait_time_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_max_wait_time_key;
		str_err_reason_for_user = "参数错误";
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
	//判定是否含有conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}

	//判定是否含有最大超时时间
	if (!json_conn_value.isMember(CCarve::ms_str_max_wait_time_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_max_wait_time_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_max_wait_time_key;
		str_err_reason_for_user = "参数错误";
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

	//判定是否含有conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}


	//判定是否含有最大超时时间
	if (!json_conn_value.isMember(CCarve::ms_str_max_wait_time_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_max_wait_time_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_max_wait_time_key;
		str_err_reason_for_user = "参数错误";
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
	//判定是否含有conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}

	//判定是否含有文件路径
	if (!json_conn_value.isMember(CCarve::ms_str_file_path_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_file_path_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_file_path_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}


	//判定是否含有最大超时时间
	if (!json_conn_value.isMember(CCarve::ms_str_max_wait_time_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_max_wait_time_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_max_wait_time_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}


	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	const string& str_nc_file_path = json_conn_value[CCarve::ms_str_file_path_key].asString();
	unsigned short nMax_wait_time = json_conn_value[CCarve::ms_str_max_wait_time_key].asInt();

	//获取文件名（不含有扩展名）
	boost::filesystem::path path_nc_file(str_nc_file_path);
	//判定文件是否存在
	if (!boost::filesystem::exists(path_nc_file))
	{
		LError("Can not find file:{}", str_nc_file_path);
		str_err_reason_for_debug = "Can not find file:" + str_nc_file_path;
		str_err_reason_for_user = "没找到指定文件";
		return false;
	}

	string str_filename = path_nc_file.filename().string();
	//设置待雕刻文件名
	bool bSuccess = set_RString(nConn_idx, 17022, str_filename.size()
		, str_filename.c_str(), nMax_wait_time * 3, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to set RString, file name:{}, reason:{}", str_filename, str_err_reason_for_debug);
		return false;
	}

	//设置模式：0为手动模式；1为自动模式
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

	//启动加工
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
	//判定是否含有conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}

	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();

	//nConn_idx值必須小於 scif_Init 函式初始化時，struct DLL_USE_SETTING 中 ConnectNum 所設定的連線數目。
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
	//当状态为Ready时，需要再次判定：细分为就绪太还是雕刻完成态
	if (nCarve_ready == nCarve_status)
	{
		
		//读取A500的值，判定其是否为1，为1，则为雕刻完成
		int nValue = m_sc2_obj.memA(nConn_idx, ADDR_CARVE_COMPLETED);
		if (1 == nValue)
	    {//为雕刻完成态
			nCarve_status =  5;
			//将ADDR_CARVE_COMPLETED状态设置为0
			m_sc2_obj.DWrite1A(nConn_idx, ADDR_CARVE_COMPLETED, 0);
	    }
	}
	return true;
}

/************************************
* Method:    stop_fast
* Brief:  急停
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: const Json::Value & json_conn_value -[in/out]  
*Parameter: string & str_kernel_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::stop_fast(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//判定是否含有conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}

	if (!json_conn_value.isMember(CCarve::ms_str_max_wait_time_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_max_wait_time_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_max_wait_time_key;
		str_err_reason_for_user = "参数错误";
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
	//判定是否含有conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}
	if (!json_conn_value.isMember(CCarve::ms_str_max_wait_time_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_max_wait_time_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_max_wait_time_key;
		str_err_reason_for_user = "参数错误";
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
	//判定参数合法性
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
		str_err_reason_for_user = "设备连接异常";
		return false;
	}
	return true;
}

bool CBaoyuan_Lib::is_connected(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//判定参数合法性
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
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
		str_err_reason_for_user = "设备连接异常";
		return false;
	}
	return true;
}

bool CBaoyuan_Lib::get_current_line_num(const Json::Value& json_conn_value, int& nCurrent_line_num, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::get_current_line_num");
	//判定是否含有conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}

	//获取连接索引
	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}

	//读取指定位置数据以获取当前文件的行号
	return get_RValue(nConn_idx, 3006072, nCurrent_line_num, str_err_reason_for_debug, str_err_reason_for_user);
}

/************************************
* Method:    get_engraving_time
* Brief:  从雕刻机中获取总的加工时间和单次加工时间
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
	//判定是否含有conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}

	//获取连接索引
	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	//判定索引合法性
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}

	//读取累计加工对应的毫秒
	int nTime_ms = 0;
	bool bSuccess  = get_RValue(nConn_idx, ADDR_R_TOTAL_TIME_1th_MS, nTime_ms, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("ail to get RValue, reason:{}", str_err_reason_for_debug);
		return false;
	}

	//读取累计加工时间对应的小时
	int nTime_hour = 0;
	bSuccess = get_RValue(nConn_idx, ADDR_R_TOTAL_TIME_1th_HOUR, nTime_hour, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("ail to get RValue, reason:{}", str_err_reason_for_debug);
		return false;
	}
	//将毫秒和小时换算为分钟
	nTotal_engraving_time_minute = int(nTime_ms * 1.0 / (1000 * 60) + nTime_hour * 60);
	nTime_ms = 0;
	nTime_hour = 0;
	//读取单次加工时间对应的毫秒
	bSuccess = get_RValue(nConn_idx, ADDR_R_SINGLE_ENGRAVE_TIME_1th_MS, nTime_ms, str_err_reason_for_debug, str_err_reason_for_debug);
	if (bSuccess == false)
	{
		LError("ail to get RValue, reason:{}", str_err_reason_for_debug);
		return false;
	}

	//读取单次加工时间对应的小时
	bSuccess = get_RValue(nConn_idx, ADDR_R_SINGLE_ENGRAVE_TIME_1th_HOUR, nTime_hour, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("ail to get RValue, reason:{}", str_err_reason_for_debug);
		return false;
	}

	//将小时、毫秒转换为分钟
	nSingle_engraving_time_minute = int(nTime_ms * 1.0 / (1000 * 60) + nTime_hour * 60);
	return true;
}

bool CBaoyuan_Lib::upload_1file(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::upload_1file");
	//判定参数合法性
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}
	if (!json_conn_value.isMember(CCarve::ms_str_file_path_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_file_path_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_file_path_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}


	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	const string& str_file_path = json_conn_value[CCarve::ms_str_file_path_key].asString();

	//nConn_idx值必須小於 scif_Init 函式初始化時，struct DLL_USE_SETTING 中 ConnectNum 所設定的連線數目。
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason::{}", str_err_reason_for_debug);
		return false;
	}


	//判定连接状态
	bool bSuccess = is_connected(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("Connection is over, json info:{}, reason:{}", json_conn_value.toStyledString(), str_err_reason_for_debug);
		return false;
	}


	//所有連線共用同一個檔案傳輸功能，需於傳輸前用 FtpSetConnection 函式設定所對應的連。
	//設定 FTP 索引
	int nRet_baoyuan = m_sc2_obj.FtpSetConnection(nConn_idx);
	if (nRet_baoyuan == 0)
	{
		LError("fail to set ftp connection, conn idx:{}", nConn_idx);
		str_err_reason_for_debug = "fail to set ftp connection, conn idx:" + std::to_string(nConn_idx);
		str_err_reason_for_user = "操作设备失败";
		return false;
	}

	//上傳一個檔案
	boost::filesystem::path file_boost_path(str_file_path);
	//判定文件是否存在
	if (!boost::filesystem::exists(file_boost_path))
	{
		LError("Can not find file::{}, conn idx:{}", str_file_path, nConn_idx);
		str_err_reason_for_debug = "Can not find file:" + str_file_path + ",conn idx :" + std::to_string(nConn_idx);
		str_err_reason_for_user = "无法找到指定文件";
		return false;
	}

	nRet_baoyuan = m_sc2_obj.FtpUpload1File(FTP_FOLDER_RUN_NCFILES, ""
		, (char*)file_boost_path.filename().string().c_str(), (char*)str_file_path.c_str());

	if (nRet_baoyuan == 0)
	{
		LError("fail to upload file:{}, conn idx:{}", str_file_path, nConn_idx);
		str_err_reason_for_debug = "fail to upload file:" + str_file_path + ",conn idx :" + std::to_string(nConn_idx);
		str_err_reason_for_user = "上传文件失败";
		return false;
	}
	//取得執行結果  ---  一個執行結果只會回傳一次,然後就被清除
	size_t nCost_time_ms = 0; //耗费的时间
	size_t nThreshold_time_ms = 2 * 60 * 60 * 1000; //时间阈值
	size_t nWait_time_ms = 50; //每次休眠时间

	//判定发送文件时，有没有超时，避免死循环
	while (true)
	{
		//休眠一会
		boost::this_thread::sleep(boost::posix_time::millisec(nWait_time_ms));
		//检测ftp操作是否完成
		nRet_baoyuan = m_sc2_obj.FtpCheckDone();
		if ( 1 ==  nRet_baoyuan)
		{//动作已经完成
			//获取ftp操作结果
			nRet_baoyuan = m_sc2_obj.GetLibraryMsg(SCIF_FTP_RESULT);
			//检测是否上传成功，如果上传失败，则直接报错返回
			if (!(FTP_RESULT_SUCCESS == nRet_baoyuan))
			{
				LError("ftp failed ,file:{}, conn idx:{},ret:{},ftp result note:{}",
					str_file_path, nConn_idx, nRet_baoyuan, strerror_ftp(nRet_baoyuan));
				str_err_reason_for_debug = "ftp failed, file:" + str_file_path + ", conn idx :" + std::to_string(nConn_idx) +
					"ret :" + std::to_string(nRet_baoyuan) + " ftp result note :" + strerror_ftp(nRet_baoyuan);
				str_err_reason_for_user = "通过ftp上传文件到设备失败";
				return false;
			}
			//此时表明上传成功
			return true;
		} 
		else
		{//动作还未完成，则累加耗费时间
			nCost_time_ms += nWait_time_ms;
			//判定是否超时
			if (!(nCost_time_ms < nThreshold_time_ms))
			{
				LError("timeout to upload file:{}, cost time:{}ms, conn idx:{}",str_file_path, nCost_time_ms, nConn_idx);
				str_err_reason_for_debug = "timeout to upload file:" + str_file_path + ", cost time :" + 
					std::to_string(nCost_time_ms) + ",conn idx :" + std::to_string(nConn_idx);
				str_err_reason_for_user = "上传文件超时";
				return false;
			}
		}
	}
	//此时已经成功上传完成
	return true;
}

bool CBaoyuan_Lib::delete_1file(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::delete_1file");
	//判定参数合法性
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key :" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}

	if (!json_conn_value.isMember(CCarve::ms_str_file_path_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_file_path_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key :" + CCarve::ms_str_file_path_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}


	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	const string& str_file_path = json_conn_value[CCarve::ms_str_file_path_key].asString();

	//nConn_idx值必須小於 scif_Init 函式初始化時，struct DLL_USE_SETTING 中 ConnectNum 所設定的連線數目。
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:", str_err_reason_for_debug);
		return false;
	}

	//查询连接状态
	bool bSuccess = is_connected(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("Connection is over, json info:{}, reason:{}", json_conn_value.toStyledString(), str_err_reason_for_debug);
		return false;
	}

	//所有連線共用同一個檔案傳輸功能，需於傳輸前用 FtpSetConnection 函式設定所對應的连接
	//設定 FTP 索引
	int nRet_baoyuan = m_sc2_obj.FtpSetConnection(nConn_idx);
	if (nRet_baoyuan == 0)
	{
		LError("fail to set ftp connection, conn idx:{}", nConn_idx);
		str_err_reason_for_debug = "fail to set ftp connection, conn idx:" + std::to_string(nConn_idx);
		str_err_reason_for_user = "操作设备失败";
		return false;
	}
	//上傳一個檔案
	boost::filesystem::path file_boost_path(str_file_path);
	LInfo("conn idx:{}, file name:{}, file path:{}", nConn_idx, file_boost_path.filename().string(), str_file_path);
	
	nRet_baoyuan = m_sc2_obj.FtpDelete1File(FTP_FOLDER_RUN_NCFILES, "", (char*)file_boost_path.filename().string().c_str());
	if (nRet_baoyuan == 0)
	{
		LError("fail to delete file:{}, conn idx:{}", str_file_path, nConn_idx);
		str_err_reason_for_debug = "fail to delete file:" + str_file_path + ", conn idx:" + std::to_string(nConn_idx);
		str_err_reason_for_user = "删除设备中的文件失败";
		return false;
	}

	//取得執行結果  ---  一個執行結果只會回傳一次,然後就被清除
	size_t nCost_time_ms = 0; //耗费的时间
	size_t nThreshold_time_ms = 2 * 60 * 60 * 1000; //时间阈值
	size_t nWait_time_ms = 50; //每次休眠时间

	//判定发送文件时，有没有超时，避免死循环
	while (true)
	{
		//休眠一会
		boost::this_thread::sleep(boost::posix_time::millisec(nWait_time_ms));
		//检测ftp操作是否完成
		nRet_baoyuan = m_sc2_obj.FtpCheckDone();
		if ( 1 ==  nRet_baoyuan)
		{//动作已经完成
			//获取ftp操作结果
			nRet_baoyuan = m_sc2_obj.GetLibraryMsg(SCIF_FTP_RESULT);
			//检测是否操作成功，如果操作失败，则直接报错返回
			if (nRet_baoyuan == 0)
			{
				LError("ftp failed,conn idx:{},file name:{}, ret:{}, ftp result note:{}", nConn_idx, file_boost_path.filename().string(), nRet_baoyuan, strerror_ftp(nRet_baoyuan));
				str_err_reason_for_debug = "ftp failed,conn idx:" + std::to_string(nConn_idx) + ", file name:" + file_boost_path.filename().string()
					+ ",ret:" + std::to_string(nRet_baoyuan) + ",ftp result note:" + strerror_ftp(nRet_baoyuan);
				str_err_reason_for_user = "删除设备中的文件失败";
				return false;
			}

			//此时表明操作成功
			return true;
		} 
		else
		{//动作还未完成，则累加耗费时间
			nCost_time_ms += nWait_time_ms;
			//判定是否超时
			if (!(nCost_time_ms < nThreshold_time_ms))
			{
				LError("timeout to delete file:{}, cost time:{}ms, conn idx:{}", file_boost_path.filename().string(), nCost_time_ms, nConn_idx);
				str_err_reason_for_debug = "timeout to delete file:" + file_boost_path.filename().string() + ", cost time:"
					+ std::to_string(nCost_time_ms) + "ms, conn idx:" + std::to_string(nConn_idx);
				str_err_reason_for_user = "删除设备中的文件超时";
				return false;
			}
		}
	}
	//此时已经操作成功
	return true;
}

bool CBaoyuan_Lib::adjust_speed(const Json::Value& json_conn_value,string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::adjust_speed");
	//判定参数合法性
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key :" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}

	const string& str_key = "speed_percent";	
	if (!json_conn_value.isMember(str_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), str_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key :" + str_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}

	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();

	unsigned int n_speed_percent = json_conn_value[str_key].asInt();
	unsigned short nMax_wait_time = json_conn_value[CCarve::ms_str_max_wait_time_key].asInt();

	//nConn_idx值必須小於 scif_Init 函式初始化時，struct DLL_USE_SETTING 中 ConnectNum 所設定的連線數目。
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}

	//查询连接状态
	bool bSuccess = is_connected(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("Connection is over, json info:{}, reason:{}", json_conn_value.toStyledString(),str_err_reason_for_debug);
		return false;
	}

	//设置速度
	bSuccess = set_RValue(nConn_idx,17000, n_speed_percent*10000, nMax_wait_time * 3, str_err_reason_for_debug, str_err_reason_for_user);
	return bSuccess;
}
	


//程式再启动
bool CBaoyuan_Lib::program_restart(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//判定是否含有conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key :" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}

	//判定是否含有最大超时时间
	if (!json_conn_value.isMember(CCarve::ms_str_max_wait_time_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_max_wait_time_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key :" + CCarve::ms_str_max_wait_time_key;
		str_err_reason_for_user = "参数错误";
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
	case 0://未就绪
		str_carve_status_description += "Not Ready";
		break;
	case 1: //已经就绪
		str_carve_status_description += "Ready";
		break;
	case 2: //Cycle Start:雕刻中
		str_carve_status_description += "Engraving";
		break;
	case 3: //Feed Hold : 加工暂停
		str_carve_status_description += "Pause";
		break;
	case 4: //Block Stop:区段停止，運行一半，發生錯誤時，會是這個狀態
		str_carve_status_description += "Error";
		break;
	case 5: //completed：雕刻完成（自定义的）
		str_carve_status_description += "Completed";
		break;
	default:
		LError("jnvalid status value:{}, must be [0, 5]", nCarve_status);
		str_err_reason_for_debug = "jnvalid status value:" + std::to_string(nCarve_status) + ", must be [0, 5]";
		str_err_reason_for_user = "参数错误";
		return false;
	}
	return true;
}

/************************************
* Method:    acquire_conn_idx
* Brief:  申请一个空闲可用的连接索引
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
		{//找到一个空闲的索引
			nConn_idx = i;
			m_vec_conn_idx_used[i] = true;
			return true;
		}
	}
	//此时查找一圈都没有找到空闲的，即管理的设备过多，则出错
	LError("The num of carves is lager than {}", MAX_CONNECTIONS_NUM);
	str_err_reason_for_debug = "The num of carves is lager than " + std::to_string(MAX_CONNECTIONS_NUM);
	str_err_reason_for_user = "超过宝元所支持的设备数目，宝元最多支持设备数目为:"+ std::to_string(MAX_CONNECTIONS_NUM);
	return false;
}

/************************************
* Method:    release_conn_idx
* Brief:  归还之前申请的连接索引
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: int nConn_idx -[in/out]  
*Parameter: string & str_kernel_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::release_conn_idx(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//判定是否含有conn idx
	if (!json_conn_value.isMember(CCarve::ms_str_conn_idx_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_conn_idx_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key :" + CCarve::ms_str_conn_idx_key;
		str_err_reason_for_user = "参数错误";
		return false;
	}

	int nConn_idx = json_conn_value[CCarve::ms_str_conn_idx_key].asInt();
	//判定参数合法性
	if (!(nConn_idx > -1 && nConn_idx < MAX_CONNECTIONS_NUM))
	{
		LError("Invalid conn idx:{}, should be [0,{}]", nConn_idx, MAX_CONNECTIONS_NUM);
		str_err_reason_for_debug = "Invalid conn idx:" + std::to_string(nConn_idx) + ", should be [0," + std::to_string(MAX_CONNECTIONS_NUM) + "]";
		str_err_reason_for_user = "参数错误";
		return false;
	}


	boost::mutex::scoped_lock guard(m_mutex_conn_idx);
	//判定索引当前是否处于忙碌状态
	if (!m_vec_conn_idx_used[nConn_idx])
	{
		LError("conn idx:{} is already idle", nConn_idx);
		str_err_reason_for_debug = "conn idx:{}" + std::to_string(nConn_idx) + " is already idle";
		str_err_reason_for_user = "参数错误";
		return false;
	}
	m_vec_conn_idx_used[nConn_idx] = false;
	return true;
}

bool CBaoyuan_Lib::start_timer(string& str_kernel_err_reason)
{
	m_bStop = false;
	m_thread_timer = boost::thread(boost::bind(&CBaoyuan_Lib::svc, this));
	//不关心退出状态，则可以线程分离
//	m_thread_timer.detach();
	return true;
}

bool CBaoyuan_Lib::stop_timer()
{
	if (true == m_bStop)
	{//之前已经停止了
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
* Brief:  判定参数合法性，如果合法，则将0或者1作为R值写入位址nAddr的nBitIdx位元位址
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: int nConn_idx -[in]  连接编号，其值小于总的连接数目，为-1时表示对所有连接进行操作
*Parameter: unsigned int nAddr -[in]  要寫入 R 值的位址 
*Parameter: unsigned char nBitIdx -[in] 要寫入 R 值的位元位址  [0, 31]
*Parameter: unsigned char nBitValue -[in] 設定值，0 或 1
*Parameter: size_t nMax_wait_time -[in] 执行命令的最大等待时间（ms）
*Parameter: string & str_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::set_RBit(int nConn_idx, unsigned int nAddr, unsigned char nBitIdx, unsigned char nBitValue
							, unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::set_RBit");
	//判定参数合法性
	//nConn_idx值必須小於 scif_Init 函式初始化時，struct DLL_USE_SETTING 中 ConnectNum所設定的連線數目。
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}
	//設定值只能为 0 或 1
	if (!(0 == nBitValue || 1 == nBitValue))
	{
		LError("Invalid BitValue:{}, must 0 or 1.", nBitValue);
		str_err_reason_for_debug = "Invalid BitValue:" + std::to_string(nBitValue) + ", must 0 or 1.";
		str_err_reason_for_user = "参数错误，操作设备失败";
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

	//判定函数库是否可用
	{
		Thread_Read_Lock guard(m_rw_mutex_for_available);
		if (!m_bAvailable)
		{
			LError("The Baoyuan Lib is unavailable:{}", str_err_reason_for_debug);
			str_err_reason_for_debug = "The Baoyuan Lib is unavailable:" + str_err_reason_for_debug;
			str_err_reason_for_user = "参数错误，操作设备失败";
			return false;
		}
	}

	//連續資料讀取設定
	unsigned int nAddr_for_ReadR = 0;
	unsigned int nNum_for_ReadR = 32; 
//	int nRet_baoyuan =  scif_cmd_ReadR(SC_POLLING_CMD, nConn_idx, nAddr_for_ReadR, nNum_for_ReadR);
	int nRet_baoyuan =  m_sc2_obj.LReadNR(nConn_idx, nAddr_for_ReadR, nNum_for_ReadR);
	//返回值为0，则表示设定命令失败
	if (nRet_baoyuan == 0)
	{
		LError("fail to read, Conn index:{}, Addr:{}, num:{}, ret:{}", nConn_idx, nAddr_for_ReadR, nNum_for_ReadR, nRet_baoyuan);
		str_err_reason_for_debug = "fail to read, Conn index:" + std::to_string(nConn_idx) + ", Addr:"
			+ std::to_string(nAddr_for_ReadR) + ", num:" + std::to_string(nNum_for_ReadR) + ", ret:" + std::to_string(nRet_baoyuan);
		str_err_reason_for_user = "操作设备失败";
		return false;
	}

	if(1 == nRet_baoyuan)
	{//該設定會被重新組合
		LWarn("the cmd has been regrouped, conn index:{}, Addr:{}, Num:{}", nConn_idx, nAddr_for_ReadR, nNum_for_ReadR);
	}
	
	//鏡射記憶體設定，必须在对记忆体读写前进行操作
// 	nRet_baoyuan = scif_SetMirror(nConn_idx);
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, ServerIdx:"
// 		<< nConn_idx, str_err_reason, false);
	
	//执行 R 值單個 bit 位址資料的寫入
	nRet_baoyuan = m_sc2_obj.DWrite1RBit(nConn_idx, nAddr, nBitIdx, nBitValue);
	if (nRet_baoyuan == 0)
	{
		LError("fail to WriteRBit, Conn index:{}, Addr:{}, BitIdx:{}, BitValue:{}", nConn_idx, nAddr, nBitIdx, nBitValue);
		str_err_reason_for_debug = "fail to WriteRBit, Conn index:" + std::to_string(nConn_idx) + ", Addr:"
			+ std::to_string(nAddr) + ", BitIdx:" + std::to_string(nBitIdx) + ", BitValue:" + std::to_string(nBitValue);
		str_err_reason_for_user = "操作设备失败";
		return false;
	}
	//讓等待先前所設定的直接命令完成後，再繼續執行下去
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time/*1000*/);
	if (nRet_baoyuan == 0)
	{
		LError("Time out to do cmd, conn idx:{}, Max wait time(ms):{}", nConn_idx, nMax_wait_time);
		str_err_reason_for_debug = "Time out to do cmd, conn idx:" + std::to_string(nConn_idx) + ", Max wait time(ms):" + std::to_string(nMax_wait_time);
		str_err_reason_for_user = "操作设备超时";
		return false;
	}
	return true;
}

/************************************
* Method:    set_RValue
* Brief:  写入一无符号整数到指定的R值中
* Access:    private 
* Returns:   bool ture:成功；false:失败
* Qualifier:
*Parameter: int nConn_idx -[in] 连线索引 
*Parameter: unsigned int nAddr -[in]  要写入的地址
*Parameter: unsigned int nVal -[in]  要写入的值
*Parameter: unsigned short nMax_wait_time -[in] 最大等待时间，单位mm 
*Parameter: string & str_kernel_err_reason -[out]  
************************************/
bool CBaoyuan_Lib::set_RValue(int nConn_idx, unsigned int nAddr, unsigned int nVal, unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::set_RValue");
	//检查参数合法性
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

	//連續資料讀取設定
//	int nRet_baoyuan = scif_cmd_ReadR(SC_POLLING_CMD, nServer_idx, 0, 32);
	int nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 0, 32);

	if (nRet_baoyuan==0)
	{
		LError("fail to ReadR, server index:{}", nConn_idx);
		str_err_reason_for_debug = "fail to ReadR, server index:" + std::to_string(nConn_idx);
		str_err_reason_for_user = "操作设备失败";
		return false;
	}
	if (1 == nRet_baoyuan)
	{
		LWarn("the cmd has been regrouped, conn index:{}", nConn_idx);
	}
// 	nRet_baoyuan = scif_SetMirror(nConn_idx);		//鏡射記憶體設定
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, server index:" << nConn_idx
// 		, str_kernel_err_reason, false);
	//直接写入一个四字节到R值
	nRet_baoyuan = m_sc2_obj.DWrite1R(nConn_idx, nAddr, nVal);
	if (nRet_baoyuan == 0)
	{
		LError("fail to WriteR, conn index:{}", nConn_idx);
		str_err_reason_for_debug = "fail to WriteR, conn index:" + std::to_string(nConn_idx);
		str_err_reason_for_user = "操作设备失败";
		return false;
	}


	//讓等待先前所設定的直接命令完成後，再繼續執行下去
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time);
	if (nRet_baoyuan == 0)
	{
		LError("Time out to do cmd, nConn_idx:{}, max wait time(ms):{}", nConn_idx, nMax_wait_time);
		str_err_reason_for_debug = "Time out to do cmd, nConn_idx:" + std::to_string(nConn_idx)+",max wait time(ms):"+ std::to_string(nMax_wait_time);
		str_err_reason_for_user = "操作设备超时";
		return false;
	}
	return true;
}

/************************************
* Method:    set_RString
* Brief:  寫入字串到控制器中的 R 值
* Access:    private 
* Returns:   bool true：成功；false：失败
* Qualifier:
*Parameter: int nConn_idx -[in]  连接索引, -1表示所有连接
*Parameter: size_t nAddr -[in]  要寫入的資料位址
*Parameter: size_t nBuff_size -[in]  要写入的数量，单位字节
*Parameter: const char * pBuff -[in]  要写入的字符串
*Parameter: unsigned short nMax_wait_time -[in]  超时时间，单位ms
*Parameter: string & str_kernel_err_reason -[out] 出错原因 
************************************/
bool CBaoyuan_Lib::set_RString(int nConn_idx, size_t nAddr, size_t nBuff_size, const char* pBuff, unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	LTrace("CBaoyuan_Lib::set_RString");
	//检查参数合法性
	if (!is_valid_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}

	if (!(NULL != pBuff))
	{
		LError("p is NULL");
		str_err_reason_for_debug = "p is NULL";
		str_err_reason_for_user = "参数错误";
		return false;
	}

	if (!is_valid_addr(nAddr, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("err reason:{}", str_err_reason_for_debug);
		return false;
	}

	//連續資料讀取設定	
	int nRet_baoyuan = m_sc2_obj.LReadNR(nConn_idx, 0, nBuff_size);
	if (nRet_baoyuan == 0)
	{
		LError("fail to ReadR, conn index:{}", nConn_idx);
		str_err_reason_for_debug = "fail to ReadR, conn index:"+ std::to_string(nConn_idx);
		str_err_reason_for_user = "操作设备失败";
		return false;
	}

	if (1 == nRet_baoyuan)
	{
		LWarn("the cmd has been regrouped, conn index:{}", nConn_idx);
	}
	
	//鏡射記憶體設定
// 	nRet_baoyuan = scif_SetMirror(nConn_idx);		
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, server index:" << nConn_idx
// 		, str_kernel_err_reason, false);
	//寫入字串到 R
	nRet_baoyuan = m_sc2_obj.DWriteRString(nConn_idx, nAddr, nBuff_size, (char*)pBuff);
	if (0 == nRet_baoyuan)
	{//失败
		ERROR_MSG err_msg;
		m_sc2_obj.GetConnectionError(nConn_idx, &err_msg);
		LError("conn idx:{}, Error:{}", nConn_idx, err_msg.Error);
		if (nRet_baoyuan == 0)
		{
			LError("fail to WriteRString, conn index:{}, addr:{}, buff size:{}", nConn_idx, nAddr, nBuff_size);
			str_err_reason_for_debug = "fail to WriteRString, conn index:" + std::to_string(nConn_idx)+",addr:"+ std::to_string(nAddr)
				+",buff size:"+ std::to_string(nBuff_size);
			str_err_reason_for_user = "操作设备失败";
			return false;
		}

	}
	//讓等待先前所設定的直接命令完成後，再繼續執行下去
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time);
	if (nRet_baoyuan == 0)
	{
		LError("Time out to do cmd, conn idx:{}, max wait time(ms):{}", nConn_idx, nMax_wait_time);
		str_err_reason_for_debug = "Time out to do cmd, conn idx:" + std::to_string(nConn_idx) + ", max wait time(ms):" + std::to_string(nMax_wait_time);
		str_err_reason_for_user = "操作设备超时";
		return false;
	}
	return true;
}

/************************************
* Method:    set_CValue
* Brief:  写入一个整数到Command中
* Access:    private 
* Returns:   bool
* Qualifier:
*Parameter: int nConn_idx -[in] 连线索引 
*Parameter: int nAddr -[in]  要写入的地址
*Parameter: int nValue -[in] 要写入的值 
*Parameter: unsigned short nMax_wait_time -[in] 单位ms 
*Parameter: string & str_kernel_err_reason -[in/out]  
************************************/
bool CBaoyuan_Lib::set_CValue(int nConn_idx, int nAddr, int nValue, unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//参数合法性判定
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


	//連續資料讀取設定
	int nRet_baoyuan = m_sc2_obj.LReadNC(nConn_idx, 0, 32);
	if (nRet_baoyuan == 0)
	{
		LError("Time out to do cmd, conn idx:{}, max wait time(ms):{}", nConn_idx, nMax_wait_time);
		str_err_reason_for_debug = "Time out to do cmd, conn idx:" + std::to_string(nConn_idx) + ", max wait time(ms):" + std::to_string(nMax_wait_time);
		str_err_reason_for_user = "操作设备超时";
		return false;
	}


	if (1 == nRet_baoyuan)
	{
		LWarn("the cmd has been regrouped, conn index:{}", nConn_idx);
	}
	//鏡射記憶體設定
// 	nRet_baoyuan = scif_SetMirror(nServer_idx);
// 	businlog_error_return_err_reason(0 != nRet_baoyuan, __CLASS_FUNCTION__ << " | fail to set mirror, server index:" << nServer_idx
// 		, str_kernel_err_reason, false);

	//写入單筆資料到command
	nRet_baoyuan =  m_sc2_obj.DWrite1C(nConn_idx, nAddr, nValue);		
	if (nRet_baoyuan == 0)
	{
		LError("fail to WriteC, conn idx:{}, addr:{}, value:{}", nConn_idx, nAddr, nValue);
		str_err_reason_for_debug = "fail to WriteC, conn idx:"+ std::to_string(nConn_idx) +", addr:"+ std::to_string(nAddr) +", value:" + std::to_string(nValue);
		str_err_reason_for_user = "操作设备失败";
		return false;
	}
    //讓等待先前所設定的直接命令完成後，再繼續執行下去
	nRet_baoyuan = m_sc2_obj.DWaitDone(nConn_idx, nMax_wait_time);
	if (nRet_baoyuan == 0)
	{
		LError("Time out to do cmd, nConn_idx:{}", nConn_idx);
		str_err_reason_for_debug = "Time out to do cmd, nConn_idx:{}" + std::to_string(nConn_idx);
		str_err_reason_for_user = "操作设备超时";
		return false;
	}
	return true;
}

/************************************
* Method:    get_RValue
* Brief:  先判定雕刻机是否连接成功，如果连接成功，则读取R值整数
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
	//参数合法性判定
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

	//判定连接状态
	bool bSuccess = is_connected(Conn_idx, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("Connection is over, reason:{}", str_err_reason_for_debug);
		return false;
	}

	//读取数据
	nValue = m_sc2_obj.memR(Conn_idx, nAddr);
	return true;
}

bool CBaoyuan_Lib::get_RBit(int nConn_idx, int nAddr, int nBitIdx, int& nValue, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//参数合法性判定
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

	//判定连接是否正常
	bool bSuccess = is_connected(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("Connection is over, err reason : {}", str_err_reason_for_debug);
		return false;
	}
	//读取数据
	nValue = m_sc2_obj.memRBit(nConn_idx, nAddr, nBitIdx);
	return true;
}

CBaoyuan_Lib::CBaoyuan_Lib() 
	: m_bAvailable(false)
	, m_bStop(true)
{
	memset(&m_sDLL_setting, 0, sizeof(DLL_USE_SETTING));
	m_sDLL_setting.SoftwareType = 4;		//軟體種類   
	/*	m_sDLL_setting.ConnectNum = 200;			//連線數目*/
	m_sDLL_setting.MemSizeI = I_NUM;
	m_sDLL_setting.MemSizeO = O_NUM;
	m_sDLL_setting.MemSizeC = C_NUM;
	m_sDLL_setting.MemSizeS = S_NUM;
	m_sDLL_setting.MemSizeA = A_NUM;
	/*	m_sDLL_setting.MemSizeR = 4000000;*/
	m_sDLL_setting.MemSizeTimer = 0;
	m_sDLL_setting.MemSizeF = F_NUM;
	//将索引都设置未使用
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
	//nConn_idx值必須小於 scif_Init 函式初始化時，struct DLL_USE_SETTING 中 ConnectNum 所設定的連線數目。
	if (!(nConn_idx >= -1 && nConn_idx < (int)m_sDLL_setting.ConnectNum))
	{
		LError("Invalid connection index:{}, must be [-1,{}]", nConn_idx, m_sDLL_setting.ConnectNum - 1);
		str_err_reason_for_debug = "Invalid connection index:" + std::to_string(nConn_idx)
			+ ", must be [-1," + std::to_string(m_sDLL_setting.ConnectNum - 1) + "]";
		str_err_reason_for_user = "参数错误";
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
		str_err_reason_for_user = "参数错误";
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
		str_err_reason_for_user = "参数错误";
		return false;
	}
	return true;
}

/************************************
* Method:    strerror_ftp
* Brief:  将FTP的处理结果转换为对应的提示信息
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
	size_t nWait_time_ms = 100; //每次休眠时间，根据带宽、控制器数据、通讯即时性来设置，范围20-1000ms
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
