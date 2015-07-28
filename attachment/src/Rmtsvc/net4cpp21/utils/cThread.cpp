   /*******************************************************************
   *	cThread.cpp
   *    DESCRIPTION:�߳����ʵ��
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2004-10-10
   *	net4cpp 2.1
   *******************************************************************/

#include "../include/sysconfig.h"  
#include "../include/cThread.h"
#include "../include/cLogger.h"

#include <vector>
using namespace std;
using namespace net4cpp21;

cThread::cThread()
{
	m_thrid=0;
	m_bStarted=false;
	m_threadfunc=NULL;
	m_pArgs=NULL;
}
cThread::~cThread()
{
	join(0); //yyc add 2009-12-15
	//�̶߳����ͷ�ʱ�������߳��Ƿ����������û�Ҫ�����������Լ�����join����
#ifdef WIN32 //windowsϵͳƽ̨
	if(m_thrid) CloseHandle((HANDLE)m_thrid);//�رմ򿪵��߳̾��
#else //unix/linuxƽ̨
	if(m_thrid) pthread_detach(m_thrid);//���̷߳���,�߳̽������Լ��ͷ������Դ
#endif
}

bool cThread::start(THREAD_CALLBACK *pfunc,void *pargs)
{
	if((m_threadfunc=pfunc)==NULL) return false;
	m_pArgs=pargs;
#ifdef WIN32 //windowsϵͳƽ̨
	unsigned int id;
	m_thrid=_beginthreadex(0,0,&threadfunc,(void *)this,0,&id);
	if(m_thrid==0)  return false;
#else //unix/linuxƽ̨
	int res = 0;
    	res = pthread_create(&m_thrid, 0, &threadfunc, (void *)this);
    	if (res != 0){ m_thrid=0; return false;}
#endif
	return true;
}

void cThread::join(time_t timeout)//ֹͣ�̲߳��ȴ��߳̽����ŷ���
{
	if(m_thrid==0) return;
#ifdef WIN32 //windowsϵͳƽ̨
	DWORD dwMilliseconds=INFINITE;
	if(timeout>=0) dwMilliseconds=timeout*1000;
	int res=WaitForSingleObject((HANDLE)m_thrid, dwMilliseconds);
	if(res==WAIT_TIMEOUT) ::TerminateThread((HANDLE)m_thrid,0);
//	if(res==WAIT_TIMEOUT){printf("aaaaaaqqqq\r\n"); ::TerminateThread((HANDLE)m_thrid,0);}
	CloseHandle((HANDLE)m_thrid);
#else //unix/linuxƽ̨
	//���������������ȴ��ĺ�������usleep(),select()��...
	pthread_kill(m_thrid,SIGALRM);
	pthread_join(m_thrid, 0);
#endif
	m_thrid=0;
	return;
}

#ifdef WIN32 //windowsϵͳƽ̨
unsigned int __stdcall cThread::threadfunc(void* param)
#else //unix/linuxƽ̨
void* cThread::threadfunc(void* param)
#endif
{
	cThread *pthread=(cThread *)param;
	pthread->m_bStarted=true;
	(*(pthread->m_threadfunc))(pthread->m_pArgs);
	//yyc 2009-12-15�ָ� ��ʹ��cTread����ʱ���뱣֤cThread�����ͷ�ʱ�߳��Ѿ�����
	pthread->m_bStarted=false; //yyc remove cThread *��������Ѿ���Ч��
	pthread_exit(0);
	return 0;	
}

