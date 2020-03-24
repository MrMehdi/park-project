/*
*********************************************************************************************************
*
*	ģ������ : NAND Flash����ģ��
*	�ļ����� : NAND_STM32F1xx.c
*	��    �� : V1.0
*	˵    �� : �ṩNAND Flash (HY27UF081G2A�� 8bit 128K�ֽ� ��ҳ)�ĵײ�ӿں�������������ԭ����
*              ����bsp_nand_flash.c�ļ��޸Ķ�����
*
*	�޸ļ�¼ :
*		�汾��  ����        ����       ˵��
*		V1.0    2020-03-24  leonGuo  ���԰汾
*
*	Copyright (C), 2020-present, lz_kwok@163.com
*
*********************************************************************************************************
*/

/* ����PG6�ĳ�ʱ�жϺ�ʹ�� */
#define TIMEOUT_COUNT			168000000  
#define ENABLE_TIMEOUT		0

/*
	��Ӳ��˵����
	���������������õ�NAND FlahsΪ����ʿ��HY27UF081G2A,  439�������Ϊ H27U4G8F2DTR
	��1��NAND Flash��Ƭѡ�ź����ӵ�CPU��FSMC_NCE2���������NAND Flash�ĵ�ַ�ռ�Ϊ 0x70000000����CPU������
		�ֲ��FSMC�½�)
	��2����FSMC�������ж�������豸����TFT��SRAM��CH374T��NOR������˱���ȷ�����������豸��Ƭѡ���ڽ�ֹ
		״̬�����򽫳������߳�ͻ���� ���μ����ļ���ʼ��FSMC GPIO�ĺ�����

	��NAND Flash �ṹ���塿
     ��������16x4�ֽڣ�ÿpage 2048�ֽڣ�ÿ512�ֽ�һ��������ÿ��������Ӧ16�Լ��ı�������

	 ÿ��PAGE���߼��ṹ��ǰ��512Bx4����������������16Bx4�Ǳ�����
	��������������������������������������������������������������������������������������������������������������������������������
	�� Main area  ���� Main area  ���� Main area  ����Main area   ���� Spare area ���� Spare area ���� Spare area ����Spare area  ��
	��            ����            ����            ����            ����            ����            ����            ����            ��
	��   512B     ����    512B    ����    512B    ����    512B    ����    16B     ����     16B    ����     16B    ����    16B     ��
	��������������������������������������������������������������������������������������������������������������������������������

	 ÿ16B�ı��������߼��ṹ����:(FlashFS�е����ã�
	������������������������������������������������������������������������������������������������������������������������������������������������������
	��LSN0  ����LSN1  ����LSN2  ����LSN3  ����COR ����BBM   ����ECC0  ����ECC1  ����ECC2����ECC3����ECC4����ECC5����  ECC6����  ECC7���� ECC8 ���� ECC9 ��
	��      ����      ����      ����      ����    ����      ����      ����      ����    ����    ����    ����    ����      ����      ����      ����      ��
	������������������������������������������������������������������������������������������������������������������������������������������������������

    - LSN0 ~ LSN3 : �߼�������(logical sector number) ��
		- COR         : �����𻵱�ǣ�data corrupted marker����
		- BBM         : �����־(Bad Block Marker)��
    - ECC0 ~ ECC9 : 512B����������ECCУ�顣

	K9F1G08U0A �� HY27UF081G2A �Ǽ��ݵġ�оƬ����ʱ�����̱�֤оƬ�ĵ�1�����Ǻÿ顣����ǻ��飬���ڸÿ�ĵ�1��PAGE�ĵ�1���ֽ�
	���ߵ�2��PAGE������1��PAGE�����޷����Ϊ0xFFʱ���ĵ�1���ֽ�д���0xFFֵ��������ֵ������ģ����ֱ���ж��Ƿ����0xFF���ɡ�

	ע�⣺������Щ����˵NAND Flash���̵�Ĭ�������ǽ������Ƕ��ڵ�1��PAGE�ĵ�6���ֽڴ������˵���Ǵ��󡣻������ڵ�6���ֽڽ���Բ���С������512�ֽڣ���NAND Flash
	���������е�NAND Flash���������׼������ڸ���NAND Flashʱ������ϸ�Ķ�оƬ�������ֲᡣ

*/

