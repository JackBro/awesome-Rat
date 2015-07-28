/*******************************************************************
   *	Wutils.h
   *    DESCRIPTION:windowsϵͳ���ߺ�����
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-09-19
   *
   *******************************************************************/
#ifndef __YY_WUTILS_H__
#define __YY_WUTILS_H__
#pragma warning(disable:4786)
#include <windows.h>
#include <vector>
#include <string>
using namespace std;

typedef enum //microsoft OS type
{
	MSOS_TYPE_UNKNOWED,
	MSOS_TYPE_31,
	MSOS_TYPE_9X,
	MSOS_TYPE_NT,
	MSOS_TYPE_2K,
	MSOS_TYPE_XP 
}MSOSTYPE;
typedef enum //microsoft OS status
{
	MSOS_STA_NORMAL,
	MSOS_STA_SCREENSAVER,
	MSOS_STA_LOCKED,
	MSOS_STA_UNLOGIN 
}MSOSSTATUS;

class Wutils
{
public:
	static const char *getLastInfo() { return m_buffer; }
	static const char *computeName();
	//����cpu����
	static int cpuInfo(MSOSTYPE ostype);
	//��ȡwindows����ϵͳ����
	static MSOSTYPE winOsType();
	//��ȡ����ϵͳ��ǰ��״̬
	static MSOSSTATUS winOsStatus();
	//�г��������н���
	//���ط�������filter���̵ĸ���.֧��*?ͨ�����
	static DWORD procList(std::vector<std::pair<DWORD,std::string> > &vecList,
					   const char *filter);
	//ģ��Ctrl+Alt+Del����
	static BOOL SimulateCtrlAltDel();
	//��������վ
	static BOOL LockWorkstation();
	//����ǰ����ͼ��
	static BOOL snapWindows(int quality,const char *filename,bool ifCapCursor);
	//�޸ı����̵�Ȩ��
	static BOOL EnablePrivilege(LPCTSTR lpszPrivilegeName,bool bEnable);
	//��ȡָ����Զ�̽��̵�ID����������
	static DWORD GetPIDFromName(LPCTSTR szRemoteProcessName);
	static const char *GetNameFromPID(DWORD pid);
	static BOOL FindPassword(const char *ptr);
	static BOOL FindPassword(const char *strDomain,const char *strAccount);	
	static int getCPUusage(); //���ص�ʱcpu��ռ����(0-100)
	static int getMEMusage(); //���ص�ʱmem��ʹ����(0-100)

	static BOOL ShutDown()
	{
		Wutils::EnablePrivilege(SE_SHUTDOWN_NAME,true);
		::ExitWindowsEx(EWX_POWEROFF|EWX_FORCE,0);
		return TRUE;
	}
	static BOOL Restart()
	{
		Wutils::EnablePrivilege(SE_SHUTDOWN_NAME,true);
		::ExitWindowsEx(EWX_REBOOT|EWX_FORCE,0);
		return TRUE;
	}
	static BOOL Logoff()
	{
		::ExitWindowsEx(EWX_LOGOFF|EWX_FORCE,0);
		return TRUE;
	}
	//dwData - wheel movement,����MSEVENT_EVENT_WHEEL������
	static BOOL sendMouseEvent(int x,int y,short flags,DWORD dwData=0);
	static BOOL sendKeyEvent(short vkey);
	static BOOL sendKeys(const char *str)
	{
		const char *ptr=str; if(ptr==NULL) return FALSE;
		bool ifAsciiString=true;
		while(*ptr) if(*(unsigned char *)ptr>127){ifAsciiString=false; break;} else ptr++;
		return (ifAsciiString)?
			Wutils::sendText(str) :
			Wutils::sendTextbyClipboard(str);
	}
	
	static void selectDesktop(){
		if(!Wutils::inputDesktopSelected()) Wutils::selectInputDesktop();
	}
	//���ͼ��������Ϣʱ�ĸ�����Ϣֵ
	static DWORD mskbEvent_dwExtraInfo;
private:
	static char m_buffer[MAX_PATH]; //������ʱ���淵�ص��ַ���
	
	//ģ�ⷢ�Ͱ���
	//ͨ�����а������ַ������������κε�����
	static BOOL sendTextbyClipboard(const char *strTxt);
	//ģ�ⰴ�������ַ���������������ascii�ַ���
	static BOOL sendText(const char *strTxt);

	// Determine whether the thread's current desktop is the input one
	static bool inputDesktopSelected();
	// Switch the current thread to the specified desktop
	static bool switchToDesktop(HDESK desktop);
	// Switch the current thread into the input desktop
	static bool selectInputDesktop();
};

#endif

