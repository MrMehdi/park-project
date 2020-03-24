/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     misonyo   first version
 * 2019-09-12     Guolz     v1.0.0
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <at.h>
#include <string.h>
#include <stdarg.h>
#include  <math.h>
#include "RTL.h"
#include "File_Config.h"
#include "drv_nand.h"

static const char * ReVal_Table[]= 
{
	"0���ɹ�",				                        
	"1��IO����I/O������ʼ��ʧ�ܣ�����û�д洢�豸�������豸��ʼ��ʧ��",
	"2������󣬹���ʧ�ܣ�����FAT�ļ�ϵͳ��ζ����Ч��MBR��������¼���߷�FAT��ʽ",
	"3��FAT��־��ʼ��ʧ�ܣ�FAT��ʼ���ɹ��ˣ�������־��ʼ��ʧ��",
};

#define gprs_power          GET_PIN(G, 0)
#define gprs_rst            GET_PIN(G, 1)
#define usbd                GET_PIN(C, 9)


void phy_reset(void)
{
    rt_pin_mode(phy_rst_pin,PIN_MODE_OUTPUT);
    rt_pin_write(phy_rst_pin, PIN_HIGH);
    rt_thread_mdelay(500);
    rt_pin_write(phy_rst_pin, PIN_LOW);
    rt_thread_mdelay(500);
    rt_kprintf("%s done\r\n",__func__);
    rt_pin_write(phy_rst_pin, PIN_HIGH);
}

static void DotFormat(uint64_t _ullVal, char *_sp) 
{
	/* ��ֵ���ڵ���10^9 */
	if (_ullVal >= (U64)1e9) 
	{
		_sp += sprintf (_sp, "%d.", (uint32_t)(_ullVal / (uint64_t)1e9));
		_ullVal %= (uint64_t)1e9;
		_sp += sprintf (_sp, "%03d.", (uint32_t)(_ullVal / (uint64_t)1e6));
		_ullVal %= (uint64_t)1e6;
		sprintf (_sp, "%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* ��ֵ���ڵ���10^6 */
	if (_ullVal >= (uint64_t)1e6) 
	{
		_sp += sprintf (_sp,"%d.", (uint32_t)(_ullVal / (uint64_t)1e6));
		_ullVal %= (uint64_t)1e6;
		sprintf (_sp,"%03d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* ��ֵ���ڵ���10^3 */
	if (_ullVal >= 1000) 
	{
		sprintf (_sp, "%d.%03d", (uint32_t)(_ullVal / 1000), (uint32_t)(_ullVal % 1000));
		return;
	}
	
	/* ������ֵ */
	sprintf (_sp,"%d",(U32)(_ullVal));
}

static void ViewNandCapacity(void)
{
	uint8_t result;
	Media_INFO info;
	uint64_t ullNANDCapacity;
	FAT_VI *mc0;  
	uint8_t buf[15];
	
	/* 
	   �ͼ���ʽ����ע��һ��Ҫִ���˺���NAND_Init����ִ��NAND_Format()����Ϊ��
	   1. finit("N0:") �����NAND_Init����FlahFS��nandflash�Ľӿ��ļ�FS_NAND_FlashPrg.c
	   2. funinit("N0:") �����UnInit����FlahFS��nandflash�Ľӿ��ļ�FS_NAND_FlashPrg.c
	*/
	// rt_hw_mtd_nand_init();
	// rt_kprintf("���ڽ��еͼ���ʽ����....\r\n");
	// NAND_EraseChip();
	// rt_kprintf("�ͼ���ʽ�����....\r\n");

	/* ����SD�� */
	result = finit("N0:");
	if(result != NULL)
	{
		rt_kprintf("�����ļ�ϵͳʧ�� (%s)\r\n", ReVal_Table[result]);
		rt_kprintf("����ʧ�ܣ�NAND Flash��Ҫ����FAT32��ʽ��\r\n");
	}
	else
	{
		rt_kprintf("�����ļ�ϵͳ�ɹ� (%s)\r\n", ReVal_Table[result]);
	}
	
	rt_kprintf("���ڽ���FAT32��ʽ����....\r\n");
	if (fformat ("N0:") != 0)  
	{            
		rt_kprintf ("��ʽ��ʧ��\r\n");
	}
	else  
	{
		rt_kprintf ("��ʽ���ɹ�\r\n");
	}
	
	rt_kprintf("------------------------------------------------------------------\r\n");
	/* ��ȡvolume label */
	if (fvol ("N0:", (char *)buf) == 0) 
	{
		if (buf[0]) 
		{
			rt_kprintf ("NAND Flash��volume label�� %s\r\n", buf);
		}
		else 
		{
			rt_kprintf ("NAND Flashû��volume label\r\n");
		}
	}
	else 
	{
		rt_kprintf ("Volume���ʴ���\r\n");
	}

	/* ��ȡNAND Flashʣ������ */
	ullNANDCapacity = ffree("N0:");
	DotFormat(ullNANDCapacity, (char *)buf);
	rt_kprintf("NAND Flashʣ������ = %10s�ֽ�\r\n", buf);
	
	/* ж��NAND Flash */
	result = funinit("N0:");
	if(result != NULL)
	{
		rt_kprintf("ж���ļ�ϵͳʧ��\r\n");
	}
	else
	{
		rt_kprintf("ж���ļ�ϵͳ�ɹ�\r\n");
	}
	
	/* ��ȡ��Ӧ�洢�豸�ľ�� */
	mc0 = ioc_getcb("N0");          
   
	/* ��ʼ��FAT�ļ�ϵͳ��ʽ�Ĵ洢�豸 */
	if (ioc_init (mc0) == 0) 
	{
		/* ��ȡ�洢�豸��������Ϣ */
		ioc_read_info (&info, mc0);

		/* �ܵ������� * ������С��NAND Flash��������С��512�ֽ� */
		ullNANDCapacity = (uint64_t)info.block_cnt << 9;
		DotFormat(ullNANDCapacity, (char *)buf);
		rt_kprintf("NAND Flash������ = %10s�ֽ�\r\nNAND Flash���������� = %d \r\n", buf, info.block_cnt);
	
		rt_kprintf("NAND Flash��������С = %d�ֽ�\r\n", info.read_blen);
		rt_kprintf("NAND Flashд������С = %d�ֽ�\r\n", info.write_blen);
	}
	else 
	{
		rt_kprintf("��ʼ��ʧ��\r\n");
	}
	
	/* ж��NAND Flash */
	if(ioc_uninit (mc0) != NULL)
	{
		rt_kprintf("ж��ʧ��\r\n");		
	}
	else
	{
		rt_kprintf("ж�سɹ�\r\n");	
	}
	rt_kprintf("------------------------------------------------------------------\r\n");
}


int main(void)
{
    rt_pin_mode(usbd,PIN_MODE_OUTPUT);
    rt_pin_write(usbd, PIN_HIGH);
    rt_thread_mdelay(2000);
    ViewNandCapacity();
    while (RT_TRUE)
    {   
       

        // g_Gui_show_pic("5");
        rt_thread_mdelay(2000);
        // g_Gui_show_pic("F");
    }

    return RT_EOK;
}