/* ����NAND Flash�������ַ���������Ӳ�������� */
#define Bank2_NAND_ADDR    ((uint32_t)0x70000000)
#define Bank_NAND_ADDR     Bank2_NAND_ADDR

/* �������NAND Flash�õ�3���� */
#define NAND_CMD_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | CMD_AREA)
#define NAND_ADDR_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | ADDR_AREA)
#define NAND_DATA_AREA		*(__IO uint8_t *)(Bank_NAND_ADDR | DATA_AREA)

static uint8_t FSMC_NAND_GetStatus(void);

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_Init
*	����˵��: ����FSMC��GPIO����NAND Flash�ӿڡ�������������ڶ�дnand flashǰ������һ�Ρ�
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void FSMC_NAND_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	FSMC_NANDInitTypeDef  FSMC_NANDInitStructure;
	FSMC_NAND_PCCARDTimingInitTypeDef  p;


	/*--NAND Flash GPIOs ����  ------
		PD0/FSMC_D2
		PD1/FSMC_D3
		PD4/FSMC_NOE
		PD5/FSMC_NWE
		PD7/FSMC_NCE2
		PD11/FSMC_A16
		PD12/FSMC_A17
		PD14/FSMC_D0
		PD15/FSMC_D1

		PE7/FSMC_D4
		PE8/FSMC_D5
		PE9/FSMC_D6
		PE10/FSMC_D7

		PG6/FSMC_INT2	(�������ò�ѯ��ʽ��æ���˿�����Ϊ��ͨGPIO���빦��ʹ��)
	*/

	/* ʹ�� GPIO ʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG, ENABLE);

	/* ʹ�� FSMC ʱ�� */
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

	/*  ����GPIOD */
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
	                            GPIO_Pin_7 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/*  ����GPIOE */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/*  ����GPIOG, PG6��Ϊæ��Ϣ������Ϊ���� */

	/* INT2 ��������Ϊ�ڲ��������룬����æ�ź� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	/* ���� FSMC ʱ�� 2014-06-29 ����ʱ�� */
	/*
		Defines the number of HCLK cycles to setup address before the command assertion for NAND-Flash
		read or write access  to common/Attribute or I/O memory space (depending on the memory space
		timing to be configured).This parameter can be a value between 0 and 0xFF.
	*/
  	//p.FSMC_SetupTime = 0x01;  ---- 01 err, 02 ok
  	p.FSMC_SetupTime = 0x2;

	/*
		Defines the minimum number of HCLK cycles to assert the command for NAND-Flash read or write
		access to common/Attribute or I/O memory space (depending on the memory space timing to be
		configured). This parameter can be a number between 0x00 and 0xFF
	*/
	//p.FSMC_WaitSetupTime = 0x03; ---- 03 err;  05 ok
	p.FSMC_WaitSetupTime = 0x5;

	/*
		Defines the number of HCLK clock cycles to hold address (and data for write access) after the
		command deassertion for NAND-Flash read or write access to common/Attribute or I/O memory space
		 (depending on the memory space timing to be configured).
		This parameter can be a number between 0x00 and 0xFF
	*/
	//p.FSMC_HoldSetupTime = 0x02;  --- 02 err  03 ok
	p.FSMC_HoldSetupTime = 0x3;

	/*
		Defines the number of HCLK clock cycles during which the databus is kept in HiZ after the start
		of a NAND-Flash  write access to common/Attribute or I/O memory space (depending on the memory
		space timing to be configured). This parameter can be a number between 0x00 and 0xFF
	*/
	//p.FSMC_HiZSetupTime = 0x01;
	p.FSMC_HiZSetupTime = 0x1;


	FSMC_NANDInitStructure.FSMC_Bank = FSMC_Bank2_NAND;  					/* ����FSMC NAND BANK �� */
	FSMC_NANDInitStructure.FSMC_Waitfeature = FSMC_Waitfeature_Disable;		/* ����ȴ�ʱ��ʹ�� */
	FSMC_NANDInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_8b;	/* ���ݿ�� 8bit */
	FSMC_NANDInitStructure.FSMC_ECC = FSMC_ECC_Disable; 					/* ECC������;�������ʹ�� */
	FSMC_NANDInitStructure.FSMC_ECCPageSize = FSMC_ECCPageSize_2048Bytes;	/* ECC ҳ���С */
	FSMC_NANDInitStructure.FSMC_TCLRSetupTime = 0x01;						/* CLE�ͺ�RE��֮����ӳ٣�HCLK������ */	
	FSMC_NANDInitStructure.FSMC_TARSetupTime = 0x01;						/* ALE�ͺ�RE��֮����ӳ٣�HCLK������ */

	FSMC_NANDInitStructure.FSMC_CommonSpaceTimingStruct = &p;				/* FSMC Common Space Timing */
	FSMC_NANDInitStructure.FSMC_AttributeSpaceTimingStruct = &p;			/* FSMC Attribute Space Timing */

	FSMC_NANDInit(&FSMC_NANDInitStructure);

	/* FSMC NAND Bank ʹ�� */
	FSMC_NANDCmd(FSMC_Bank2_NAND, ENABLE);
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_Reset
*	����˵��: ��λNAND Flash
*	��    ��:  ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_Reset(void)
{
	NAND_CMD_AREA = NAND_CMD_RESET;

		/* ������״̬ */
	if (FSMC_NAND_GetStatus() == NAND_READY)
	{
		return NAND_OK;
	}

	return NAND_FAIL;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_ReadStatus
*	����˵��: ʹ��Read statuc �����NAND Flash�ڲ�״̬
*	��    ��:  - Address: �������Ŀ��������ַ
*	�� �� ֵ: NAND����״̬�������¼���ֵ��
*             - NAND_BUSY: �ڲ���æ
*             - NAND_READY: �ڲ����У����Խ����²�����
*             - NAND_ERROR: ��ǰ������ִ��ʧ��
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_ReadStatus(void)
{
	uint8_t ucData;
	uint8_t ucStatus = NAND_BUSY;

	/* ��״̬���� */
	NAND_CMD_AREA = NAND_CMD_STATUS;
	ucData = *(__IO uint8_t *)(Bank_NAND_ADDR);

	if((ucData & NAND_ERROR) == NAND_ERROR)
	{
		ucStatus = NAND_ERROR;
	}
	else if((ucData & NAND_READY) == NAND_READY)
	{
		ucStatus = NAND_READY;
	}
	else
	{
		ucStatus = NAND_BUSY;
	}

	return (ucStatus);
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_GetStatus
*	����˵��: ��ȡNAND Flash����״̬
*	��    ��:  - Address: �������Ŀ��������ַ
*	�� �� ֵ: NAND����״̬�������¼���ֵ��
*             - NAND_TIMEOUT_ERROR  : ��ʱ����
*             - NAND_READY          : �����ɹ�
*             - NAND_ERROR: ��ǰ������ִ��ʧ��
*********************************************************************************************************
*/
static uint8_t FSMC_NAND_GetStatus(void)
{
	uint32_t ulTimeout = 0x10000;
	uint8_t ucStatus = NAND_READY;

	ucStatus = FSMC_NAND_ReadStatus();

	/* �ȴ�NAND������������ʱ����˳� */
	while ((ucStatus != NAND_READY) &&( ulTimeout != 0x00))
	{
		ucStatus = FSMC_NAND_ReadStatus();
		if(ucStatus == NAND_ERROR)
		{
			/* ���ز���״̬ */
			return (ucStatus);
		}
		ulTimeout--;
	}

	if(ulTimeout == 0x00)
	{
		ucStatus =  NAND_TIMEOUT_ERROR;
	}

	/* ���ز���״̬ */
	return (ucStatus);
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_ReadID
*	����˵��: ��NAND Flash��ID��ID�洢���β�ָ���Ľṹ������С�
*	��    ��: ��
*	�� �� ֵ: 32bit��NAND Flash ID
*********************************************************************************************************
*/
uint32_t NAND_ReadID(void)
{
	uint32_t data = 0;

	/* �������� Command to the command area */
	NAND_CMD_AREA = 0x90;
	NAND_ADDR_AREA = 0x00;

	/* ˳���ȡNAND Flash��ID */
	data = *(__IO uint32_t *)(Bank_NAND_ADDR | DATA_AREA);
	data =  ((data << 24) & 0xFF000000) |
			((data << 8 ) & 0x00FF0000) |
			((data >> 8 ) & 0x0000FF00) |
			((data >> 24) & 0x000000FF) ;
	return data;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_WritePage
*	����˵��: дһ��������NandFlashָ��ҳ���ָ��λ�ã�д������ݳ��Ȳ�����һҳ�Ĵ�С��
*	��    ��:  - _pBuffer: ָ�������д���ݵĻ�����
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInPage : ҳ�ڵ�ַ����ΧΪ��0-2111
*             - _usByteCount: д����ֽڸ���
*	�� �� ֵ: ִ�н����
*             RTV_NOERR         - ҳд��ɹ�
*             ERR_NAND_PROG     - ҳд��ʧ��
*             ERR_NAND_HW_TOUT  - ��ʱ
*********************************************************************************************************
*/
uint8_t FSMC_NAND_WritePage(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount)
{
	uint32_t i;
	uint8_t ucStatus;

	/* ����ҳд���� */
	NAND_CMD_AREA = NAND_CMD_WRITE0;

	/* ����ҳ�ڵ�ַ �� ���� HY27UF081G2A
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20

		H27U4G8F2DTR (512MB)
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
		��5�ֽڣ� A28  A29  A30  A31  0    0    0    0
	*/
	NAND_ADDR_AREA = _usAddrInPage;
	NAND_ADDR_AREA = _usAddrInPage >> 8;
	NAND_ADDR_AREA = _ulPageNo;
	NAND_ADDR_AREA = (_ulPageNo & 0xFF00) >> 8;

	#if NAND_ADDR_5 == 1
		NAND_ADDR_AREA = (_ulPageNo & 0xFF0000) >> 16;
	#endif

	/* tADL = 100ns,  Address to Data Loading */
	for (i = 0; i < 20; i++);	/* ��Ҫ���� 100ns */

	/* д���� */
	for(i = 0; i < _usByteCount; i++)
	{
		NAND_DATA_AREA = _pBuffer[i];
	}
	NAND_CMD_AREA = NAND_CMD_WRITE_TRUE1;

	/* WE High to Busy , 100ns */
	for (i = 0; i < 20; i++);	/* ��Ҫ���� 100ns */
	
#if ENABLE_TIMEOUT
	for (i = TIMEOUT_COUNT; i; i--) 
	{
		 if (GPIOG->IDR & GPIO_Pin_6) break;
	}
	
	/* ���س�ʱ */
	if(i == 0)
	{
		return (ERR_NAND_HW_TOUT);
	}	
#else
	while((GPIOG->IDR & GPIO_Pin_6) == 0);
#endif 
	
	/* ����״̬ */
	ucStatus = FSMC_NAND_GetStatus();
	if(ucStatus == NAND_READY)   
	{
		ucStatus = RTV_NOERR;
	}
	else if(ucStatus == NAND_ERROR)
	{
		ucStatus = ERR_NAND_PROG;		
	}
	else if(ucStatus == NAND_TIMEOUT_ERROR)
	{
		ucStatus = ERR_NAND_HW_TOUT;		
	}
	
	return (ucStatus);
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_ReadPage
*	����˵��: ��NandFlashָ��ҳ���ָ��λ�ö�һ�����ݣ����������ݳ��Ȳ�����һҳ�Ĵ�С��
*	��    ��:  - _pBuffer: ָ�������д���ݵĻ�����
*             - _ulPageNo: ҳ�ţ����е�ҳͳһ���룬��ΧΪ��0 - 65535
*			  - _usAddrInPage : ҳ�ڵ�ַ����ΧΪ��0-2111
*             - _usByteCount: �ֽڸ�����  ����� 2048 + 64��
*	�� �� ֵ: ִ�н����
*             RTV_NOERR         - ҳ��ȡ�ɹ�
 *            ERR_NAND_HW_TOUT  - ��ʱ
*********************************************************************************************************
*/
uint8_t FSMC_NAND_ReadPage(uint8_t *_pBuffer, uint32_t _ulPageNo, uint16_t _usAddrInPage, uint16_t _usByteCount)
{
	uint32_t i;

    /* ����ҳ������� */
    NAND_CMD_AREA = NAND_CMD_AREA_A;

	/* ����ҳ�ڵ�ַ �� ���� HY27UF081G2A  (128MB)
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20

		H27U4G8F2DTR (512MB)
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
		��5�ֽڣ� A28  A29  A30  A31  0    0    0    0
	*/
	NAND_ADDR_AREA = _usAddrInPage;
	NAND_ADDR_AREA = _usAddrInPage >> 8;
	NAND_ADDR_AREA = _ulPageNo;
	NAND_ADDR_AREA = (_ulPageNo & 0xFF00) >> 8;

	#if NAND_ADDR_5 == 1
		NAND_ADDR_AREA = (_ulPageNo & 0xFF0000) >> 16;
	#endif

	NAND_CMD_AREA = NAND_CMD_AREA_TRUE1;

	 /* ����ȴ���������������쳣, �˴�Ӧ���жϳ�ʱ */
	for (i = 0; i < 20; i++);

#if ENABLE_TIMEOUT
	for (i = TIMEOUT_COUNT; i; i--) 
	{
		if (GPIOG->IDR & GPIO_Pin_6) break;
	}
	
	/* ���س�ʱ */
	if(i == 0)
	{
		return (ERR_NAND_HW_TOUT);
	}	
#else
	while((GPIOG->IDR & GPIO_Pin_6) == 0);
#endif 

	/* �����ݵ�������pBuffer */
	for(i = 0; i < _usByteCount; i++)
	{
		_pBuffer[i] = NAND_DATA_AREA;
	}

	return RTV_NOERR;
}

/*
*********************************************************************************************************
*	�� �� ��: FSMC_NAND_EraseBlock
*	����˵��: ����NAND Flashһ���飨block��
*	��    ��:  - _ulBlockNo: ��ţ���ΧΪ��0 - 1023,   0-4095
*	�� �� ֵ: NAND����״̬�������¼���ֵ��
*             RTV_NOERR         - �����ɹ�
*             ERR_NAND_ERASE    - ����ʧ��
*             ERR_NAND_HW_TOUT  - ��ʱ
*********************************************************************************************************
*/
uint8_t FSMC_NAND_EraseBlock(uint32_t _ulBlockNo)
{
	uint8_t ucStatus;
	
	/* HY27UF081G2A  (128MB)
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12    A18�����ǿ��
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20

		H27U4G8F2DTR (512MB)
				  Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
		��1�ֽڣ� A7   A6   A5   A4   A3   A2   A1   A0		(_usPageAddr ��bit7 - bit0)
		��2�ֽڣ� 0    0    0    0    A11  A10  A9   A8		(_usPageAddr ��bit11 - bit8, ��4bit������0)
		��3�ֽڣ� A19  A18  A17  A16  A15  A14  A13  A12    A18�����ǿ��
		��4�ֽڣ� A27  A26  A25  A24  A23  A22  A21  A20
		��5�ֽڣ� A28  A29  A30  A31  0    0    0    0
	*/

	/* ���Ͳ������� */
	NAND_CMD_AREA = NAND_CMD_ERASE0;

	_ulBlockNo <<= 6;	/* ���ת��Ϊҳ��� */

	#if NAND_ADDR_5 == 0	/* 128MB�� */
		NAND_ADDR_AREA = _ulBlockNo;
		NAND_ADDR_AREA = _ulBlockNo >> 8;
	#else		/* 512MB�� */
		NAND_ADDR_AREA = _ulBlockNo;
		NAND_ADDR_AREA = _ulBlockNo >> 8;
		NAND_ADDR_AREA = _ulBlockNo >> 16;
	#endif

	NAND_CMD_AREA = NAND_CMD_ERASE1;

	/* ����״̬ */
	ucStatus = FSMC_NAND_GetStatus();
	if(ucStatus == NAND_READY)   
	{
		ucStatus = RTV_NOERR;
	}
	else if(ucStatus == NAND_ERROR)
	{
		ucStatus = ERR_NAND_PROG;		
	}
	else if(ucStatus == NAND_TIMEOUT_ERROR)
	{
		ucStatus = ERR_NAND_HW_TOUT;		
	}
	
	return (ucStatus);
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_IsBadBlock
*	����˵��: ���ݻ����Ǽ��NAND Flashָ���Ŀ��Ƿ񻵿�
*	��    ��: _ulBlockNo ����� 0 - 1023 ������128M�ֽڣ�2K Page��NAND Flash����1024���飩
*	�� �� ֵ: 0 ���ÿ���ã� 1 ���ÿ��ǻ���
*********************************************************************************************************
*/
uint8_t NAND_IsBadBlock(uint32_t _ulBlockNo)
{
	uint8_t ucFlag;

	/* ���NAND Flash����ǰ�Ѿ���עΪ�����ˣ������Ϊ�ǻ��� */
	FSMC_NAND_ReadPage(&ucFlag, _ulBlockNo * NAND_BLOCK_SIZE, NAND_PAGE_SIZE + BBM_OFFSET, 1);
	if (ucFlag != 0xFF)
	{
		return 1;
	}

	FSMC_NAND_ReadPage(&ucFlag, _ulBlockNo * NAND_BLOCK_SIZE + 1, NAND_PAGE_SIZE + BBM_OFFSET, 1);
	if (ucFlag != 0xFF)
	{
		return 1;
	}
	return 0;	/* �Ǻÿ� */
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_Format
*	����˵��: NAND Flash��ʽ�����������е����ݣ��ؽ�LUT
*	��    ��:  ��
*	�� �� ֵ: NAND_OK : �ɹ��� NAND_Fail ��ʧ�ܣ�һ���ǻ����������ർ�£�
*********************************************************************************************************
*/
uint8_t NAND_Format(void)
{
#if 0
	uint16_t i;
	uint16_t usGoodBlockCount;

	/* ����ÿ���� */
	usGoodBlockCount = 0;
	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		/* ����Ǻÿ飬����� */
		if (!NAND_IsBadBlock(i))
		{
			FSMC_NAND_EraseBlock(i);
			usGoodBlockCount++;
		}
	}

	/* ����ÿ����������100����NAND Flash���� */
	if (usGoodBlockCount < 100)
	{
		return NAND_FAIL;
	}

	return NAND_OK;
#else
	uint16_t i;

	for (i = 0; i < NAND_BLOCK_COUNT; i++)
	{
		FSMC_NAND_EraseBlock(i);
	}

	return NAND_OK;
#endif
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_Init
*	����˵��: ��ʼ��NAND Flash�ӿ�
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void NAND_Init(void)
{
	FSMC_NAND_Init();			/* ����FSMC��GPIO����NAND Flash�ӿ� */

	FSMC_NAND_Reset();			/* ͨ����λ���λNAND Flash����״̬ */
}

/*
*********************************************************************************************************
*	�� �� ��: NAND_UnInit
*	����˵��: ж��NAND Flash
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void NAND_UnInit(void)
{
	FSMC_NANDDeInit(FSMC_Bank2_NAND);
	
	/* FSMC NAND Bank ���� */
	FSMC_NANDCmd(FSMC_Bank2_NAND, DISABLE);
}

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
