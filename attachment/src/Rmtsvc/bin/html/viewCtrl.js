var autoRefresh=0; //�Զ�ˢ��ʱ��ms
var imgLoaded=true;//��Ļ�����Ƿ����
var loadTimes=500; //�¼��������ʱˢ��imageʱ��ms
var ptX,ptY;
var ptX_drag,ptY_drag;
var timerID_key=0;
var timerID_click=0;
var timerID_move=0;
var txtKeyEvent="";
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
}

function msPosition(e) 
{ 
	var o=window.document.all("divScreen");
 	ptX=e.x+o.scrollLeft-o.parentElement.offsetLeft;
  	ptY=e.y+o.scrollTop-o.parentElement.offsetTop;
  	var w=window.parent.frmLeft;
  	w.document.all("lblXY").innerText="X:"+ptX+" , Y:"+ptY;
  	document.all("txtHide").focus();
}

function window_onload()
{
	var o=window.parent.frmLeft.document.all("chkAuto");
	if( o.checked ) autoRefresh=o.value;
	if(!xmlHttp) createXMLHttpRequest();
	loadImg();		
}

function processRequest() 
{
	if (xmlHttp.readyState == 4) { // �ж϶���״̬
		if (xmlHttp.status == 200) 
		{ // ��Ϣ�Ѿ��ɹ����أ���ʼ������Ϣ
			if(loadTimes>0)
				window.setTimeout(loadImg,loadTimes);
			else loadImg();
            	}
        }
}
function sendEvent(strurl,param,bLoad)
{
	if(timerID_move!=0)
	{
		window.clearTimeout(timerID_move);
		timerID_move=0;
	}
	if(timerID_click!=0)
	{
		window.clearTimeout(timerID_click);
		timerID_click=0;
	}
	if(bLoad)
		loadTimes=0;
	else loadTimes=500;
	xmlHttp.open("POST", strurl, true);
	xmlHttp.onreadystatechange = processRequest;
	xmlHttp.send(param);
}

function msmove()
{
	var x=ptX;
	var y=ptY;
 	msPosition(window.event);
 	if(autoRefresh>0 && (ptX-x)==0 && (ptY-y)==0)
 	{
 		var param="x="+ptX+"&y="+ptY+"&altk=0&button=0&act=0";
 		if(timerID_move==0)
			timerID_move=window.setTimeout("sendEvent(\"/msevent\",\""+param+"\",true)",autoRefresh);
	}
	else if(timerID_move!=0)
	{
		window.clearTimeout(timerID_move);
		timerID_move=0;
	}
}

function msclick()
{
	msPosition(window.event);
	var altk=0;
	if(window.event.ctrlKey) altk=altk | 1;
	if(window.event.shiftKey) altk=altk | 2;
	if(window.event.altKey) altk=altk | 4;
	var param="x="+ptX+"&y="+ptY+"&altk="+altk+"&button=1&act=1";
	if(timerID_click!=0) window.clearTimeout(timerID_click);
	if(timerID_move!=0) window.clearTimeout(timerID_move);
	timerID_click=window.setTimeout("sendEvent(\"/msevent\",\""+param+"\",true)",500);
}
function msdblclick()
{
	if(timerID_click!=0)
	{
		window.clearTimeout(timerID_click);
		timerID_click=0;
	}
	msPosition(window.event);
	var altk=0;
	if(window.event.ctrlKey) altk=altk | 1;
	if(window.event.shiftKey) altk=altk | 2;
	if(window.event.altKey) altk=altk | 4;
	var param="x="+ptX+"&y="+ptY+"&altk="+altk+"&button=1&act=2";
	sendEvent("/msevent",param,false);
}

function msdrop()
{
	msPosition(window.event);
	var altk=0;
	if(window.event.ctrlKey) altk=altk | 1;
	if(window.event.shiftKey) altk=altk | 2;
	if(window.event.altKey) altk=altk | 4;
	var param="x="+ptX+"&y="+ptY+"&altk="+altk+"&button=1&act=4&dragx="+ptX_drag+"&dragy="+ptY_drag;
	sendEvent("/msevent",param,false);
}
//��ȡ��갴���¼�����¼drag����ʼ�㡣 ondrag�¼���ȡ��ʼ�㲻��ȷ��ƫ��
function msdown()
{
	if(window.event.button==1)
	{
		msPosition(window.event);
		ptX_drag=ptX;
		ptY_drag=ptY;
	}
}
//��ȡ������ĵ�������
function msup()
{
	var b=window.event.button;
	if(b==1) return; //ȥ���������
	msPosition(window.event);
	var altk=0;
	if(window.event.ctrlKey) altk=altk | 1;
	if(window.event.shiftKey) altk=altk | 2;
	if(window.event.altKey) altk=altk | 4;
	var param="x="+ptX+"&y="+ptY+"&altk="+altk+"&button="+b+"&act=1";
	sendEvent("/msevent",param,false);
}

function mswheel()
{
	var altk=0;
	if(window.event.ctrlKey) altk=altk | 1;
	if(window.event.shiftKey) altk=altk | 2;
	if(window.event.altKey) altk=altk | 4;
	var d=window.event.wheelDelta;
	var param="x="+ptX+"&y="+ptY+"&altk="+altk+"&button=0&act=5&wheel="+window.event.wheelDelta;
	sendEvent("/msevent",param,false);
	window.event.returnValue=false;
}

function Kevent()
{
	var param=txtKeyEvent;
	txtKeyEvent="";
	if(param=="")
	{
		if(timerID_key!=0)
			window.clearInterval(timerID_key);
		timerID_key=0;
	}
	else
		sendEvent("/keyevent","vkey="+param,true);
}


function keyup()
{
	var altk=0;
	if(window.event.ctrlKey) altk=altk | 1;
	if(window.event.shiftKey) altk=altk | 2;
	if(window.event.altKey) altk=altk | 4;
	var kevent=altk*256+window.event.keyCode;
	txtKeyEvent=txtKeyEvent+kevent+",";
	if(timerID_key==0)
		timerID_key=window.setInterval(Kevent,1000);
	window.event.keyCode=0;
}
