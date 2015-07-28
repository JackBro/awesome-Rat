/*******************************************************************
   *	sniffer.h
   *    DESCRIPTION:sniffer������(����ģʽ)
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:2005-12-10
   *	
   *	net4cpp 2.1
   *******************************************************************/
#ifndef __YY_SNIFFER_H__
#define __YY_SNIFFER_H__

#include "socketRaw.h"
#include "IPRules.h"
#include "cThread.h"

namespace net4cpp21
{
	class sniffer : public socketRaw
	{
	public:
		sniffer(){}
		virtual ~sniffer();
		virtual void Close();
		iprules &rules() { return m_rules;}
		void setLogfile(const char *filename){
			if(filename!=NULL && filename[0]!=0)
				m_logfile.assign(filename);
			else m_logfile="";
			return;
		}
		//�Ƿ��¼ip�����ļ�
		bool ifLog()  const { return m_logfile!=""; }
		//��ip��¼�ļ���ģ��sniff
		bool openLogfile(const char *logfile);

		//bindip==NULL��==""Ĭ�ϰ󶨱�����һ��ip
		//�����ָ����ip,����Raw socket����sniff
		//�ɹ�����SOCKSERR_OK��(����sniff�����IP���Ա�ָ�����û���ģʽ������,����10022����)
		SOCKSRESULT sniff(const char *bindip);
	protected:
		//�����ݵ���
		virtual void onData(char *dataptr);
	private:
		std::string m_logfile; //ip����¼�ļ�
		iprules m_rules;//���˹���
		cThread m_thread;
		static void sniffThread(sniffer *psniffer);
		static void sniffThread_fromfile(sniffer *psniffer);
	}; 

}//?namespace net4cpp21

#endif

