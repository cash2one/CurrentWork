#include "liveonline.h"
#include "LkImageProcess.h"
#include "LkFileSource.h"
#include "LkEncoderThread.h"
#include "LkImageProcessThread.h"
#include "LkRtmpSendThread.h"
#include "LkUtil.h"
#include "LkWindowsCaptureSource.h"
#include "LkPictureSource.h"
#include "LkTextSource.h"
#include "LkCameraSource.h"
#include "LkSettingDlg.h"
#include "LkAudioCapture.h"
#include "LkErrno.h"
#include "LkAVQueue.h"
#include "LkDefine.h"
#include "LkCameraSelector.h"
#include "LkDesktopAreaSelector.h"
#include "LkTextSelectDlg.h"
#include <QSettings>
#include <QPushButton>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QApplication>
#include <QtMultimedia/QCameraInfo>
#include <QMessageBox>
#include <QVariant>
#include "LkButton.h"
#include "LkSendTime.h"
#include "QTopWidget.h"

#ifdef Q_OS_WIN
#include <Dwmapi.h>
#pragma comment(lib,"Dwmapi.lib")  

#endif



LiveOnLine::LiveOnLine(QWidget *parent)
	: QWidget(parent),m_bSnedToRtmp(false)
{
	Init();
}

LiveOnLine::~LiveOnLine()
{
	if (m_bSnedToRtmp)
	{
		OnActionStartorEndRtmp();
		SAFE_STOP_THREAD(m_pCurrentSendTime);
	}
}

