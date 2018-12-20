#ifndef SCIF2_DEFINEH
#define SCIF2_DEFINEH


//-------------------------------   
#define MAX_SYNC_COUNT                 10    //Mapper中所用的最大 Sync 數
#define MAX_CONTROLLER_NUM_PER_MAKER  300    //向 Media 讀取控制器清單時，最大的允許數量

//-------------------------------
#define BIT_CB_SIZE                  4096    //Combin封包中包含的最大位址數 for Bit (I,O,C,S,A)
#define INT_CB_SIZE                 20000    //Combin封包中包含的最大位址數 for Int (R)
#define FIX_CB_SIZE                  4096    //Combin封包中包含的最大位址數 for Fix (double)
#define MAX_LOOP_SIZE                  64    //Loop Queue 可容納的通訊筆數
#define MAX_DIRECT_SIZE                64    //Direct Queue 可容納的通訊筆數
#define DIRECT_ADDR_MASK             0x3F    //Direct 位址的 mask，要與 MAX_DIRECT_SIZE搭配

//-------------------------------
#define MAX_DATA_BYTES    1440
#define MAX_BIT_NUM       1440      // MAX_DATA_BYTES / 1
#define MAX_WORD_NUM      720       // MAX_DATA_BYTES / 2
#define MAX_INT_NUM       360       // MAX_DATA_BYTES / 4
#define MAX_FIX_NUM       180       // MAX_DATA_BYTES / 8
#define MAX_PTR_NUM       180       // MAX_DATA_BYTES / 8
#define MAX_CB_NUM        288      //MAX_DATA_BYTES /(4+1)   //位址4bytes, data為byte時(1byte)
#define MAX_CB_BIT_NUM    288      // MAX_DATA_BYTES /(4+1)
#define MAX_CB_WORD_NUM   240      // MAX_DATA_BYTES /(4+2)
#define MAX_CB_INT_NUM    180      // MAX_DATA_BYTES /(4+4)
#define MAX_CB_FIX_NUM    120      // MAX_DATA_BYTES /(4+8)
#define MAX_CB_PTR_NUM    120      // MAX_DATA_BYTES /(4+8)

#define I_OFFSET 0
#define O_OFFSET (5120*1)
#define C_OFFSET (5120*2)
#define S_OFFSET (5120*3)
#define A_OFFSET (5120*4)
#define TT_OFFSET (5120*5)
#define CT_OFFSET (5120*6)
#define RBIT_OFFSET  100000

#define R_OFFSET     0
#define RI_OFFSET   8000000
#define RO_OFFSET   8100000
#define RC_OFFSET   8200000
#define RS_OFFSET   8300000
#define RA_OFFSET   8400000
#define RTT_OFFSET  8500000
#define RCT_OFFSET  8600000
#define TV_OFFSET  10000000
#define TS_OFFSET  10500000
#define CV_OFFSET  11000000
#define CS_OFFSET  11500000
#define F_OFFSET   10000000

#define I_NUM 4096
#define O_NUM 4096
#define C_NUM 4096
#define S_NUM 4096
#define A_NUM 4096
#define R_NUM 6000000
#define F_NUM 100000
#define TIMER_NUM 256
#define COUNTER_NUM 256

//---------scif_GetLibraryMsg 的引數
#define SCIF_PROC_COUNTER           1  //porcess counter
#define SCIF_MAKER_ID               7  //製造商編號(Group)
#define SCIF_FTP_STATE             11  //FTP 狀態
#define SCIF_FTP_RESULT            12  //FTP 處理結果  
#define SCIF_FTP_STEP              13  //FTP 處理步驟
#define SCIF_FTP_TOTAL_PACKAGE     21  //FTP 傳送總封包數
#define SCIF_FTP_CURRENT_PACKAGE   22  //FTP 已處理的封包數  
#define SCIF_FTP_TOTAL_FILE        31  //FTP 傳輸檔案
#define SCIF_FTP_CURRENT_FILE      32  //FTP 已處理的檔案數

#define SCIF_MEM_SIZE_I    40   //I點個數
#define SCIF_MEM_SIZE_O    41   //O點個數
#define SCIF_MEM_SIZE_C    42   //C點個數
#define SCIF_MEM_SIZE_S    43   //S點個數
#define SCIF_MEM_SIZE_A    44   //A點個數
#define SCIF_MEM_SIZE_TIMER     45   //TT點個數
#define SCIF_MEM_SIZE_COUNTER   46   //CT點個數
#define SCIF_MEM_SIZE_R    47   //R點個數
#define SCIF_MEM_SIZE_F    52   //F點個數

