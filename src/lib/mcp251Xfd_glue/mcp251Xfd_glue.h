#pragma once

#include "../mcp251Xfd/MCP251XFD.h"

eERRORRESULT
    mcp251xfd_glue_spi_init(void* p_int_dev, uint8_t chip_select, const uint32_t sck_freq);
eERRORRESULT mcp251xfd_glue_spi_transfer(
    void* p_int_dev,
    uint8_t chip_select,
    uint8_t* tx_data,
    uint8_t* rx_data,
    size_t size);
uint32_t mcp251xfd_glue_get_ms(void);
uint8_t mcp251xfd_glue_dlc_to_len(eMCP251XFD_DataLength dlc);
const char* mcp251xfd_glue_error_to_string(eERRORRESULT error);
