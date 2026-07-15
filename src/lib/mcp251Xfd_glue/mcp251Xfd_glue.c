#include "mcp251Xfd_glue.h"

#include <furi.h>
#include <furi_hal_spi.h>

#define MCP251XFD_GLUE_SPI_TIMEOUT_MS 100U

eERRORRESULT
    mcp251xfd_glue_spi_init(void* p_int_dev, uint8_t chip_select, const uint32_t sck_freq) {
    UNUSED(chip_select);
    UNUSED(sck_freq);
    FuriHalSpiBusHandle* handle = p_int_dev;
    furi_hal_spi_bus_handle_init(handle);
    return ERR_OK;
}

eERRORRESULT mcp251xfd_glue_spi_transfer(
    void* p_int_dev,
    uint8_t chip_select,
    uint8_t* tx_data,
    uint8_t* rx_data,
    size_t size) {
    UNUSED(chip_select);
    FuriHalSpiBusHandle* handle = p_int_dev;
    bool ok;

    furi_hal_spi_acquire(handle);
    ok = furi_hal_spi_bus_trx(handle, tx_data, rx_data, size, MCP251XFD_GLUE_SPI_TIMEOUT_MS);
    furi_hal_spi_release(handle);

    return ok ? ERR_OK : ERR__SPI_TIMEOUT;
}

uint32_t mcp251xfd_glue_get_ms(void) {
    return furi_get_tick();
}

uint8_t mcp251xfd_glue_dlc_to_len(eMCP251XFD_DataLength dlc) {
    static const uint8_t can20_dlc_to_len[MCP251XFD_DLC_COUNT] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8};
    if(dlc >= MCP251XFD_DLC_COUNT) return 0;
    return can20_dlc_to_len[dlc];
}

const char* mcp251xfd_glue_error_to_string(eERRORRESULT error) {
    eERRORRESULT error_def = ERR_ERROR_Get(error);
    if(error_def < ERR__ERRORS_MAX) return ERR_ErrorStrings[error_def];
    return "Unknown error";
}
