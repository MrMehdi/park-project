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
#include "drv_nand.h"

#define LocalPort_NUM       8080
#define IP1                 192
#define IP2                 168
#define IP3                 0
#define IP4                 10

#define gprs_power          GET_PIN(G, 0)
#define gprs_rst            GET_PIN(G, 1)
#define usbd                GET_PIN(C, 9)

const char * ReVal_Table[]= 
{
	" 0: SCK_SUCCESS       Success                             ",
	"-1: SCK_ERROR         General Error                       ",
	"-2: SCK_EINVALID      Invalid socket descriptor           ",
	"-3: SCK_EINVALIDPARA  Invalid parameter                   ",
	"-4: SCK_EWOULDBLOCK   It would have blocked.              ",
	"-5: SCK_EMEMNOTAVAIL  Not enough memory in memory pool    ",
	"-6: SCK_ECLOSED       Connection is closed or aborted     ",
	"-7: SCK_ELOCKED       Socket is locked in RTX environment ",
	"-8: SCK_ETIMEOUT      Socket, Host Resolver timeout       ",
	"-9: SCK_EINPROGRESS   Host Name resolving in progress     ",
	"-10: SCK_ENONAME      Host Name not existing              ",
};

rt_uint8_t sendbuf[1024];


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




int main(void)
{
    // char dbuf[10];
	// int len;
	// int sock, sd, res;
	// SOCKADDR_IN addr;
	// SOCKADDR_IN ReAddr;

    rt_pin_mode(usbd,PIN_MODE_OUTPUT);
    rt_pin_write(usbd, PIN_HIGH);
    finit("N0:");
    init_TcpNet ();
    // rt_thread_mdelay(10000);        //��Э��ջ��ʼ����

    // unsigned long sck_mode;
    
    while (RT_TRUE)
    {   
        // /* ����һ��socket 
        // ��1������AF_INET����ǰ��֧��������͵ĵ�ַ�塣
        // ��2������SOCK_STREAM����ʾ������ͨ�����ͣ���ʹ�õ�TCP��
        // ��3������0 ������Ϊ0�Ļ����Զ�����2����������Э��ƥ�䣬�������TCPЭ�顣
        // */
        // sock = socket (AF_INET, SOCK_STREAM, 0);

        // /* �˿ں�����Ϊ1001 */
        // addr.sin_port        = htons(LocalPort_NUM);

        // /* �뺯��socket�е�AF_INET����һ�� */
        // addr.sin_family      = PF_INET;
        // /* 
        //     INADDR_ANY����ָ����ַΪ0.0.0.0�ĵ�ַ�������ַ��ʵ�ϱ�ʾ��ȷ����ַ�������е�ַ��
        //     �����ַ����������Ļ��ͱ�ʾ��ض˿ں�Ϊddr.sin_port������IP��ַ��Ϣ��һ����Ҫ��
        //     ���ж����������IP��ַ�������
        // */
        // addr.sin_addr.s_addr = INADDR_ANY;

        // /* ��socket��IP�Ͷ˿ں� */
        // bind (sock, (SOCKADDR *)&addr, sizeof(addr));

        // /* ���ü�����������1������ */
        // listen (sock, 2);

        // /* 
        //     �ȴ�soket���������еĻ����Զ�����1���µ�socket��������ͨ�ţ�û�еĻ����ȴ����ӡ�
        //     ע�⣬�ܹ�accept�ĸ����ܵ�listen���������ƣ���listen�������ܵ�Net_Config.c�к궨��
        //     BSD_NUMSOCKS �����ơ�
        // */
        // len = sizeof(ReAddr);
        // sd = accept (sock, (SOCKADDR *)&ReAddr, &len);
        // rt_kprintf ("Զ�̿ͻ�����������IP: %d.%d.%d.%d\n", ReAddr.sin_addr.s_b1,
        //                                                         ReAddr.sin_addr.s_b2,
        //                                                         ReAddr.sin_addr.s_b3,
        //                                                         ReAddr.sin_addr.s_b4);
        // rt_kprintf ("Զ�̿ͻ��˶˿ں�: %d\n", ntohs(ReAddr.sin_port));

        // /* �رռ���socket���������socket�ǵ��ú���socket���Զ������� */
        // closesocket (sock);
        // sock = sd;           

        /* ����һ��socket 
		   ��1������AF_INET����ǰ��֧��������͵ĵ�ַ�塣
		   ��2������SOCK_STREAM����ʾ������ͨ�����ͣ���ʹ�õ�TCP��
		   ��3������0 ������Ϊ0�Ļ����Զ�����2����������Э��ƥ�䣬�������TCPЭ�顣
		*/
		// sock = socket (AF_INET, SOCK_STREAM, 0);
		
		// /* ����ʹ��KEEP ALIVE���ÿͻ��˺ͷ������������� */
		// sck_mode = 1;
		// res = ioctlsocket (sock, FIO_KEEP_ALIVE, &sck_mode);
		// if (res == SCK_SUCCESS) 
		// {
		// 	rt_kprintf("KEEP ALIVE set done\r\n");
		// }
		// else 
		// {
		// 	rt_kprintf("KEEP ALIVE set fail\r\n");
		// }

		// /* �˿ں�����Ϊ1001 */
		// addr.sin_port = htons(LocalPort_NUM);
		
		// /* �뺯��socket�е�AF_INET����һ�� */
		// addr.sin_family = PF_INET;
		
		// addr.sin_addr.s_b1 = IP1;
		// addr.sin_addr.s_b2 = IP2;
		// addr.sin_addr.s_b3 = IP3;
		// addr.sin_addr.s_b4 = IP4;

		// /* �ͻ�������Զ�̷����������Զ�̷�������δ�������˺������������� */
		// res = connect (sock, (SOCKADDR *)&addr, sizeof (addr));
		// rt_kprintf("�ͻ�������Զ�̷�����״̬%s\r\n", ReVal_Table[abs(res)]);



        rt_thread_mdelay(5000);

        // for(;;){
            /*
            socket���ݽ��պ��������recv����������ģʽ��ʹ���������ע���������
            1. �˺��������ʱ���ܵ�Net_Config.c�к궨�� BSD_RCVTOUT �����ơ����ʱ�䵽���Զ��˳���
            2. ����������յ�һ�����ݰ��ͻ᷵�أ����ڻ���С�����õĻ�������С��û�й�ϵ�����������
                ���ڽ��ջ�������С���û�ֻ���ε��ú���recv���н��ռ��ɡ�
            3. ʵ�ʽ��յ����ݴ�Сͨ���жϴ˺����ķ���ֵ���ɡ�
            */
            rt_thread_mdelay(50);
        //     res = recv (sock, dbuf, sizeof(dbuf), 0);
        //     if (res <= 0) 
        //     {
        //         // // closesocket (sock);
        //         // rt_kprintf("�˳����պ��������¿�ʼ����%s\r\n", ReVal_Table[abs(res)]);
        //         // break;
        //     }
        //     else
        //     {
        //         rt_kprintf("Receive Data Length = %d\r\n", res);
        //         switch(dbuf[0])
        //         {
        //             /* �ַ����� 1 */
        //             case '1':
        //                 sendbuf[0] = '1';
        //                 sendbuf[1] = '2';
        //                 sendbuf[2] = '3';
        //                 sendbuf[3] = '4';
        //                 sendbuf[4] = '5';
        //                 sendbuf[5] = '6';
        //                 sendbuf[6] = '7';
        //                 sendbuf[7] = '8';
        //                 sendbuf[8] = '\r';
        //                 sendbuf[9] = '\n';						
        //                 res = send (sock, (char *)sendbuf, 10, 0);
        //                 if (res < 0) 
        //                 {
        //                     rt_kprintf("����send��������ʧ��\r\n");
        //                 }
        //                 else
        //                 {
        //                     rt_kprintf("����send�������ݳɹ�\r\n");							
        //                 }
        //                 break;
                    
        //             /* �ַ����� 2 */
        //             case '2':
        //                 /* �����ݻ���������ַ�0����������������ֲ鿴���� */
        //                 len = sizeof(sendbuf);
        //                 memset(sendbuf, 48, len);
                    
        //                 /* �������ʼ�������ݰ���ǰ4���ֽں����4���ֽ� */
        //                 sendbuf[0] = 'a';
        //                 sendbuf[1] = 'b';
        //                 sendbuf[2] = 'c';
        //                 sendbuf[3] = 'd';
        //                 sendbuf[len - 4] = 'e';
        //                 sendbuf[len - 3] = 'f';
        //                 sendbuf[len - 2] = 'g';
        //                 sendbuf[len - 1] = 'h';					
        //                 res = send (sock, (char *)sendbuf, len, 0);
        //                 if (res < 0) 
        //                 {
        //                     rt_kprintf("����send��������ʧ��%s\r\n", ReVal_Table[abs(res)]);
        //                 }
        //                 else
        //                 {
        //                     rt_kprintf("����send�ɹ��������� = %d�ֽ�\r\n", res);							
        //                 }
        //                 break;
                
        //             /* ������ֵ�������� */
        //             default:                     
        //                 break;
        //         }
        //     }
        // }
		
    
    }

    /* 
        ���ʱ�䵽��Զ���豸�Ͽ����ӵȣ����򶼻�ִ�е��������������ر�socket��
        ���򷵻ص���һ����whileѭ���Ŀ�ͷ���´���socket��������
    */
    // closesocket (sock);

    return RT_EOK;
}



