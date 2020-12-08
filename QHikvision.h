#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QHikvision.h"
#include <QDateTime>
#include <qDebug>
#include <QTimer>
#include <QMessageBox>

#include "HCNetSDK.h"
#include <windows.h>

class QHikvision : public QMainWindow
{
	Q_OBJECT

	long play(HWND hWnd, NET_DVR_PREVIEWINFO struPlayInfo);
	bool initSDK();
	bool uninitSDK();


public:
	QHikvision(QWidget *parent = Q_NULLPTR);
	void msgCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen);

	public slots:
	void OnClickedLogin();
	void OnClickedLogout();
	void OnClickedPreview();
	void OnClickedStopPreview();
	void OnClickedCapture();
   
	void SetupAlarm();//±¨¾¯²¼·À

private:
	Ui::QHikvisionClass ui;
	long userID;
	long previewID;
	
};