//--------------------------------------------------------------------------------------------
//---------------------cThreadPool���ʵ��-----------------------------------------------------
cThreadPool :: cThreadPool()
{
	m_taskid=0;
}
cThreadPool :: ~cThreadPool()
{
	join();
}
//ֹͣ�̳߳��е������̲߳��ȴ��߳̽����ŷ���
void cThreadPool :: join(time_t timeout)
{
	vector<pthread_t> vec;//��ʱ�������й����̵߳ľ����id
	m_mutex.lock();
	m_tasklist.clear(); //��������б�
	if(!m_thrmaps.empty()){
		map<pthread_t,THREADPARAM>::iterator it=m_thrmaps.begin();
		for(;it!=m_thrmaps.end();it++)
		{
			THREADPARAM &thrparam=(*it).second;
			vec.push_back(thrparam.m_thrid);
			thrparam.m_thrid=0;//�����߳̿��Բ��ش�m_thrmaps��erase
			thrparam.m_waittime=0;
			thrparam.m_pcond->active();
		}
	}//?if(!m_thrmaps.empty()){
	m_mutex.unlock();
	//�ȴ�������ִ�еĹ����߳̽���
	if(!vec.empty()){
		DWORD dwMilliseconds=INFINITE;
		if(timeout>=0) dwMilliseconds=timeout*1000;
		vector<pthread_t>::iterator itVec=vec.begin();
		for(;itVec!=vec.end();itVec++)
		{
#ifdef WIN32 //windowsϵͳƽ̨
			WaitForSingleObject((HANDLE)(*itVec), dwMilliseconds);
#else //unix/linuxƽ̨
			//���������������ȴ��ĺ�������usleep(),select()��...
			pthread_kill(*itVec,SIGALRM);
			pthread_join(*itVec, 0);
#endif		
		}//?for(;itVec...
	}//?if(!vec.empty()){
	m_thrmaps.clear();
}

//��ʼ�������̸߳���
//threadnum --- Ҫ�´������̸߳���
//waittime --- �´������߳��������ָ����ʱ�����Ȼû���������Զ�����
//		���==-1��һֱ����֪����ָ��������Ҫ����
//���ص�ǰ�ܵĹ����̸߳���
long cThreadPool :: initWorkThreads(long threadnum,time_t waittime)
{
	for(long i=0;i<threadnum;i++)
		createWorkThread(waittime);
	long threads=0;
	m_mutex.lock();
	threads=m_thrmaps.size();
	m_mutex.unlock();
	return threads;
}
//���һ����������������
//pfunc --- ������ָ��
//pargs --- ���ݸ��������Ĳ���
//waittime --- �����ǰ�̳߳��е��̶߳���ռ���Ƿ���ʱ����һ���µ��߳̽����̳߳�
//		���<0�򲻴������ȴ������߳̿��к������򴴽�����ʱwaittimeΪ�����߳���ɴ��������̳߳��е�������ߵȴ�ʱ��
//����ɹ��򷵻�����TASKID�����򷵻�0
TASKID cThreadPool :: addTask(THREAD_CALLBACK *pfunc,void *pargs,time_t waittime)
{
	if(pfunc==NULL) return 0;
	TASKPARAM taskparam;
	taskparam.m_taskid=++m_taskid;
	taskparam.m_pFunc=pfunc;
	taskparam.m_pArgs=pargs;
	m_mutex.lock();
	m_tasklist.push_back(taskparam); //��ӵ������б���
	m_mutex.unlock();
	bool bStarted=false; long threads; //��ǰ�̸߳���
	m_mutex.lock();
	threads=m_thrmaps.size();
	map<pthread_t,THREADPARAM>::iterator it=m_thrmaps.begin();
	for(;it!=m_thrmaps.end();it++)
	{
		THREADPARAM &thrparam=(*it).second;
		if(thrparam.m_pcond && thrparam.m_pcond->status()) //���߳�����������״̬
		{
			thrparam.m_pcond->active();
			bStarted=true;
			break;
		}
	}//?for(;it!=...
	m_mutex.unlock();
	if(!bStarted && waittime>=0 ){ //����һ����ʱ�߳�ִ�д�������
		if(createWorkThread(waittime)==0){ //�����߳�ʧ��
			if(delTask(taskparam.m_taskid)){
				RW_LOG_PRINT(LOGLEVEL_WARN,0,"��һ������δ��ִ��!\r\n");
				return 0; //ɾ���������ɾ�����ɹ�˵�������ѱ������߳�����
			}//?if(delTask(taskparam.m_taskid))
		}
		else
			threads++;
	}//?if(!bStarted &&...
//	RW_LOG_DEBUG("taskid=%d,threads=%d\r\n",taskparam.m_taskid,threads);
	return taskparam.m_taskid;
}
//���ĳ�������Ƿ��������б��д�ִ��
//���bRemove==true��������б���ɾ��
//����������б����򷵻�����򷵻ؼ�
bool cThreadPool :: delTask(TASKID taskid,bool bRemove)
{
	bool bRet=false;
	m_mutex.lock();
	deque<TASKPARAM>::iterator it=m_tasklist.begin();
	for(;it!=m_tasklist.end();it++)
	{
		TASKPARAM &taskparam=*it;
		if(taskparam.m_taskid==taskid)
		{
			if(bRemove) m_tasklist.erase(it);
			bRet=true;
			break;
		}
	}//?for(;it!=...
	m_mutex.unlock();
	return bRet;
}
//������д�ִ�е�����
void cThreadPool :: clearTasks()
{
	m_mutex.lock();
	m_tasklist.clear();
	m_mutex.unlock();
	return;
}

