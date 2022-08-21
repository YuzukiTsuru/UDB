//--------------------------------------------------------------
// VOL
//--------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <windows.h>

#include "vslib.h"
#include "udb.h"


//--------------------------------------------------------------
// ��ͨ�˲���
//
// ����
//  UDBOPTION *option	
//
// ����
//  0:�ɹ�, 1:ʧ��
//
// ��Ҫ
//  ���й��˴���������������Wave�ļ���
//  (tmpWaveData��outputWaveFile)
//--------------------------------------------------------------
int UDBStep2(UDBOPTION* option)
{
	// ��ͨ�˲���
	UDBLowPassFilter(option);

	// ��������
	UDBVolume(option);

	// Wave�ļ����
	int retVal = UDBOutputWaveFile(option);

	// ��ʱ���ݽ��
	free(option->tmpWaveData);
	option->tmpWaveData = NULL;

	return retVal;
}


//--------------------------------------------------------------
// ��ͨ�˲�������
//
// ����
//  UDBOPTION *option	
//
// ����
//  0:�ɹ�
//
// ��Ҫ
//  �õ�ͨ�˲�������tmpWaveData
//  (tmpWaveData��tmpWaveData)
//--------------------------------------------------------------
int UDBLowPassFilter(UDBOPTION* option)
{
	short* waveData = option->tmpWaveData;
	int nSample = option->tmpSample;
	int sampFreq = option->tmpSmpFreq;

	// Ƶ�ʼ���
	// H��־��Ƶ�ʵĶ�Ӧ��
	// H : 100  50  20  10
	// f :  2k  4k 10k 20k
	double freq = 16000.0;
	if (option->lowPassfilter > 0) { freq = 200000.0 / option->lowPassfilter; }
	if (freq > 16000.0) { freq = 16000.0; }
	if (freq > sampFreq * 0.40) { freq = sampFreq * 0.40; }

	// ������ϵ������
	double q = 1.0 / sqrt(2.0);
	double c1 = tan(M_PI * freq / sampFreq);
	double c2 = c1 * c1;

	double b0 = c2 / (1.0 + c1 / q + c2);
	double b1 = c2 / (1.0 + c1 / q + c2) * 2.0;
	double b2 = c2 / (1.0 + c1 / q + c2);
	double a1 = 2.0 * (c2 - 1.0) / (1.0 + c1 / q + c2);
	double a2 = (1.0 - c1 / q + c2) / (1.0 + c1 / q + c2);

	double x0 = 0.0, x1 = 0.0, x2 = 0.0;
	double y0 = 0.0, y1 = 0.0, y2 = 0.0;

	// ����
	for (int i = 0; i < nSample; i++)
	{
		x2 = x1;
		x1 = x0;
		y2 = y1;
		y1 = y0;
		x0 = waveData[i];
		y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

		if (y0 >= 0)
		{
			int v = (int)(y0 + 0.5);
			if (v > 32767) { v = 32767; }
			waveData[i] = (short)v;
		}
		else
		{
			int v = (int)(y0 - 0.5);
			if (v < -32768) { v = -32768; }
			waveData[i] = (short)v;
		}
	}

	return 0;
}


//--------------------------------------------------------------
// �������
//
// ����
//  UDBOPTION *option	
//
// ����
//  0:�ɹ�
//
// ��Ҫ
//  �ı�tmpWaveDita��������
//  (tmpWaveData��tmpWaveData)
//--------------------------------------------------------------
int UDBVolume(UDBOPTION* option)
{
	short* waveData = option->tmpWaveData;
	int nSample = option->tmpSample;

	// �����������
	int dynMax = 0;
	for (int i = 0; i < nSample; i++)
	{
		int dyn = abs(waveData[i]);
		if (dyn > dynMax) { dynMax = dyn; }
	}

	// ��������
	if (dynMax != 0)
	{
		double volume = option->volume / 100.0;
		double peekCmp = option->peekCmp / 100.0;
		double v1 = volume;
		double v2 = volume * 16384 / dynMax;
		double v3 = v2 * peekCmp + v1 * (1.0 - peekCmp);

		for (int i = 0; i < nSample; i++)
		{
			double w = waveData[i] * v3;
			if (w >= 0)
			{
				int v = (int)(w + 0.5);
				if (v > 32767) { v = 32767; }
				waveData[i] = (short)v;
			}
			else
			{
				int v = (int)(w - 0.5);
				if (v < -32768) { v = -32768; }
				waveData[i] = (short)v;
			}
		}
	}

	return 0;
}


//--------------------------------------------------------------
// Wave�ļ����
//
// ����
//  UDBOPTION *option	
//
// ����
//  0:�ɹ�, 1:ʧ��
//
// ��Ҫ
//  ��tmpWaveDita�����Wave�ļ���
//  (tmpWaveData��outputWaveFile)
//--------------------------------------------------------------
int UDBOutputWaveFile(UDBOPTION* option)
{
	int retVal = 0;

	short* waveData = option->tmpWaveData;
	int nSample = option->tmpSample;
	int sampFreq = option->tmpSmpFreq;
	int bit = 16;
	int channel = 1;
	int datasize = nSample * (bit / 8) * channel;

	FILE* fpOut;
	fopen_s(&fpOut, option->outputWaveFile, "wb");

	if (fpOut != NULL)
	{
		//����ͷ��
		char header[44];
		*(int*)(header + 0) = 0x46464952; // "RIFF"
		*(int*)(header + 4) = 36 + datasize; // �ļ���С- 8
		*(int*)(header + 8) = 0x45564157; // "WAVE"
		*(int*)(header + 12) = 0x20746d66; // "fmt "
		*(int*)(header + 16) = 16;
		*(short*)(header + 20) = (short)WAVE_FORMAT_PCM;
		*(short*)(header + 22) = (short)channel;
		*(int*)(header + 24) = sampFreq;
		*(int*)(header + 28) = sampFreq * channel * (bit / 8);
		*(short*)(header + 32) = (short)(channel * (bit / 8));
		*(short*)(header + 34) = (short)bit;
		*(int*)(header + 36) = 0x61746164; // "data"
		*(int*)(header + 40) = datasize; // datasize

		// ��ͷд��
		fseek(fpOut, 0, SEEK_SET);
		fwrite(header, sizeof(char), 44, fpOut);

		// WAVE����д��
		fseek(fpOut, 44, SEEK_SET);
		fwrite(waveData, sizeof(short), nSample, fpOut);

		// �ر�����ļ�
		fclose(fpOut);
	}
	else
	{
		SetColor(12);
		printf("UDB:���������������ļ����������QAQ\n");
		SetColor(15);
		system("pause");
		retVal = 1;
	}
	return retVal;
}
