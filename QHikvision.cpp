#include "QHikvision.h"
#include <QMessageBox>
#include<QScreen>

QHikvision::QHikvision(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);


	userID = -1;
	previewID = -1;

	connect(ui.LoginButton, &QPushButton::clicked, this, &QHikvision::OnClickedLogin);
	connect(ui.LogoutButton, &QPushButton::clicked, this, &QHikvision::OnClickedLogout);
	connect(ui.PreviewButton, &QPushButton::clicked, this, &QHikvision::OnClickedPreview);
	connect(ui.StopPreviewButton, &QPushButton::clicked, this, &QHikvision::OnClickedStopPreview);
	connect(ui.CaptureButton, &QPushButton::clicked, this, &QHikvision::OnClickedCapture);
	if (initSDK())
		qDebug() << "SDK init Success!" << endl;

	OnClickedLogin();

	QTimer *timer = new QTimer(this);     
	QTimer::singleShot(3000, this, SLOT(OnClickedPreview()));
	timer->start(3000);

	

}



long QHikvision::play(HWND hWnd, NET_DVR_PREVIEWINFO struPlayInfo)
{
	struPlayInfo = { 0 };
	struPlayInfo.hPlayWnd = hWnd;//需要 SDK 解码时句柄设为有效值，仅取流不解码时可设为空
	struPlayInfo.lChannel = 1;//预览通道号
	struPlayInfo.dwStreamType = 0;//码流类型：0-主码流，1-子码流，2-三码流，3-虚拟码流，以此类推
	struPlayInfo.dwLinkMode = 0;//0- TCP 方式，1- UDP 方式，2- 多播方式，3- RTP 方式，4-RTP/RTSP，5-RSTP/HTTP
	struPlayInfo.bBlocked = 1;//0- 非阻塞取流，1- 阻塞取流

	long IRealPlayHandle = NET_DVR_RealPlay_V40(userID, &struPlayInfo, NULL, NULL);

	if (IRealPlayHandle < 0)
	{
		qDebug() << "NET_DVR_RealPlay_V40 error;error number " << NET_DVR_GetLastError();
		QMessageBox::warning(this, "error", "NET_DVR_RealPlay_V40 error;error number " + QString::number(NET_DVR_GetLastError()));
		NET_DVR_Logout(userID);
		userID = -1;
		return -1;
	}

	return IRealPlayHandle;
}

bool QHikvision::initSDK()
{
	bool isok = NET_DVR_Init();
	if (isok == false)
	{
		qDebug() << "NET_DVR_Init error;error number is " << NET_DVR_GetLastError();
		QMessageBox::warning(this, "error", "NET_DVR_Init error;error number is " + QString::number(NET_DVR_GetLastError()));
		return isok;
	}
	//设置连接时间与重连时间
	NET_DVR_SetConnectTime(2000, 1);
	NET_DVR_SetReconnect(10000, true);
	return isok;
}

bool QHikvision::uninitSDK()
{
	bool isok = NET_DVR_Cleanup();
	if (isok == false)
	{
		qDebug() << "NET_DVR_Cleanup error;error number is " << NET_DVR_GetLastError();
		QMessageBox::warning(this, "error", "NET_DVR_Cleanup error ; error number " + QString::number(NET_DVR_GetLastError()));
		return isok;
	}
	return isok;
}

void QHikvision::OnClickedLogin() //登录
{
	NET_DVR_DEVICEINFO_V30 DeviceInfoTmp;
	userID = NET_DVR_Login_V30("192.168.0.102", 8000, "admin", "hk123456789", &DeviceInfoTmp);
	if (userID < 0)
	{
		qDebug() << "NET_DVR_Login_V30 error;" << "error number is " << NET_DVR_GetLastError();
		QMessageBox::warning(this, "error", "NET_DVR_Login_V30 error;error number " + QString::number(NET_DVR_GetLastError()));
		return;
	}
	qDebug() << "Login Success,userID:" << userID << endl;
	SetupAlarm();
}

void QHikvision::OnClickedLogout()
{
	if (userID != -1)
	{
		if (NET_DVR_Logout(userID) == false)
		{
			qDebug() << "NET_DVR_Logout error;" << "error number is " << NET_DVR_GetLastError();
			QMessageBox::warning(this, "error", "NET_DVR_Logout error;error number " + QString::number(NET_DVR_GetLastError()));
			return;
		}
		userID = -1;
		qDebug() << "Logout Success!" << endl;
	}
}

