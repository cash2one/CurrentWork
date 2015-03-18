#ifndef _LK_CONFIG_H_
#define  _LK_CONFIG_H_

#include "LkDefine.h"
class LkConfigOpt
{
public:
	LkConfigOpt(void);
	~LkConfigOpt(void);

	static LkConfigOpt* Instance();
	void ReadConfig();
	void WriteConfig();

	void ReadXmlConfig();
	void WriteXmlConfig();
	int GetWidth(const QString& str );
	int GetHeight(const QString& str );

public:
	static LkConfigOpt* m_pConfigOpt; //ʵ��

	//video
	//audio
	int n_Lk_Audio_DeviceId;
	QString str_Lk_Audio_Format;
	QString str_Lk_Audio_Channels;
	int n_Lk_Audio_SampleRate;
	int n_Lk_Audio_Bitrate;

	//x264
	QString str_Lk_x264_Preset;
	QString str_Lk_x264_Tune;
	QString str_Lk_x264_Profile;
	QString str_Lk_x264_BitrateMode;
	int n_Lk_x264_KeyFrameInterval;
	int n_Lk_x264_Quality;
	int n_Lk_x264_ThreadCount;
	bool b_Lk_x264_EnableBFrame;

	//encoder
	QString str_Lk_Encoder_Format;
	QSize size_Lk_Encoder_Scale;
	int n_Lk_Encoder_Fps;
	int n_Lk_Encoder_Bitrate;

	//network
	QString str_Lk_NetWork_URL;

	//other
	bool b_Lk_Min;			//��ʼ����ʱ�Ƿ���С��
	bool b_Lk_Mouse;		//�Ƿ��������
	bool b_Lk_Aero;			//�Ƿ�ر�Aero��Ч
	bool b_Lk_DesktopMin;	//�������Դʱ�Զ���С��

	//save path
	QString str_Lk_SavePath;

	//�Ƿ񱣴�
	bool b_Lk_SaveFile;

	//version
	QString str_Lk_Version;
};
#endif
