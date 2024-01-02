#include "pch.h"
#include "DLT645_2007.h"
#include "ext.h"
#include <vector>

int DataField[] = {
	QUERY_INCP,
	QUERY_INCQ,
	QUERY_DECP,
	QUERY_DECQ,
	QUERY_COS,
	QUERY_P,
	QUERY_S,
	QUERY_Q,
	QUERY_AV,
	QUERY_AA,
	QUERY_BV,
	QUERY_BA,
	QUERY_CV,
	QUERY_CA,
	QUERY_ACOS,
	QUERY_BCOS,
	QUERY_CCOS,
	QUERY_AXJ,
	QUERY_BXJ,
	QUERY_CXJ
};

Dlt645_Rule::Dlt645_Rule()
{
	m_nCurrDevNo = 0;
	
}

Dlt645_Rule::Dlt645_Rule(int identifier)
{
	
	m_nCurrDevNo = 0;
}

Dlt645_Rule::~Dlt645_Rule()
{
	printf("Dlt645_Rule线程结束");
}

int Dlt645_Rule::InitDevStru()
{
	return 0;
}

void Dlt645_Rule::Unpack_Data()
{
	//数据标识
	int Meter_DI;
	if (UnSetupData())
	{
		Meter_DI = ((m_pRevBuf[13] - 0x33) * 256 * 256 * 256
			+ (m_pRevBuf[12] - 0x33) * 256 * 256
			+ (m_pRevBuf[11] - 0x33) * 256
			+ (m_pRevBuf[10] - 0x33));
	}

	BYTE temp[4];
	for (int i = 0; i < 4; i++)
	{
		temp[i] = m_pRevBuf[14 + i] - 0x33;
		temp[i] = B2H(temp[i]);
	}

	float fvalue0 = float(temp[3]) * 1000000 + float(temp[2]) * 10000+ float(temp[1]) * 100 + float(temp[0]);
	float fvalue0_1 = (float)(fvalue0 / 100.0);

	float fvalue1 = float(temp[1]) * 100 + float(temp[0]);
	float fvalue1_1 = (float)(fvalue1 / 1000.0);
	float fvalue1_2 = (float)(fvalue1 / 10.0);

	float fvalue2 = float(temp[2]) * 10000 + float(temp[1]) * 100 + float(temp[0]);
	float fvalue2_1 = (float)(fvalue2 / 10000.0);
	float fvalue2_2 = (float)(fvalue2 / 1000.0);

	switch (Meter_DI)
	{
	case 0x00010000:
		m_Met_Dev.INCPfvlaue = fvalue0_1;
		break;
	case 0x00030000:
		m_Met_Dev.INCQfvlaue = fvalue0_1;
		break;
	case 0x00020000:
		m_Met_Dev.DECPfvlaue = fvalue0_1;
		break;
	case 0x00040000:
		m_Met_Dev.DECQfvlaue = fvalue0_1;
		break;
	case 0x02060000:
		m_Met_Dev.COSfvlaue = fvalue1_1;
		break;
	case 0x02030000:
		m_Met_Dev.Pfvlaue = fvalue2_1;
		break;
	case 0x02040000:
		m_Met_Dev.Qfvlaue = fvalue2_1;
		break;
	case 0x02050000:
		m_Met_Dev.Sfvlaue = fvalue2_1;
		break;
	case 0x02010100:
		m_Met_Dev.AVfvlaue = fvalue1_2;
		break;
	case 0x02010200:
		m_Met_Dev.BVfvlaue = fvalue1_2;
		break;
	case 0x02010300:
		m_Met_Dev.CVfvlaue = fvalue1_2;
		break;
	case 0x02020100:
		m_Met_Dev.AAfvlaue = fvalue2_2;
		break;
	case 0x02020200:
		m_Met_Dev.BAfvlaue = fvalue2_2;
		break;
	case 0x02020300:
		m_Met_Dev.CAfvlaue = fvalue2_2;
		break;
	case 0x02060100:
		m_Met_Dev.ACOSfvlaue = fvalue1_1;
		break;
	case 0x02060200:
		m_Met_Dev.BCOSfvlaue = fvalue1_1;
		break;
	case 0x02060300:
		m_Met_Dev.CCOSfvlaue = fvalue1_1;
		break;
	case 0x02070100:
		m_Met_Dev.AXJfvlaue = fvalue1_2;
		break;
	case 0x02070200:
		m_Met_Dev.BXJfvlaue = fvalue1_2;
		break;
	case 0x02070300:
		m_Met_Dev.CXJfvlaue = fvalue1_2;
		break;
	}

	m_Vdt_Sys.RtuData.m_pRevBuf = m_pRevBuf;
}