//-----scif_GetConnectionMsg 的引數
#define SCIF_CONNECT_STATE          2  //連線狀態
#define SCIF_REMOTE_IPLONG          3  //目前的連線對象
#define SCIF_CONNECT_STEP           4  //連線步驟
#define SCIF_CONNECT_RESPONSE       5  //連線回應狀態
#define SCIF_TALK_STATE             6  //資料通訊狀態
#define SCIF_RESPONSE_TIME          11  //目前封包的反應時間
#define SCIF_OK_COUNT               12  //正確封包次數
#define SCIF_CRC_ERR_CNT            13  //CRC錯誤次數
#define SCIF_LOOP_QUEUE_PKG_COUNT   21  //LOOP QUEUE中的封包筆數
#define SCIF_DIRECT_QUEUE_PKG_COUNT 22  //Direct Queue中的封包筆數
#define SCIF_LOOP_COUNT             23  //LOOP QUEUE的查詢迴圈次數
#define SCIF_TX_PKG_CNT             24  //傳送封包個數
#define SCIF_TX_PKG_RETRY_CNT       25  //封包重送次數
#define SCIF_TX_CONNECT_CNT         26  //送出要求連線封包次數
#define SCIF_RX_UNEXPECT_CNT        27  //不期待收到的封包
#define SCIF_RX_ERR_FMT_CNT         28  //格式錯誤的封包
#define SCIF_RX_CONNECT_CNT         29  //接收連線封包次數



//連線狀態 由 GetConnectionMsg(SCIF_CONNECT_STATE)  取得
#define SC_CONN_STATE_DISCONNECT    0   //連線關閉
#define SC_CONN_STATE_CONNECTING    1   //連線中
#define SC_CONN_STATE_FAIL          2   //連線失敗
#define SC_CONN_STATE_OK            3   //連線正常
#define SC_CONN_STATE_NORESPONSE    4   //連線無回應


//------------連線回應狀態
#define CONNECT_RESULT_NORESPONSE          0
#define CONNECT_RESULT_OLD_SOFTWARE_CLEAR  1      //原本佔用的軟體已經清除
#define CONNECT_RESULT_INVALID_SOFTWARE    11     //無效的軟體代號
#define CONNECT_RESULT_DISABLE_SOFTWARE    12     //軟體功能停用
#define CONNECT_RESULT_DISABLE_INTERNET    13     //停用自外網來的連線
#define CONNECT_RESULT_CLOSED_SOFTWARE     14     //軟體功能關閉
#define CONNECT_RESULT_CLOSED_INTERNET     15     //關閉自外網來的連線 
#define CONNECT_RESULT_INVALID_MAKERID     16     //不相符的MakerID
#define CONNECT_RESULT_PASSWORD_WRONG      17     //密碼錯誤 
#define CONNECT_RESULT_NOT_IN_ALLOW        18     //不在允許清單中
#define CONNECT_RESULT_ALL_IN_USE          19     //五個連線都已經被使用。
#define CONNECT_RESULT_WAIT_CONFIRM        21     //等待人機確認中
#define CONNECT_RESULT_SOFTWARE_CONNECTED  31     //軟體已連線
#define CONNECT_RESULT_SOFTWARE_REJECTED   32     //停用自外網來的連線
#define CONNECT_RESULT_SOFTWARE_INREQ      41     //其他使用者佔用此軟體
#define CONNECT_RESULT_PENDING             50     //



//-----FTP目標資料夾
#define FTP_FOLDER_RUN_NCFILES    1
#define FTP_FOLDER_NCFILES_RUN    1
#define FTP_FOLDER_RUN_DXF        2
#define FTP_FOLDER_DXF_RUN        2  
#define FTP_FOLDER_TEACH_RUN         3
#define FTP_FOLDER_RECIPE_RUN        4
#define FTP_FOLDER_WORKDATA_RUN      5
//------------------
#define FTP_FOLDER_NCFILES           10
#define FTP_FOLDER_MACRO             21
#define FTP_FOLDER_MACRO_MAKER       20
#define FTP_FOLDER_OPEN_HMI          24
#define FTP_FOLDER_NCFILES_ROOT       25
#define FTP_FOLDER_MACRO_MAKER_ROOT   26
#define FTP_FOLDER_MACRO_ROOT         27
#define FTP_FOLDER_DXF               28
#define FTP_FOLDER_OPEN_HMI_BOTTOM   29
#define FTP_FOLDER_TEACH             15
#define FTP_FOLDER_RECIPE            16
#define FTP_FOLDER_WORKDATA          17

#define FTP_FOLDER_MACHINE       30
#define FTP_FOLDER_SETUP         40
#define FTP_FOLDER_SETUP_MACHINE 41
#define FTP_FOLDER_BAK           50
#define FTP_FOLDER_DATA          51
#define FTP_FOLDER_LANGUAGE      52
#define FTP_FOLDER_LANGUAGE_DEF  53
#define FTP_FOLDER_LOG           54
#define FTP_FOLDER_RECORD        55

