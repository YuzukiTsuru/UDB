//--------------------------------------------------------------
// �����ļ�
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
//ѡ������
//
//����
//int argc����������
//charvargv[]����
//���õ��ղ�ѡ��
//
// ����ֵ
//    0 :�ɹ�1��ʧ��
//
// 
//��ȡ�����в���������ѡ�
//--------------------------------------------------------------
int UDBGetOption(int argc, char* argv[], UDBOPTION* option)
{
	// ѡ���ʼ��
	memset(option, 0, sizeof(UDBOPTION));
	option->peekCmp = 86;
	option->breathiness = 50;

	// 1.�����ļ�
	strcpy_s(option->inputWaveFile, UDB_MAX_PATH, argv[1]);

	// 2.����ļ�
	strcpy_s(option->outputWaveFile, UDB_MAX_PATH, argv[2]);

	// 3.������
	option->noteNum = UDBCnvNoteNum(argv[3]);

	// 4.����ԭ�����ı仯���֣������ٶȣ�
	option->fixSpeed = LimitDoubleValue(atof(argv[4]), 0.0, 400.0);

	// 5.��־
	UDBSetFlgOption(argv[5], option);

	// 6.ƫ��
	option->offset = atof(argv[6]);

	// 7.Ҫ��
	option->reqLength = atof(argv[7]);

	// 8.gFile
	option->fixedLength = atof(argv[8]);

	// 9.�հ�
	option->endBlank = atof(argv[9]);

	// 10.����
	option->volume = LimitDoubleValue(atof(argv[10]), 0.0, 200.0);

	// 11.�԰�
	option->modulation = LimitDoubleValue(atof(argv[11]), -400.0, 400.0);

	// 12��13.���
	if (argc >= 13) { UDBSetPitchBendOption(argv, option); }

	// ����Ƶ�ʱ��ļ�������
	strcpy_s(option->GfrqFile, UDB_MAX_PATH, option->inputWaveFile);
	AddExtStr(option->GfrqFile, UDB_MAX_PATH, (char*)".Gfrq");

	// ��׼Ƶ�ʱ��ļ�������
	strcpy_s(option->frqFile, UDB_MAX_PATH, option->inputWaveFile);
	AddExtStr(option->frqFile, UDB_MAX_PATH, (char*)".frq");

	// ��Ŀ�ļ�������
	strcpy_s(option->gFile, UDB_MAX_PATH, option->outputWaveFile);
	AddExtStr(option->gFile, UDB_MAX_PATH, (char*)".vshp");

	return 0;
}


//--------------------------------------------------------------
// ���������ַ���ת���ɺ���
//
// ����
//  char *nnStr		������������(C0��B9)
//
// ����
//  (C4=60һ������gλ)
//
// ��Ҫ
//  ���������ַ���ת���ɺ��롣
//--------------------------------------------------------------
int UDBCnvNoteNum(char* nnStr)
{
	int noteNum = 0; // ����
	int octStrOffset = 1; // �ַ�INDEX

	// ������������ֳ���
	int length = strlen(nnStr);
	if (length < 2) { return 60; }

	// ��������ת���ɺ���
	switch (nnStr[0])
	{
	case 'C':
		noteNum = 0;
		break;
	case 'D':
		noteNum = 2;
		break;
	case 'E':
		noteNum = 4;
		break;
	case 'F':
		noteNum = 5;
		break;
	case 'G':
		noteNum = 7;
		break;
	case 'A':
		noteNum = 9;
		break;
	case 'B':
		noteNum = 11;
		break;
	default:
		noteNum = 0;
		break;
	}

	// #���ϰ����
	if (nnStr[1] == '#')
	{
		noteNum++;
		octStrOffset = 2;
	}

	// ת����
	if (length > octStrOffset)
	{
		int oct = nnStr[octStrOffset] - '0';
		if (oct < 0) { oct = 0; }
		else if (oct > 10) { oct = 10; }
		noteNum += (oct + 1) * 12;
	}

	return noteNum;
}


//--------------------------------------------------------------
// ��־������ѡ������
//
// ����
//  char *flgStr	��־������
//
// ����ֵ
//  0:�ɹ� 1:ʧ��
//
// ��Ҫ
//  ��־������ѡ�����á�
//--------------------------------------------------------------
int UDBSetFlgOption(char* flgStr, UDBOPTION* option)
{
	int length = strlen(flgStr);

	// �趨
	int offset = 0; // ����λ��
	for (;;)
	{
		//�жϽ���
		if (offset == length) { break; }

		// ��־����ȡ
		char flg = flgStr[offset];
		offset++;

		// �����־ֵ���ַ�������
		int valueLength = 0;
		for (;;)
		{
			// �����յ�
			if (offset + valueLength == length) { break; }
			// ����һ����־����ĸ��
			if (isalpha(flgStr[offset + valueLength])) { break; }
			valueLength++;
		}

		// ����־ֵת��Ϊ����
		int valueData = 0;
		if ((valueLength > 0) && (valueLength < 32))
		{
			char tmpStr[32];
			memcpy_s(tmpStr, 32, flgStr + offset, valueLength);
			tmpStr[valueLength] = 0;
			valueData = atoi(tmpStr);
			offset += valueLength;
		}

		// ��־�趨
		switch (flg)
		{
		case 'N':
			option->noFormantFlg = 1;
			break;
		case 'g':
			option->genderFactor = LimitIntValue(valueData, -100, 100);
			break;
		case 't':
			option->pitchShift = LimitIntValue(valueData, -240, 240);
			break;
		case 'T':
			option->pitchShift2 = LimitIntValue(valueData, -2400, 2400);
			break;
		case 'P':
			option->peekCmp = LimitIntValue(valueData, 0, 100);
			break;
		case 'H':
			option->lowPassfilter = LimitIntValue(valueData, 0, 99);
			break;
		case 'B':
			option->breathiness = LimitIntValue(valueData, 0, 100);
			break;
		case 'O':
			option->brightness = LimitIntValue(valueData, -100, 100);
			break;
		case 'S':
			option->clearness = LimitIntValue(valueData, -100, 100);
			break;
		case 'Z':
			option->outputVshpFlg = valueData;
			break;
		case 'V':
			option->reGenerateFrqFlg = 1;
			break;
		case 'W':
			option->disablePitchShift = valueData;
			break;
		case 'D':
			option->ShowOption = 1;
			break;
		}
	}
	return 0;
}


