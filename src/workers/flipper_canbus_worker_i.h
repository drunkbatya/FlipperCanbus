#pragma once

#include "flipper_canbus_worker.h"

#include <furi.h>
#include <furi_hal_gpio.h>
#include <furi_hal_resources.h>
#include <furi_hal_spi.h>
#include <furi_hal_spi_config.h>
#include <mlib/m-dict.h>

#include "../lib/mcp251Xfd/MCP251XFD.h"
#include "../lib/mcp251Xfd_glue/mcp251Xfd_glue.h"

#define FLIPPER_CANBUS_RX_QUEUE_SIZE  32U
#define FLIPPER_CANBUS_RX_FIFO        MCP251XFD_FIFO1
#define FLIPPER_CANBUS_SPI_CLOCK_HZ   2000000U
#define FLIPPER_CANLIN_MODULE_XTAL_HZ 40000000U
#define FLIPPER_CANBUS_BITRATE        500000U

typedef struct {
    uint32_t id;
    uint32_t data_size;
    uint8_t data[FLIPPER_CANBUS_MAX_FRAME_SIZE];
} FlipperCanbusRxMessage;

typedef struct {
    uint32_t count;
    uint32_t last_len;
    uint8_t last_data[FLIPPER_CANBUS_MAX_FRAME_SIZE];
} FlipperCanbusMsg;

DICT_DEF2(can_msgs_dict, uint32_t, M_DEFAULT_OPLIST, FlipperCanbusMsg, M_POD_OPLIST)

struct FlipperCanbusWorker {
    FuriThread* thread;
    FuriMessageQueue* can_rx_queue;
    FuriMutex* can_messages_mutex;
    can_msgs_dict_t can_messages;

    FuriHalSpiBusHandle spi_handle;
    MCP251XFD mcp;
    MCP251XFD_BitTimeStats bit_time_stats;
    MCP251XFD_RAMInfos rx_fifo_ram;
    uint32_t sysclk;

    FlipperCanbusWorkerErrorCallback error_callback;
    void* error_callback_context;
    FlipperCanbusWorkerErrorResult last_error;
    const char* last_driver_error;

    volatile bool started;
    bool mcp_ready;
    bool error_reported;
    uint32_t can_rx_dropped;
};

typedef enum {
    FlipperCanbusWorkerEventStop = (1 << 0),
    FlipperCanbusWorkerEventMsgReceived = (1 << 1),
    FlipperCanbusWorkerEventAll = FlipperCanbusWorkerEventStop |
                                  FlipperCanbusWorkerEventMsgReceived,
} FlipperCanbusWorkerEvent;
