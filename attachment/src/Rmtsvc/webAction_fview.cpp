/*******************************************************************
   *	webAction_fview.cpp web������ - �ļ�����
   *    DESCRIPTION:
   *
   *    AUTHOR:yyc
   *
   *    HISTORY:
   *
   *    DATE:
   *	
   *******************************************************************/
#include "rmtsvc.h"
#include "net4cpp21/utils/utils.h"

//�г����������ļ�
//buffer - ���ص�xml�ĵ�,��ʽ:
//<?xml version="1.0" encoding="gb2312" ?>
//<xmlroot>
//<folders>
//<fitem>
//<bhide></bhide>
//<hassub>+</hassub>
//<alias></alias>
//<fname></fname>
//<fsize></fsize>
//<ftype></ftype>
//<ftime></ftime>
//</fitem>
//....
//</folders>
//
//<files>
//<fitem>
//<bhide></bhide>
//<hassub>+</hassub>
//<alias></alias>
//<fname></fname>
//<fsize></fsize>
//<lsize></lsize>    //0���룬��������
//<ftype></ftype>
//<ftime></ftime>
//</fitem>
//....
//</files>
//</xmlroot>
static char DRIVE_TYPE[][16]={"δ֪����","NO_ROOT_DIR","���ƶ�����","���ش���","������",
				"����","RAM ����",""};
const char * getFileType(const char *filename);
const char * getFileOpmode(const char *filename);
double folderSize(const char *spath,const char *name,unsigned long &folders,unsigned long &files);
bool folderList(cBuffer &buffer,const char *spath,bool bdsphide);
bool fileList(cBuffer &buffer,const char *spath,bool bdsphide);
//listWhat -- ָ���о�ʲô��Ŀ 1 : list folder 2 : list file
bool webServer :: httprsp_filelist(socketTCP *psock,httpResponse &httprsp,const char *spath,int listWhat,bool bdsphide)
{
	bool bret=false;
	cBuffer buffer(1024);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<?xml version=\"1.0\" encoding=\"gb2312\" ?><xmlroot>");
	
	if(listWhat & 1) bret|=folderList(buffer,spath,bdsphide);
	if(listWhat & 2) bret|=fileList(buffer,spath,bdsphide);

	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"</xmlroot>");
	
//	printf("**************************************\r\n%s\r\n",buffer.str());
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buffer.len());
	if(!bret)
		httprsp.send_rspH(psock,400,"Bad Request");
	else httprsp.send_rspH(psock,200,"OK");
	
	if(buffer.str()) psock->Send(buffer.len(),buffer.str(),-1);
	return true;
}
//ɾ����Ŀ¼
bool webServer :: httprsp_folder_del(socketTCP *psock,httpResponse &httprsp,const char *spath,
									 const char *fname,bool bdsphide)
{
	if(fname && fname[0]!=0)
	{
		string strpath(spath);
		strpath.append("\\");strpath.append(fname);
		FILEIO::fileio_deleteDir(strpath.c_str());
	}
	return httprsp_filelist(psock,httprsp,spath,3,bdsphide);
}
//������Ŀ¼
bool webServer :: httprsp_folder_ren(socketTCP *psock,httpResponse &httprsp,const char *spath,
									 const char *fname,const char *newname,bool bdsphide)
{
	if(fname && fname[0]!=0 && newname && newname[0]!=0)
	{
		string strpath(spath),newpath(spath);
		strpath.append("\\");strpath.append(fname);
		newpath.append("\\");newpath.append(newname);
		FILEIO::fileio_rename(strpath.c_str(),newpath.c_str());
	}
	return httprsp_filelist(psock,httprsp,spath,3,bdsphide);
}
//�½���Ŀ¼
bool webServer :: httprsp_folder_new(socketTCP *psock,httpResponse &httprsp,const char *spath,
									 const char *fname,bool bdsphide)
{
	if(fname && fname[0]!=0)
	{
		string strpath(spath);
		strpath.append("\\");strpath.append(fname);
		FILEIO::fileio_createDir(strpath.c_str());
	}
	return httprsp_filelist(psock,httprsp,spath,1,bdsphide);
}
//ɾ���ļ�
bool webServer :: httprsp_file_del(socketTCP *psock,httpResponse &httprsp,const char *spath,
									 const char *fname,bool bdsphide)
{
	if(fname && fname[0]!=0)
	{
		string strpath(spath);
		strpath.append("\\");strpath.append(fname);
		FILEIO::fileio_deleteFile(strpath.c_str());
	}
	return httprsp_filelist(psock,httprsp,spath,2,bdsphide);
}
//�����ļ�
bool webServer :: httprsp_file_ren(socketTCP *psock,httpResponse &httprsp,const char *spath,
									 const char *fname,const char *newname,bool bdsphide)
{
	if(fname && fname[0]!=0 && newname && newname[0]!=0)
	{
		string strpath(spath),newpath(spath);
		strpath.append("\\");strpath.append(fname);
		newpath.append("\\");newpath.append(newname);
		FILEIO::fileio_rename(strpath.c_str(),newpath.c_str());
	}
	return httprsp_filelist(psock,httprsp,spath,2,bdsphide);
}
//Զ������/���ļ�
bool webServer :: httprsp_file_run(socketTCP *psock,httpResponse &httprsp,const char *spath)
{
	cBuffer buffer(512);
	
	unsigned long iret=(unsigned long)::ShellExecute(NULL,"open",spath,NULL,NULL,SW_SHOW );
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<?xml version=\"1.0\" encoding=\"gb2312\" ?><xmlroot>");
	if(iret>32)
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>����ִ�гɹ�!</retmsg>");
	else if(iret==ERROR_FILE_NOT_FOUND)
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>����ִ��ʧ��! ERROR_FILE_NOT_FOUND</retmsg>");
	else if(iret==ERROR_BAD_FORMAT)
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>����ִ��ʧ��! The .exe file is invalid.</retmsg>");
	else if(iret==ERROR_FILE_NOT_FOUND)
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>����ִ��ʧ��! ERROR_FILE_NOT_FOUND</retmsg>");
	else if(iret==SE_ERR_ACCESSDENIED)
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>����ִ��ʧ��! system denied access to the specified file</retmsg>");
	else if(iret==SE_ERR_ASSOCINCOMPLETE )
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>����ִ��ʧ��! The file name association is incomplete or invalid</retmsg>");
	else if(iret==SE_ERR_NOASSOC)
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>����ִ��ʧ��! There is no application associated with the given file name extension</retmsg>");
	else if(iret==SE_ERR_OOM)
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>����ִ��ʧ��! There was not enough memory to complete the operation</retmsg>");
	else 
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<retmsg>����ִ��ʧ��!</retmsg>");
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"</xmlroot>");
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buffer.len());
	httprsp.send_rspH(psock,200,"OK");
	psock->Send(buffer.len(),buffer.str(),-1);
	return true;
}
//��ȡ�ļ����� 
//<?xml version="1.0" encoding="gb2312" ?>
//<xmlroot>
//<fname></fname>
//<ftype></ftype>
//<opmode></opmode>
//<fpath></fpath>
//<fsize></fsize>
//<fctime></fctime>
//<fmtime></fmtime>
//<fatime></fatime>
//<protype></protype>
//</xmlroot>
bool webServer :: httprsp_profile(socketTCP *psock,httpResponse &httprsp,const char *spath,const char *prof)
{

	cBuffer buffer(1024);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<?xml version=\"1.0\" encoding=\"gb2312\" ?><xmlroot>");

	WIN32_FIND_DATA finddata; SYSTEMTIME st; //FILETIME localFtime;
	HANDLE hd=::FindFirstFile(spath, &finddata);
	//��ȡ·�����ļ���
	const char *ptr_name=strrchr(spath,'\\');
	if(hd!=INVALID_HANDLE_VALUE && ptr_name)
	{
		if(prof) //�����ļ�����
		{
			DWORD dwFileAttributes=finddata.dwFileAttributes;
			if(strchr(prof,'R'))
				dwFileAttributes |=FILE_ATTRIBUTE_READONLY;
			else dwFileAttributes &=~FILE_ATTRIBUTE_READONLY;
			if(strchr(prof,'H'))
				dwFileAttributes |=FILE_ATTRIBUTE_HIDDEN;
			else dwFileAttributes &=~FILE_ATTRIBUTE_HIDDEN;
			if(strchr(prof,'A'))
				dwFileAttributes |=FILE_ATTRIBUTE_ARCHIVE;
			else dwFileAttributes &=~FILE_ATTRIBUTE_ARCHIVE;
			if(::SetFileAttributes(spath,dwFileAttributes))
				finddata.dwFileAttributes=dwFileAttributes;
		}

		*(char *)ptr_name=0; ptr_name++;
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fname>%s</fname>",ptr_name);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ftype>%s</ftype>",getFileType(ptr_name));
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fpath>%s</fpath>",spath);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<opmode>%s</opmode>",getFileOpmode(ptr_name));
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fsize>%d KB (%d �ֽ�)</fsize>",
			finddata.nFileSizeLow/1024,finddata.nFileSizeLow);
		 
