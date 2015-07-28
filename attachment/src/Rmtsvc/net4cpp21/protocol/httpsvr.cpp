/*******************************************************************
   *	httpsvr.cpp
   *    DESCRIPTION:HTTPЭ������ʵ��
   *
   *    AUTHOR:yyc
   *
   *    http://hi.baidu.com/yycblog/home
   *
   *    DATE:
   *
   *	net4cpp 2.1
   *	HTTP/1.1����Э��
   *******************************************************************/

#include "../include/sysconfig.h"
#include "../include/httpsvr.h"
#include "../utils/cTime.h"
#include "../utils/utils.h"
#include "../include/cLogger.h"

using namespace std;
using namespace net4cpp21;

const long httpSession::SESSION_VALIDTIME=20*60;//20����
const char httpSession::SESSION_IDNAME[]="aspsessionidgggggled";//"ASPSESSIONIDGGGGGLEC";

httpSession::httpSession()
{
	m_startTime=time(NULL);
	m_lastTime=m_startTime;
	//����ΨһsessionID
	srand( (unsigned)m_startTime );
	/* Display 23 ��д��ĸ. */
	for(int i = 0;   i < 23;i++ )
	{
		 m_sessionID[i]=(rand()*25)/RAND_MAX+65;
	} m_sessionID[23]=0;
}
//�û������Լ������µ�sessionID���������Զ����ɵ�
bool httpSession::SetSessionID(const char *strID)
{
	if(strID==NULL || strID[0]==0) return false;
	for(int i=0;i<23,*strID;i++) m_sessionID[i]=*strID++;
	m_sessionID[i]=0; return true;
}

httpServer :: httpServer()
{
	m_strSvrname="HTTP Server";
}

httpServer :: ~httpServer()
{
	Close();
	//HTTP��������ǰҪ��֤�̶߳���������Ϊ�߳��з�����httpServer��Ķ���
	m_threadpool.join(); 

	std::map<std::string,httpSession *>::iterator it=m_sessions.begin();
	for(;it!=m_sessions.end();it++) delete (*it).second;
}

//����web�������Ŀ¼�Լ���Ŀ¼
bool httpServer :: setvpath(const char *vpath,const char *rpath,long lAccess)
{
	if(vpath==NULL || rpath==NULL) return false;
	if(vpath[0]!='/') return false;
	std::string str_vpath(vpath);
	::_strlwr((char *)str_vpath.c_str());
	if(str_vpath[str_vpath.length()-1]!='/') str_vpath.append("/");
	std::pair<std::string,long> p(rpath,lAccess);
#ifdef WIN32
	if(p.first!="") //ĩβ���'\'
		if(p.first[p.first.length()-1]!='\\') p.first.append("\\");
#endif
	m_dirAccess[str_vpath]=p;
	return true;
}

//�����ǰ���������ڵ�ǰ�趨������������򴥷����¼�
void httpServer :: onTooMany(socketTCP *psock)
{
	psock->Send(0,"HTTP/1.1 200 OK\r\n\r\n"
				"<html><head><title>Too many connections</title></head>"
				"<body>Too many connections,try later!</body></html>",-1);
	return;
}

