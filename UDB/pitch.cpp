//--------------------------------------------------------------
// Pit�任
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
// Pit�任
//
// ����
//  UDBOPTION *option	
//
// ����
//  0:�ɹ� 1:ʧ��
//
// ��Ҫ
//  (inputWaveFile��tmpWaveData)
//--------------------------------------------------------------
int UDBStep1(UDBOPTION* option)
{
	VSPRJINFO vsPrjInfo;
	VSITEMINFO vsItemInfo;

	// ������Ŀ
	HVSPRJ hVsprj = UDBLoadProject(option);
	if (hVsprj == NULL) { return 1; }

	// ʱ����չ�趨
	UDBSetTimeStretchPrm(hVsprj, option);

	// �������
	UDBSetPitPrm(hVsprj, option);

	// �����ϳɷ�ʽ����
	if (option->disablePitchShift == -2)
	{
		// �趨Ϊ����ģʽ
		VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
		vsItemInfo.synthMode = SYNTHMODE_P;
		VslibSetItemInfo(hVsprj, 0, &vsItemInfo);
	}
	else
	{
		if (option->noFormantFlg != 0)
		{
			// ���û�е���pit�������óɵ���ģʽ
			VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
			vsItemInfo.synthMode = SYNTHMODE_M;
			VslibSetItemInfo(hVsprj, 0, &vsItemInfo);
		}
		else
		{
			// �����pitУ���Ļ������趨Ϊ����pitУ��ģʽ
			VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
			vsItemInfo.synthMode = SYNTHMODE_MF;
			VslibSetItemInfo(hVsprj, 0, &vsItemInfo);
		}
	}

	// EQ����
	int eq1[15] = {0, 0, 0, 0, 0, 0, 0, 20, 60, 100, 120, 120, 80, 0, 0};
	int eq2[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40, 120, 120};
	VslibSetEQGain(hVsprj, 0, 0, eq1);
	VslibSetEQGain(hVsprj, 0, 1, eq2);

	// ȡ�ÿ��Ʒ���
	VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
	int nCtrlPnt = vsItemInfo.ctrlPntNum;

	// ���Ƶ��������
	for (int i = 0; i < nCtrlPnt; i++)
	{
		// ȡ�ÿ��Ʒ���
		VSCPINFOEX vsCpInfo;
		VslibGetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);

		// ��������
		vsCpInfo.formant = -20 * option->genderFactor;

		// ��������
		vsCpInfo.breathiness = (option->breathiness - 50) * 200;

		// EQ��������
		vsCpInfo.eq1 = option->brightness * 100;

		// EQ2������
		vsCpInfo.eq2 = option->clearness * 100;

		// ���Ƶ��������
		VslibSetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
	}

	// ����Ƶ�ʡ�������ȡ��
	VslibGetProjectInfo(hVsprj, &vsPrjInfo);
	option->tmpSmpFreq = vsPrjInfo.sampFreq;
	VslibGetMixSample(hVsprj, &option->tmpSample);

	// ��ȡWAVE����
	option->tmpWaveData = (short*)calloc(option->tmpSample, sizeof(short));
	VslibGetMixData(hVsprj, option->tmpWaveData, 16, 1, 0, option->tmpSample);

	// ��Ŀ�ļ����
	if (option->outputVshpFlg == 12345) { VslibSaveProject(hVsprj, option->gFile); }

	// ɾ����Ŀ
	VslibDeleteProject(hVsprj);

	return 0;
}


