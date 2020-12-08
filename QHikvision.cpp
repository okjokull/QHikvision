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
	struPlayInfo.hPlayWnd = hWnd;//��Ҫ SDK ����ʱ�����Ϊ��Чֵ����ȡ��������ʱ����Ϊ��
	struPlayInfo.lChannel = 1;//Ԥ��ͨ����
	struPlayInfo.dwStreamType = 0;//�������ͣ�0-��������1-��������2-��������3-�����������Դ�����
	struPlayInfo.dwLinkMode = 0;//0- TCP ��ʽ��1- UDP ��ʽ��2- �ಥ��ʽ��3- RTP ��ʽ��4-RTP/RTSP��5-RSTP/HTTP
	struPlayInfo.bBlocked = 1;//0- ������ȡ����1- ����ȡ��

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
	//��������ʱ��������ʱ��
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

void QHikvision::OnClickedLogin() //��¼
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

void QHikvision::OnClickedPreview()  //Ԥ��
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

void QHikvision::OnClickedStopPreview()  //ֹͣԤ��
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


 //�����澯
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
	//��������
	//NET_DVR_SetDVRMessageCallBack_V30(MSesGCallback, NULL);
	NET_DVR_SetupAlarmChan_V30(userID);
	NET_DVR_SetDVRMessageCallBack_V30(MessageCallback, this);
	NET_DVR_SETUPALARM_PARAM struSetupParam = { 0 };
	struSetupParam.dwSize = sizeof(NET_DVR_SETUPALARM_PARAM);

	struSetupParam.byAlarmInfoType = 1;//�ϴ�������Ϣ���ͣ�0-�ϱ�����Ϣ(NET_DVR_PLATE_RESULT), 1-�±�����Ϣ(NET_ITS_PLATE_RESULT)
	struSetupParam.byLevel = 0;//��߼��𲼷�
	//bySupport ��λ��ʾ��ֵ��0 - �ϴ���1 - ���ϴ�;  bit0 - ��ʾ���������Ƿ��ϴ�ͼƬ;

	LONG  IHandle = -1;
	IHandle = NET_DVR_SetupAlarmChan_V41(userID, &struSetupParam);//���������ϴ�ͨ������ȡ��������Ϣ��
	if (IHandle < 0)
	{
	
		qDebug() << "NET_DVR_SetupAlarmChan_V41 Failed!Error number";
		NET_DVR_Logout(userID);
		NET_DVR_Cleanup();
		return;
	}


}