//����һ�������̣߳��ɹ������߳�ID�����򷵻�0
pthread_t cThreadPool :: createWorkThread(time_t waittime)
{
	THREADPARAM thrparam;
	pthread_t thrid=0;
	//��ʼ��THREADPARAM
	thrparam.m_waittime=waittime;
	if( (thrparam.m_pcond=new cCond())==NULL) return 0;
	thrparam.m_thrid=0;
	//���������߳�
	m_mutex.lock();//�������������߳����к����������ֱ������m_thrmaps[thrid]=thrparam;
#ifdef WIN32 //windowsϵͳƽ̨
	if( (thrparam.m_thrid=_beginthreadex(0,0,&workThread,(void *)this,0,(unsigned int *)&thrid))==0 )
		thrid=0;
#else //unix/linuxƽ̨
	int res = 0;
    	res = pthread_create(&m_thrid, 0, &threadfunc, (void *)this);
    	if (res != 0)
    		thrparam.m_thrid=0;
    	else
    		thrid=thrparam.m_thrid;
#endif
	//�̴߳�����Ϻ���ӵ��̶߳�����
	if(thrid!=0)
	{
		m_thrmaps[thrid]=thrparam;
		thrparam.m_pcond=NULL;
	}
	m_mutex.unlock();
	if(thrparam.m_pcond) delete thrparam.m_pcond;	
	return thrid;
}
	
//�̳߳��еĹ����߳�
#ifdef WIN32 //windowsϵͳƽ̨
unsigned int __stdcall cThreadPool::workThread(void* param)
#else //unix/linuxƽ̨
void* cThreadPool::workThread(void* param)
#endif
{
	cThreadPool *pthreadpool=(cThreadPool *)param;
	if(pthreadpool==NULL) { pthread_exit(0); return 0; }
	pthread_t thrid=pthread_self();//�õ�Ψһ��ʶ���̵߳�identifier
	THREADPARAM *pthrparam=NULL;
	pthreadpool->m_mutex.lock();
	if (pthreadpool->m_thrmaps.count(thrid)>0 )
		pthrparam=&pthreadpool->m_thrmaps[thrid];
	pthreadpool->m_mutex.unlock();
	if(pthrparam)
	{
#ifdef WIN32
		HANDLE Hthread=(HANDLE)pthrparam->m_thrid;
#endif
		cCond *pcond=pthrparam->m_pcond;
		TASKPARAM taskparam;
		do
		{//��ȡ��������
NEXTTASK:		 
			taskparam.m_taskid=0;
			pthreadpool->m_mutex.lock();
			if(pthreadpool->m_tasklist.size()>0)
			{
				taskparam=pthreadpool->m_tasklist.front();
				pthreadpool->m_tasklist.pop_front();
			}
			pthreadpool->m_mutex.unlock();
			if(taskparam.m_taskid!=0)
			{
				(*(taskparam.m_pFunc))(taskparam.m_pArgs);
				goto NEXTTASK;
			}
		}while(pcond->wait(pthrparam->m_waittime));
		pthrparam->m_pcond=NULL; delete pcond;
		if(pthrparam->m_thrid!=0){
			pthreadpool->m_mutex.lock();
			pthreadpool->m_thrmaps.erase(thrid);
			pthreadpool->m_mutex.unlock();
		}
#ifdef WIN32
		CloseHandle(Hthread);//�رմ򿪵��߳̾��
#endif
	}//?if(pthrparam->)
	//�����߳̽���
	pthread_exit(0);
	return 0;	
}

