/*******************************************************************
   *	cMutex.h
   *    DESCRIPTION:����������,������������
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2004-10-10
   *	net4cpp 2.1
   *******************************************************************/
  
#ifndef __YY_CMUTEX_H__
#define __YY_CMUTEX_H__

#include <ctime>

#ifdef WIN32 //windowsϵͳƽ̨
	//win32������
	#define pthread_mutex_t CRITICAL_SECTION 
	#define pthread_mutex_lock(m) EnterCriticalSection(m) 
	#define pthread_mutex_unlock(m) LeaveCriticalSection(m) 
	#define pthread_mutex_init(m,p) InitializeCriticalSection(m)
	#define pthread_mutex_destroy(m) DeleteCriticalSection(m)
	
	//condition
	#define timespec time_my
	#define pthread_cond_t HANDLE
	#define pthread_cond_init(pCond,p) \
	{ \
		*(pCond)=CreateEvent(NULL,false,false,NULL); \
	}
	#define pthread_cond_destroy(pCond) CloseHandle(*(pCond))
	#define pthread_cond_wait(pCond,pMutex) WaitForSingleObject(*(pCond),INFINITE)
	#define pthread_cond_timedwait(pCond,pMutex,pt) WaitForSingleObject(*(pCond),(pt)->tv_nsec*1000)
	#define pthread_cond_signal(pCond) SetEvent(*(pCond))
#elif defined MAC //��ʱ��֧��
	//....
#else  //unix/linuxƽ̨ //����POSIX��Ķ��̺߳ͻ�������������������
	extern "C"
	{ //���̺߳���,������
		#include <pthread.h>
	}
#endif


namespace net4cpp21
{
	class cMutexBase
	{
	protected:
		unsigned long m_lLock;//�Ƿ�������״̬
	public:
		cMutexBase():m_lLock(0){}
		virtual ~cMutexBase(){}
		bool isLock(){return (m_lLock==0);}
		virtual void lock()
		{
			m_lLock++;
		}
		virtual void unlock()
		{
			m_lLock--;
		}
		virtual pthread_mutex_t *address()
		{
			return NULL;
		}
	};
	class cMutex:public cMutexBase
	{
	private:
		pthread_mutex_t m_mutex;
	public:
		cMutex()
		{
			pthread_mutex_init(&m_mutex,NULL);
		}
		virtual ~cMutex()
		{
			pthread_mutex_destroy(&m_mutex);
		}
		
		void lock()
		{
			cMutexBase::lock();
			pthread_mutex_lock(&m_mutex);
		}
		void unlock()
		{
			pthread_mutex_unlock(&m_mutex);
			cMutexBase::unlock();
		}

		pthread_mutex_t *address()
		{
			return &m_mutex;
		}
	};
	
	
	struct time_my
	{
		unsigned int tv_sec;
		long tv_nsec;
	};
	//��������
	class cCond
	{
	private:
#ifdef WIN32
		cMutexBase m_mutex; //�ٵĻ�����������ʵ��û���κ����壬��Ϊwindows�����������ͻ�����û�й�ϵ
#else
		cMutex m_mutex; //linux/unix�µȴ����õ���������
		//linux/unix��������������ͻ��������ʹ��,�Է�ֹ����߳�ͬʱ����pthread_cond_wait()����pthread_cond_timedwait()����ͬ���ľ���
		//�ڵ���pthread_cond_wait()ǰ�����ɱ��̼߳���,mutex��������״̬�������̹߳������ȴ�ǰ����������������Ӷ��뿪pthread_cond_wait()֮ǰ��mutex�������¼������������pthread_cond_wait()ǰ�ļ���������Ӧ
#endif
        pthread_cond_t m_cond;
		int m_status; //0���ڷǵȴ�״̬�������ڵȴ�״̬
		unsigned long m_args;//�û��ɴ��ݶ������
	public:
		cCond()
		{
			m_status=0;
			m_args=0;
            		pthread_cond_init(&m_cond,NULL);
		}
		~cCond()
		{
			if(m_status!=0) pthread_cond_signal(&m_cond);
            		pthread_cond_destroy(&m_cond);
		}
		bool wait(time_t seconds=-1) //������--���������ʱ
		{
			if(m_status!=0) return false;
			m_status=1;//�ȴ�����״̬
			m_mutex.lock();
			if(seconds<0) //���޵ȴ���ֱ��������
			{
				 pthread_cond_wait(&m_cond,m_mutex.address());
			}
			else
			{
                struct  timespec t;
				t.tv_sec = time(NULL) + seconds;
				t.tv_nsec = seconds;//windows ϵͳ��ͨ��tv_nsecȡ�ö�ʱ����!!!
				pthread_cond_timedwait(&m_cond,m_mutex.address(),&t);
			}
            m_mutex.unlock();
            if(m_status==0) return true;
            m_status=0;
            return false;
		}
		
		void active()
		{
            if(m_status!=0){
            	m_status=0;
            	pthread_cond_signal(&m_cond);
            }
            return;
		}

		pthread_cond_t *address()
		{
			return &m_cond;
		}

		bool status() //�Ƿ�Ϊ�ȴ�״̬
		{
			return (m_status!=0)?true:false;
		}

		unsigned long getArgs(){return m_args;}
		void setArgs(unsigned long arg){m_args=arg;return;}
	};
}//?namespace net4cpp21

#endif