//--------------------------------------------------------------
// ������Ŀ
//
// ����
//  UDBOPTION *option	ѡ��
//
// ����
//  ��Ŀָ��
//
// ��Ҫ
//  ��ȡ���Ե�Ƶ�ʱ���������Ŀ����
//  ���û�е�����Ƶ�ʱ�������������
//  ���ʧ���˷���NULL��
//--------------------------------------------------------------
HVSPRJ UDBLoadProject(UDBOPTION* option)
{
	int sta;

	// WAVE�ļ����ڼ��
	if (CheckFileExist(option->inputWaveFile) == 0)
	{
		SetColor(12);
		printf("UDB:�ۣ���û������Դ���ҵ������������û�취��������QAQ,ȱ�ٵķ�����:%s\n", option->inputWaveFile);
		SetColor(15);
		if (option->ShowOption == 1) { system("pause"); }
		return NULL;
	}

	// ��׼Ƶ�ʱ���ڼ��
	int existFrqFlg = CheckFileExist(option->frqFile);

	// ����Ƶ�ʱ���ڼ��
	int existVsfrqFlg = CheckFileExist(option->GfrqFile);

	// ������Ŀ
	HVSPRJ hVsprj = NULL;

	if (option->reGenerateFrqFlg != 0) { existVsfrqFlg = 0; }
	else
	{
		if (existVsfrqFlg != 0)
		{
			// ��ȡ�Լ���Ƶ�ʱ�
			sta = VslibOpenProject(&hVsprj, option->GfrqFile);
			if (sta != VSERR_NOERR)
			{
				SetColor(12);
				printf("UDB:���ڶ�ȡ���Լ���Gfrq��ʱ���ֺ���Ƶ�ʱ�����ˣ�Master����°ɣ�����ɾ��Ƶ�ʱ�����������һ��OwO\n");
				SetColor(15);
				if (option->ShowOption == 1) { system("pause"); }
				existVsfrqFlg = 0;
			}
		}
	}

	if (existVsfrqFlg == 0)
	{
		// �������Ե�Ƶ�ʱ�Gfrq��
		SetColor(11);
		printf("UDB:����UDB�Լ��õ�һ��СƵ�ʱ�OwO\n");
		SetColor(15);

		// ������Ŀ
		sta = VslibCreateProject(&hVsprj);
		if (sta != VSERR_NOERR)
		{
			SetColor(12);
			printf("UDB:�ۣ���Ȼ��ʼ��������Ҳ��֪��ʲôԭ�򣬺�����\n");
			SetColor(15);
			if (option->ShowOption == 1) { system("pause"); }
			return NULL;
		}

		// �����ļ�����Ŀ��
		int itemNum;
		sta = VslibAddItemEx(hVsprj, option->inputWaveFile, &itemNum, 36, 48, 0);
		if (sta != VSERR_NOERR)
		{
			SetColor(12);
			printf("UDB:�ļ����������QAQ\n");
			SetColor(15);
			if (option->ShowOption == 1) { system("pause"); }
			return NULL;
		}

		// ʹ�ñ�׼Ƶ�ʱ�������Ƶ��
		if (existFrqFlg != 0) { UDBLoadFrqFile(hVsprj, option); }

		// ����Ƶ�ʱ���
		sta = VslibSaveProject(hVsprj, option->GfrqFile);
		if (sta != VSERR_NOERR)
		{
			SetColor(12);
			printf("UDB:�ۡ�������Ƶ�ʱ�ʧ���ǣ�Master���Ҽ������QAQ(�������ļ���û��д��Ȩ�޵��µģ�\n");
			SetColor(15);
			if (option->ShowOption == 1) { system("pause"); }
		}
		else
		{
			SetColor(11);
			printf("UDB:Ү������Ƶ�ʱ�ɹ���OwO\n");
			SetColor(15);
		}
	}

	return hVsprj;
}


