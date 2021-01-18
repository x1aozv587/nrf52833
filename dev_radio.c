#include "dev_radio.h"
#include "nrf52.h"
#include "nrf52_bitfields.h"
#include "nrf_error.h"
#include "nrf_nvic.h"
#include "app_error.h"

/**< 基础频率 */
#define     BASE_FREQENCY_1     2360
#define     BASE_FREQENCY_2     2400

/**< SO S1 和 LENGTH 的长度 */
#define PACKET_S1_FIELD_SIZE      (0UL)  /**< Packet S1 field size in bits. */
#define PACKET_S0_FIELD_SIZE      (0UL)  /**< Packet S0 field size in bits. */
#define PACKET_LENGTH_FIELD_SIZE  (0UL)  /**< Packet length field size in bits. */

/**
 * @brief 设置 RADIO 发射功率
 * 
 * @param m_tx_power[in].m_tx_power_t < 发送功率值
 * 
 * @retval 无
*/
static void radio_set_txpower(m_tx_power_t m_tx_power)
{
    NRF_RADIO -> TXPOWER = m_tx_power;    
}

/**
 * @brief 设置 RADIO 工作模式
 * 
 * @param m_radio_mode[in].m_radio_mode_t < RADIO 发射模式
 * 
 * @retval 无
*/
static void radio_set_mode(m_radio_mode_t m_radio_mode)
{
    NRF_RADIO -> MODE = m_radio_mode;
}

/**
 * @brief 设置 RADIO 工作频道
 * 
 * @param freq[in].uint16_t < RADIO 工作频道，若 freq > 2460 则 工作模式在 2400 - 2500，否则在 2360 - 2460 之间 
 *                            频率 [2360 ~ 2500]
 * 
 * @retval 无
*/
static void radio_set_freq(unsigned short freq)
{
    unsigned char mode = 0;
    unsigned char offset = 0;

    if(freq >= BASE_FREQENCY_2)
    {
        mode = 0;     
        offset = freq - BASE_FREQENCY_2;   
    }else {
        mode = 1;
        offset = freq - BASE_FREQENCY_1;
    }

    NRF_RADIO -> FREQUENCY = ((mode << 8) | offset) & 0xFFFF;   
}

/**
 * @brief 设置 RADIO 发射地址 / 接收地址，目前只做简单的赋值
 * 
 * @retval 无
*/
static void radio_set_addr(void)
{
    NRF_RADIO -> PREFIX0 = 0; /**< AP[3 ... 0] = 0 */
    NRF_RADIO -> PREFIX1 = 0; /**< AP[7 ... 4] = 0 */

    NRF_RADIO -> BASE0 = 0x01234567; 
    NRF_RADIO -> BASE1 = 0x89ABCDEF;

    NRF_RADIO -> TXADDRESS   = 0x00; /**< 逻辑 0 地址 */
    NRF_RADIO -> RXADDRESSES = 0x01; /**< 逻辑 0 地址 */
}

/**
 * @brief 配置 RADIO 
 * 
 * @param m_radio[in].m_radio_t < RADIO 配置对象
 * 
 * @retval 无
*/
void dev_radio_config(m_radio_t *m_radio)
{

    if(NULL == m_radio)
    {
        return ;
    }

    /**< 设置发射功率 */
    radio_set_txpower(m_radio -> m_tx_power);

    /**< 设置工作模式 */
    radio_set_mode(m_radio -> m_radio_mode);

    /**< 设置工作频道 */
    radio_set_freq(m_radio -> freq);

    /**< 设置地址 */
    radio_set_addr();

    /**< 配置 S0 S1 LENGTH 均为 0 */
    NRF_RADIO -> PCNF0 = (PACKET_S0_FIELD_SIZE     << RADIO_PCNF0_S0LEN_Pos) |
                         (PACKET_S1_FIELD_SIZE     << RADIO_PCNF0_S1LEN_Pos) |
                         (PACKET_LENGTH_FIELD_SIZE << RADIO_PCNF0_LFLEN_Pos);

    /**< payload 长度设置 Packet configuration */
    NRF_RADIO -> PCNF1 = (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos) |
                       (RADIO_PCNF1_ENDIAN_Big       << RADIO_PCNF1_ENDIAN_Pos)  |
                       (PACKET_BASE_ADDRESS_LENGTH   << RADIO_PCNF1_BALEN_Pos)   |
                       (PACKET_STATIC_LENGTH         << RADIO_PCNF1_STATLEN_Pos) |
                       (PACKET_PAYLOAD_MAXSIZE       << RADIO_PCNF1_MAXLEN_Pos);

    /**< CRC Config */
    NRF_RADIO -> CRCCNF = (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos); // Number of checksum bits
    if ((NRF_RADIO->CRCCNF & RADIO_CRCCNF_LEN_Msk) == (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos))
    {
        NRF_RADIO->CRCINIT = 0xFFFFUL;   // Initial value
        NRF_RADIO->CRCPOLY = 0x11021UL;  // CRC poly: x^16 + x^12^x^5 + 1
    }
    else if ((NRF_RADIO->CRCCNF & RADIO_CRCCNF_LEN_Msk) == (RADIO_CRCCNF_LEN_One << RADIO_CRCCNF_LEN_Pos))
    {
        NRF_RADIO->CRCINIT = 0xFFUL;   // Initial value
        NRF_RADIO->CRCPOLY = 0x107UL;  // CRC poly: x^8 + x^2^x^1 + 1
    }

    /**< 配置发送缓冲区 */
    NRF_RADIO -> PACKETPTR = m_radio -> pack;

    /**< 配置 SHORT CUT */
    NRF_RADIO -> SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) | /**< READY START */
                          (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);  /**< END_DISABLE */
                     
    /**< 配置 READY 中断，产生中断时发送数据 */
    NRF_RADIO->INTENSET = (RADIO_INTENSET_END_Enabled << RADIO_INTENSET_END_Pos);

    /**< 使能中断 */
    APP_ERROR_CHECK(sd_nvic_EnableIRQ(RADIO_IRQn));

    /**< 配置 RADIO 中断优先级 */
    APP_ERROR_CHECK(sd_nvic_SetPriority(RADIO_IRQn, 1));

    /**< 清除中断标志位 */
    APP_ERROR_CHECK(sd_nvic_ClearPendingIRQ(RADIO_IRQn));
}

/**
 * @brief 使能 radio
 * 
 * @retval 无
*/
void dev_radio_send(void)
{
    // send the packet:
    NRF_RADIO->EVENTS_READY = 0U;
    NRF_RADIO->TASKS_TXEN   = 1;    
}






