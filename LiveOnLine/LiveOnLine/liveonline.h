#ifndef LIVEONLINE_H
#define LIVEONLINE_H

#include <QtWidgets/QWidget>
#include <QThread>
#include <QLabel>
#include <QDateTime>
class LkImageProcessWidget;
class LkImageProcessThread;
class LkEncoderThread;
class LkRtmpSendThread;
class LkAudioCapture;
class LkButton;
class QPushButton;
class LkSendTime;
class QLabel;
class QTopWidget;
class ShowTime;

class LiveOnLine : public QWidget
{
	Q_OBJECT

public:
	LiveOnLine(QWidget *parent = 0);
	~LiveOnLine();

	friend class ShowTime;
	public slots:
		void OnActionAddText();			//��Ӧ�������
		void OnActionAddImage();		//��Ӧ���ͼƬ
		void OnActionAddCamera();		//��Ӧ������
		void OnActionAddDesktop();		//��Ӧ�������
		void OnActionAddVideoFile();	//��Ӧ�����Ƶ�ļ�
		void OnActionStartorEndRtmp();	//��Ӧ��ʼ����
		void OnActionSetting();			//����

		void MouseMovePos(const QPoint& pt);


private:
	void Init(); //��ʼ����

	void Update(bool b);	//���½���

	//void ShowCurrentTime(const QString& str){m_pLabelMsg->setText(str);};


private:
	LkImageProcessWidget* m_pImageProcessWidget;	//��ʾ�Ի���
	LkImageProcessThread* m_pImageThread;	//�����߳�
	LkEncoderThread* m_pEncoderThread;		//�����߳�
	LkRtmpSendThread* m_pRtmpThread;		//rtmp�߳�
	LkAudioCapture *m_pAudioCaptureThread;	//��Ƶ�߳�

	bool m_bSnedToRtmp;						//���͡�ֹͣ����
	LkButton* m_pPushToRtmpBtn;				//���ͣ�ֹͣ��ť	
	LkButton* m_pBtnSetting;				//���ð�ť
	//QLabel* m_pLabelMsg;					//��ʾ��Ϣ

	LkSendTime* m_pCurrentSendTime;			//��ȡ����ʱ��
	QString m_strSentStartTime;				//��ʼ����ʱ��

	QTopWidget* m_pTopWidget;				//�ϲ�������
	QWidget* m_pBottomWidget;				//�ϲ�������

};

//��ʾʱ��
class ShowTime : public QThread
{
	Q_OBJECT
public:
	ShowTime(){};
	~ShowTime(){};
signals:
	void currentTime(const QString& str);
public:
	void run()
	{
		while (true)
		{
			QDateTime time = QDateTime::currentDateTime();
			QString current_date = time.toString("\thh:mm:ss");
			emit currentTime(current_date);
		}
	};
};

#endif // LIVEONLINE_H
