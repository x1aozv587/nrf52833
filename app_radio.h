#ifndef __APP_RADIO_H
#define __APP_RADIO_H

#include "dev_radio.h"

typedef void (*app_radio_hdl)(void *p_context); /**< 回调函数入口 */

#define     APP_RADIO_HDL_MAX       5 /**< 回调函数注册个数 */

/**
 * @brief 初始化 RADIO 
 * 
 * @param pack[in,out].unsigned int * < RADIO 发送 / 接收的数组指针
 * 
 * @retval 无
*/
void app_raido_init(unsigned int *pack);

/**
 * @brief 发送 radio
 * 
 * @retval 无
*/
void app_radio_send(void);

/**
 * @brief 注册回调函数 
 * 
 * @param m_radio_hdl[in].app_radio_hdl < 回调函数
 * 
 * @retval 无
*/
void app_radio_set_register(app_radio_hdl m_radio_hdl);
#endif
