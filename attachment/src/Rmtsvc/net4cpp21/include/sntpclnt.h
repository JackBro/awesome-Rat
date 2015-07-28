/*******************************************************************
   *	sntpclnt.h
   *    DESCRIPTION:SNTP������ʱ��ͬ��Э��ͻ�������
   *
   *	http://hi.baidu.com/yycblog/home
   *	net4cpp 2.1
   *******************************************************************/
#ifndef __YY_SNTP_CLINET_H__
#define __YY_SNTP_CLINET_H__

#include "sntpdef.h"
#include "socketUdp.h"

namespace net4cpp21
{

	class CSNTPClient
	{
	public:
		CSNTPClient();
		~CSNTPClient(){}
		DWORD GetTimeout() const { return m_dwTimeout; };
		void  SetTimeout(DWORD dwTimeout) { m_dwTimeout = dwTimeout; };
		const char *GetLastError() { return m_strLastError.c_str(); }
		//General functions
		bool  GetServerTime(const char * szHostName, NtpServerResponse& response,
					int nPort = SNTP_SERVER_PORT);
		//���ݷ��ؽ��ͬ������ʱ��
		bool SynchroniseClock(NtpServerResponse& response);
		
	protected:
		bool EnableSetTimePriviledge();
		void RevertSetTimePriviledge();
		bool  SetClientTime(const CNtpTime& NewTime);

		DWORD            m_dwTimeout; //������Ӧ��ʱ
		HANDLE           m_hToken;
		TOKEN_PRIVILEGES m_TokenPriv;
		std::string		 m_strLastError;	//������Ϣ
	};
};//namespace net4cpp21

#endif

