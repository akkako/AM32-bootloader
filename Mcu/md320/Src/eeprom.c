/*
 * eeprom.c
 *
 *  Created on: Sept. 28, 2025
 *      Author: Alka, akako
 *
 */

#include "eeprom.h"
#include "main.h"
#include <string.h>

/**
 * @brief  FLASH erase function
 * @param  None
 * @retval None
 */
static inline void FlashErase(uint32_t addr_start, uint32_t length)
{
  uint32_t flash_program_start = addr_start; /* Start address of user erase page */
  uint32_t flash_program_end = addr_start + length;

  while (flash_program_start < flash_program_end)
  {
    /* Wait Busy=0 */
    while (LL_FLASH_IsActiveFlag_BUSY(FLASH) == 1)
      ;

    /* Enable EOP */
    LL_FLASH_EnableIT_EOP(FLASH);

    /* Enable Page Erase */
    LL_FLASH_EnablePageErase(FLASH);

    /* Set Erase Address */
    LL_FLASH_SetEraseAddress(FLASH, flash_program_start);

    /* Wait Busy=0 */
    while (LL_FLASH_IsActiveFlag_BUSY(FLASH) == 1)
      ;

    /* Wait EOP=1 */
    while (LL_FLASH_IsActiveFlag_EOP(FLASH) == 0)
      ;

    /* Clear EOP */
    LL_FLASH_ClearFlag_EOP(FLASH);

    /* Disable EOP */
    LL_FLASH_DisableIT_EOP(FLASH);

    /* Disable Page Erase */
    LL_FLASH_DisablePageErase(FLASH);
    flash_program_start += FLASH_PAGE_SIZE;
  }
}

static inline void FlashProgram(uint32_t addr, const uint8_t *data, uint32_t len)
{
  uint32_t flash_program_start = addr;     /* Start address of user write flash */
  uint32_t flash_program_end = addr + len; /* End address of user write flash */
  uint32_t *src = (uint32_t *)data;        /* Pointer to array */

  while (flash_program_start < flash_program_end)
  {
    /* Wait Busy=0 */
    while (LL_FLASH_IsActiveFlag_BUSY(FLASH) == 1)
      ;

    /* Enable EOP */
    LL_FLASH_EnableIT_EOP(FLASH);

    /* Enable Program */
    LL_FLASH_EnablePageProgram(FLASH);

    /* Page Program */
    LL_FLASH_PageProgram(FLASH, flash_program_start, src);

    /* Wait Busy=0 */
    while (LL_FLASH_IsActiveFlag_BUSY(FLASH) == 1)
      ;

    /* Wait EOP=1 */
    while (LL_FLASH_IsActiveFlag_EOP(FLASH) == 0)
      ;

    /* Clear EOP */
    LL_FLASH_ClearFlag_EOP(FLASH);

    /* Disable EOP */
    LL_FLASH_DisableIT_EOP(FLASH);

    /* Disable Program */
    LL_FLASH_DisablePageProgram(FLASH);
    flash_program_start += FLASH_PAGE_SIZE; /* Point to the start address of the next page to be written */
    src += FLASH_PAGE_SIZE / 4;             /* Point to the next data to be written */
  }
}

bool save_flash_nolib(const uint8_t *data, uint32_t length, uint32_t add)
{
  if ((add & 0x7) != 0 || (length & 0x7) != 0)
  {
    // address and length must be on 8 byte boundary
    return false;
  }

  /* Unlock FLASH */
  LL_FLASH_Unlock(FLASH);

  LL_FLASH_TIMMING_SEQUENCE_CONFIG_24M();

  // erase page
  FlashErase(add, length);

  // program page
  FlashProgram(add, data, length);

  /* Lock FLASH */
  LL_FLASH_Lock(FLASH);

  // ensure data is correct
  return memcmp(data, (const void *)add, length) == 0;
}

void read_flash_bin(uint8_t *data, uint32_t add, int out_buff_len)
{
  memcpy(data, (void *)add, out_buff_len);
}