//--------------------------------------------------------------
// ʹ�ñ�׼Ƶ�ʱ�������Ƶ��
//
// ����
//  HVSPRJ hVsprj	
//  UDBOPTION *option	
//
// ����
//  0:�ɹ� 1:ʧ��
//
// ��Ҫ
//  ���ñ�׼Ƶ�ʱ����ü�����ֵ��
//--------------------------------------------------------------
int UDBLoadFrqFile(HVSPRJ hVsprj, UDBOPTION* option)
{
	VSPRJINFO vsPrjInfo;
	VSITEMINFO vsItemInfo;
	int retVal = 0;

	FILE* fpFrq;
	fopen_s(&fpFrq, option->frqFile, "rb");

	if (fpFrq != NULL)
	{
		// ���ݼ��[����]
		int itvSmp = 256;
		fseek(fpFrq, 8, SEEK_SET);
		fread(&itvSmp, sizeof(int), 1, fpFrq);

		// ��Ƶ��
		double keyFreq = 440.0;
		fseek(fpFrq, 12, SEEK_SET);
		fread(&keyFreq, sizeof(double), 1, fpFrq);

		// ������
		int nData = 0;
		fseek(fpFrq, 36, SEEK_SET);
		fread(&nData, sizeof(int), 1, fpFrq);

		// Ƶ������
		FREQDATA* freqData = (FREQDATA*)calloc(nData, sizeof(FREQDATA));
		if (freqData == nullptr)
		{
			SetColor(12);
			printf("��ȡƵ�ʱ������ֹ��QAQ\n");
			SetColor(15);
			if (option->ShowOption == 1) { system("pause"); }
			return -1;
		}
		else
		{
			fseek(fpFrq, 40, SEEK_SET);
			fread(freqData, sizeof(FREQDATA), nData, fpFrq);
		}

		// ȡ�ÿ��Ʒ���
		VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
		int nCtrlPnt = vsItemInfo.ctrlPntNum;

		// ����Ƶ��ȡ��
		VslibGetProjectInfo(hVsprj, &vsPrjInfo);
		int sampFreq = vsPrjInfo.sampFreq;
		double cpSec = 1.0 / vsItemInfo.ctrlPntPs; // ���Ƶ������

		// Ƶ������
		for (int i = 0; i < nCtrlPnt; i++)
		{
			// �ڱ�׼Ƶ�ʱ��ϼ����
			double smp = i * cpSec * sampFreq;
			int pnt = (int)(smp / itvSmp);

			// Ƶ������
			double freq = 0.0;
			if ((pnt >= 0) && (pnt < nData)) { freq = freqData[pnt].freq; }

			if (freq > 55.0)
			{
				// ���
				// �趨�༭ǰ�����޼��ı�־
				VSCPINFOEX vsCpInfo;
				VslibGetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
				vsCpInfo.pitOrg = (int)(log(freq / 440.0) / log(2.0) * 1200.0) + 6900;
				vsCpInfo.pitFlgEdit = 1;
				VslibSetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
			}
			else
			{
				// �޼��
				// �趨�༭ǰ�����޼��ı�־
				VSCPINFOEX vsCpInfo;
				VslibGetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
				vsCpInfo.pitOrg = (int)(log(keyFreq / 440.0) / log(2.0) * 1200.0) + 6900;
				vsCpInfo.pitFlgEdit = 0;
				VslibSetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
			}
		}

		// Ƶ�������ͷ�
		free(freqData);
		freqData = NULL;

		// �ر��ļ�
		fclose(fpFrq);
	}
	else { retVal = 1; }

	return retVal;
}


