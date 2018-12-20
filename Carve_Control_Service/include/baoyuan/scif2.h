#ifndef SC2_H
#define SC2_H

#include "scif2_define.h"

#ifdef __QT
    #include <QtCore/qglobal.h>
    #define __CLASS
    #if defined(SC2_LIBRARY)
    #  define SC2SHARED_EXPORT Q_DECL_EXPORT
    #else
    #  define SC2SHARED_EXPORT Q_DECL_IMPORT
    #endif
    #define IMEXPORT
    #define CALBY
#else
    #if defined(__CLASS)
        #if defined(SC2_LIBRARY)
            #define SC2SHARED_EXPORT __declspec(dllexport)
            #define IMEXPORT
            #define CALBY
        #else
            #define SC2SHARED_EXPORT __declspec(dllimport)
            #define IMEXPORT
            #define CALBY
        #endif
    #else
        #ifdef __BCB
            #if defined(SC2_LIBRARY)
                #define IMEXPORT __declspec(dllexport)
                #define CALBY      __stdcall
            #else
                #define IMEXPORT __declspec(dllimport)
                #define CALBY    __stdcall
            #endif
        #endif
        #ifdef __GCC
            #define IMEXPORT
            #define CALBY 
        #endif
        #ifdef __VC
            #if defined(SC2_LIBRARY)
                #define IMEXPORT __declspec(dllexport)
                #define CALBY    __cdecl
            #else
                #define IMEXPORT __declspec(dllimport)
                #define CALBY    __cdecl
            #endif
        #endif
    #endif
#endif

