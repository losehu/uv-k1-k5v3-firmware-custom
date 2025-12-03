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

#ifndef ADDIN_UART_FLASH_CMD_H
#define ADDIN_UART_FLASH_CMD_H

#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_UART_FLASH_CMD

// Command IDs
#define CMD_ID_FLASH_READ_32BIT   0x542B
#define CMD_ID_FLASH_WRITE_32BIT  0x5438
#define REPLY_ID_FLASH_READ       0x542C
#define REPLY_ID_FLASH_WRITE      0x5439

// Maximum data size per command
#define FLASH_CMD_MAX_DATA_SIZE   128

/**
 * @brief Process UART command for flash operations
 * @param Port UART port number
 * @param CommandID Command ID (0x542B or 0x5438)
 * @param pBuffer Pointer to command buffer
 * @return true if command was handled, false otherwise
 */
bool UART_FlashCmd_Process(uint32_t Port, uint16_t CommandID, const uint8_t *pBuffer);

#endif // ENABLE_UART_FLASH_CMD

#endif // ADDIN_UART_FLASH_CMD_H