void LiveOnLine::Init()
{	
	this->setWindowFlags(Qt::FramelessWindowHint);
	//��ʼ��������
	int nMainWndHeight = QApplication::desktop()->height()*0.8;
	int nMainWndWidth = nMainWndHeight*1.4;
	this->resize(nMainWndWidth,nMainWndHeight);

	//��ȡ�����ļ�
	LkConfigOpt::Instance()->ReadXmlConfig();

	if (LkConfigOpt::Instance()->str_Lk_Version.compare(LK_VERSION_STR) != 0 )
	{
		QMessageBox::critical(this, QStringLiteral("����"), QStringLiteral("�����ļ��汾�����汾��ƥ�䣬�⽫������Щ���ܿ����޷�����ʹ�ã�"));
	}

	//���
	int nLeftWidth = nMainWndWidth/7;
	int nLeftHeight = nMainWndHeight/12;
	QWidget* pLeftWidget = new QWidget(this);
	pLeftWidget->setGeometry(0,0,nLeftWidth,nMainWndHeight);
	pLeftWidget->setStyleSheet(GetStyle(g_LeftColor));
	
	int nLeftStartX = 0;
	int nLeftStartY = 0;

	//���⣬�汾����
	QLabel* pLabelLogo = new QLabel(pLeftWidget);	
	pLabelLogo->setAlignment(Qt::AlignCenter);
	pLabelLogo->setStyleSheet("font-size:18px;color:#ffffff;font-weight:bold");
	pLabelLogo->setText(g_LogoText);
	pLabelLogo->setGeometry(nLeftStartX,nLeftStartY,nLeftWidth,nLeftHeight);


	nLeftStartY+= nLeftHeight;
	nLeftStartY+=10;

	QList<LkButton*> pFunction;
	LkButton* pText = new LkButton(QPixmap("image/wenzi.png"),pLeftWidget);
	connect(pText,SIGNAL(releaseWidget()),this,SLOT(OnActionAddText()));
	pText->setToolTip(QStringLiteral("�������Դ"));
	pFunction.append(pText);


	LkButton* pTextImage = new LkButton(QPixmap("image/image.png"),pLeftWidget);
	connect(pTextImage,SIGNAL(releaseWidget()),this,SLOT(OnActionAddImage()));
	pTextImage->setToolTip(QStringLiteral("��ӱ���ͼƬԴ"));
	pFunction.append(pTextImage);

	LkButton* pTextVideo = new LkButton(QPixmap("image/video.png"),pLeftWidget);
	connect(pTextVideo,SIGNAL(releaseWidget()),this,SLOT(OnActionAddVideoFile()));
	pTextVideo->setToolTip(QStringLiteral("��ӱ�����ƵԴ"));
	pFunction.append(pTextVideo);

	LkButton* pTextJp = new LkButton(QPixmap("image/jieping.png"),pLeftWidget);
	connect(pTextJp,SIGNAL(releaseWidget()),this,SLOT(OnActionAddDesktop()));
	pTextJp->setToolTip(QStringLiteral("��������������Դ"));
	pFunction.append(pTextJp);

	LkButton* pTextCamera = new LkButton(QPixmap("image/camera.png"),pLeftWidget);
	connect(pTextCamera,SIGNAL(releaseWidget()),this,SLOT(OnActionAddCamera()));
	pTextCamera->setToolTip(QStringLiteral("�������ͷ����Դ"));
	pFunction.append(pTextCamera);

	m_pPushToRtmpBtn = new LkButton(QPixmap("image/start.png"),pLeftWidget);
	connect(m_pPushToRtmpBtn,SIGNAL(releaseWidget()),this,SLOT(OnActionStartorEndRtmp()));
	m_pPushToRtmpBtn->setToolTip(QStringLiteral("����/�ر�����"));
	pFunction.append(m_pPushToRtmpBtn);

	m_pBtnSetting = new LkButton(QPixmap("image/settings.png"),pLeftWidget);
	connect(m_pBtnSetting,SIGNAL(releaseWidget()),this,SLOT(OnActionSetting()));
	m_pBtnSetting->setToolTip(QStringLiteral("����"));
	pFunction.append(m_pBtnSetting);

	//����λ��
	for (int i=0;i<pFunction.size();++i)
	{
		if (pFunction.at(i) != NULL)
		{
			pFunction.at(i)->setGeometry(nLeftStartX,nLeftStartY,nLeftWidth,(nMainWndHeight*2/3)/pFunction.size());
			nLeftStartY+=((nMainWndHeight*2/3)/pFunction.size()+5);
		}
	}
	pFunction.clear();

	//��ʾ��ǰʱ��
	m_pLabelMsg = new QLabel(pLeftWidget);
	m_pLabelMsg->setStyleSheet("font-size:16px;color:#ffffff;font-weight:bold");
	m_pLabelMsg->setGeometry(0,pLeftWidget->height()-30,pLeftWidget->width(),25);
	m_pLabelMsg->setAlignment(Qt::AlignHCenter);



	//�ϱ�
	int nTopStartX = nLeftStartX+nLeftWidth;
	int nTopStartY = 0;
	int nTopWidth = nMainWndWidth-nLeftWidth;
	int nTopHeight = nLeftHeight*2/3;
	m_pTopWidget = new QTopWidget(this);
	m_pTopWidget->setGeometry(nTopStartX,nTopStartY,nTopWidth,nTopHeight);
	//m_pTopWidget->setStyleSheet(GetStyle(g_TopColor));
	m_pTopWidget->setAutoFillBackground(true);
	QPalette palette;
	palette.setColor(QPalette::Background, g_TopColor);
	m_pTopWidget->setPalette(palette);
	connect(m_pTopWidget,SIGNAL(MovePostion(const QPoint&)),this,SLOT(MouseMovePos(const QPoint&)));

	int nMcWidth = 20;
	int nMcHeight = 20;
	LkButton* pTextMin = new LkButton(QPixmap("image/min.png"),g_TopColor,m_pTopWidget);
	pTextMin->setGeometry(m_pTopWidget->width()-(2*nMcWidth+20),(nTopHeight-nMcHeight)/2,nMcWidth,nMcHeight);
	connect(pTextMin,SIGNAL(pressWidget()),this,SLOT(showMinimized()));
	pTextMin->SetPressColor(g_TopColor);

	LkButton* pTextClose = new LkButton(QPixmap("image/close.png"),g_TopColor,m_pTopWidget);
	pTextClose->setGeometry(m_pTopWidget->width()-(nMcWidth+10),(nTopHeight-nMcHeight)/2,nMcWidth,nMcHeight);
	connect(pTextClose,SIGNAL(pressWidget()),this,SLOT(close()));
	pTextClose->SetPressColor(g_TopColor);

	//�м�
	int nMinStartX = nLeftStartX+nLeftWidth+5;
	int nMinStartY = nTopHeight+5;
	int nMinWidth = nMainWndWidth-nLeftWidth-10;
	int nMinHeight = nMainWndHeight-nTopHeight-50;
	m_pImageProcessWidget = new LkImageProcessWidget(this);
	m_pImageProcessWidget->setGeometry(nMinStartX,nMinStartY,nMinWidth,nMinHeight);
	m_pImageProcessWidget->setStyleSheet(GetStyle(g_MidleColor));

	//�±�
	int nBottomHeight = 40;
	int nBottomStartX = nTopStartX;
	int nBottomStartY = nMainWndHeight-nBottomHeight;
	int nBottomWidth = nTopWidth;
	QWidget* pBottomWidget = new QWidget(this);
	pBottomWidget->setGeometry(nBottomStartX,nBottomStartY,nBottomWidth,nBottomHeight);
	pBottomWidget->setStyleSheet(GetStyle(g_BottomColor));

	QLabel* pLabelMsg = new QLabel(pBottomWidget);
	pLabelMsg->setStyleSheet("font-size:14px;color:#ffffff;font-weight:bold");
	pLabelMsg->setGeometry(10,0,pBottomWidget->width(),pBottomWidget->height());
	pLabelMsg->setAlignment(Qt::AlignVCenter);
	pLabelMsg->setText(g_Version);



	//��ʼ������
	m_pImageThread = NULL;
	m_pEncoderThread = NULL;
	m_pRtmpThread = NULL;
	m_pAudioCaptureThread = NULL;

	//��ʾ����ʱ��
	//m_pCurrentSendTime = new LkSendTime(this);
	//connect(m_pCurrentSendTime,SIGNAL(SengCurrentTime(const QString&)),this,SLOT(OnShowCurrentTime(const QString&)));
	//START_THREAD(m_pCurrentSendTime);


	ShowTime* pShowTime = new ShowTime();
	connect(pShowTime,SIGNAL(currentTime(const QString&)),this,SLOT(OnShowCurrentTime(const QString&)));
	START_THREAD(pShowTime);

}

