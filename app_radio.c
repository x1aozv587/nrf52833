#include "app_radio.h"
#include "dev_radio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_error.h"

DEF_RADIO(m_radio);

static unsigned char m_index = 0; 
static app_radio_hdl m_app_radio_hdl[APP_RADIO_HDL_MAX];

/**
 * @brief radio 中断，上抛给应用层
 * 
 * @retval 无
*/
void RADIO_IRQHandler(void)
{
    unsigned char i = 0;
    
   NRF_LOG_INFO(">> RAIDO_IRQHandle");
   if(NRF_RADIO->EVENTS_END)                   /**< 接收 或 发送 完成 */
   {
        NRF_RADIO->EVENTS_END = 0;
        /**< 接收 或 发送 完成 相关代码 */     
        for(i = 0; i < m_index; i++)
        {
            NRF_LOG_INFO(">> for %d", i);
            if(m_app_radio_hdl[i] != NULL)
            {
                m_app_radio_hdl[i]((unsigned int *)m_radio.pack);
                NRF_LOG_INFO(">> for %d callback", i);
            }
        }                       
   }
}

/**
 * @brief 初始化 RADIO 
 * 
 * @param pack[in,out].unsigned int * < RADIO 发送 / 接收的数组指针
 * 
 * @retval 无
*/
void app_raido_init(unsigned int *pack)
{
    m_radio.pack = pack;

    dev_radio_config(&m_radio);

}

/**
 * @brief 发送 radio
 * 
 * @retval 无
*/
void app_radio_send(void)
{
    dev_radio_send();
}

/**
 * @brief 注册回调函数 
 * 
 * @param m_radio_hdl[in].app_radio_hdl < 回调函数
 * 
 * @retval 无
*/
void app_radio_set_register(app_radio_hdl m_radio_hdl)
{

    if(m_index >= APP_RADIO_HDL_MAX)
    {
        NRF_LOG_INFO("arr overflow");
        return;
    }

    m_app_radio_hdl[m_index++] = m_radio_hdl;
}


