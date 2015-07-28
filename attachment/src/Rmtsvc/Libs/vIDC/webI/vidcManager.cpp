/*******************************************************************
   *	vidcManager.cpp
   *    DESCRIPTION:vIDC���Ϲ�����
   *
   *    AUTHOR:yyc
   *
   *******************************************************************/

#include "vidcManager.h"

void getAbsolutfilepath(std::string &spath);

vidcManager :: vidcManager()
{
}

vidcManager :: ~vidcManager(){}

void vidcManager :: initSetting()
{
	std::map<std::string,mportTCP *>::iterator it=m_tcpsets.begin();
	for(;it!=m_tcpsets.end();it++){
		mportTCP *ptr_mtcp=(*it).second;
		TMapParam *p=(ptr_mtcp)?((TMapParam *)(ptr_mtcp->Tag())):NULL;
		ptr_mtcp->Stop();
		delete p; delete ptr_mtcp;
	}
	m_tcpsets.clear();
	m_upnp.Clear();
	m_vidccSets.Destroy(); //��������vidcc
}

void vidcManager :: Destroy()
{
	initSetting();
	m_vidcsvr.Stop();
}
//��������Ϊ�Զ������ñ��ض˿�ӳ�����
//�ڳ���ʼ����ʱ����
void vidcManager :: mtcpl_Start()
{
	std::map<std::string,mportTCP *>::iterator it=m_tcpsets.begin();
	for(;it!=m_tcpsets.end();it++){
		mportTCP * ptr_mtcp=(*it).second;
		TMapParam *p=(TMapParam *)(ptr_mtcp->Tag());
		if(ptr_mtcp && p && p->bAutorun)
		{
			ptr_mtcp->rules().addRules_new(RULETYPE_TCP,p->ipaccess,p->ipRules.c_str());
			if(ptr_mtcp->getSSLType()==SSLSVR_TCPSVR){ //SSL���ܷ���,���ÿͻ���֤��
				std::string clicert=p->clicert,clikey=p->clikey;
				getAbsolutfilepath(clicert); getAbsolutfilepath(clikey);
				ptr_mtcp->setCacert(clicert.c_str(),clikey.c_str(),p->clikeypswd.c_str(),false,NULL,NULL);
			}
			SOCKSRESULT sr=ptr_mtcp->Start(g_strMyCert.c_str(),g_strMyKey.c_str(),g_strKeyPswd.c_str(),
				g_strCaCert.c_str(),g_strCaCRL.c_str());
			if(sr<=0) RW_LOG_PRINT(LOGLEVEL_WARN,"[mtcpl] ��������ӳ����� %s ʧ��, err=%d\r\n",(*it).first.c_str(),sr);
		}
	}//?for
	return;
}

void vidcManager :: xml_list_localip(cBuffer &buffer)
{
	std::vector<std::string> vec;//�õ�����IP�����صõ�����IP�ĸ���
	long n=socketBase::getLocalHostIP(vec);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<localip>");
	for(int i=0;i<n;i++) buffer.len()+=sprintf(buffer.str()+buffer.len(),"%s ",vec[i].c_str());
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"</localip>");
}