#if defined(__CLASS)
class SC2SHARED_EXPORT SC2
{
public:
    IMEXPORT CALBY SC2();
#else
    extern "C" {
#endif
    //=================== ͨӍ���к�ʽ����ԓÿ10~100ms����һ�� ======================
    IMEXPORT int  CALBY MainProcess();
    //=================== ��ʽ���ʼ�� ===================
    IMEXPORT int  CALBY LibraryInitial (DLL_USE_SETTING *pUseSetting, int MakerID, char* EncString);
    IMEXPORT void CALBY LibraryDestroy ();
    IMEXPORT void CALBY LibrarySetDebug (int level);   //level : 0���@ʾ   1���e�`���@ʾ   2ͨӍ����ȫ���@ʾ(���e�ã�ͨӍ�ٶ�׃��ÿ��һ��)
    //--==========================ȡ�ÃȲ��YӍ =================================
    IMEXPORT int   CALBY GetLibraryMsg (int id );                   //ȡ��һ����YӍ
    IMEXPORT int   CALBY GetConnectionMsg (int TgrConn, int MsgID);         //ȡ���B��ͨӍ���YӍ
    IMEXPORT void  CALBY GetConnectionError (int TgrConn, ERROR_MSG *Msg);  //ȡ���e�`ӍϢ

    //===================�^��W·�Ђɜy���C����=====================================
    IMEXPORT int  CALBY LocalDetectControllers ();                                    //�Ԅӂɜy���C����
    IMEXPORT int  CALBY LocalReadControllerCount ();                                  //�xȡȡ�õĿ������Y�ϹP��
    IMEXPORT int  CALBY LocalReadController (int Index, LOCAL_CONTROLLER_INFO *Info); //�xȡȡ�õĿ������Y��

    //==================================�B������=================================
    IMEXPORT int  CALBY ConnectSetPwd (int TgrConn, char *Pwd);     //�O���B���ܴa
    IMEXPORT int  CALBY ConnectLocalList (int TgrConn, int Index);  //�cȡ�õĿ���������е�Indexֵ�M���B��
    IMEXPORT int  CALBY ConnectLocalIP (int TgrConn, char* IP);     //ֱ��ݔ�������IP�M���B��
    IMEXPORT int  CALBY Disconnect (int TgrConn);                   //�Д��B��

    //==================================�n����ݔ����=================================
    IMEXPORT int  CALBY FtpSetConnection (int TgrConn);                                                     //�O�� FTP ����
    //==================================�h�˙n�����
    IMEXPORT int  CALBY FtpGetFileList (int Folder, char *SubFolder, char *HeadFilter, char *TailFilter);   //ȡ�Ùn�����
    IMEXPORT int  CALBY FtpReadFileCount ();                                                                //�xȡFTP�n����ιP��
    IMEXPORT int  CALBY FtpReadFile (int Index, FTP_FILE *file);                                            //�xȡFTP�n�����Q
    //==================================�n����ݔ����=================================
    IMEXPORT int  CALBY FtpMakeDir (int Folder, char *DirName);                                             //�����Y�ϊA
    IMEXPORT int  CALBY FtpUpload1File (int Folder, char *SubFolder, char *Filename, char *LocalFilename);   //�ς��n��
    IMEXPORT int  CALBY FtpDownload1File (int Folder, char *SubFolder, char *Filename, char *LocalFilename); //���d�n��
    IMEXPORT int  CALBY FtpDelete1File (int Folder, char *SubFolder, char *Filename);                        //�h���n��
    IMEXPORT int  CALBY FtpTransferFileReset ();
    IMEXPORT int  CALBY FtpTransferFileAdd (int Folder, char *SubFolder, char *Filename, char *LocalFilename);
    IMEXPORT int  CALBY FtpUploadFiles ();
    IMEXPORT int  CALBY FtpDownloadFiles ();
    IMEXPORT int  CALBY FtpDeleteFiles ();
    //==================================
    IMEXPORT int  CALBY FtpCheckDone ();                                             //ȡ�È��нY��
    IMEXPORT int  CALBY FtpWaitDone (int MaxWaitTime);                               //ȡ�È��нY��
    //==================================���ض˙n�����
    IMEXPORT int  CALBY LocalGetFileList (char *Path, char *HeadFilter, char *TailFilter);  //ȡ�ñ��ض˙n�����
    IMEXPORT int  CALBY LocalReadFileCount ();                                              //�xȡ���ؙn����ιP��
    IMEXPORT int  CALBY LocalReadFile (int Index, FTP_FILE *file);                          //�xȡ���ؙn�����Q
    IMEXPORT int  CALBY LocalDeleteFile (int Index);                                        //�h�����ؙn�����Q

    //====================== �ɱ���ӛ���w�xȡ�Y��
    IMEXPORT int  CALBY memI (int TgrConn, int addr );
    IMEXPORT int  CALBY memO (int TgrConn, int addr );
    IMEXPORT int  CALBY memC (int TgrConn, int addr );
    IMEXPORT int  CALBY memS (int TgrConn, int addr );
    IMEXPORT int  CALBY memA (int TgrConn, int addr );
    IMEXPORT int  CALBY memR (int TgrConn, int addr );
    IMEXPORT int  CALBY memRBit (int TgrConn, int addr, int BitIdx);
    IMEXPORT int  CALBY memRString (int TgrConn, int addr, int RSize, char *Buf );           //���R��ӛ���w���xȡ�ִ�
    IMEXPORT double CALBY memF (int TgrConn, int addr );
    IMEXPORT char* CALBY memRPtr (int TgrConn, int addr );
    //----
    /*
    IMEXPORT int  CALBY memTimerSet (int TgrConn, int addr );
    IMEXPORT int  CALBY memTimerVal (int TgrConn, int addr );
    IMEXPORT int  CALBY memTimerTrg (int TgrConn, int addr );
    IMEXPORT int  CALBY memCounterSet (int TgrConn, int addr );
    IMEXPORT int  CALBY memCounterVal (int TgrConn, int addr );
    IMEXPORT int  CALBY memCounterTrg (int TgrConn, int addr );
    */

    //====================== ���뱾��ӛ���w
    IMEXPORT int  CALBY memSetO (int TgrConn, int addr, char val);
    IMEXPORT int  CALBY memSetC (int TgrConn, int addr, char val);
    IMEXPORT int  CALBY memSetS (int TgrConn, int addr, char val);
    IMEXPORT int  CALBY memSetA (int TgrConn, int addr, char val);
    IMEXPORT int  CALBY memSetR (int TgrConn, int addr, int val);
    IMEXPORT int  CALBY memSetRBit (int TgrConn, int addr, int BitIdx, int val);
    IMEXPORT int  CALBY memSetRString (int TgrConn, int addr, int RSize, char *Buf );           //���R��ӛ���w���xȡ�ִ�
    IMEXPORT int  CALBY memSetF (int TgrConn, int addr, double val);


    //============================= ȡ��ͨӍ̎���B
    IMEXPORT int       CALBY GetTranState (int pTran);
    IMEXPORT SC_DATA*  CALBY GetDataPointer (int pTran);
    // ========================================================
    //���º�ʽ�؂�ֵ���� 0 ������ָ����ܽ^�������� 0 ��������ԓ�PͨӍ��ָ��
    //�˕r�����䎧�� GetTranState �������У�ȡ��ԓ�PͨӍ�Ġ�B�������� SC_TRANSACTION_RESET
    //һ�Εr�g֮���َ��� GetTranState �������У��؂�ֵ���� SC_TRANSACTION_FINISH ����ԓ�PͨӍ�ѱ�̎��
    IMEXPORT void CALBY LClearQueue (int TgrConn);             //�������
    //  ============================= Loop read ============
    IMEXPORT void CALBY LReadBegin (int TgrConn);        //�O���ԄӽM�����
    IMEXPORT int  CALBY LReadNI (int TgrConn, int addr, int num);
    IMEXPORT int  CALBY LReadNO (int TgrConn, int addr, int num);
    IMEXPORT int  CALBY LReadNC (int TgrConn, int addr, int num);
    IMEXPORT int  CALBY LReadNS (int TgrConn, int addr, int num);
    IMEXPORT int  CALBY LReadNA (int TgrConn, int addr, int num);
    IMEXPORT int  CALBY LReadNR (int TgrConn, int addr, int num );
    IMEXPORT int  CALBY LReadNF (int TgrConn, int addr, int num );
    IMEXPORT int  CALBY LReadEnd(int TgrConn);        //����ԄӽM���O���K�_ʼ�a���M�Ϸ��
    //-----
    /*
    IMEXPORT int  CALBY LReadNTimerSet (int TgrConn, int addr, int num );
    IMEXPORT int  CALBY LReadNTimerVal (int TgrConn, int addr, int num );
    IMEXPORT int  CALBY LReadNTimerTrg (int TgrConn, int addr, int num );
    IMEXPORT int  CALBY LReadNCounterSet (int TgrConn, int addr, int num );
    IMEXPORT int  CALBY LReadNCounterVal (int TgrConn, int addr, int num );
    IMEXPORT int  CALBY LReadNCounterTrg (int TgrConn, int addr, int num );
    */

    //  ============================= L write ============
    IMEXPORT void CALBY LWriteBegin (int TgrConn);        //�O���ԄӽM�����
    IMEXPORT int  CALBY LWriteNO (int TgrConn,  int addr, int num);
    IMEXPORT int  CALBY LWriteNC (int TgrConn,  int addr, int num);
    IMEXPORT int  CALBY LWriteNS (int TgrConn,  int addr, int num);
    IMEXPORT int  CALBY LWriteNA (int TgrConn,  int addr, int num);
    IMEXPORT int  CALBY LWriteNR (int TgrConn,  int addr, int num);
    IMEXPORT int  CALBY LWriteNF (int TgrConn,  int addr, int num);
    IMEXPORT int  CALBY LWriteEnd(int TgrConn);        //����ԄӽM���O���K�_ʼ�a���M�Ϸ��


    IMEXPORT void CALBY DClearQueue (int TgrConn);             //�������
    IMEXPORT int  CALBY DWaitDone (int TgrConn, int MaxWaitTime);  //�ȴ�ֱ���������
    //  ============================= Direct read ============
    IMEXPORT void CALBY DReadBegin (int TgrConn);       //�O���ԄӽM�����
    IMEXPORT int  CALBY DReadNI (int TgrConn, int addr, int num);
    IMEXPORT int  CALBY DReadNO (int TgrConn, int addr, int num);
    IMEXPORT int  CALBY DReadNC (int TgrConn, int addr, int num);
    IMEXPORT int  CALBY DReadNS (int TgrConn, int addr, int num);
    IMEXPORT int  CALBY DReadNA (int TgrConn, int addr, int num);
    IMEXPORT int  CALBY DReadNR (int TgrConn, int addr, int num );
    IMEXPORT int  CALBY DReadNF (int TgrConn, int addr, int num );
    IMEXPORT int  CALBY DReadEnd (int TgrConn);      //����ԄӽM���O���K�_ʼ�a���M�Ϸ��

    //  ============================= Direct write ============
    IMEXPORT void CALBY DWriteBegin (int TgrConn);       //�O���ԄӽM�����
    IMEXPORT int  CALBY DWrite1O (int TgrConn, int addr, int val);
    IMEXPORT int  CALBY DWrite1C (int TgrConn, int addr, int val);
    IMEXPORT int  CALBY DWrite1S (int TgrConn, int addr, int val);
    IMEXPORT int  CALBY DWrite1A (int TgrConn, int addr, int val);
    IMEXPORT int  CALBY DWrite1R (int TgrConn,  int addr, int val);
    IMEXPORT int  CALBY DWrite1F (int TgrConn,  int addr, double val);
    IMEXPORT int  CALBY DWrite1RBit (int TgrConn, int addr, int BitIdx, int BitValue);             //���� R bit
    IMEXPORT int  CALBY DWriteRString (int TgrConn, int addr, int RSize, char *Buf);            //�����ִ��� R
    IMEXPORT int  CALBY DWriteNO (int TgrConn,  int addr, int num, int *data);
    IMEXPORT int  CALBY DWriteNC (int TgrConn,  int addr, int num, int *data);
    IMEXPORT int  CALBY DWriteNS (int TgrConn,  int addr, int num, int *data);
    IMEXPORT int  CALBY DWriteNA (int TgrConn,  int addr, int num, int *data);
    IMEXPORT int  CALBY DWriteNR (int TgrConn,  int addr, int num, int *data);
    IMEXPORT int  CALBY DWriteNF (int TgrConn,  int addr, int num, double *data);
    IMEXPORT int  CALBY DWriteEnd (int TgrConn);      //����ԄӽM���O���K�_ʼ�a���M�Ϸ��
};
#endif // SCIF2_H