//		::FileTimeToLocalFileTime(&finddata.ftCreationTime,&localFtime);
//		::FileTimeToSystemTime(&localFtime,&st);
		::FileTimeToSystemTime(&finddata.ftCreationTime,&st);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fctime>%04d-%02d-%02d %02d:%02d</fctime>",
				st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute);
//		::FileTimeToLocalFileTime(&finddata.ftLastWriteTime,&localFtime);
//		::FileTimeToSystemTime(&localFtime,&st);
		::FileTimeToSystemTime(&finddata.ftLastWriteTime,&st);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fmtime>%04d-%02d-%02d %02d:%02d</fmtime>",
				st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute);
//		::FileTimeToLocalFileTime(&finddata.ftLastAccessTime,&localFtime);
//		::FileTimeToSystemTime(&localFtime,&st);
		::FileTimeToSystemTime(&finddata.ftLastAccessTime,&st);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fatime>%04d-%02d-%02d %02d:%02d</fatime>",
				st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<protype>%c%c%c</protype>",
				((finddata.dwFileAttributes & FILE_ATTRIBUTE_READONLY)?'R':' '),
				((finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)?'H':' '),
				((finddata.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)?'A':' '));
	}//?if(ptr)
	::FindClose(hd);

	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"</xmlroot>");
	
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buffer.len());
	httprsp.send_rspH(psock,200,"OK");
	
	if(buffer.str()) psock->Send(buffer.len(),buffer.str(),-1);
	return true;
}
//��ȡ�ļ������� 
//<?xml version="1.0" encoding="gb2312" ?>
//<xmlroot>
//<fname></fname>
//<fpath></fpath>
//<fsize></fsize>
//<fsubs></fsubs>
//<fctime></fctime>
//<protype></protype>
//</xmlroot>
bool webServer :: httprsp_profolder(socketTCP *psock,httpResponse &httprsp,const char *spath,const char *prof)
{

	cBuffer buffer(1024);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<?xml version=\"1.0\" encoding=\"gb2312\" ?><xmlroot>");
	if(spath && strlen(spath)>3)
	{
		WIN32_FIND_DATA finddata; SYSTEMTIME st; //FILETIME localFtime;
		HANDLE hd=::FindFirstFile(spath, &finddata);
		//��ȡ·�����ļ���
		const char *ptr_name=strrchr(spath,'\\');
		if(hd!=INVALID_HANDLE_VALUE && ptr_name)
		{
			if(prof) //�����ļ�������
			{
				DWORD dwFileAttributes=finddata.dwFileAttributes;
				if(strchr(prof,'R'))
					dwFileAttributes |=FILE_ATTRIBUTE_READONLY;
				else dwFileAttributes &=~FILE_ATTRIBUTE_READONLY;
				if(strchr(prof,'H'))
					dwFileAttributes |=FILE_ATTRIBUTE_HIDDEN;
				else dwFileAttributes &=~FILE_ATTRIBUTE_HIDDEN;
				if(strchr(prof,'A'))
					dwFileAttributes |=FILE_ATTRIBUTE_ARCHIVE;
				else dwFileAttributes &=~FILE_ATTRIBUTE_ARCHIVE;
				if(::SetFileAttributes(spath,dwFileAttributes))
					finddata.dwFileAttributes=dwFileAttributes;
			}
			*(char *)ptr_name=0; ptr_name++;
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fname>%s</fname>",ptr_name);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fpath>%s</fpath>",spath);
			
//			::FileTimeToLocalFileTime(&finddata.ftCreationTime,&localFtime);
//			::FileTimeToSystemTime(&localFtime,&st);
			::FileTimeToSystemTime(&finddata.ftCreationTime,&st);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fctime>%04d-%02d-%02d %02d:%02d</fctime>",
					st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<protype>%c%c%c</protype>",
					((finddata.dwFileAttributes & FILE_ATTRIBUTE_READONLY)?'R':' '),
					((finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)?'H':' '),
					((finddata.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)?'A':' '));
			unsigned long folders=0,files=0;
			double dbsize=folderSize(spath,ptr_name,folders,files);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fsize>%d KB (%d �ֽ�)</fsize>",
				(unsigned long)(dbsize/1024),(DWORD64)dbsize);
			buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fsubs>%d ���ļ� , %d ���ļ���</fsubs>",files,folders);
		}//?if(ptr)
		::FindClose(hd);
	}//?if(spath && strlen(spath)>=2)
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"</xmlroot>");
	
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buffer.len());
	httprsp.send_rspH(psock,200,"OK");
	
	if(buffer.str()) psock->Send(buffer.len(),buffer.str(),-1);
	return true;
}
//��ȡ���������� 
//<?xml version="1.0" encoding="gb2312" ?>
//<xmlroot>
//<fname></fname>
//<ftype></ftype>
//<fvolu></fvolu>
//<fsyst></fsyst>
//<fused></fused>
//<ffree></ffree>
//<fsize></fsize>
//</xmlroot>
bool webServer :: httprsp_prodrive(socketTCP *psock,httpResponse &httprsp,const char *spath,const char *svolu)
{
	cBuffer buffer(1024);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<?xml version=\"1.0\" encoding=\"gb2312\" ?><xmlroot>");
	if(spath && strlen(spath)>1)
	{
		char dr[4]; char volumname[64],FileSystemName[16];
		dr[0]=spath[0];dr[1]=':';dr[2]='\\';dr[3]=0;
		if(svolu) SetVolumeLabel(dr,svolu); //���þ����
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fname>������ %c</fname>",spath[0]);
		UINT drtype=::GetDriveType(dr); //��ȡ����
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ftype>%s</ftype>",DRIVE_TYPE[drtype]);
		BOOL bret=::GetVolumeInformation(dr,volumname,64,0,0,0,FileSystemName,16);
		if(!bret){ volumname[0]=0; FileSystemName[0]=0;}
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fvolu>%s</fvolu>",volumname);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fsyst>%s</fsyst>",FileSystemName);
		DWORD dwSectors,dwBytes,dwFreeClusters,dwAllClusters;
		bret=::GetDiskFreeSpace(dr,&dwSectors,&dwBytes,&dwFreeClusters,&dwAllClusters);
		if(!bret){ dwSectors=dwBytes=dwFreeClusters=dwAllClusters=0; }
		double dblSize=(double)dwAllClusters*dwSectors*dwBytes;
		double dblFree=(double)dwFreeClusters*dwSectors*dwBytes;
		double dblUsed=dblSize-dblFree;
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fused>%.2f GB</fused>",dblUsed/1024/1024/1024);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ffree>%.2f GB</ffree>",dblFree/1024/1024/1024);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<fsize>%.2f GB</fsize>",dblSize/1024/1024/1024);
	}//?if(spath && strlen(spath)>=2)
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"</xmlroot>");
	
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buffer.len());
	httprsp.send_rspH(psock,200,"OK");
	
	if(buffer.str()) psock->Send(buffer.len(),buffer.str(),-1);
	return true;
}
//��ȡ�ϴ��ļ��Ľ���
bool webServer :: httprsp_get_upratio(socketTCP *psock,httpResponse &httprsp,httpSession &session)
{
	cBuffer buffer(256);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<?xml version=\"1.0\" encoding=\"gb2312\" ?><xmlroot>");
	
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ratio>%s</ratio>",session["up_ratio"].c_str());
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<rbyte>%s</rbyte>",session["up_rbyte"].c_str());
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<abyte>%s</abyte>",session["up_abyte"].c_str());
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<speed>%s</speed>",session["up_speed"].c_str());

	buffer.len()+=sprintf(buffer.str()+buffer.len(),"</xmlroot>");
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buffer.len());
	httprsp.send_rspH(psock,200,"OK");
	
	if(buffer.str()) psock->Send(buffer.len(),buffer.str(),-1);
	return true;
}
//����xmlhttp�ļ�����
bool webServer :: httprsp_upload(socketTCP *psock,httpRequest &httpreq,httpResponse &httprsp,httpSession &session)
{
	bool bSuccess=false;
	const char *ptr_path=httpreq.Request("path");
	const char *ptr_name=httpreq.Request("name");

	session["up_ratio"]="0%"; //�ϴ����� �ٷֱ�
	session["up_speed"]="0";  //�ϴ��ٶ�
	session["up_rbyte"]="0"; //��ǰ�ѽ����ֽ�
	session["up_abyte"]="0"; //���ļ���С
	if(ptr_path && ptr_name)
	{
		string destpath(ptr_path); 
		if(destpath[destpath.length()-1]!='\\') destpath.append("\\");
		destpath.append(ptr_name);
		FILE *fp=::fopen(destpath.c_str(),"wb");
//		printf("1111111111111111111111111\r\nfp=%d,upfile=%s\r\n",fp,destpath.c_str());
		if(fp){
			long allbytes,receivedBytes; char fmtbuf[64];
			cBuffer &buff=httpreq.get_contentData();
			allbytes=httpreq.get_contentLen();
			receivedBytes=buff.len();
			if(buff.len()>0) ::fwrite(buff.str(),sizeof(char),buff.len(),fp);
			
			time_t t,tStart=time(NULL); //��������
			sprintf(fmtbuf,"%d",allbytes); session["up_abyte"]=string(fmtbuf);
			sprintf(fmtbuf,"%d",receivedBytes); session["up_rbyte"]=string(fmtbuf);
			while(receivedBytes<allbytes)
			{
				//�������HTTP_MAX_RESPTIMEOUT��û�յ����ݿ���Ϊ�ͻ����쳣
				int wlen=psock->Receive(buff.str(),buff.size(),HTTP_MAX_RESPTIMEOUT);
				if(wlen<0) break;
				if(wlen==0){ cUtils::usleep(SCHECKTIMEOUT); continue; }//==0������������������������
				::fwrite(buff.str(),sizeof(char),wlen,fp); receivedBytes+=wlen;
				sprintf(fmtbuf,"%d",receivedBytes); session["up_rbyte"]=string(fmtbuf);
				sprintf(fmtbuf,"%d%%",receivedBytes*100/allbytes); session["up_ratio"]=string(fmtbuf);
				if( (t=(time(NULL)-tStart))>0 )
				{  sprintf(fmtbuf,"%.2fK/s",receivedBytes/t/1000.0); session["up_speed"]=string(fmtbuf); }
			}//?while
			::fclose(fp); bSuccess=(receivedBytes>=allbytes)?true:false;
		}
	}//?if(ptr_path && ptr_name)

	httprsp.NoCache();//CacheControl("No-cache");
	//������Ӧ���ݳ���
	httprsp.lContentLength(0);
	httprsp.send_rspH(psock,((bSuccess)?200:500),"OK");	
	return true;
}

