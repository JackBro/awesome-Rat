/*******************************************************************
   *	httpclnt.h
   *    DESCRIPTION:HTTPЭ��ͻ�������
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    LAST MODIFY DATE:2006-02-08
   *
   *	net4cpp 2.1
   *	HTTP/1.1
   *******************************************************************/

#ifndef __YY_HTTP_CLIENT_H__
#define __YY_HTTP_CLIENT_H__

#include "httpdef.h"
#include "httpreq.h"
#include "httprsp.h"
#include "proxyclnt.h"

namespace net4cpp21
{
	class httpClient : public socketProxy
	{
	public:
		httpClient(){}
		virtual ~httpClient(){}
		//���http����������������
		void add_reqHeader(const char *szname,const char *szvalue);
		void add_reqCookie(const char *szname,const char *szvalue);
		void add_reqPostdata(const char *szname,const char *szvalue);
		void set_reqPostdata(const char *buf,long buflen);
		//���httpreq�����http�������
		void cls_httpreq(bool ifKeepHeader=false) { m_httpreq.init_httpreq(ifKeepHeader); return; }
		//����http���� lstartRange,lendRange����web���������ļ��ķ�Χ
		//�ɹ�����SOCKSERR_OK  //���ȴ���Ӧ����
		SOCKSRESULT send_httpreq_headX(const char *strurl,long lTimeOut,long lstartRange,long lendRange);
		SOCKSRESULT send_httpreq_head(const char *strurl,long lstartRange=0,long lendRange=-1)
		{//����http��Ӧͷ�����ȴ�����
			return send_httpreq_headX(strurl,-1,lstartRange,lendRange);
		}
		//�ɹ�����http��Ӧ��, <0�������� 0:δ֪����Ӧ��
		SOCKSRESULT send_httpreq(const char *strurl,long lstartRange=0,long lendRange=-1);

		httpResponse & Response() { return m_httprsp; }
		long rspContentLen() { return m_httprsp.lContentLength(); }
		//����http��ӦΪָ�����ļ�(������http��Ӧͷ)
		//���ر����ļ��Ĵ�С��==0��������
		unsigned long save_httpresp(const char *filename)
		{
			return m_httprsp.save_resp(this,filename);
		}
		
	private:
		httpRequest m_httpreq; //http�����������
		httpResponse m_httprsp; //http��Ӧ�������
	};
}//?namespace net4cpp21

#endif