//--------------------------------------------------------------
// Base64����
//
// ����
//  char *str	�ַ���
//
// ����
//  ����ֵ
//
// ��Ҫ
//  ��2λ����BASE64���н��룬����- 2048��2047��ֵ��
//--------------------------------------------------------------
int UDBDecodeBase64(char* str)
{
	int value = 0;

	for (int i = 0; i < 2; i++)
	{
		int c = str[i];
		int v = 0;

		if ((c >= 'A') && (c <= 'Z')) { v = c - 'A'; }
		else if ((c >= 'a') && (c <= 'z')) { v = c - 'a' + 26; }
		else if ((c >= '0') && (c <= '9')) { v = c - '0' + 52; }
		else if (c == '+') { v = 62; }
		else if (c == '/') { v = 63; }

		value = value * 64 + v;
	}

	if (value >= 2048) { value -= 4096; }

	return value;
}


//--------------------------------------------------------------
// ������ѡ������
//
// ����
//  int		argc		����ֵ
//  char	*argv[]		����
//  UDBOPTION *option	ѡ��
//
// ����
//  0:�ɹ� 1:ʧ��
//
// ��Ҫ
//  ��ȡ�����߲��������ü��ѡ�
//--------------------------------------------------------------
int UDBSetPitchBendOption(char* argv[], UDBOPTION* option)
{
	char* pitchBendStr = argv[12];
	option->tempo = 0.0;

	// �����ȡ
	if (pitchBendStr[0] == '!')
	{
		option->tempo = atof(pitchBendStr + 1);
		pitchBendStr = argv[13];
	}

	// ��ȡ��������
	int length = strlen(pitchBendStr); // ���ֳ���
	int nData = 0; // ������
	int offset = 0; // ��ȡλ��
	int lastData = 0; // �ϴ�����

	for (;;)
	{
		// �����ж�
		if (offset >= length) { break; }

		if (pitchBendStr[offset] == '#')
		{
			// �������ֳ�
			int i = offset + 1;
			int offset2 = i;
			for (;;)
			{
				if (i == length) { break; }
				if (pitchBendStr[i] == '#')
				{
					offset2 = i;
					break;
				}
				i++;
			}
			int repNumLength = offset2 - offset - 1;

			// �������������
			int repNum = 0;
			if (repNumLength < 8)
			{
				char tmp[8];
				memcpy_s(tmp, 8, pitchBendStr + offset + 1, repNumLength);
				tmp[repNumLength] = NULL;
				repNum = atoi(tmp);
				for (int i = 0; i < repNum; i++)
				{
					if (nData + i < UDB_PBSIZE) { option->pitchBend[nData + i] = lastData; }
					else { break; }
				}
			}

			nData += repNum;
			offset = offset2 + 1;
		}
		else
		{
			// ֱ��ָ��
			if (length >= offset + 2)
			{
				lastData = UDBDecodeBase64(pitchBendStr + offset);
				if (nData < UDB_PBSIZE) { option->pitchBend[nData] = lastData; }
				nData++;
				offset += 2;
			}
			else { break; }
		}
	}


	// ���ݱ�ʾ��debug��
	if (option->ShowOption == 1)
	{
		SetColor(9);
		printf("\n\n");
		printf("�����ļ�      ��%s\n", option->inputWaveFile);
		printf("����ļ�      ��%s\n", option->outputWaveFile);
		printf("�������      ��%d\n", option->noteNum);
		printf("offset        ��%f\n", option->offset);
		printf("Ҫ��        ��%f\n", option->reqLength);
		printf("����          ��%f\n", option->fixedLength);
		printf("�հ�          ��%f\n", option->endBlank);
		printf("BPM           ��%f\n", option->fixSpeed);
		printf("����          ��%f\n", option->volume);
		printf("������        ��%f\n", option->modulation);
		printf("\n\n");
		SetColor(15);
	}

	// ����������
	if (nData > UDB_PBSIZE) { nData = UDB_PBSIZE; }
	option->nPitchBend = nData;

	return 0;
}

//--------------------------------------------------------------
// ��ʾBPM
//
// ����
//  UDBOPTION *option	ѡ��
//
// ����
//  0
//
// ��Ҫ
//  ��ȡ�����߲��������BPM
//--------------------------------------------------------------
int ShowBPM(UDBOPTION* option)
{
	printf("\nBPM:%f\n", option->fixSpeed);
	return 0;
}