/*���� _beginthreadex��CreateThread������
_beginthreadex��΢���C/C++����ʱ�⺯����CreateThread�ǲ���ϵͳ�ĺ�����
_beginthreadexͨ������CreateThread��ʵ�ֵģ�����CreateThread��������๤��.
CreateThread��_beginthread��_beginthreadex�������������̵߳�
��_beginthread��_beginthreadex�Ĺ����Ӽ�����Ȼ_beginthread�ڲ��ǵ���
_beginthreadex������������ȫ���������Ĺ��ܣ�����_beginthread��CreateThread����ͬ�ȼ���
_beginthreadex��CreateThread�ڹ�������ȫ����������Ǿ����Ƚ�һ��_beginthreadex��CreateThread!

CRT�ĺ��������̳߳���֮ǰ���Ѿ����ڣ�����ԭ�е�CRT��������֧���̣߳��⵼�������ڱ�̵�ʱ��
����CRT���ѡ����MSDN�в���CRT�ĺ���ʱ���У�
Libraries
LIBC.LIB Single thread static library, retail version 
LIBCMT.LIB Multithread static library, retail version 
MSVCRT.LIB Import library for MSVCRT.DLL, retail version 
��������ʾ��
�����̵߳�֧���Ǻ������£�
��Ҳ���������CRT�ĺ����ڶ��̵߳�����±����������֧�֣����ܼ򵥵�ʹ��CreateThread��OK��
����CRT������������CreateThread�߳���ʹ�ã�������˵ֻ��signal()���������ԣ��ᵼ�½�����ֹ��
�������ò�����˵û�����⣡

��ЩCRT�ĺ�����malloc(), fopen(), _open(), strtok(), ctime(), ��localtime()�Ⱥ�����Ҫר��
���ֲ߳̾��洢�����ݿ飬������ݿ�ͨ����Ҫ�ڴ����̵߳�ʱ��ͽ��������ʹ��CreateThread��
������ݿ��û�н�����Ȼ��������أ����������߳��л��ǿ���ʹ����Щ��������û�г���
ʵ���Ϻ�������������ݿ��ָ��Ϊ��ʱ�����Լ�����һ����Ȼ�������߳���ϵ��һ������ζ��
�������CreateThread�������̣߳�Ȼ��ʹ�������ĺ���������һ���ڴ��ڲ�֪�����д������ź���
�ǣ���Щ������������ɾ������CreateThread��ExitThreadҲ�޷�֪������£����Ǿͻ���
Memory Leak�����߳�Ƶ�������������(����ĳЩ���������)���������ϵͳ���ڴ���Դ�ľ���

_beginthreadex(�ڲ�Ҳ����CreateThread)��_endthreadex�Ͷ�����ڴ�����˴�������û�����⣡
(�������˹�����CreateThread����Ȼ����_endthreadex��ֹ�ɣ������̵߳���ֹ��ò�Ҫ��ʽ�ĵ���
��ֹ��������Ȼ�˳���ã�)

̸��Handle�����⣬_beginthread�Ķ�Ӧ����_endthread�Զ��ĵ�����CloseHandle����
_beginthreadex�Ķ�Ӧ����_endthreadex��û�У�����CloseHandle������ζ���Ҫ���õĲ���
_endthread���԰���ִ���Լ�����д���������־���Ҫ�Լ�д��(Jeffrey Richterǿ���Ƽ�����
������ʽ����ֹ����������Ȼ�˳��ķ�ʽ����Ȼ�˳���Ȼ��һ��Ҫ�Լ�дCloseHandle)
*/
