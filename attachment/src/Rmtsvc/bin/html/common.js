var strUserAgent="";
var xmlHttp=false;
var oPopup=false;
var timerID_popup=0;
var ACCESS_SCREEN_ALL=0x0003;
var ACCESS_REGIST_ALL=0x000c
var ACCESS_SERVICE_ALL=0x0030
var ACCESS_TELNET_ALL=0x00c0
var ACCESS_FILE_ALL=0x0300
var ACCESS_FTP_ADMIN=0x0c00
var ACCESS_VIDC_ADMIN=0x3000

function Str2Bytes(str,charset)
{
	var ms=false;
	try
	{
		ms = new ActiveXObject("ADODB.Stream"); //����������
		if(!ms) throw "error";
	}
	catch(e)
	{
		return str;
	}
	
	ms.Type = 2;			//Text
        ms.Charset = charset;		//����������ı��뷽ʽΪ charset
        ms.Open();
        ms.WriteText(str);		//��strд����������
        
        ms.Position = 0;		//�������������ʼλ����0 ������Charset����
        ms.Type = 1;			//Binary
        vout = ms.Read(ms.Size);		//ȡ�ַ���
        ms.close();			//�ر�������
        return vout;
}

function createXMLHttpRequest() {
    if (window.ActiveXObject) {
        xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
    } 
    else if (window.XMLHttpRequest) {
        xmlHttp = new XMLHttpRequest();
    }
    if(!xmlHttp) alert("����XMLHTTP����ʧ��");
}

function createpopup()
{
	if(strUserAgent.indexOf("MSIE")!=-1)
	{//����IE֧��
	oPopup = window.createPopup();
	var oPopBody = oPopup.document.body;
    	oPopBody.style.backgroundColor = "lightyellow";
    	oPopBody.style.border = "solid black 1px";
    	oPopBody.innerHTML = "&nbsp;&nbsp;���ڴ�����... ";
	}
}
var ista=0;
function moviePopup()
{
	var oPopBody = oPopup.document.body;
	if(ista==0)
	{
		oPopBody.innerHTML = "&nbsp;&nbsp;���ڴ�����../ ";
		ista=1;
	}
	else if(ista==1)
	{
		oPopBody.innerHTML = "&nbsp;&nbsp;���ڴ�����..- ";
		ista=2;
	}
	else if(ista==2)
	{
		oPopBody.innerHTML = "&nbsp;&nbsp;���ڴ�����..\\ ";
		ista=0;
	}
}
function showPopup(lx,ty,w,h)
{
	if(!oPopup) return;
	oPopup.show(lx, ty, w, h, document.body);
	timerID_popup=window.setInterval(moviePopup,500);
}

function hidePopup()
{
	if(!oPopup) return;
	if(timerID_popup!=0) window.clearInterval(timerID_popup);
	if(oPopup.isOpen) oPopup.hide();
	var oPopBody = oPopup.document.body;
	oPopBody.innerHTML = "&nbsp;&nbsp;���ڴ�����... ";
	ista=0;
}
