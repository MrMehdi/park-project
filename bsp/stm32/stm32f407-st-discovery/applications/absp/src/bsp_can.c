#include "bsp_can.h"


/*****************************************************************************************
** 函数名称：can1发送数据给逆变器
** 函数描述：
** 输入参数：
** 返回值  ：无
******************************************************************************************/
void bsp_can_send_sbox_data( uint32_t can_id, uint8_t len, uint8_t *buf )
{
    basic_send_can(DEV_CAN1, can_id,  CAN1_IDE, len,  buf);
}