//--------------------------------------------------------------
// ʱ����չ�趨
//
// ����
//  HVSPRJ hVsprj	
//  UDBOPTION *option	
//
// ����
//  0:�ɹ�
//
// ��Ҫ
//  ʱ����չ�趨��
//--------------------------------------------------------------
int UDBSetTimeStretchPrm(HVSPRJ hVsprj, UDBOPTION* option)
{
	VSPRJINFO vsPrjInfo;
	VSITEMINFO vsItemInfo;

	// ����Ƶ��ȡ��
	VslibGetProjectInfo(hVsprj, &vsPrjInfo);
	int sampFreq = vsPrjInfo.sampFreq;

	// ��ȡ��Ŀ��Ϣ
	VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
	double orgSmp = vsItemInfo.sampleOrg; // Դ������
	double offsetSmp = option->offset * 0.001 * sampFreq; // ƫ�Ƶ�������
	double fixedSmp = option->fixedLength * 0.001 * sampFreq; // ��������������
	double endBlankSmp = option->endBlank * 0.001 * sampFreq; // blank��������
	double reqSmp = option->reqLength * 0.001 * sampFreq; // Ҫ�󳤶ȵ�������

	// blank�Ը�����ƫ�Ƶ����ֵ��ת��Ϊ�ӽ�β���Ŀհ�
	if (endBlankSmp < 0) { endBlankSmp = orgSmp - offsetSmp + endBlankSmp; }

	// ����������չ����
	double fixStretch = pow(2.0, 1.0 - option->fixSpeed / 100.0);

	// ��Ӷ�ʱ���Ƶ�
	if (reqSmp >= fixedSmp * fixStretch)
	{
		// Ҫ�󳤳���������
		VslibSetTimeCtrlPnt(hVsprj, 0, 1, (int)orgSmp, (int)reqSmp);
		VslibAddTimeCtrlPnt(hVsprj, 0, (int)(orgSmp - endBlankSmp), (int)reqSmp);
		VslibAddTimeCtrlPnt(hVsprj, 0, (int)(offsetSmp + fixedSmp), (int)(fixedSmp * fixStretch));
		VslibAddTimeCtrlPnt(hVsprj, 0, (int)offsetSmp, 0);
	}
	else
	{
		// Ҫ��δ��������
		VslibSetTimeCtrlPnt(hVsprj, 0, 1, (int)orgSmp, (int)reqSmp);
		VslibAddTimeCtrlPnt(hVsprj, 0, (int)(offsetSmp + reqSmp / fixStretch), (int)reqSmp);
		VslibAddTimeCtrlPnt(hVsprj, 0, (int)offsetSmp, 0);
	}

	return 0;
}


//--------------------------------------------------------------
// ĸ�����ֵļ��ƽ��ֵ����
//
// ����
//  HVSPRJ hVsprj	
//  UDBOPTION *option	
//
// ����
//  0:�ɹ�
//
// ��Ҫ
//  ����ĸ�����ֵļ��ƽ��ֵ���㡣
//--------------------------------------------------------------
int UDBCalcAveragePit(HVSPRJ hVsprj, UDBOPTION* option)
{
	VSITEMINFO vsItemInfo;

	// ȡ�ÿ��Ʒ���
	VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
	int nCtrlPnt = vsItemInfo.ctrlPntNum;

	// ������Ƶ������
	double ctrpntSec = 1.0 / vsItemInfo.ctrlPntPs;

	// ����ĸ������Ŀ��Ƶ����
	int startPnt = (int)((option->offset + option->fixedLength) * 0.001 / ctrpntSec);
	int endPnt = (int)(nCtrlPnt - option->endBlank * 0.001 / ctrpntSec);
	startPnt = LimitIntValue(startPnt, 0, nCtrlPnt);
	endPnt = LimitIntValue(endPnt, 0, nCtrlPnt);

	// ĸ���ļ��ƽ��ֵ����
	int pitSum = 0;
	int count = 0;
	for (int i = startPnt; i < endPnt; i++)
	{
		VSCPINFOEX vsCpInfo;
		VslibGetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
		if (vsCpInfo.pitFlgEdit == 1)
		{
			// ֻ�м��Ĳ��ּ���
			pitSum += vsCpInfo.pitOrg;
			count++;
		}
	}
	int pitAverage = 6000;
	if (count != 0) { pitAverage = pitSum / count; }

	// ĸ���ļ��ƽ��ֵ���㣨�ڶ��Σ�
	pitSum = 0;
	count = 0;
	for (int i = startPnt; i < endPnt; i++)
	{
		VSCPINFOEX vsCpInfo;
		VslibGetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
		if (vsCpInfo.pitFlgEdit == 1)
		{
			// ֻ�м��Ĳ��ּ���
			if (abs(vsCpInfo.pitOrg - pitAverage) < 200)
			{
				pitSum += vsCpInfo.pitOrg;
				count++;
			}
		}
	}
	pitAverage = 6000;
	if (count != 0) { pitAverage = pitSum / count; }

	return pitAverage;
}


