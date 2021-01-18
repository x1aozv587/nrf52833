#ifndef __DEV_RADIO_H
#define __DEV_RADIO_H

#include <stdio.h>
#include <stdlib.h>
#include "nrf52.h"
#include "nrf52_bitfields.h"

#define DEF_RADIO(_name) \
m_radio_t _name

/**
 * @brief 发射功率
 * 
 * @ref < "nrf52_bitfields.h" RADIO output power.
 * 
*/
typedef enum{
    RADIO_TXPOWER_0DBM     = 0x00,
    RADIO_TXPOWER_Pos3dBm  = 0x03,
    RADIO_TXPOWER_Pos4dBm  = 0x04,
    RADIO_TXPOWER_Neg40dBm = 0xD8,
    RADIO_TXPOWER_Neg20dBm = 0xEC,
    RADIO_TXPOWER_Neg16dBm = 0xF0,
    RADIO_TXPOWER_Neg12dBm = 0xF4,
    RADIO_TXPOWER_Neg8dBm  = 0xF8,
    RADIO_TXPOWER_Neg4dBm  = 0xFC
} m_tx_power_t;

/**
 * @brief 工作模式
 * 
 * @ref < "nrf52_bitfields.h" Radio data rate and modulation setting. The radio supports Frequency-shift Keying (FSK) modulation.
 * 
*/
typedef enum{
    RADIO_MODE_Nrf_1Mbit = 0,
    RADIO_MODE_Nrf_2Mbit,
    RADIO_MODE_Nrf_125Kbit,
    RADIO_MODE_Ble_1Mbit,
    RADIO_MODE_Ble_2Mbit
} m_radio_mode_t;

/**
 * @brief RADIO 对象结构体
 *
*/
typedef struct 
{
    m_radio_mode_t m_radio_mode; /**< 模式 */

    m_tx_power_t m_tx_power;     /**< 发送功率 */

    unsigned short freq;         /**< 发送频率 */

    unsigned int *pack;          /**< 发送缓冲区 */

} m_radio_t;

#define PACKET_BASE_ADDRESS_LENGTH  (4UL)                   //!< Packet base address length field size in bytes
#define PACKET_STATIC_LENGTH        (1UL)                   //!< Packet static length in bytes
#define PACKET_PAYLOAD_MAXSIZE      (PACKET_STATIC_LENGTH)  //!< Packet payload maximum size in bytes

/**
 * @brief 配置 RADIO 
 * 
 * @param m_radio[in].m_radio_t < RADIO 配置对象
 * 
 * @retval 无
*/
void dev_radio_config(m_radio_t *m_radio);

/**
 * @brief 使能 radio
 * 
 * @retval 无
*/
void dev_radio_send(void);
#endif