//��Ӧ�������
void LiveOnLine::OnActionAddText()
{
	LkTextSelectDlg* pTextDlg = new LkTextSelectDlg(this);
	if (pTextDlg->exec() == QDialog::Accepted)
	{
		LkTextSource *pSourceText = new LkTextSource();
		pSourceText->setText(pTextDlg->GetInputString());
		m_pImageProcessWidget->addCaptureSource(pSourceText, 0, 0, 320, 240);
	}
}
//��Ӧ���ͼƬ
void LiveOnLine::OnActionAddImage()
{
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("��ѡ��һ��ͼƬ�ļ�"),
		"",tr("Images (*.png *.bmp *.jpg)"));

	if (fileName.isEmpty()) return;

	LkPictureSource *pPicSource = new LkPictureSource(fileName);
	m_pImageProcessWidget->addCaptureSource(pPicSource, 20, 20, 320, 240);
}
//��Ӧ������
void LiveOnLine::OnActionAddCamera()
{
	LkCameraSelectorDlg* pCameraSelect  = new LkCameraSelectorDlg(this);
	if (pCameraSelect->exec() == QDialog::Accepted)
	{
		LkCameraSource *source = new LkCameraSource();
		source->setCameraInfo(0, pCameraSelect->GetSelect());
		int nFps = LkConfigOpt::Instance()->n_Lk_Encoder_Fps;
		source->setCaptureInterval(1000 / nFps / 1.5);
		source->start();
		m_pImageProcessWidget->addCaptureSource(source, 40, 40, 640, 480);
	}
}
//��Ӧ�������
void LiveOnLine::OnActionAddDesktop()
{
	//��ȡ���ڴ�С
	if (LkConfigOpt::Instance()->b_Lk_DesktopMin) //������С��
	{
		this->showMinimized();
	}

	LkDesktopAreaSelector* pSelectArae = new LkDesktopAreaSelector();
	if (pSelectArae->exec() == QDialog::Accepted)
	{
		LkWindowsCaptureSource* pWndSource = new LkWindowsCaptureSource();
		pWndSource->setGrabInfo(QApplication::desktop()->winId(),pSelectArae->GetRect().x(), pSelectArae->GetRect().y(),pSelectArae->GetRect().width(),pSelectArae->GetRect().height());
		int nFps = LkConfigOpt::Instance()->n_Lk_Encoder_Fps;
		if (nFps > 0)
		{
			pWndSource->setCaptureInterval(1000 / nFps);
		}
		pWndSource->start();
		m_pImageProcessWidget->addCaptureSource(pWndSource, 30, 30, pSelectArae->GetRect().width(),pSelectArae->GetRect().height());
	}
	this->showNormal();
	
}
//��Ӧ�����Ƶ�ļ�
void LiveOnLine::OnActionAddVideoFile()
{
	QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("��ѡ��һ����Ƶ�ļ�"),
		"",tr("File (*.flv *.rmvb *.mp4 *.mkv *.avi *.wmv)"));

	if (fileName.isEmpty()) return;

	LkFileSource *pFileSource = new LkFileSource();
	pFileSource->setFileName(fileName);
	pFileSource->start();

	m_pImageProcessWidget->addCaptureSource(pFileSource, 60, 60, 320, 240);

}

