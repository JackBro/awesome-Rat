   /*******************************************************************
   *	cInjectDll.h
   *    DESCRIPTION:Զ��DLLע�䣬ִ��ָ���ĺ���
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2003-02-25
   *
   *******************************************************************/

  
#ifndef __CINJECTDLL_H__
#define __CINJECTDLL_H__

typedef HINSTANCE (WINAPI *PLOADLIBRARY)(LPCTSTR );
typedef BOOL (WINAPI *PFREELIBRARY)(HINSTANCE);
typedef HMODULE (WINAPI* PGETMODULEHANDLE)(LPCTSTR );
typedef PVOID (WINAPI* PGETPROCADDRESS)(HINSTANCE,LPCSTR);
typedef DWORD (WINAPI* PGETLASTERROR)(VOID);
typedef VOID (WINAPI* PEXITTHREAD)(DWORD);
typedef BOOL (WINAPI* PVIRTUALFREE)(LPVOID,SIZE_T,DWORD);
typedef BOOL (WINAPI *PCLOSEHANDLE)(HANDLE);
typedef HANDLE (WINAPI *PCREATEFILE)(LPCTSTR,DWORD,DWORD,LPSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE);
typedef BOOL (WINAPI *PWRITEFILE)(HANDLE,LPCVOID,DWORD,LPDWORD,LPOVERLAPPED);

#define MAXINJECTCODESIZE 1024 //ע���������ֽ���
//Զ��ִ��ĳdll�еĺ�����������
typedef DWORD (WINAPI *PCALLBACKFUNC)(PVOID);

typedef struct _INJECTLIBINFO
{
	PLOADLIBRARY pfnLoadLibrary;
	PFREELIBRARY pfnFreeLibrary;
	PGETPROCADDRESS pfnGetProcaddr;
	PGETLASTERROR pfnGetLastError;
	PEXITTHREAD pfnExitThread;
	PVIRTUALFREE pfnVirtualFree;
	PCLOSEHANDLE pfnCloseHandle;
	PCREATEFILE pfnCreateFile;
	PWRITEFILE pfnWriteFile;

	TCHAR szDllName[256];
	TCHAR szFuncName[256];
	bool bFree;//ͬ��ģʽ�£�Զ���߳�ִ����Ϻ��Ƿ�ж��Զ���̼߳��ص�dll
			   //�첽��ʽִ��ʱbFree������Ч��Զ���߳�ִ����󶼻��ͷż��ص�DLL
	HINSTANCE hModule;//[in|out] - ���ָ����hModule����szDllName��Ч��������LoadLibrary����dll
					  //�������szDllNameָ��dll��Ŀ����̣�ͬʱͬ��ִ��ģʽ�»᷵�ؼ���DLL��ģ����
	DWORD dwReturnValue;
	DWORD dwParam;//���ݸ����ý��̵Ĳ���,���û�����
} INJECTLIBINFO;
//Զ���߳�ִ�к�����������
typedef DWORD (WINAPI *PREMOTEFUNC)(INJECTLIBINFO *pInfo);

class cInjectDll
{
private:
	INJECTLIBINFO m_InjectLibInfo;
	DWORD m_dwProcessId;//Զ�̽���ID

	static DWORD WINAPI remoteThreadProc(INJECTLIBINFO *pInfo);
	//Զ��ִ�й��̣��첽���÷�ʽ
	static DWORD WINAPI remoteThreadProc_syn(INJECTLIBINFO *pInfo);

	//����ֵ��0--Զ�̺���ִ�гɹ������������󣬷��ش����
	//���szDllName==NULL,��szFunctionNameΪPREMOTEFUNC����ָ��
	//BOOL ifSyn --- �Ƿ��첽ִ��Զ���߳�
	DWORD _run(LPCTSTR szDllName,LPCTSTR szFunctionName,BOOL ifSyn,PVOID param=NULL,DWORD dwParamSize=0);
public:
	//�޸ı����̵�Ȩ��
	static bool EnablePrivilege(LPCTSTR lpszPrivilegeName,bool bEnable);
	//-----------------------------------------------------
	// ��ȡĿ�꺯����������ڵ�ַ��
	// ��debug���У���Щ�����ĵ�ַʵ��������Ӧ����ת���ַ
	// �����������ת���ַ�õ���������ڵ�ַ
	//-----------------------------------------------------
	static PVOID GetFuncAddress(PVOID addr);
	//��ȡָ����Զ�̽��̵�ID����������
	static DWORD GetPIDFromName(LPCTSTR szRemoteProcessName);
	
private:
	static DWORD FindProcessID (LPCTSTR szRemoteProcessName);
public:
	cInjectDll(LPCTSTR szRemoteProcessName);
	~cInjectDll(){}
	DWORD Inject(LPCTSTR szRemoteProcessName); //�趨ע���Ŀ��exe
	//ͬ��ִ��Զ���߳�
	//������m_InjectLibInfo.bFree����Զ���߳�ִ������Ƿ��ͷż��ص�DLL��Ĭ��ж��
	//�ɶ�m_InjectLibInfo.hModule����Զ���̼߳���dll��ģ����
	//����ֵ��0--Զ�̺���ִ�гɹ������������󣬷��ش����
	DWORD run(LPCTSTR szDllName,LPCTSTR szFunctionName,PVOID param=NULL,DWORD dwParamSize=0)
	{
		if(szDllName==NULL||szFunctionName==NULL) return (DWORD)(-1);
		return _run(szDllName,szFunctionName,FALSE,param,dwParamSize);
	}
	//�첽ִ��Զ���߳�
	//�첽��ʽִ��ʱbFree������Ч��Զ���߳�ִ����󶼻��ͷż��ص�DLL
	//����ֵ��0--Զ�̺���ִ�гɹ������������󣬷��ش����
	DWORD run_syn(LPCTSTR szDllName,LPCTSTR szFunctionName,PVOID param=NULL,DWORD dwParamSize=0)
	{
		if(szDllName==NULL||szFunctionName==NULL) return (DWORD)(-1);
		return _run(szDllName,szFunctionName,TRUE,param,dwParamSize);
	}
	//Զ��ִ�б�exe��ĳ������
	//�����Ķ���ԭ��ΪPREMOTEFUNC���ͣ������е��κ�ϵͳAPIҪ��ʾ����
	DWORD Call(DWORD pid,PREMOTEFUNC pfunc,PVOID param,DWORD paramLen)
	{
		if(pid!=0) m_dwProcessId=pid;
		return _run(NULL,(LPCTSTR)pfunc,FALSE,param,paramLen);
	}	
	//ж��ָ��Ŀ������е�ĳ��dll
	DWORD DeattachDLL(DWORD pid,HMODULE hmdl);

//-------------------2005-01-25 ���������Ƿ��쳣�˳� begin-------------
	//���ӽ�������,�ɹ�����0������������
	DWORD spySelf(HANDLE hEvent,DWORD dwCreationFlags,const char *commandline);
//-------------------2005-01-25 ���������Ƿ��쳣�˳�  end -------------
};


#endif

