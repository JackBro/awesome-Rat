var autoRefresh=0; //�Զ�ˢ��ʱ��ms
var imgLoaded=true;//��Ļ�����Ƿ����
function loadImg()
{
	if(imgLoaded)
	{
		document.screenimage.src="/capDesktop";
		imgLoaded=false;
	}
}

function Imgloaded()
{
	imgLoaded=true;
	if(autoRefresh>0) window.setTimeout("loadImg()",autoRefresh);
}

function msmove_IE()
{
 	var o=window.document.all("divScreen");
 	var x1=window.event.x+o.scrollLeft-o.parentElement.offsetLeft; 
  	var y1=window.event.y+o.scrollTop-o.parentElement.offsetTop; 
  	var w=window.parent.frmLeft;
  	w.document.all("lblXY").innerText="X:"+x1+" , Y:"+y1;
}

function processRequest() 
{
	if (xmlHttp.readyState == 4) { // �ж϶���״̬
		if (xmlHttp.status == 200) 
		{ // ��Ϣ�Ѿ��ɹ����أ���ʼ������Ϣ
			var xmlobj = xmlHttp.responseXML;
			var wtext=xmlobj.getElementsByTagName("wtext");
			var hwnd=xmlobj.getElementsByTagName("hwnd");
			if(hwnd!=null && hwnd.length>0)
			{
				loadImg();
				var sHelp="\r\n����AltȻ���������������ȡ����";
				if(hwnd.item(0).text==0)
					alert("��ǰ��������������棬���Ҫ����ָ������\r\n����Ctrl+ShiftȻ���������ָ���Ĵ���"+sHelp);
				else  alert("��ǰ���� "+"\""+wtext.item(0).text+"\" ����\r\n����Ctrl+ShiftȻ���Ҽ�����ȡ���Դ˴��ڵĲ���"+sHelp);
			}
			var iret=xmlobj.getElementsByTagName("result");
			if(iret!=null && iret.length>0)
			{
				if(iret.item(0).text==0)
					alert("�����������Ϊ: "+wtext.item(0).text);
				else if(iret.item(0).text==1)
					alert("�������,'"+wtext.item(0).text+"'")
				else alert("��ȡʧ��, err="+iret.item(0).text);
			}
            	}
        }
}

//��ȡ��������
function msup()
{
	var o=window.document.all("divScreen");
 	var x1=window.event.x+o.scrollLeft-o.parentElement.offsetLeft; 
  	var y1=window.event.y+o.scrollTop-o.parentElement.offsetTop;
	var altk=0;
	if(window.event.ctrlKey) altk=altk | 1;
	if(window.event.shiftKey) altk=altk | 2;
	if(window.event.altKey) altk=altk | 4;
	
  	if(altk==4) //��ȡָ�������������
  	{	
  		var qx=parent.frmLeft.userQX;
  		if((qx & ACCESS_SCREEN_ALL)==ACCESS_SCREEN_ALL)
  			xmlHttp.open("GET", "/getPassword?x="+x1+"&y="+y1, true);
  		else{ alert("��û��Ȩ�޲�����鿴�����������"); return; }
  	}else{
  		var act=0;
  		if(altk==3 && window.event.button==1) act=1; //����ָ���Ĵ���
  		else if(altk==3 && window.event.button==2) act=2; //������������
  		xmlHttp.open("GET", "/capWindow?act="+act+"&x="+x1+"&y="+y1, true);
  	}
	xmlHttp.onreadystatechange = processRequest;
	xmlHttp.send(null);
}

function window_onload()
{
	var o=window.parent.frmLeft.document.all("chkAuto");
	if( o.checked ) autoRefresh=o.value;
	if(!xmlHttp) createXMLHttpRequest();
	loadImg()
	return;	
}