//�ڴ��¼��м��Session��ʱ
void httpServer :: onIdle(void)
{
	static time_t checkedTime=time(NULL);
	if(checkedTime-time(NULL)>20){//20���Ӽ��һ��session�Ƿ�ʱ
		checkedTime=time(NULL);
		m_mutex.lock();
		std::map<std::string,httpSession *>::iterator it=m_sessions.begin();
		for(;it!=m_sessions.end();it++)
		{
			httpSession *psession=(*it).second;
			if(!psession->isValid(checkedTime)) { delete psession; m_sessions.erase(it); }
		}
		m_mutex.unlock(); 
	}
	return;
}
//����һ���µĿͻ����Ӵ˷��񴥷��˺���
void httpServer :: onAccept(socketTCP *psock)
{
	httpRequest httpreq;
	while(true){ //ѭ��������ն��http����
		SOCKSRESULT sr=httpreq.recv_reqH(psock,HTTP_MAX_RESPTIMEOUT);
		if(sr<=HTTP_REQ_UNKNOWN) break; //����http������߳�ʱ
		httpResponse httprsp; httpSession *psession;//������ȡsession����
		const char *strSessionID=httpreq.Cookies(httpSession::SESSION_IDNAME);
		m_mutex.lock();
		if(strSessionID){
			std::map<std::string,httpSession *>::iterator it=m_sessions.find(strSessionID);
			if(it!=m_sessions.end()) psession=(*it).second; else psession=NULL;
		}else psession=NULL;
		if(psession==NULL){ //�����µ�sessionID
			if( (psession=new httpSession)==NULL ) return;
			m_sessions[psession->sessionID()]=psession;	
			httprsp.SetCookie(httpSession::SESSION_IDNAME,psession->sessionID(),"/");
		}//��ֹ�û������������߷ǳ���ʱ�������趨��session����Чʱ�䣬���´�session��ɾ��
		psession->m_lastTime=0x7fffffff; 
		m_mutex.unlock();
		if(!httpreq.ifReceivedAll() && httpreq.get_contentType(NULL)==HTTP_CONTENT_APPLICATION)
			if(!httpreq.recv_remainder(psock,-1)) return; //������ʣ���form�ύ����,δ�������򷵻عر�
	//	RW_LOG_PRINTMAPS(httpreq.Cookies(),"Cookies");
	//	RW_LOG_PRINTMAPS(httpreq.QueryString(),"Get param");
	//	RW_LOG_PRINTMAPS(httpreq.Form(),"Post Param");

		//��ʼ׼������http����
		if(!onHttpReq(psock,httpreq,*psession,m_application,httprsp)){
			string vpath=httpreq.url(); //ת��web����Ĭ�ϴ���
			long lAccess=cvtVPath2RPath(vpath);
			if( lAccess!=HTTP_ACCESS_NONE) //����Ŀ¼ת��ΪʵĿ¼����ȡĿ¼�ķ���Ȩ��
			{
				if(vpath.length()>0 && vpath[vpath.length()-1]=='\\') 
					vpath.erase(vpath.length()-1);
				long iret=FILEIO::fileio_exist(vpath.c_str());
				if(iret==-1) //ָ�����ļ���Ŀ¼������
					httprsp_fileNoFind(psock,httprsp);
				else if(iret==-2) //ָ������Ŀ¼,listĿ¼�е�����
				{	
					if((lAccess & HTTP_ACCESS_LIST)==0)
						httprsp_listDenied(psock,httprsp);
					else{
						httprsp.NoCache(); //CacheControl("No-cache");
						vpath.append("\\*"); 
						httprsp_listDir(psock,vpath,httpreq,httprsp);
					}
				}else{ //ָ�������ļ�
					//�ж��ļ��Ƿ��޸�-------------- start---------------------
					cTime ct0; time_t t0=0,t1=1;
					const char *p=httpreq.Header("If-Modified-Since");
					if(p && ct0.parseDate(p) ){
						WIN32_FIND_DATA finddata;
						HANDLE hd=::FindFirstFile(vpath.c_str(), &finddata);
						if(hd!=INVALID_HANDLE_VALUE)
						{
							FILETIME ft=finddata.ftLastWriteTime;
							::FindClose(hd); t0=ct0.Gettime();
							cTime ct1(ft);
							t1=ct1.Gettime()+_timezone;//����ʱ������						 
						}
					}//?if(p)
					if(t1<=t0) //�ļ�û�б��޸Ĺ�
						httprsp_NotModify(psock,httprsp);
					else
					//�ж��ļ��Ƿ��޸�--------------  end ---------------------
					{
						long lstartpos,lendpos;//��ȡ�ļ��ķ�Χ
						int iRangeNums=httpreq.get_requestRange(&lstartpos,&lendpos,0);
						if(iRangeNums>1){
							long *lppos=new long[iRangeNums*2];
							if(lppos){
								for(int i=0;i<iRangeNums;i++) httpreq.get_requestRange(&lppos[i],&lppos[iRangeNums+i],i);
								httprsp.sendfile(psock,vpath.c_str(),MIMETYPE_UNKNOWED,&lppos[0],&lppos[iRangeNums],iRangeNums);
							} delete[] lppos;
						}else{
							if(iRangeNums==0) { lstartpos=0; lendpos=-1; }
							httprsp.sendfile(psock,vpath.c_str(),MIMETYPE_UNKNOWED,lstartpos,lendpos);
						}//?if(iRangeNums>1)
					}//?�ļ����޸Ĺ�
				} //ָ�������ļ�
			} else httprsp_accessDenied(psock,httprsp);
		}//?if(!onHttpReq(psock,httpreq,*psession,m_application,httprsp))

		psession->m_lastTime=time(NULL); //����session��������ʱ��
		if(!httpreq.bKeepAlive()) break;
	}//?while(true)
}

//��������·��ת��Ϊ����ʵ·��(!!!��·�������ִ�Сд)
//���ضԵ�ǰʵ·���Ŀɲ���Ȩ��
//vpath -- [in|out] ���������·�����������ʵ·��
long httpServer :: cvtVPath2RPath(std::string &vpath)
{
	if(vpath=="" || vpath[0]!='/') return HTTP_ACCESS_NONE; //�쳣����
	::_strlwr((char *)vpath.c_str());
	std::map<std::string,std::pair<std::string,long> >::iterator it=m_dirAccess.end();
	const char *ptr=vpath.c_str();
	const char *ptrBegin=ptr;
	do
	{	
		ptr++; char c=*ptr;
		*(char *)ptr=0;
		std::map<std::string,std::pair<std::string,long> >::iterator itTmp=m_dirAccess.find(ptrBegin);
		*(char *)ptr=c;
		if(itTmp==m_dirAccess.end()) break;
		it=itTmp;
	}while( (ptr=strchr(ptr,'/'))!=NULL );

	//ʵ���ϲ����ܷ����������,m_dirAccess��Ҫ���ø�'/'
	if(it==m_dirAccess.end()) { return HTTP_ACCESS_NONE;}
 
	vpath.erase(0,(*it).first.length());
	long lAccess=(*it).second.second;
	if((lAccess & HTTP_ACCESS_SUBDIR_INHERIT)!=0)
	{
		if(strchr(vpath.c_str(),'/')!=NULL) lAccess=0; //Ŀ¼Ȩ�޽�ֹ�̳У����¼�Ŀ¼��Ȩ��Ϊ0
	}
	for(int i=0;i<vpath.length();i++) if(vpath[i]=='/') vpath[i]='\\';
	vpath.insert(0,(*it).second.first.c_str());
	return lAccess;
}