//--------------------------------------------------------------
// �������
//
// ����
//  HVSPRJ hVsprj	
//  UDBOPTION *option	
//
// ����
//  0:�ɹ�
//
// ��Ҫ
//  �趨��������
//--------------------------------------------------------------
int UDBSetPitPrm(HVSPRJ hVsprj, UDBOPTION* option)
{
	VSPRJINFO vsPrjInfo;
	VSITEMINFO vsItemInfo;

	// ��ȡ����Ƶ��
	VslibGetProjectInfo(hVsprj, &vsPrjInfo);
	int sampFreq = vsPrjInfo.sampFreq;

	// ȡ�ÿ��Ʒ���
	VslibGetItemInfo(hVsprj, 0, &vsItemInfo);
	int nCtrlPnt = vsItemInfo.ctrlPntNum;

	// ������Ƶ������
	double ctrpntSec = 1.0 / vsItemInfo.ctrlPntPs;

	// ������ƽ��ֵ
	int pitAverage = UDBCalcAveragePit(hVsprj, option);

	// ��������ļ��
	int globalShift = 10 * option->pitchShift + option->pitchShift2;

	// ������[�뵥λ]�ļ��
	double pbitvSec = 256.0 / sampFreq;
	if (option->tempo > 0.0)
	{
		pbitvSec = 60.0 / option->tempo / 96; // 4������1/96��
	}

	if ((option->disablePitchShift == -1) || (option->disablePitchShift == -2))
	{
		for (int i = 0; i < nCtrlPnt; i++)
		{
			// �������
			VSCPINFOEX vsCpInfo;
			VslibGetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
			vsCpInfo.pitEdit = vsCpInfo.pitOrg;
			VslibSetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
		}
	}
	else
	{
		// �༭��ļ���������
		for (int i = 0; i < nCtrlPnt; i++)
		{
			// �༭���λ�ã��뵥λ������
			double edtSec;
			VslibGetStretchEditSec(hVsprj, 0, i * ctrpntSec, &edtSec);

			// ������
			int bendPnt = (int)(edtSec / pbitvSec);
			int bend = 0;
			if ((bendPnt >= 0) && (bendPnt < option->nPitchBend)) { bend = option->pitchBend[bendPnt]; }

			// �������
			VSCPINFOEX vsCpInfo;
			VslibGetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
			int modPit = (int)((vsCpInfo.pitOrg - pitAverage) * option->modulation / 100.0);
			vsCpInfo.pitEdit = option->noteNum * 100 + modPit + bend + globalShift;
			VslibSetCtrlPntInfoEx(hVsprj, 0, i, &vsCpInfo);
		}


		// �༭��ļ�������UDB������
		// ���������ȡ��
		int mixSample;
		VslibGetMixSample(hVsprj, &mixSample);

		// �������������
		double mixSec = (double)mixSample / sampFreq;
		int nPitchBend = (int)(mixSec / pbitvSec) + 1;

		for (int i = 0; i < nPitchBend; i++)
		{
			// �����Ӧ�ڼ������λ�õĿ��Ƶ����
			double orgSec;
			double edtSec = LimitDoubleValue(i * pbitvSec, 0.0, mixSec);
			VslibGetStretchOrgSec(hVsprj, 0, edtSec, &orgSec);
			int pnt = LimitIntValue((int)(orgSec / ctrpntSec), 0, nCtrlPnt - 1);

			// ȡ�ñ༭ǰ�ļ��
			VSCPINFOEX vsCpInfo;
			VslibGetCtrlPntInfoEx(hVsprj, 0, pnt, &vsCpInfo);
			int pitOrg = vsCpInfo.pitOrg;

			// ����༭��ļ��
			int modPit = (int)((pitOrg - pitAverage) * option->modulation / 100.0);
			int pitEdt = option->noteNum * 100 + modPit + globalShift + option->pitchBend[i];

			// ���ü����
			option->pitchBend[i] = pitEdt - pitOrg;
		}

		// �趨�������
		VslibSetPitchArray(hVsprj, 0, option->pitchBend, nPitchBend, pbitvSec * sampFreq);
	}

	return 0;
}