void Dlt645_Rule::Pack_Data()
{	
	int fanco = 1;
	m_struDev[fanco] = m_Vdt_Dev;
	m_sSendLen = 0;
	if (m_sSendLen == 0)
	{
		if (m_nCurrDevNo <= 0 || m_nCurrDevNo >= 5)
		{
			m_nCurrDevNo = 1;
		}
		while (!m_struDev[m_nCurrDevNo].isworking)
		{
			Sleep(1);
			if (m_nCurrDevNo >= 5)
			{
				m_nCurrDevNo = 1;
			}
		}
		//起始码
		m_pSendBuf[0] = 0x68;
		//表号
		if (m_Met_Dev.ammeterNum)
		{
			BYTE* pBData = CStringToBYTE(m_Met_Dev.ammeterNum);
			for (int i = 1; i <= 6; i++)
			{
				m_pSendBuf[i] = H2B(pBData[6 - i]);
			}
		}
		//起始符
		m_pSendBuf[7] = 0x68;
		//控制码 /改自主选择	
		m_pSendBuf[8] = 0x11;
		//数据域长度
		m_pSendBuf[9] = 0x04;
		//数据标识
		int length = sizeof(DataField) / sizeof(DataField[0]);
		m_CmdCode = DataField[LoopNum++];
		IntToBYTE(m_CmdCode);
		if (LoopNum == length){ LoopNum = 0; }
		//校验码
		m_pSendBuf[14] = Check_SumCode(m_pSendBuf, 14);
		m_pSendBuf[15] = 0x16;
		m_sSendLen = 16;
		m_Vdt_Sys.RtuData.m_sSendLen = m_sSendLen;
		m_Vdt_Sys.RtuData.m_pSendBuf = m_pSendBuf;
		
	}
}

void Dlt645_Rule::Pack_Data_02()
{
	int fanco = 1;
	m_struDev[fanco] = m_Vdt_Dev;
	m_sSendLen = 0;
	if (m_sSendLen == 0)
	{
		if (m_nCurrDevNo <= 0 || m_nCurrDevNo >= 5)
		{
			m_nCurrDevNo = 1;
		}
		while (!m_struDev[m_nCurrDevNo].isworking)
		{
			Sleep(1);
			if (m_nCurrDevNo >= 5)
			{
				m_nCurrDevNo = 1;
			}
		}
		//起始码
		m_pSendBuf[0] = 0x68;
		//表号
		if (m_Met_Dev.ammeterNum)
		{
			BYTE* pBData = CStringToBYTE(m_Met_Dev.ammeterNum);
			for (int i = 1; i <= 6; i++)
			{
				m_pSendBuf[i] = H2B(pBData[6 - i]);
			}
		}
		//起始符
		m_pSendBuf[7] = 0x68;
		//控制码	
		m_pSendBuf[8] = m_Met_Dev.Conchar;
		//数据域长度
		m_pSendBuf[9] = 0x10;

		if (m_Met_Dev.Conchar == 0x1C)
		{
			m_pSendBuf[10] = m_Met_Dev.Pa;
			if (m_Met_Dev.Passwd)
			{
				BYTE* pBData = CStringToBYTE(m_Met_Dev.Passwd);
				for (int i = 1; i <= 3; i++)
				{
					m_pSendBuf[10 + i] = H2B(pBData[3 - i]) + 0x33;
				}
			}
			if (m_Met_Dev.OperatorCode)
			{
				BYTE* pBData = CStringToBYTE(m_Met_Dev.OperatorCode);
				for (int i = 1; i <= 4; i++)
				{
					m_pSendBuf[13 + i] = H2B(pBData[4 - i]) + 0x33;
				}
			}
			m_pSendBuf[18] = m_Met_Dev.Com;
			m_pSendBuf[19] = 0x33;
			CTime time = CTime::GetCurrentTime();
			CString str = time.Format("%Y%m%d%H%M%S");//20 23 12 05 08 42 18
			if (str)
			{
				BYTE* pBData = CStringToBYTE(str);
				for (int i = 1; i <= 6; i++)
				{
					m_pSendBuf[19 + i] = H2B(pBData[7 - i]);
				}
			}

		}
		else if (m_Met_Dev.Conchar == 0x14)
		{
			BYTE* pBData = CStringToBYTE(m_Met_Dev.DI);
			for (int i = 1; i <= 4; i++)
			{
				m_pSendBuf[9 + i] = H2B(pBData[4 - i]) + 0x33;
			}
			m_pSendBuf[14] = m_Met_Dev.Pa;

			if (m_Met_Dev.Passwd)
			{
				BYTE* pBData = CStringToBYTE(m_Met_Dev.Passwd);
				for (int i = 1; i <= 3; i++)
				{
					m_pSendBuf[14 + i] = H2B(pBData[3 - i]) + 0x33;
				}
			}

			if (m_Met_Dev.OperatorCode)
			{
				BYTE* pBData = CStringToBYTE(m_Met_Dev.OperatorCode);
				for (int i = 1; i <= 4; i++)
				{
					m_pSendBuf[17 + i] = H2B(pBData[4 - i]) + 0x33;
				}
			}

			if (m_Met_Dev.Data)
			{
				BYTE* pBData = CStringToBYTE(m_Met_Dev.Data);
				for (int i = 1; i <= 4; i++)
				{
					m_pSendBuf[21 + i] = H2B(pBData[4 - i]) + 0x33;
				}
			}

		}
		else if (m_Met_Dev.Conchar == 0x11)
		{

		}
		m_pSendBuf[26] = Check_SumCode(m_pSendBuf, 26);
		m_pSendBuf[27] = 0x16;
		m_sSendLen = 28;
		m_Vdt_Sys.RtuData.m_sSendLen = m_sSendLen;
		m_Vdt_Sys.RtuData.m_pSendBuf = m_pSendBuf;
	}
}

