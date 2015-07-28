/*******************************************************************
   *	mportsvr.h
   *    DESCRIPTION:���ض˿�ӳ�����
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:
   *
   *	net4cpp 2.1
   *	
   *******************************************************************/

#ifndef __YY_MAPPORTSVR_H__
#define __YY_MAPPORTSVR_H__

#include "../../net4cpp21/include/socketSvr.h"
#include "../../net4cpp21/include/buffer.h"
#include "mportdef.h"
#include "deelx.h"
typedef CRegexpT<char> regexp;

namespace net4cpp21
{
	typedef struct _RegCond
	{
		std::string strHeader; //Ҫ�޸ĵ�httpͷ
		std::string strPattern;//Regexpƥ��ģʽ
		std::string strReplto; //Ҫ�滻���ַ���
		_RegCond(){}
		~_RegCond(){}
	}RegCond;
	//HTTP������Ӧͷ��������
	//����TCP�˿�ӳ�������
	class mportTCP : public socketSvr
	{
	public:
		mportTCP();
		virtual ~mportTCP();
		long &Tag() { return m_lUserTag; }
		SSLTYPE getSSLType() const { return m_ssltype; }
		bool getIfLogdata() const { return m_bLogdatafile; }
		void setIfLogdata(bool b){ m_bLogdatafile=b; }
		SOCKSRESULT Start(const char *strMyCert,const char *strMyKey,const char *strKeypwd,
					   const char *strCaCert,const char *strCaCRL); //����ӳ�����
		SOCKSRESULT StartX();
		void Stop(); //ֹͣӳ�����
		
		//����Ҫӳ���Ӧ�÷���
		void setAppsvr(const char *appsvr,int apport,const char *appdesc,MPORTTYPE apptype=MPORTTYPE_UNKNOW);
		void setMapping(int mportStart,int mportEnd,const char *bindip=NULL);
		void setSSLType(SSLTYPE ssltype,bool bSSLVerify);
		void setMaxRatio(unsigned long maxratio) { m_maxratio=maxratio; }

		void xml_info_mtcp(cBuffer &buffer);
		int  str_info_mtcp(const char *mapname,char *buf);

		bool addRegCond(int rspcode,const char *header,const char *pattern,const char *replto);
		bool addRegCond(const char *url,const char *header,const char *pattern,const char *replto);
		bool str_info_regcond(const char *mapname,std::string &strini);
	protected:
		virtual socketTCP * connectAppsvr(char *strHost,socketTCP *psock);
		std::pair<std::string,int> * GetAppsvr(){
			std::pair<std::string,int> *p=NULL;
			int n=m_appSvr.size();
			if(n==1) p=&m_appSvr[0];
			else if(n>1){
				srand(clock());
				p=&m_appSvr[rand()%n]; //�����ȡһ��Ӧ�÷������Ϣ
			}
			return p; 
		}

	private:
		virtual void onAccept(socketTCP *psock); //����һ���µĿͻ����Ӵ˷��񴥷��˺���
		bool AnalysePASV(mportTCP* &pftpDatasvr,char *buf,int len,socketTCP *ppeer);
		static void transDataThread(std::pair<socketTCP *,FILE *> *p);
	private:
		//��ӳ���Ӧ�÷���
		std::vector<std::pair<std::string,int> > m_appSvr;
		MPORTTYPE m_apptype;//ӳ��Ӧ�÷�������
		int m_mportBegin;  //Ҫ��ӳ��˿ڷ�Χ
		int m_mportEnd;
		char m_bindLocalIP[16]; //Ҫ��󶨵ı���IP
		SSLTYPE m_ssltype; //SSLת������
		bool m_bSSLVerify; //SSL�����Ƿ���Ҫ��֤�ͻ���֤��
		long m_lUserTag; //�����Զ���������־,���ڱ���������
		unsigned long m_maxratio; //���������� kb/s
		bool m_bLogdatafile; //�Ƿ��¼�������ת�������ݵ���־�ļ�
		//�޸�HTTP��Ӧͷ��Ϣ int - http��Ӧ����
		std::map<int,std::vector<RegCond> > m_modRspHeader;
		//�޸�HTTP����ͷ��Ϣ string - ƥ��http����URL
		std::map<std::string,std::vector<RegCond> > m_modReqHeader;
		//yyc add 2010-02-13 ���Ӷ�url��д֧��
		//first - Ҫƥ���url(֧��Regexp) ��second - �滻Ϊ(Regexp)
		std::map<std::string,std::string> m_modURLRewriter;
	};
}//?namespace net4cpp21

#endif

