/* Copyright 2025
 * Extended UART commands for full SPI Flash access (2MB)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#ifdef ENABLE_UART_FLASH_CMD

#include "uart_flash_cmd.h"
#include <string.h>
#include "driver/py25q16.h"

// External variables from uart.c
#if defined(ENABLE_UART)
extern uint32_t UART_Timestamp;
#endif
#if defined(ENABLE_USB)
extern uint32_t VCP_Timestamp;
#endif

extern uint16_t gSerialConfigCountDown_500ms;

#ifdef ENABLE_FMRADIO
extern uint16_t gFmRadioCountdown_500ms;
extern const uint16_t fm_radio_countdown_500ms;
#endif

// UART port definitions from uart.c
#define UART_PORT_UART 0
#define UART_PORT_VCP  1

// Data structures
typedef struct {
    uint16_t ID;
    uint16_t Size;
} Header_t;

// CMD_542B: Read SPI Flash with 32-bit address
typedef struct {
    Header_t Header;
    uint32_t Address;      // 32-bit address (0x00000000 - 0x001FFFFF)
    uint8_t  Size;         // Read size (max 128 bytes)
    uint8_t  Padding[3];
    uint32_t Timestamp;
} CMD_542B_t;

typedef struct {
    Header_t Header;
    struct {
        uint32_t Address;
        uint8_t  Size;
        uint8_t  Padding[3];
        uint8_t  Data[FLASH_CMD_MAX_DATA_SIZE];
    } Data;
} REPLY_542C_t;

// CMD_5438: Write SPI Flash with 32-bit address
typedef struct {
    Header_t Header;
    uint32_t Address;      // 32-bit address
    uint8_t  Size;         // Write size (must be multiple of 8)
    uint8_t  Padding[3];
    uint32_t Timestamp;
    uint8_t  Data[0];      // Variable length data
} CMD_5438_t;

typedef struct {
    Header_t Header;
    struct {
        uint32_t Address;
        uint8_t  Status;   // 0=success, 1=error
        uint8_t  Padding[3];
    } Data;
} REPLY_5439_t;

// External SendReply function from uart.c
extern void SendReply(uint32_t Port, void *pReply, uint16_t Size);

// Read SPI Flash with 32-bit address
static void CMD_542B(uint32_t Port, const uint8_t *pBuffer)
{
    const CMD_542B_t *pCmd = (const CMD_542B_t *)pBuffer;
    REPLY_542C_t      Reply;

    uint32_t Timestamp = 0;

    if(0) {}
#if defined(ENABLE_UART)
    else if (Port == UART_PORT_UART)
    {
        Timestamp = UART_Timestamp;
    }
#endif
#if defined(ENABLE_USB)
    else if (Port == UART_PORT_VCP)
    {
        Timestamp = VCP_Timestamp;
    }
#endif
    else
    {
        return;
    }

    if (pCmd->Timestamp != Timestamp)
        return;

    gSerialConfigCountDown_500ms = 12; // 6 sec

    #ifdef ENABLE_FMRADIO
        gFmRadioCountdown_500ms = fm_radio_countdown_500ms;
    #endif

    memset(&Reply, 0, sizeof(Reply));
    Reply.Header.ID   = REPLY_ID_FLASH_READ; // 0x542C
    Reply.Header.Size = pCmd->Size + 8;
    Reply.Data.Address = pCmd->Address;
    Reply.Data.Size   = pCmd->Size;

    // Check address range (0 - 2MB) and size
    if (pCmd->Address < 0x00200000 && pCmd->Size > 0 && pCmd->Size <= FLASH_CMD_MAX_DATA_SIZE)
    {
        PY25Q16_ReadBuffer(pCmd->Address, Reply.Data.Data, pCmd->Size);
    }
    
    SendReply(Port, &Reply, pCmd->Size + 12);
}

// Write SPI Flash with 32-bit address
static void CMD_5438(uint32_t Port, const uint8_t *pBuffer)
{
    const CMD_5438_t *pCmd = (const CMD_5438_t *)pBuffer;
    REPLY_5439_t Reply;

    uint32_t Timestamp = 0;

    if(0) {}
#if defined(ENABLE_UART)
    else if (Port == UART_PORT_UART)
    {
        Timestamp = UART_Timestamp;
    }
#endif
#if defined(ENABLE_USB)
    else if (Port == UART_PORT_VCP)
    {
        Timestamp = VCP_Timestamp;
    }
#endif
    else
    {
        return;
    }

    if (pCmd->Timestamp != Timestamp)
        return;

    gSerialConfigCountDown_500ms = 12; // 6 sec

    #ifdef ENABLE_FMRADIO
        gFmRadioCountdown_500ms = fm_radio_countdown_500ms;
    #endif

    Reply.Header.ID   = REPLY_ID_FLASH_WRITE; // 0x5439
    Reply.Header.Size = sizeof(Reply.Data);
    Reply.Data.Address = pCmd->Address;
    Reply.Data.Status = 0;  // Success by default

    // Check address range and size (must be multiple of 8)
    if (pCmd->Address >= 0x00200000 || pCmd->Size == 0 || pCmd->Size > FLASH_CMD_MAX_DATA_SIZE || (pCmd->Size % 8) != 0)
    {
        Reply.Data.Status = 1;  // Error: invalid parameters
    }
    else
    {
        // Write to SPI Flash
        // Note: Append=false means it will erase sector if needed
        PY25Q16_WriteBuffer(pCmd->Address, pCmd->Data, pCmd->Size, false);
    }

    SendReply(Port, &Reply, sizeof(Reply));
}

// Public interface function
bool UART_FlashCmd_Process(uint32_t Port, uint16_t CommandID, const uint8_t *pBuffer)
{
    switch (CommandID)
    {
        case CMD_ID_FLASH_READ_32BIT: // 0x542B
            CMD_542B(Port, pBuffer);
            return true;

        case CMD_ID_FLASH_WRITE_32BIT: // 0x5438
            CMD_5438(Port, pBuffer);
            return true;

        default:
            return false;
    }
}

#endif // ENABLE_UART_FLASH_CMD