//��Ӧ��ʼ����
void LiveOnLine::OnActionStartorEndRtmp()
{
	if (!m_bSnedToRtmp)
	{
		m_pImageThread = new LkImageProcessThread(this);
		m_pImageProcessWidget->setProcessThread(m_pImageThread);

		QSize size = LkConfigOpt::Instance()->size_Lk_Encoder_Scale;
		int nFps = LkConfigOpt::Instance()->n_Lk_Encoder_Fps;
		m_pImageThread->setResolution(size.width(), size.height());
		m_pImageThread->setInternal(1000 / nFps);

		//���������ɼ��߳�
		m_pAudioCaptureThread = new LkAudioCapture();


		//���������߳�
		m_pEncoderThread = new LkEncoderThread(this);
		m_pEncoderThread->setProcessThread(m_pImageThread);
		m_pEncoderThread->init();

		m_pRtmpThread = new LkRtmpSendThread(this);

		
		QString strAudioChannel = "Stereo";
		int nAudioSampleRate = 44100;
		int nAudioChannels = -1;
		if (strAudioChannel == "Mono") {
			nAudioChannels  = 1;
		} else if (strAudioChannel == "Stereo") {
			nAudioChannels  = 2;
		}
		LkAssert(nAudioChannels != -1);

		int nAudioBitrate = 96;
		int nDevID = 0; 
		int ret = m_pAudioCaptureThread->startCapture(nAudioBitrate, nAudioSampleRate, nAudioChannels, nDevID);
		if (ret != LK_SUCESS) {
			QMessageBox::critical(this, QStringLiteral("����"), QStringLiteral("�����ɼ�ʧ�ܣ�������Ƶ�ɼ��豸��"));
		}

		if (!m_pRtmpThread) {
			m_pRtmpThread = new LkRtmpSendThread(this);
			START_THREAD(m_pRtmpThread);

			/*connect(m_pRtmpThread, SIGNAL(status(int,int,int,int))
			, this, SLOT(onStatus(int,int,int,int)));*/

			START_THREAD(m_pEncoderThread);
			START_THREAD(m_pImageThread);
		}
		START_THREAD(m_pRtmpThread);
		START_THREAD(m_pEncoderThread);
		START_THREAD(m_pImageThread);
		START_THREAD(m_pAudioCaptureThread);
		
		m_pPushToRtmpBtn->setPixmap(QPixmap("image/end.png"));
		QDateTime time = QDateTime::currentDateTime();
		m_strSentStartTime = time.toString("\thh:mm:ss");
		
	}
	else
	{
		SAFE_STOP_THREAD(m_pRtmpThread);
		SAFE_STOP_THREAD(m_pEncoderThread);
		SAFE_STOP_THREAD(m_pImageThread);
		SAFE_STOP_THREAD(m_pAudioCaptureThread);

		if (m_pAudioCaptureThread) {
			m_pAudioCaptureThread->stopCapture();
		}

		if (m_pEncoderThread) {
			m_pEncoderThread->fini();
		}

		LkFree(m_pRtmpThread);
		LkFree(m_pEncoderThread);
		LkFree(m_pImageThread);
		LkFree(m_pAudioCaptureThread);

		m_pImageProcessWidget->setProcessThread(m_pImageThread);

		LkAVQueue::destroy();
		m_pPushToRtmpBtn->setPixmap(QPixmap("image/start.png"));
		m_strSentStartTime = "";
	}

	m_bSnedToRtmp = !m_bSnedToRtmp;
	Update(m_bSnedToRtmp);

}
//����
void LiveOnLine::OnActionSetting()
{
	LkSettingDlg* pSetting = new LkSettingDlg(this);
	connect(pSetting, SIGNAL(settingChanged())
		, m_pImageProcessWidget, SLOT(onSettingChanged()));
	if(pSetting->exec() == QDialog::Accepted)
	{
		if (m_bSnedToRtmp)
		{
			OnActionStartorEndRtmp();
			OnActionStartorEndRtmp();
		}		
		
	}
}

void LiveOnLine::OnShowCurrentTime(const QString& str)
{
	//QString strV = g_Version;
	//strV += QStringLiteral("\t      ��ǰʱ�䣺");
	//strV += str;
	//if (m_bSnedToRtmp)
	//{
	//	strV += QStringLiteral("\t��ʼ����ʱ��: ");
	//	strV+= m_strSentStartTime;
	//}
	m_pLabelMsg->setText(str);
}

//���½���
void LiveOnLine::Update(bool b)
{
	if (b) //��ʼ����
	{
		//m_pBtnSetting->setDisabled(b); //���ð�ť״̬
		if (LkConfigOpt::Instance()->b_Lk_Min) //������С��
		{
			this->showMinimized();
		}

		if(LkConfigOpt::Instance()->b_Lk_Aero) //�ر�Area
		{
			HRESULT result = DwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
			//HRESULT result = ::DwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
		}

	}
	else
	{
		//m_pBtnSetting->setDisabled(b); //���ð�ť״̬
		this->showNormal();
		HRESULT result = ::DwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
	}
	
	
}

void LiveOnLine::MouseMovePos(const QPoint& pt)
{
	QRect rect = QRect(this->geometry().left()+pt.x(),this->geometry().top()+pt.y(),this->geometry().width(),this->geometry().height());
	this->setGeometry(rect);
}