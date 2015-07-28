/*******************************************************************
   *	cTthread.h
   *    DESCRIPTION:�߳���
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2004-10-10
   *	net4cpp 2.1
   *******************************************************************/
  
#ifndef __YY_CTHREAD_H__
#define __YY_CTHREAD_H__

#ifdef WIN32 //windowsϵͳƽ̨
	// ��ʹ�ô˶��߳���ʱ��This program requires the multithreaded library. 
	// ��vc����ʱҪָ�� /MT��/MD����
	// /MT��ʾͬC���п�Ķ��̰߳澲̬���ӡ����Ҫ��̬���ӣ���/MD��
	//		���VC++��v5.0�Ļ������и߰汾��msvcrt.dll��Ӧ���ö�̬���ӡ�
	// If you are using the Visual C++ development environment, select the 
   	// Multi-Threaded runtime library in the compiler Project Settings dialog box.	
   	// The Multithread Libraries Compile Option
   	// To build a multithread application that uses the C run-time libraries, 
   	// you must tell the compiler to use a special version of the libraries (LIBCMT.LIB).
   	// To select these libraries, first open the Project Settings dialog box (Build menu) 
   	// and click the C/C++ tab. Select Code Generation from the Category drop-down list box. 
   	// From the Use Run-Time Library drop-down box, select Multithreaded. Click OK to return to editing.
	extern "C"
	{//���߳�
		#include <process.h>
	}
	#define pthread_t unsigned long
	#define pthread_self GetCurrentThreadId
	#define pthread_exit(retcode) \
	{ \
		_endthreadex(retcode); \
	}
#elif defined MAC //��ʱ��֧��
	//....
#else  //unix/linuxƽ̨
	//Ϊ�˱�֤����ֲ���˴��Ķ��̺߳����õ���POSIX�Ķ��߳̿⣬��˱���ʱӦָ������ -lpthread�⡣�����sunϵͳ����Ķ��̺߳�����Ӧ�ƶ�����-lthread�⡣
	//����STL�Ķ��̰߳�ȫ
	//��sun��ϵͳ�±�д���̳߳���ʱ���������c++��STL�⣬��ҪС�ģ���Ϊ��ᷢ����sun���п��ܻᷢ��core dump����ͬ���ĳ�����linux��ȱʼ��û�����⡣
	//��gdb��λcore dumpλ�ã����ֿ��ܻ����������κεط���������STL���stl_alloc.h���ڴ���䴦�������⣬ĳ����ֵַ���޸�Ϊһ���Ƿ��ĵ�ַ���³�
	//���⣬��Ȼ��Ҳ�п�������������ڴ�Ƿ����������⣬���ͷ���ĳ���ڴ�ռ�ȴ�������ô˵�ַ�����stl_alloc.h�ļ���ᷢ��ԭ��STL���ڴ����Ĭ��Ϊ
	//single_pthreadģʽ������붨��ĳ����Ż�֧�ֶ��߳�ģʽ������POSIX���̡߳�win32���̣߳�Solaris���̵߳ȡ�
	//���sunϵͳ�±�дPOSIX��׼�Ķ��̳߳�������õ���STL�⣬���ڱ���ʱһ��Ҫָ��-D__STL_PTHREADS�������
	//linux�¿���Ĭ�����Զ��̰߳�ȫ��ʽ����ģ���˲�������⡣�����Ķ��̺߳궨����ɿ���stl_alloc.hͷ�ļ���
	//���߳�
	extern "C"
	{ //���̺߳���,������
		#include <pthread.h>
	}
#endif

#include "cMutex.h"
#include <map>
#include <deque>