void QHikvision::OnClickedPreview()  //预览
{
	if (userID == -1)
	{
		qDebug() << "Need Login!" << endl;
		QMessageBox::warning(this, "error", "Need Login!");
		return;
	}
	HWND hWnd = (HWND)ui.label->winId();
	NET_DVR_PREVIEWINFO struPlayInfo;
	previewID = play(hWnd, struPlayInfo);
	if (previewID == -1)
	{
		return;
	}
	qDebug() << "Preview Success!" << userID << endl;
}

void QHikvision::OnClickedStopPreview()  //停止预览
{
	if (previewID != -1)
	{
		if (NET_DVR_StopRealPlay(previewID) == false)
		{
			qDebug() << "NET_DVR_StopRealPlay error;" << "error number is " << NET_DVR_GetLastError();
			QMessageBox::warning(this, "error", "NET_DVR_StopRealPlay error;error number " + QString::number(NET_DVR_GetLastError()));
			return;
		}
		previewID = -1;
		qDebug() << "Stop Preview Success!" << endl;
	}
}

void QHikvision::OnClickedCapture() 
{

	if (userID != -1)
	{
		NET_DVR_JPEGPARA IpJpegPara2;
		IpJpegPara2.wPicQuality = 0;
		IpJpegPara2.wPicSize = 0xff;

		QDateTime current_date_time = QDateTime::currentDateTime();
		QString current_date = current_date_time.toString("yyyy-MM-dd_hh-mm-ss_zzz");
		current_date = "E:\\Picture\\" + current_date + ".jpg";
		char*  fileName;
		QByteArray ba = current_date.toLatin1(); // must
		fileName = ba.data();


		if (NET_DVR_CaptureJPEGPicture(userID, 1, &IpJpegPara2, fileName) == false)
		{
			qDebug() << "NET_DVR_CaptureJPEGPicture error;" << "error number is " << NET_DVR_GetLastError();
			QMessageBox::warning(this, "error", "NET_DVR_CaptureJPEGPicture error;error number " + QString::number(NET_DVR_GetLastError()));
			return;
		}
		qDebug() << "Capture Success!" << endl;
	}
}


 //监听告警
 void CALLBACK MessageCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser)
 {
	 ((QHikvision*)pUser)->msgCallback(lCommand, pAlarmer, pAlarmInfo, dwBufLen);
	 return;
 }

 void QHikvision::msgCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen)
 {
	 int i = 0;
	 switch (lCommand)
	 {
	 case COMM_ITS_PLATE_RESULT:
	 {
		 NET_ITS_PLATE_RESULT struITSPlateResult = { 0 };
		 memcpy(&struITSPlateResult, pAlarmInfo, sizeof(struITSPlateResult));
		 break;
	 }
	 default:
	 {
	
		 OnClickedCapture();
		// slotCutScreen();
		 break;
	 }
	 }
	 return;
 }

void QHikvision::SetupAlarm()
{
	//启动布防
	//NET_DVR_SetDVRMessageCallBack_V30(MSesGCallback, NULL);
	NET_DVR_SetupAlarmChan_V30(userID);
	NET_DVR_SetDVRMessageCallBack_V30(MessageCallback, this);
	NET_DVR_SETUPALARM_PARAM struSetupParam = { 0 };
	struSetupParam.dwSize = sizeof(NET_DVR_SETUPALARM_PARAM);

	struSetupParam.byAlarmInfoType = 1;//上传报警信息类型：0-老报警信息(NET_DVR_PLATE_RESULT), 1-新报警信息(NET_ITS_PLATE_RESULT)
	struSetupParam.byLevel = 0;//最高级别布防
	//bySupport 按位表示，值：0 - 上传，1 - 不上传;  bit0 - 表示二级布防是否上传图片;

	LONG  IHandle = -1;
	IHandle = NET_DVR_SetupAlarmChan_V41(userID, &struSetupParam);//建立报警上传通道，获取报警等信息。
	if (IHandle < 0)
	{
	
		qDebug() << "NET_DVR_SetupAlarmChan_V41 Failed!Error number";
		NET_DVR_Logout(userID);
		NET_DVR_Cleanup();
		return;
	}


}



