#ifndef SCIF2_DEFINEH
#define SCIF2_DEFINEH


//-------------------------------   
#define MAX_SYNC_COUNT                 10    //Mapper�����õ���� Sync ��
#define MAX_CONTROLLER_NUM_PER_MAKER  300    //�� Media �xȡ��������Εr���������S����

//-------------------------------
#define BIT_CB_SIZE                  4096    //Combin����а��������λַ�� for Bit (I,O,C,S,A)
#define INT_CB_SIZE                 20000    //Combin����а��������λַ�� for Int (R)
#define FIX_CB_SIZE                  4096    //Combin����а��������λַ�� for Fix (double)
#define MAX_LOOP_SIZE                  64    //Loop Queue ���ݼ{��ͨӍ�P��
#define MAX_DIRECT_SIZE                64    //Direct Queue ���ݼ{��ͨӍ�P��
#define DIRECT_ADDR_MASK             0x3F    //Direct λַ�� mask��Ҫ�c MAX_DIRECT_SIZE����

//-------------------------------
#define MAX_DATA_BYTES    1440
#define MAX_BIT_NUM       1440      // MAX_DATA_BYTES / 1
#define MAX_WORD_NUM      720       // MAX_DATA_BYTES / 2
#define MAX_INT_NUM       360       // MAX_DATA_BYTES / 4
#define MAX_FIX_NUM       180       // MAX_DATA_BYTES / 8
#define MAX_PTR_NUM       180       // MAX_DATA_BYTES / 8
#define MAX_CB_NUM        288      //MAX_DATA_BYTES /(4+1)   //λַ4bytes, data��byte�r(1byte)
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

//---------scif_GetLibraryMsg ������
#define SCIF_PROC_COUNTER           1  //porcess counter
#define SCIF_MAKER_ID               7  //�u���̾�̖(Group)
#define SCIF_FTP_STATE             11  //FTP ��B
#define SCIF_FTP_RESULT            12  //FTP ̎��Y��  
#define SCIF_FTP_STEP              13  //FTP ̎���E
#define SCIF_FTP_TOTAL_PACKAGE     21  //FTP ���Ϳ������
#define SCIF_FTP_CURRENT_PACKAGE   22  //FTP ��̎��ķ����  
#define SCIF_FTP_TOTAL_FILE        31  //FTP ��ݔ�n��
#define SCIF_FTP_CURRENT_FILE      32  //FTP ��̎��ęn����

#define SCIF_MEM_SIZE_I    40   //I�c����
#define SCIF_MEM_SIZE_O    41   //O�c����
#define SCIF_MEM_SIZE_C    42   //C�c����
#define SCIF_MEM_SIZE_S    43   //S�c����
#define SCIF_MEM_SIZE_A    44   //A�c����
#define SCIF_MEM_SIZE_TIMER     45   //TT�c����
#define SCIF_MEM_SIZE_COUNTER   46   //CT�c����
#define SCIF_MEM_SIZE_R    47   //R�c����
#define SCIF_MEM_SIZE_F    52   //F�c����

//-----scif_GetConnectionMsg ������
#define SCIF_CONNECT_STATE          2  //�B����B
#define SCIF_REMOTE_IPLONG          3  //Ŀǰ���B������
#define SCIF_CONNECT_STEP           4  //�B�����E
#define SCIF_CONNECT_RESPONSE       5  //�B���ؑ���B
#define SCIF_TALK_STATE             6  //�Y��ͨӍ��B
#define SCIF_RESPONSE_TIME          11  //Ŀǰ����ķ����r�g
#define SCIF_OK_COUNT               12  //���_����Δ�
#define SCIF_CRC_ERR_CNT            13  //CRC�e�`�Δ�
#define SCIF_LOOP_QUEUE_PKG_COUNT   21  //LOOP QUEUE�еķ���P��
#define SCIF_DIRECT_QUEUE_PKG_COUNT 22  //Direct Queue�еķ���P��
#define SCIF_LOOP_COUNT             23  //LOOP QUEUE�Ĳ�ԃޒȦ�Δ�
#define SCIF_TX_PKG_CNT             24  //���ͷ������
#define SCIF_TX_PKG_RETRY_CNT       25  //������ʹΔ�
#define SCIF_TX_CONNECT_CNT         26  //�ͳ�Ҫ���B������Δ�
#define SCIF_RX_UNEXPECT_CNT        27  //���ڴ��յ��ķ��
#define SCIF_RX_ERR_FMT_CNT         28  //��ʽ�e�`�ķ��
#define SCIF_RX_CONNECT_CNT         29  //�����B������Δ�



//�B����B �� GetConnectionMsg(SCIF_CONNECT_STATE)  ȡ��
#define SC_CONN_STATE_DISCONNECT    0   //�B���P�]
#define SC_CONN_STATE_CONNECTING    1   //�B����
#define SC_CONN_STATE_FAIL          2   //�B��ʧ��
#define SC_CONN_STATE_OK            3   //�B������
#define SC_CONN_STATE_NORESPONSE    4   //�B���o�ؑ�


//------------�B���ؑ���B
#define CONNECT_RESULT_NORESPONSE          0
#define CONNECT_RESULT_OLD_SOFTWARE_CLEAR  1      //ԭ�����õ�ܛ�w�ѽ����
#define CONNECT_RESULT_INVALID_SOFTWARE    11     //�oЧ��ܛ�w��̖
#define CONNECT_RESULT_DISABLE_SOFTWARE    12     //ܛ�w����ͣ��
#define CONNECT_RESULT_DISABLE_INTERNET    13     //ͣ������W����B��
#define CONNECT_RESULT_CLOSED_SOFTWARE     14     //ܛ�w�����P�]
#define CONNECT_RESULT_CLOSED_INTERNET     15     //�P�]����W����B�� 
#define CONNECT_RESULT_INVALID_MAKERID     16     //�������MakerID
#define CONNECT_RESULT_PASSWORD_WRONG      17     //�ܴa�e�` 
#define CONNECT_RESULT_NOT_IN_ALLOW        18     //�������S�����
#define CONNECT_RESULT_ALL_IN_USE          19     //�傀�B�����ѽ���ʹ�á�
#define CONNECT_RESULT_WAIT_CONFIRM        21     //�ȴ��˙C�_�J��
#define CONNECT_RESULT_SOFTWARE_CONNECTED  31     //ܛ�w���B��
#define CONNECT_RESULT_SOFTWARE_REJECTED   32     //ͣ������W����B��
#define CONNECT_RESULT_SOFTWARE_INREQ      41     //����ʹ���߁��ô�ܛ�w
#define CONNECT_RESULT_PENDING             50     //



//-----FTPĿ���Y�ϊA
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

//------FTP ��B
#define FTP_STATE_IDLE              0  //�f��
#define FTP_STATE_UPLOAD            1  //�ς�
#define FTP_STATE_DOWNLOAD          2  //���d
#define FTP_STATE_DELETE            3  //���d
#define FTP_STATE_LIST             11  //ȡ��Ŀ�
#define FTP_STATE_UPLOAD_MANY      21  //�ς�����
#define FTP_STATE_DOWNLOAD_MANY    22  //���d����
#define FTP_STATE_DELETE_MANY      23  //�h������
#define FTP_STATE_MAKE_DIR         30  //����Ŀ�
#define FTP_STATE_PENDING          99  //�����O����

//------FTP ̎��Y��
#define FTP_RESULT_IDLE                         0
#define FTP_RESULT_PROCESSING                   1
#define FTP_RESULT_SUCCESS                      2

#define FTP_RESULT_FAIL_TO_READ_LOCAL_FILE     11       //�xȡ���ؙn��ʧ��
#define FTP_RESULT_FAIL_TO_WRITE_LOCAL_FILE    12       //���뱾�ؙn��ʧ��
#define FTP_RESULT_FAIL_TO_READ_REMOTE_FILE    13       //�xȡ�h�˙n��ʧ��
#define FTP_RESULT_FAIL_TO_WRITE_REMOTE_FILE   14       //�����h�˙n��ʧ��
#define FTP_RESULT_FAIL_TO_SET_COMMAND         15       //�������ʧ��
#define FTP_RESULT_FAIL_TO_COMMUNICATION       16       //ͨӍ�e�`
#define FTP_RESULT_FILE_MISMATCH               17       //�n���Ȍ������_  

//------------�Y��ͨӍ��B
#define TALK_STATE_NORMAL      0
#define TALK_STATE_ERROR       1
#define TALK_STATE_OVER_RETRY  2


//------�ιPͨӍ�Y�ϵĠ�B
#define SC_TRANSACTION_PENDING      0    //�ȴ�̎����
#define SC_TRANSACTION_PORCESSING   1    //̎����
#define SC_TRANSACTION_FINISH       2    //���
#define SC_TRANSACTION_INVALID      3    //�oЧ������

//============ͨӍ����e�`��̖
// �e�`�a�� 0                                     //�]�аl���e�`
#define SCIF_ERROR_INVALID_PACKET_SET   255      //Local�z�鵽�˷���O���oЧ
//������̖���e�`�a                      �����C���ص��e�`---ֱ��ӛ䛴��a����

//---һЩ���x
#define FILENAME_LENGTH          32   //�n�����Q�������Ԫ��
#define MAX_FILE_LIST_NUM        240  //���ęn����δ�С
#define MAX_TRANSFER_FILE_COUNT  128  //һ�΂�ݔ�����n����
#define MAX_SOFTWARE_COUNT       5    //����ܛ�w�N�

//�e�`ӍϢ��Դ
#define ERROR_TYPE_NONE       0
#define ERROR_TYPE_POLLING    1
#define ERROR_TYPE_DIRECT     2

//����N�
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


//�ιPͨӍ���Y�ϽY��
union SC_DATA
{
	char           Bytes[MAX_BIT_NUM];    //bit �� byte �Y��
	short          Words[MAX_WORD_NUM];    //word �Y��
	int            Ints[MAX_INT_NUM];      //����
	double         Fixs[MAX_FIX_NUM];      //double
};

//�Ԅӂɜy���C�Ļؑ����
struct LOCAL_CONTROLLER_INFO
{
	unsigned int   IPLong;
	char           IP[32];
	char           Name[32];
};


//FTP �򱾵����e�n����΂��صęn���YӍ
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

//FTP�n����ݔ���O���Y��
struct FTP_TRANFER_FILE
{
	int  Folder;
	char SubFolder[FILENAME_LENGTH];  
	char Filename[FILENAME_LENGTH];
	char LocalFilename[256];
};


//�����O���ĽY��
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
	unsigned int  ConnectNum;           //�B����Ŀ
	unsigned int  SoftwareType;          //ܛ�w�N�
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