#define FILEVERINFOLEN 128 //FileVerInfoÿ�������󳤶�
typedef struct _TFileVerInfo
{
	char Comments[FILEVERINFOLEN]; //ע��
	char CompanyName[FILEVERINFOLEN]; //��˾��
	char ProductName[FILEVERINFOLEN]; //��Ʒ��
	char ProductVersion[FILEVERINFOLEN]; //��Ʒ�汾
	char InternalName[FILEVERINFOLEN]; //�ڲ�����
	char FileDescription[FILEVERINFOLEN]; //�ļ�����
	char FileVersion[FILEVERINFOLEN]; //�ļ��汾
	char OriginalFilename[FILEVERINFOLEN]; //ԭʼ�ļ���
	char LegalCopyright[FILEVERINFOLEN]; //�Ϸ���Ȩ
//	char LegalTrademarks[FILEVERINFOLEN]; //�Ϸ��̱�
//	char SpecialBuild[FILEVERINFOLEN]; //��������
//	char PrivateBuild[FILEVERINFOLEN]; //˽�б����
}FileVerInfo;
bool GetVersionInfo(LPTSTR filename,FileVerInfo & fverinfo);
//��ȡ�ļ��汾��Ϣ 
bool webServer :: httprsp_profile_verinfo(socketTCP *psock,httpResponse &httprsp,const char *spath)
{
	cBuffer buffer(2048);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<?xml version=\"1.0\" encoding=\"gb2312\" ?><xmlroot>");
	
	FileVerInfo fverinfo;
	if( GetVersionInfo((LPTSTR)spath,fverinfo) )
	{
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<Comments>%s</Comments>",fverinfo.Comments);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<Company>%s</Company>",fverinfo.CompanyName);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ProductName>%s</ProductName>",fverinfo.ProductName);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<ProductVer>%s</ProductVer>",fverinfo.ProductVersion);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<InterName>%s</InterName>",fverinfo.InternalName);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<FileDesc>%s</FileDesc>",fverinfo.FileDescription);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<FileVer>%s</FileVer>",fverinfo.FileVersion);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<OriFilename>%s</OriFilename>",fverinfo.OriginalFilename);
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"<Copyright>%s</Copyright>",fverinfo.LegalCopyright);
	}//?if( GetVersionInfo(spath,fverinfo) )

	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str())
		buffer.len()+=sprintf(buffer.str()+buffer.len(),"</xmlroot>");
	
	httprsp.NoCache();//CacheControl("No-cache");
	//����MIME���ͣ�Ĭ��ΪHTML
	httprsp.set_mimetype(MIMETYPE_XML);
	//������Ӧ���ݳ���
	httprsp.lContentLength(buffer.len());
	httprsp.send_rspH(psock,200,"OK");
	
	if(buffer.str()) psock->Send(buffer.len(),buffer.str(),-1);
	return true;
}