#define THREADLIVETIME 5 //s ,��ʱ�߳��ڶ೤ʱ����û���������Զ�����
namespace net4cpp21
{
	typedef void (THREAD_CALLBACK)(void *);
	class cThread
	{
		pthread_t m_thrid;//�߳�ID���߳̾��(windows)
		THREAD_CALLBACK *m_threadfunc;
		void *m_pArgs;//���ݸ��̺߳����Ĳ���
		bool m_bStarted;//�߳��Ƿ���������
	private:
#ifdef WIN32 //windowsϵͳƽ̨
    	static unsigned int __stdcall threadfunc(void* param);
#else //unix/linuxƽ̨
        static void* threadfunc(void* param);
#endif
	public:
		cThread();
		~cThread();
		bool start(THREAD_CALLBACK *pfunc,void *pargs);
		void join(time_t timeout=-1);//ֹͣ�̲߳��ȴ��߳̽����ŷ���,timeout :s�ȴ���������
		bool status(){return m_bStarted;} //���ص�ǰ�߳��Ƿ�����
	};
	
	//*******************************************************************************
	//�̳߳���************************************************************************
	//�ɶ�̬��������߳̽����̳߳أ��̳߳ص��߳̽���һ��������Զ�ȡ�����б��е��������ִ��
	#define TASKID unsigned long
	typedef struct _TASKPARAM //��������ṹ
	{
		TASKID m_taskid;//Ψһ��ʶ�����������ID
		THREAD_CALLBACK *m_pFunc;//���������ָ��
		void *m_pArgs;//���ݸ�����Ĳ���
	}TASKPARAM;
	typedef struct _THREADPARAM //�̲߳����ṹ
	{
		pthread_t m_thrid;//�߳�ID���߳̾��(windows)
		time_t m_waittime;//�߳����ߵȴ�ʱ��
		cCond *m_pcond;
	}THREADPARAM;
	class cThreadPool
	{
		TASKID m_taskid;//�����Ψһ��ʶ
		cMutex m_mutex;//������
		std::deque<TASKPARAM> m_tasklist;//�����б�
		std::map<pthread_t,THREADPARAM> m_thrmaps;//�̶߳���
	public:
		cThreadPool();
		~cThreadPool();
		void join(time_t timeout=-1);
		//��ʼ�������̸߳���
		//threadnum --- Ҫ�´������̸߳���
		//waittime --- �´������߳��������ָ����ʱ�����Ȼû���������Զ�����
		//		���==-1��һֱ����֪����ָ��������Ҫ����
		//���ص�ǰ�ܵĹ����̸߳���
		long initWorkThreads(long threadnum,time_t waittime=-1);
		//���һ����������������
		//pfunc --- ������ָ��
		//pargs --- ���ݸ��������Ĳ���
		//waittime --- �����ǰ�̳߳��е��̶߳���ռ���Ƿ���ʱ����һ���µ��߳̽����̳߳�
		//		���<0�򲻴������ȴ������߳̿��к������򴴽�����ʱwaittimeΪ�����߳���ɴ��������̳߳��е�������ߵȴ�ʱ��
		//����ɹ��򷵻�����TASKID�����򷵻�0
		TASKID addTask(THREAD_CALLBACK *pfunc,void *pargs,time_t waittime);
		//���ĳ�������Ƿ��������б��д�ִ��
		//���bRemove==true��������б���ɾ��
		//����������б����򷵻�����򷵻ؼ�
		bool delTask(TASKID taskid,bool bRemove=true);
		//������д�ִ�е�����
		void clearTasks();
		//���ص�ǰ��ִ�е�������
		long numTasks(){ long lret=0; m_mutex.lock(); lret=m_tasklist.size();m_mutex.unlock();return lret;}
		//���ص�ǰ�Ĺ����̸߳���
		long numThreads(){ long lret=0; m_mutex.lock(); lret=m_thrmaps.size();m_mutex.unlock();return lret;}
	private:
		//����һ�������̣߳��ɹ������߳�ID�����򷵻�0
		pthread_t createWorkThread(time_t waittime);
#ifdef WIN32 //windowsϵͳƽ̨
    	static unsigned int __stdcall workThread(void* param);
#else //unix/linuxƽ̨
        static void* workThread(void* param);
#endif
	};
}//?namespace net4cpp21


#endif