//<maplist>
//<mapped name="ӳ������">Ӧ������</mapped>
//</maplist>
void vidcManager :: xml_list_mtcp(cBuffer &buffer)
{
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<maplist>");
	std::map<std::string,mportTCP *>::iterator it=m_tcpsets.begin();
	for(;it!=m_tcpsets.end();it++)
	{
		mportTCP *p=(*it).second;
		if(buffer.Space()<256) buffer.Resize(buffer.size()+256);
		if(buffer.str())
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<mapped name=\"%s\"><![CDATA[%s]]></mapped>",
				(*it).first.c_str(),p->svrname());
		else break;
	}
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"</maplist>");
	return;
}
//<mapinfo>
//<mapname></mapname>
//...
//</mapinfo>
void vidcManager :: xml_info_mtcp(cBuffer &buffer,const char *mapname)
{
	std::map<std::string,mportTCP *>::iterator it=m_tcpsets.find(mapname);
	if(it!=m_tcpsets.end()){
		mportTCP * ptr_mtcp=(*it).second;
		if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
		if(buffer.str()) buffer.len()+=sprintf(buffer.str()+buffer.len(),"<mapinfo>");
		if(buffer.Space()<64) buffer.Resize(buffer.size()+64);
		if(buffer.str()) buffer.len()+=sprintf(buffer.str()+buffer.len(),"<mapname>%s</mapname>",mapname);
		
		ptr_mtcp->xml_info_mtcp(buffer);
		TMapParam *p=(TMapParam *)(ptr_mtcp->Tag());
		if(p){
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<autorun>%d</autorun>",(p->bAutorun)?1:0);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ipfilter>");
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<access>%d</access>",p->ipaccess);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ipaddr>%s</ipaddr>",p->ipRules.c_str());
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"</ipfilter>");
			if(p->clicert!="")
			{
				int l=p->clicert.length()+p->clikey.length()+p->clikeypswd.length()+32;
				if(buffer.Space()<l) buffer.Resize(buffer.size()+l);
				if(buffer.str())
					buffer.len()+=sprintf(buffer.str()+buffer.len(),"<clicert>%s,%s,%s</clicert>",
						p->clicert.c_str(),p->clikey.c_str(),p->clikeypswd.c_str());
			}else buffer.len()+=sprintf(buffer.str()+buffer.len(),"<clicert></clicert>");
		}//?if(p)

		if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
		if(buffer.str()) buffer.len()+=sprintf(buffer.str()+buffer.len(),"</mapinfo>");
	}else{//����
		if(buffer.Space()<128) buffer.Resize(buffer.size()+128);
		if(buffer.str())
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>ָ����ӳ�� %s ������</retmsg>",mapname);
	}
	return;
}


//����ָ����ӳ�����
void vidcManager :: xml_start_mtcp(cBuffer &buffer,const char *mapname)
{
	std::map<std::string,mportTCP *>::iterator it=m_tcpsets.find(mapname);
	if(it!=m_tcpsets.end()){
		mportTCP * ptr_mtcp=(*it).second;
		TMapParam *p=(TMapParam *)(ptr_mtcp->Tag());
		if(p){//����IP���˹���
			ptr_mtcp->rules().addRules_new(RULETYPE_TCP,p->ipaccess,p->ipRules.c_str());
			if(ptr_mtcp->getSSLType()==SSLSVR_TCPSVR){ //SSL���ܷ���,���ÿͻ���֤��
				std::string clicert=p->clicert,clikey=p->clikey;
				getAbsolutfilepath(clicert); getAbsolutfilepath(clikey);
				ptr_mtcp->setCacert(clicert.c_str(),clikey.c_str(),p->clikeypswd.c_str(),false,NULL,NULL);
			}
		}
		SOCKSRESULT sr=ptr_mtcp->Start(g_strMyCert.c_str(),g_strMyKey.c_str(),g_strKeyPswd.c_str(),
				g_strCaCert.c_str(),g_strCaCRL.c_str());
		if(sr<=0){ //��������ʧ��
			if(buffer.Space()<128) buffer.Resize(buffer.size()+128);
			if(buffer.str())
				buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>����ӳ�� %s ʧ��, err=%d</retmsg>",mapname,sr);
		}
		if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
		if(buffer.str()) buffer.len()+=sprintf(buffer.str()+buffer.len(),"<mapinfo>");
		if(buffer.Space()<64) buffer.Resize(buffer.size()+64);
		if(buffer.str()) buffer.len()+=sprintf(buffer.str()+buffer.len(),"<mapname>%s</mapname>",mapname);

		ptr_mtcp->xml_info_mtcp(buffer);	
		if(p){
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<autorun>%d</autorun>",(p->bAutorun)?1:0);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ipfilter>");
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<access>%d</access>",p->ipaccess);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ipaddr>%s</ipaddr>",p->ipRules.c_str());
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"</ipfilter>");
			if(p->clicert!="")
			{
				int l=p->clicert.length()+p->clikey.length()+p->clikeypswd.length()+32;
				if(buffer.Space()<l) buffer.Resize(buffer.size()+l);
				if(buffer.str())
					buffer.len()+=sprintf(buffer.str()+buffer.len(),"<clicert>%s,%s,%s</clicert>",
						p->clicert.c_str(),p->clikey.c_str(),p->clikeypswd.c_str());
			}else buffer.len()+=sprintf(buffer.str()+buffer.len(),"<clicert></clicert>");
		}

		if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
		if(buffer.str()) buffer.len()+=sprintf(buffer.str()+buffer.len(),"</mapinfo>");
	}else{//����
		if(buffer.Space()<128) buffer.Resize(buffer.size()+128);
		if(buffer.str())
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>ָ����ӳ�� %s ������</retmsg>",mapname);
	}
	return;
}