//------FTP 狀態
#define FTP_STATE_IDLE              0  //閒置
#define FTP_STATE_UPLOAD            1  //上傳
#define FTP_STATE_DOWNLOAD          2  //下載
#define FTP_STATE_DELETE            3  //下載
#define FTP_STATE_LIST             11  //取得目錄
#define FTP_STATE_UPLOAD_MANY      21  //上傳多個
#define FTP_STATE_DOWNLOAD_MANY    22  //下載多個
#define FTP_STATE_DELETE_MANY      23  //刪除多個
#define FTP_STATE_MAKE_DIR         30  //建立目錄
#define FTP_STATE_PENDING          99  //命令設定中

//------FTP 處理結果
#define FTP_RESULT_IDLE                         0
#define FTP_RESULT_PROCESSING                   1
#define FTP_RESULT_SUCCESS                      2

#define FTP_RESULT_FAIL_TO_READ_LOCAL_FILE     11       //讀取本地檔案失敗
#define FTP_RESULT_FAIL_TO_WRITE_LOCAL_FILE    12       //寫入本地檔案失敗
#define FTP_RESULT_FAIL_TO_READ_REMOTE_FILE    13       //讀取遠端檔案失敗
#define FTP_RESULT_FAIL_TO_WRITE_REMOTE_FILE   14       //寫入遠端檔案失敗
#define FTP_RESULT_FAIL_TO_SET_COMMAND         15       //命令傳送失敗
#define FTP_RESULT_FAIL_TO_COMMUNICATION       16       //通訊錯誤
#define FTP_RESULT_FILE_MISMATCH               17       //檔案比對不正確  

//------------資料通訊狀態
#define TALK_STATE_NORMAL      0
#define TALK_STATE_ERROR       1
#define TALK_STATE_OVER_RETRY  2


//------單筆通訊資料的狀態
#define SC_TRANSACTION_PENDING      0    //等待處理中
#define SC_TRANSACTION_PORCESSING   1    //處理中
#define SC_TRANSACTION_FINISH       2    //完成
#define SC_TRANSACTION_INVALID      3    //無效的索引

//============通訊封包錯誤編號
// 錯誤碼為 0                                     //沒有發生錯誤
#define SCIF_ERROR_INVALID_PACKET_SET   255      //Local檢查到此封包設定無效
//其他編號的錯誤碼                      由主機傳回的錯誤---直接記錄代碼即可

//---一些定義
#define FILENAME_LENGTH          32   //檔案名稱的最大字元數
#define MAX_FILE_LIST_NUM        240  //最大的檔案清單大小
#define MAX_TRANSFER_FILE_COUNT  128  //一次傳輸的最大檔案量
#define MAX_SOFTWARE_COUNT       5    //最大的軟體種類數

//錯誤訊息來源
#define ERROR_TYPE_NONE       0
#define ERROR_TYPE_POLLING    1
#define ERROR_TYPE_DIRECT     2

//命令種類
#define SC_LOOP         0  // Loop Command
#define SC_DIRECT       1  // Direct Command



struct ERROR_MSG
{
	unsigned char Type;
	unsigned char Cmd;
	unsigned char Error;  
	unsigned char Reserve;
	unsigned int addr;
	unsigned int num;
};


//單筆通訊的資料結構
union SC_DATA
{
	char           Bytes[MAX_BIT_NUM];    //bit 或 byte 資料
	short          Words[MAX_WORD_NUM];    //word 資料
	int            Ints[MAX_INT_NUM];      //整數
	double         Fixs[MAX_FIX_NUM];      //double
};

//自動偵測主機的回應封包
struct LOCAL_CONTROLLER_INFO
{
	unsigned int   IPLong;
	char           IP[32];
	char           Name[32];
};


//FTP 或本地列舉檔案清單傳回的檔案資訊
struct FTP_FILE
{
	char           filename[FILENAME_LENGTH];
	unsigned int   filesize;
	unsigned short year;
	unsigned char  month;
	unsigned char  day;
	unsigned char  hour;
	unsigned char  minute;
	unsigned char  second;
	unsigned char  Reserve;
};

//FTP檔案傳輸的設定資料
struct FTP_TRANFER_FILE
{
	int  Folder;
	char SubFolder[FILENAME_LENGTH];  
	char Filename[FILENAME_LENGTH];
	char LocalFilename[256];
};


//功能設定的結構
struct FUNCTION_SETTING
{
	int  MakerID;
	int  Reserve1;
	int  Reserve2;
	int  Reserve3;
	int  Reserve4;
};

struct DLL_USE_SETTING
{
	unsigned int  ConnectNum;           //連線數目
	unsigned int  SoftwareType;          //軟體種類
	unsigned int  MemSizeI;
	unsigned int  MemSizeO;
	unsigned int  MemSizeC;
	unsigned int  MemSizeS;
	unsigned int  MemSizeA;
	unsigned int  MemSizeR;
	unsigned int  MemSizeF;
	unsigned int  MemSizeTimer;
	unsigned int  MemSizeCounter;
};


#endif