void Dlt645_Rule::Unpack_Data_02()
{
	if (UnSetupData())
	{
		m_Vdt_Sys.RtuData.m_pRevBuf = m_pRevBuf;
		m_pRev = m_pRevBuf;
	}
}

void Dlt645_Rule::Rule_Loop()
{
	if (m_bFstconnect)
	{
		m_bFstconnect = false;
		Base_Connect();
	}
	while (1)
	{
		if (m_Met_Dev.LoopFlag)
		{
			
			Pack_Data_02();
			Base_Send();
			Sleep(500);
			Base_Read();
			if (m_sRevLen > 0)
			{
				Unpack_Data_02();
			}
			m_Met_Dev.Conchar = 0;
			m_Met_Dev.Com = 0;
			m_Met_Dev.Pa = 0;
			m_Met_Dev.LoopFlag = 0;
			Sleep(500);
		
		}
		else
		{
			
			Pack_Data();
			Base_Send();
			Sleep(500);
			Base_Read();
			if (m_sRevLen > 0)
			{
				Unpack_Data();
			}
			Sleep(500);
		}
		
	}
}

BYTE Dlt645_Rule::H2B(BYTE bhex)//16进制 ----> BCD
{
	return ( ((bhex / 10) << 4) | (bhex%10) );
}

BYTE Dlt645_Rule::B2H(BYTE bBCD)//BCD ----> 16进制
{
	return (bBCD & 0x0f)+((bBCD >> 4) & 0x0f)*10;
}

BYTE* Dlt645_Rule::CStringToBYTE(CString& str)
{
	int iBytesLen = str.GetLength() / 2;
	BYTE* pBData = new BYTE[iBytesLen];
	for (int i = 0, j = 0; i < str.GetLength(); i += 2, j++)
	{
		CString subStr = str.Mid(i, 2);
		pBData[j] = _tcstoul(subStr, NULL, 10);
	}
	return pBData;
}

BYTE* Dlt645_Rule::IntToBYTE(int datacode) {
	BYTE* hexArray = new BYTE[4];
	for (int i = 0; i < 4; i++) {
		uint8_t byte = (datacode >> (i * 8)) & 0xFF; // 逐个取出每个字节
		hexArray[3 - i] = byte; // 将字节放入数组
	}
	//数据标识
	m_pSendBuf[10] = hexArray[0] + 0x33;
	m_pSendBuf[11] = hexArray[1] + 0x33;
	m_pSendBuf[12] = hexArray[2] + 0x33;
	m_pSendBuf[13] = hexArray[3] + 0x33;
	return hexArray;
}

BYTE Dlt645_Rule::Check_SumCode(BYTE* pInput, int len)//累加校验码
{
	BYTE bResult = 0;
	for (int i = 0; i < len; i++)
	{
		bResult += *pInput++;
	}
	return bResult;
}

BYTE Dlt645_Rule::Check_RevCode(int bufno)//收码校验
{
	int i;
	BYTE check_sum = 0x00;
	BOOL result;
	for (int i = 0; i < bufno; i++)
	{
		check_sum += m_pRevBuf[i];
		if (check_sum == m_pRevBuf[bufno])
		{
			result = 1;
		}
		else
		{
			result = 0;
		}
	}
	return result;
}

BOOL Dlt645_Rule::UnSetupData()
{
	//去掉前导码：FEFEFEFE 
	for (int i = 0; i < m_sRevLen; i++)
	{
		if (m_pRevBuf[i] == 0x68)
		{
			if (i != 0)
			{
				memcpy(&m_pRevBuf[0], &m_pRevBuf[i], m_sRevLen - i);
				m_sRevLen -= i;
			}
			break;
		}
	}
	if ( (m_pRevBuf[0] == 0x68 ) && (m_pRevBuf[7] == 0x68) )
	{
		if ( Check_RevCode(10 + m_pRevBuf[9]) )
		{
			return 1;
		}
	}
	return 0;
}