//-------------------------------------------------------------------------------
//��ȡĿ¼�Ĵ�С(KB) ��Ŀ¼�а�������Ŀ¼���ļ�����
double folderSize(const char *spath,const char *name,unsigned long &folders,unsigned long &files)
{
	double dblsize=0;
	WIN32_FIND_DATA finddata;
	string strPath; strPath.assign(spath);
	if(name){ strPath.append("\\"); strPath.append(name); }
	strPath.append("\\*");
	HANDLE hd=::FindFirstFile(strPath.c_str(), &finddata);
	if(hd==INVALID_HANDLE_VALUE) return false;
	do{
		if(finddata.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			if(strcmp(finddata.cFileName,".")==0 || strcmp(finddata.cFileName,"..")==0)
				continue;
			folders++;
			dblsize+=folderSize(spath,finddata.cFileName,folders,files);
		}
		else
		{
			dblsize+=finddata.nFileSizeLow;
			files++;
		}
	}while(::FindNextFile(hd,&finddata));
	::FindClose(hd); return dblsize;
}

bool ifHasSubDir(const char *spath,const char *filename)
{
	WIN32_FIND_DATA finddata;
	string strPath; strPath.assign(spath);
	strPath.append("\\"); strPath.append(filename);
	strPath.append("\\*");
	HANDLE hd=::FindFirstFile(strPath.c_str(), &finddata);
	if(hd==INVALID_HANDLE_VALUE) return false;
	do{
		if(finddata.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
		{
			if(strcmp(finddata.cFileName,".")==0 || strcmp(finddata.cFileName,"..")==0)
				continue;
			::FindClose(hd); return true;
		}
	}while(::FindNextFile(hd,&finddata));
	::FindClose(hd); return false;
}

bool folderList(cBuffer &buffer,const char *spath,bool bdsphide)
{
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	buffer.len()+=sprintf(buffer.str()+buffer.len(),"<folders>");
	
	long lret=0;
	if(spath==NULL || spath[0]==0)
	{//�г����д���
		char s[4]; char volumname[64];
		s[0]=0;s[1]=':';s[2]='\\';s[3]=0;
		DWORD dwDrives=::GetLogicalDrives();
		for(int i=0;i<26;i++)
		{
			if( (dwDrives & (1<<i))==0 ) continue;
			s[0]='A'+i; volumname[0]=0;
			UINT drtype=::GetDriveType(s); //��ȡ����
			if(drtype!=DRIVE_REMOVABLE || i!=0)  //���������A yyc modify 2006-09-14
				::GetVolumeInformation(s,volumname,64,0,0,0,0,0);
			++lret; s[2]=0; //ȥ�����ķ�б��
			buffer.len()+=sprintf(buffer.str()+buffer.len(),
				"<fitem><bhide></bhide><hassub>+</hassub><alias>%s</alias><fname>%s(%c:) %c %s</fname><fsize></fsize><ftype></ftype><ftime></ftime></fitem>"
				,s,DRIVE_TYPE[drtype],s[0],((volumname[0]==0)?' ':'-'), volumname);
			if(buffer.Space()<256) buffer.Resize(buffer.size()+256);
			if(buffer.str()==NULL) break;
		}//?for(int i=0
	}else if(spath[1]==':'){ //��Ч��·��
		WIN32_FIND_DATA finddata; SYSTEMTIME st; //FILETIME localFtime;
		string strPath; strPath.assign(spath); strPath.append("\\*");
		HANDLE hd=::FindFirstFile(strPath.c_str(), &finddata);
		if(hd!=INVALID_HANDLE_VALUE)
		{
			do{
				if(finddata.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
				{
					if(strcmp(finddata.cFileName,".")==0 || strcmp(finddata.cFileName,"..")==0)
						continue;
					if((finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && !bdsphide) continue;
					long fnlen=strlen(finddata.cFileName);
					if(fnlen<256) fnlen=256;
					if(buffer.Space()<fnlen) buffer.Resize(buffer.size()+fnlen);
					if(buffer.str()==NULL) break;
					++lret; //�жϴ�Ŀ¼�Ƿ�����Ŀ¼
					bool bHas=ifHasSubDir(spath,finddata.cFileName);
//					::FileTimeToLocalFileTime(&finddata.ftLastWriteTime,&localFtime);
//					::FileTimeToSystemTime(&localFtime,&st);
					::FileTimeToSystemTime(&finddata.ftLastWriteTime,&st);
					buffer.len()+=sprintf(buffer.str()+buffer.len(),
						"<fitem><bhide>%c</bhide><hassub>%c</hassub><alias></alias><fname><![CDATA[%s]]></fname><fsize></fsize><ftype>�ļ���</ftype><ftime>%04d-%02d-%02d %02d:%02d</ftime></fitem>"
						,((finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)?'*':' '),
						((bHas)?'+':' '),finddata.cFileName,st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute);				
				}//?if(
			}while(::FindNextFile(hd,&finddata));
			::FindClose(hd);
		}//?if(hd!=INVALID_HANDLE_VALUE)
	}else lret=-1;
	
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str()) buffer.len()+=sprintf(buffer.str()+buffer.len(),"</folders>");
	return (lret<0)?false:true;
}

bool fileList(cBuffer &buffer,const char *spath,bool bdsphide)
{
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str()) buffer.len()+=sprintf(buffer.str()+buffer.len(),"<files>");

	long lret=0;
	if(spath && spath[1]==':')
	{
		WIN32_FIND_DATA finddata; SYSTEMTIME st; //FILETIME localFtime;
		string strPath; strPath.assign(spath); strPath.append("\\*");
		HANDLE hd=::FindFirstFile(strPath.c_str(), &finddata);
		if(hd!=INVALID_HANDLE_VALUE)
		{
			do{
				if((finddata.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)==0) //���ļ���
				{
					if((finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && !bdsphide) continue;
					long fnlen=strlen(finddata.cFileName);
					if(fnlen<256) fnlen=256;
					if(buffer.Space()<fnlen) buffer.Resize(buffer.size()+fnlen);
					if(buffer.str()==NULL) break; else ++lret;
//					::FileTimeToLocalFileTime(&finddata.ftLastWriteTime,&localFtime);
//					::FileTimeToSystemTime(&localFtime,&st);
					::FileTimeToSystemTime(&finddata.ftLastWriteTime,&st);
					long lsize=finddata.nFileSizeLow/1024;
					buffer.len()+=sprintf(buffer.str()+buffer.len(),
						"<fitem><bhide>%c</bhide><hassub></hassub><alias></alias><fname><![CDATA[%s]]></fname><fsize>%d KB</fsize><lsize>%08d</lsize><ftype>%s</ftype><ftime>%04d-%02d-%02d %02d:%02d</ftime></fitem>"
						,((finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)?'*':' '),
						finddata.cFileName,lsize,lsize, getFileType(finddata.cFileName), 
						st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute);				
				}//?if(
			}while(::FindNextFile(hd,&finddata));
			::FindClose(hd);
		}//?if(hd!=INVALID_HANDLE_VALUE)
	}else lret=-1;
	
	if(buffer.Space()<16) buffer.Resize(buffer.size()+16);
	if(buffer.str()) buffer.len()+=sprintf(buffer.str()+buffer.len(),"</files>");
	return (lret<0)?false:true;
}
const char * getFileType(const char *filename)
{
	static string ftype; ftype="�ļ�";
	if(filename==NULL || filename[0]==0) return ftype.c_str();
	char *ptr=strrchr(filename,'.');
	if(ptr==NULL) return ftype.c_str();
	ftype.assign(ptr+1); ftype.append(" �ļ�");
	HKEY  hKEY; char regPath[255]; regPath[0]=0;
	if(::RegOpenKeyEx(HKEY_CLASSES_ROOT, (LPCTSTR)ptr, 0, KEY_READ, &hKEY)==ERROR_SUCCESS)
	{
		DWORD dwType=REG_SZ; DWORD dwLen=255;
		if(::RegQueryValueEx(hKEY, NULL, NULL,&dwType,(LPBYTE)regPath,&dwLen)==ERROR_SUCCESS)
		{
			regPath[dwLen]=0; ::RegCloseKey(hKEY);
			if(::RegOpenKeyEx(HKEY_CLASSES_ROOT, (LPCTSTR)regPath, 0, KEY_READ, &hKEY)==ERROR_SUCCESS)
			{
				dwType=REG_SZ; dwLen=255;
				if(::RegQueryValueEx(hKEY, NULL, NULL,&dwType,(LPBYTE)regPath,&dwLen)==ERROR_SUCCESS)
				{
					regPath[dwLen]=0;
					ftype.assign(regPath);
				}
			}
		}
		::RegCloseKey(hKEY);
	}
	if(ftype.length()>40) ftype[40]=0;
	return ftype.c_str();
}
//��ȡ�ļ��Ĵ�ģʽ
const char * getFileOpmode(const char *filename)
{
	static string fopmode; fopmode="δ֪Ӧ�ó���";
	if(filename==NULL || filename[0]==0) return fopmode.c_str();
	char *ptr=strrchr(filename,'.');
	if(ptr==NULL) return fopmode.c_str();
	HKEY  hKEY; char regPath[255]; regPath[0]=0;
	if(::RegOpenKeyEx(HKEY_CLASSES_ROOT, (LPCTSTR)ptr, 0, KEY_READ, &hKEY)==ERROR_SUCCESS)
	{
		DWORD dwType=REG_SZ; DWORD dwLen=255;
		if(::RegQueryValueEx(hKEY, NULL, NULL,&dwType,(LPBYTE)regPath,&dwLen)==ERROR_SUCCESS)
		{
			regPath[dwLen]=0; ::RegCloseKey(hKEY);
			strcat(regPath,"\\shell\\open\\Command"); //ddeexec\\Application
			if(::RegOpenKeyEx(HKEY_CLASSES_ROOT, (LPCTSTR)regPath, 0, KEY_READ, &hKEY)==ERROR_SUCCESS)
			{
				dwType=REG_SZ; dwLen=255;
				if(::RegQueryValueEx(hKEY, NULL, NULL,&dwType,(LPBYTE)regPath,&dwLen)==ERROR_SUCCESS)
				{
					regPath[dwLen]=0; fopmode.assign(regPath);
					::RegCloseKey(hKEY); //��ȡ�򿪵�exe������
					const char *p=strrchr(regPath,'\\');
					const char *p1=(p)?strchr(p+1,'"'):NULL;
					if(p && p1)
					{
						*(char *)p1=0; strcat((char *)p+1,"\\shell");
						::memmove(regPath+13,p+1,strlen(p+1)+1);
						::memcpy(regPath,"Applications\\",13);
						if(::RegOpenKeyEx(HKEY_CLASSES_ROOT, (LPCTSTR)regPath, 0, KEY_READ, &hKEY)==ERROR_SUCCESS)
						{
							dwType=REG_SZ; dwLen=255;
							if(::RegQueryValueEx(hKEY, "FriendlyCache", NULL,&dwType,(LPBYTE)regPath,&dwLen)==ERROR_SUCCESS)
							{
								regPath[dwLen]=0; fopmode.append("\r\n");
								fopmode.append(regPath);
							}
						}
					}//?if(p && p1)
				}//?if(::RegQueryValueEx(hKEY, NULL, NULL
			}//?if(::RegOpenKeyEx(HKEY_CLASSES_ROOT,
		}//?if(::RegQueryValueEx(hKEY, NULL, 
		::RegCloseKey(hKEY);
	}
	return fopmode.c_str();
}

// Structure used to store enumerated languages and code pages.

typedef DWORD (APIENTRY *PGetFileVersionInfoSize)(LPSTR,LPDWORD);
typedef BOOL (APIENTRY *PGetFileVersionInfo)(LPSTR,DWORD,DWORD,LPVOID);
typedef BOOL (APIENTRY *PVerQueryValue)(const LPVOID,LPSTR,LPVOID *,PUINT);
bool GetVersionInfo(LPTSTR filename,FileVerInfo & fverinfo)
{
	if(filename==NULL || filename[0]==0) return false;
	HMODULE hmdl=::LoadLibrary("Version.dll");
	if(hmdl==NULL) return false;
	
	PGetFileVersionInfoSize ptr_GetFileVersionInfoSize =
		(PGetFileVersionInfoSize) GetProcAddress(hmdl,"GetFileVersionInfoSizeA");
	PGetFileVersionInfo ptr_GetFileVersionInfo =
		(PGetFileVersionInfo) GetProcAddress(hmdl,"GetFileVersionInfoA");
	PVerQueryValue ptr_VerQueryValue =
		(PVerQueryValue) GetProcAddress(hmdl,"VerQueryValueA");
	if(ptr_GetFileVersionInfoSize==NULL || ptr_GetFileVersionInfo==NULL || ptr_VerQueryValue==NULL)
	{
		::FreeLibrary(hmdl);
		return false;
	}
	
	DWORD bufsize=(*ptr_GetFileVersionInfoSize)(filename,NULL);
	//bufsize==0 ˵�����ļ��ް汾��Ϣ
	if(bufsize==0){ ::FreeLibrary(hmdl); return false; }
	char *pBlock=new char[bufsize];
	if(pBlock==NULL) return false;
	if( (*ptr_GetFileVersionInfo)(filename,0,bufsize,pBlock)==0 ) 
	{ 
		::FreeLibrary(hmdl);
		delete[] pBlock; return false; 
	}
	
	// Read the list of languages and code pages.
	UINT cbTranslate=0;
	struct LANGANDCODEPAGE {
	  WORD wLanguage;
	  WORD wCodePage;
	} *lpTranslate; 
	(*ptr_VerQueryValue)(pBlock, TEXT("\\VarFileInfo\\Translation"),(LPVOID*)&lpTranslate,&cbTranslate);
	
	// only Retrieve file info for language and code page 0.
	TCHAR SubBlock[64]; LPVOID lpBuffer; UINT dwBytes;
	
	lpBuffer=NULL;  dwBytes=0;
	::memset((void *)fverinfo.Comments,0,FILEVERINFOLEN);  //��ȡ�ļ�ע��
	wsprintf( SubBlock, TEXT("\\StringFileInfo\\%04x%04x\\Comments"),lpTranslate[0].wLanguage,lpTranslate[0].wCodePage);
	(*ptr_VerQueryValue)(pBlock, SubBlock, &lpBuffer, &dwBytes);
	if(dwBytes>=FILEVERINFOLEN) dwBytes=FILEVERINFOLEN-1; //��ֹԽ��
	if(lpBuffer && dwBytes!=0) ::memcpy((void *)fverinfo.Comments,lpBuffer,dwBytes);
	
	lpBuffer=NULL;  dwBytes=0;
	::memset((void *)fverinfo.CompanyName,0,FILEVERINFOLEN);  //��ȡ��˾��
	wsprintf( SubBlock, TEXT("\\StringFileInfo\\%04x%04x\\CompanyName"),lpTranslate[0].wLanguage,lpTranslate[0].wCodePage);
	(*ptr_VerQueryValue)(pBlock, SubBlock, &lpBuffer, &dwBytes);
	if(dwBytes>=FILEVERINFOLEN) dwBytes=FILEVERINFOLEN-1; //��ֹԽ��
	if(lpBuffer && dwBytes!=0) ::memcpy((void *)fverinfo.CompanyName,lpBuffer,dwBytes);
	
	lpBuffer=NULL;  dwBytes=0;
	::memset((void *)fverinfo.ProductName,0,FILEVERINFOLEN);  //��ȡ��Ʒ��
	wsprintf( SubBlock, TEXT("\\StringFileInfo\\%04x%04x\\ProductName"),lpTranslate[0].wLanguage,lpTranslate[0].wCodePage);
	(*ptr_VerQueryValue)(pBlock, SubBlock, &lpBuffer, &dwBytes);
	if(dwBytes>=FILEVERINFOLEN) dwBytes=FILEVERINFOLEN-1; //��ֹԽ��
	if(lpBuffer && dwBytes!=0) ::memcpy((void *)fverinfo.ProductName,lpBuffer,dwBytes);
	
	lpBuffer=NULL;  dwBytes=0;
	::memset((void *)fverinfo.ProductVersion,0,FILEVERINFOLEN);  //��ȡ��Ʒ�汾
	wsprintf( SubBlock, TEXT("\\StringFileInfo\\%04x%04x\\ProductVersion"),lpTranslate[0].wLanguage,lpTranslate[0].wCodePage);
	(*ptr_VerQueryValue)(pBlock, SubBlock, &lpBuffer, &dwBytes);
	if(dwBytes>=FILEVERINFOLEN) dwBytes=FILEVERINFOLEN-1; //��ֹԽ��
	if(lpBuffer && dwBytes!=0) ::memcpy((void *)fverinfo.ProductVersion,lpBuffer,dwBytes);
	
	lpBuffer=NULL;  dwBytes=0;
	::memset((void *)fverinfo.InternalName,0,FILEVERINFOLEN);  //��ȡ�ڲ�����
	wsprintf( SubBlock, TEXT("\\StringFileInfo\\%04x%04x\\InternalName"),lpTranslate[0].wLanguage,lpTranslate[0].wCodePage);
	(*ptr_VerQueryValue)(pBlock, SubBlock, &lpBuffer, &dwBytes);
	if(dwBytes>=FILEVERINFOLEN) dwBytes=FILEVERINFOLEN-1; //��ֹԽ��
	if(lpBuffer && dwBytes!=0) ::memcpy((void *)fverinfo.InternalName,lpBuffer,dwBytes);

	lpBuffer=NULL;  dwBytes=0;
	::memset((void *)fverinfo.FileDescription,0,FILEVERINFOLEN);  //��ȡ�ļ�����
	wsprintf( SubBlock, TEXT("\\StringFileInfo\\%04x%04x\\FileDescription"),lpTranslate[0].wLanguage,lpTranslate[0].wCodePage);
	(*ptr_VerQueryValue)(pBlock, SubBlock, &lpBuffer, &dwBytes);
	if(dwBytes>=FILEVERINFOLEN) dwBytes=FILEVERINFOLEN-1; //��ֹԽ��
	if(lpBuffer && dwBytes!=0) ::memcpy((void *)fverinfo.FileDescription,lpBuffer,dwBytes);

	lpBuffer=NULL;  dwBytes=0;
	::memset((void *)fverinfo.FileVersion,0,FILEVERINFOLEN);  //��ȡ�ļ��汾
	wsprintf( SubBlock, TEXT("\\StringFileInfo\\%04x%04x\\FileVersion"),lpTranslate[0].wLanguage,lpTranslate[0].wCodePage);
	(*ptr_VerQueryValue)(pBlock, SubBlock, &lpBuffer, &dwBytes);
	if(dwBytes>=FILEVERINFOLEN) dwBytes=FILEVERINFOLEN-1; //��ֹԽ��
	if(lpBuffer && dwBytes!=0) ::memcpy((void *)fverinfo.FileVersion,lpBuffer,dwBytes);

	lpBuffer=NULL;  dwBytes=0;
	::memset((void *)fverinfo.OriginalFilename,0,FILEVERINFOLEN);  //��ȡԭʼ�ļ���
	wsprintf( SubBlock, TEXT("\\StringFileInfo\\%04x%04x\\OriginalFilename"),lpTranslate[0].wLanguage,lpTranslate[0].wCodePage);
	(*ptr_VerQueryValue)(pBlock, SubBlock, &lpBuffer, &dwBytes);
	if(dwBytes>=FILEVERINFOLEN) dwBytes=FILEVERINFOLEN-1; //��ֹԽ��
	if(lpBuffer && dwBytes!=0) ::memcpy((void *)fverinfo.OriginalFilename,lpBuffer,dwBytes);

	lpBuffer=NULL;  dwBytes=0;
	::memset((void *)fverinfo.LegalCopyright,0,FILEVERINFOLEN);  //��ȡ�Ϸ���Ȩ
	wsprintf( SubBlock, TEXT("\\StringFileInfo\\%04x%04x\\LegalCopyright"),lpTranslate[0].wLanguage,lpTranslate[0].wCodePage);
	(*ptr_VerQueryValue)(pBlock, SubBlock, &lpBuffer, &dwBytes);
	if(dwBytes>=FILEVERINFOLEN) dwBytes=FILEVERINFOLEN-1; //��ֹԽ��
	if(lpBuffer && dwBytes!=0) ::memcpy((void *)fverinfo.LegalCopyright,lpBuffer,dwBytes);
	
//	lpBuffer=NULL;  dwBytes=0;
//	::memset((void *)fverinfo.LegalTrademarks,0,FILEVERINFOLEN);  //��ȡ�Ϸ��̱�
//	wsprintf( SubBlock, TEXT("\\StringFileInfo\\%04x%04x\\LegalTrademarks"),lpTranslate[0].wLanguage,lpTranslate[0].wCodePage);
//	(*ptr_VerQueryValue)(pBlock, SubBlock, &lpBuffer, &dwBytes);
//	if(dwBytes>=FILEVERINFOLEN) dwBytes=FILEVERINFOLEN-1; //��ֹԽ��
//	if(lpBuffer && dwBytes!=0) ::memcpy((void *)fverinfo.LegalTrademarks,lpBuffer,dwBytes);

//	lpBuffer=NULL;  dwBytes=0;
//	::memset((void *)fverinfo.SpecialBuild,0,FILEVERINFOLEN);  //��ȡSpecialBuild
//	wsprintf( SubBlock, TEXT("\\StringFileInfo\\%04x%04x\\SpecialBuild"),lpTranslate[0].wLanguage,lpTranslate[0].wCodePage);
//	(*ptr_VerQueryValue)(pBlock, SubBlock, &lpBuffer, &dwBytes);
//	if(dwBytes>=FILEVERINFOLEN) dwBytes=FILEVERINFOLEN-1; //��ֹԽ��
//	if(lpBuffer && dwBytes!=0) ::memcpy((void *)fverinfo.SpecialBuild,lpBuffer,dwBytes);

//	lpBuffer=NULL;  dwBytes=0;
//	::memset((void *)fverinfo.PrivateBuild,0,FILEVERINFOLEN);  //��ȡPrivateBuild
//	wsprintf( SubBlock, TEXT("\\StringFileInfo\\%04x%04x\\PrivateBuild"),lpTranslate[0].wLanguage,lpTranslate[0].wCodePage);
//	(*ptr_VerQueryValue)(pBlock, SubBlock, &lpBuffer, &dwBytes);
//	if(dwBytes>=FILEVERINFOLEN) dwBytes=FILEVERINFOLEN-1; //��ֹԽ��
//	if(lpBuffer && dwBytes!=0) ::memcpy((void *)fverinfo.PrivateBuild,lpBuffer,dwBytes);
	
	::FreeLibrary(hmdl);
	delete[] pBlock;
	return true;
}