//ָֹͣ����ӳ�����
void vidcManager :: xml_stop_mtcp(cBuffer &buffer,const char *mapname)
{
	std::map<std::string,mportTCP *>::iterator it=m_tcpsets.find(mapname);
	if(it!=m_tcpsets.end()){
		mportTCP * ptr_mtcp=(*it).second;
		ptr_mtcp->Stop();
		if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
		if(buffer.str()) buffer.len()+=sprintf(buffer.str()+buffer.len(),"<mapinfo>");
		if(buffer.Space()<64) buffer.Resize(buffer.size()+64);
		if(buffer.str()) buffer.len()+=sprintf(buffer.str()+buffer.len(),"<mapname>%s</mapname>",mapname);

		ptr_mtcp->xml_info_mtcp(buffer);
		TMapParam *p=(TMapParam *)(ptr_mtcp->Tag());
		if(p){
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<autorun>%d</autorun>",(p->bAutorun)?1:0);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ipfilter>");
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<access>%d</access>",p->ipaccess);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ipaddr>%s</ipaddr>",p->ipRules.c_str());
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"</ipfilter>");
			if(p->clicert!="")
			{
				int l=p->clicert.length()+p->clikey.length()+p->clikeypswd.length()+32;
				if(buffer.Space()<l) buffer.Resize(buffer.size()+l);
				if(buffer.str())
					buffer.len()+=sprintf(buffer.str()+buffer.len(),"<clicert>%s,%s,%s</clicert>",
						p->clicert.c_str(),p->clikey.c_str(),p->clikeypswd.c_str());
			}else buffer.len()+=sprintf(buffer.str()+buffer.len(),"<clicert></clicert>");
		}

		if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
		if(buffer.str()) buffer.len()+=sprintf(buffer.str()+buffer.len(),"</mapinfo>");
	}else{//����
		if(buffer.Space()<128) buffer.Resize(buffer.size()+128);
		if(buffer.str())
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>ָ����ӳ�� %s ������</retmsg>",mapname);
	}
	return;
}

//ɾ��ָ����ӳ�����
void vidcManager :: xml_dele_mtcp(cBuffer &buffer,const char *mapname)
{
	std::map<std::string,mportTCP *>::iterator it=m_tcpsets.find(mapname);
	if(it!=m_tcpsets.end()){
		mportTCP * ptr_mtcp=(*it).second;
		ptr_mtcp->Stop();
		TMapParam *p=(ptr_mtcp)?((TMapParam *)(ptr_mtcp->Tag())):NULL;
		delete p; delete ptr_mtcp;
		m_tcpsets.erase(it);
		xml_list_mtcp(buffer);
	}else{//����
		if(buffer.Space()<128) buffer.Resize(buffer.size()+128);
		if(buffer.str())
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>ָ����ӳ�� %s ������</retmsg>",mapname);
	}
	return;
}

//------------------------UDPӳ�伯��----------------------

void vidcManager :: xml_list_mudp(cBuffer &buffer)
{
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<maplist>");
	
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"</maplist>");
	return;
}
void vidcManager :: xml_info_mudp(cBuffer &buffer,const char *mapname){}
void vidcManager :: xml_start_mudp(cBuffer &buffer,const char *mapname){}
void vidcManager :: xml_stop_mudp(cBuffer &buffer,const char *mapname){}
void vidcManager :: xml_dele_mudp(cBuffer &buffer,const char *mapname){}