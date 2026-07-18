#include <workers/flipper_canbus_worker_i.h>

#include <core/check.h>

#include <stdlib.h>
#include <string.h>

#define TAG "CanbusWorker"

static void flipper_canbus_worker_lock(FlipperCanbusWorker* worker) {
    furi_check(furi_mutex_acquire(worker->can_messages_mutex, FuriWaitForever) == FuriStatusOk);
}

static void flipper_canbus_worker_unlock(FlipperCanbusWorker* worker) {
    furi_check(furi_mutex_release(worker->can_messages_mutex) == FuriStatusOk);
}

typedef enum {
    FlipperCanbusWorkerPollIdle,
    FlipperCanbusWorkerPollMessageReceived,
    FlipperCanbusWorkerPollError,
    FlipperCanbusWorkerPollStop,
} FlipperCanbusWorkerPollResult;

static void flipper_canbus_worker_report_error(
    FlipperCanbusWorker* worker,
    FlipperCanbusWorkerErrorResult error,
    eERRORRESULT driver_error) {
    const char* driver_error_string = mcp251xfd_glue_error_to_string(driver_error);

    if(worker->error_callback) {
        worker->error_callback(worker->error_callback_context, error, driver_error_string);
    }
}

static void flipper_canbus_worker_update_insert_message(
    FlipperCanbusWorker* worker,
    const FlipperCanbusRxMessage* message) {
    if(message->data_size > FLIPPER_CANBUS_MAX_FRAME_SIZE) return;

    FlipperCanbusMsg* msg = can_msgs_dict_get(worker->can_messages, message->id);
    if(msg == NULL) {
        if(can_msgs_dict_size(worker->can_messages) >= FLIPPER_CANBUS_MAX_IDS) return;
        msg = can_msgs_dict_safe_get(worker->can_messages, message->id);
    }

    msg->count++;
    if((msg->last_len != message->data_size) ||
       (memcmp(msg->last_data, message->data, message->data_size) != 0)) {
        memcpy(msg->last_data, message->data, message->data_size);
        msg->last_len = message->data_size;
    }
}

static FlipperCanbusWorkerPollResult
    flipper_canbus_worker_poll_mcp251xfd(FlipperCanbusWorker* worker) {
    bool message_received = false;
    for(uint32_t i = 0; i < FLIPPER_CANBUS_RX_FIFO_DEPTH; i++) {
        if(furi_thread_flags_get() & FlipperCanbusWorkerEventStop) {
            return FlipperCanbusWorkerPollStop;
        }

        setMCP251XFD_FIFOstatus status = MCP251XFD_RX_FIFO_EMPTY;
        eERRORRESULT error =
            MCP251XFD_GetFIFOStatus(&worker->mcp, FLIPPER_CANBUS_RX_FIFO, &status);
        if(error != ERR_OK) {
            FURI_LOG_W(TAG, "GetFIFOStatus failed: %s", mcp251xfd_glue_error_to_string(error));
            flipper_canbus_worker_report_error(
                worker, FlipperCanbusWorkerErrorGetFifoStatus, error);
            return FlipperCanbusWorkerPollError;
        }
        if((status & MCP251XFD_RX_FIFO_NOT_EMPTY) == 0) break;

        uint8_t payload[FLIPPER_CANBUS_MAX_FRAME_SIZE] = {0};
        MCP251XFD_CANMessage can_message = {
            .PayloadData = payload,
        };
        error = MCP251XFD_ReceiveMessageFromFIFO(
            &worker->mcp, &can_message, MCP251XFD_PAYLOAD_8BYTE, NULL, FLIPPER_CANBUS_RX_FIFO);
        if(error != ERR_OK) {
            FURI_LOG_W(TAG, "ReceiveMessage failed: %s", mcp251xfd_glue_error_to_string(error));
            flipper_canbus_worker_report_error(
                worker, FlipperCanbusWorkerErrorReceiveMessage, error);
            return FlipperCanbusWorkerPollError;
        }

        FlipperCanbusRxMessage rx_message = {
            .id = can_message.MessageID,
            .data_size = mcp251xfd_glue_dlc_to_len(can_message.DLC),
        };
        memcpy(rx_message.data, payload, rx_message.data_size);

        flipper_canbus_worker_lock(worker);
        flipper_canbus_worker_update_insert_message(worker, &rx_message);
        flipper_canbus_worker_unlock(worker);

        message_received = true;
    }

    return message_received ? FlipperCanbusWorkerPollMessageReceived : FlipperCanbusWorkerPollIdle;
}

static void flipper_canbus_worker_gpio_isr(void* context) {
    FlipperCanbusWorker* worker = context;
    furi_thread_flags_set(furi_thread_get_id(worker->thread), FlipperCanbusWorkerEventMsgReceived);
}

static bool flipper_canbus_worker_configure_mcp251xfd(FlipperCanbusWorker* worker) {
    worker->spi_handle = furi_hal_spi_bus_handle_external;
    worker->spi_handle.cs = &gpio_ext_pb2;

    worker->mcp = (MCP251XFD){
        .DriverConfig = MCP251XFD_DRIVER_NORMAL_USE,
        .SPI_ChipSelect = 0,
        .InterfaceDevice = &worker->spi_handle,
        .SPIClockSpeed = FLIPPER_CANBUS_SPI_CLOCK_HZ,
        .fnSPI_Init = mcp251xfd_glue_spi_init,
        .fnSPI_Transfer = mcp251xfd_glue_spi_transfer,
        .fnGetCurrentms = mcp251xfd_glue_get_ms,
    };

    MCP251XFD_Config config = {
        .XtalFreq = FLIPPER_CANLIN_MODULE_XTAL_HZ,
        .OscFreq = 0,
        .SysclkConfig = MCP251XFD_SYSCLK_IS_CLKIN,
        .ClkoPinConfig = MCP251XFD_CLKO_SOF,
        .SYSCLK_Result = NULL,
        .NominalBitrate = FLIPPER_CANBUS_BITRATE,
        .DataBitrate = MCP251XFD_NO_CANFD,
        .BitTimeStats = NULL,
        .Bandwidth = MCP251XFD_NO_DELAY,
        .ControlFlags =
            MCP251XFD_CAN_RESTRICTED_MODE_ON_ERROR | MCP251XFD_CAN_ESI_REFLECTS_ERROR_STATUS |
            MCP251XFD_CAN_UNLIMITED_RETRANS_ATTEMPTS | MCP251XFD_CANFD_BITRATE_SWITCHING_DISABLE |
            MCP251XFD_CAN_PROTOCOL_EXCEPT_AS_FORM_ERROR | MCP251XFD_CANFD_USE_ISO_CRC |
            MCP251XFD_CANFD_DONT_USE_RRS_BIT_AS_SID11,
        .GPIO0PinMode = MCP251XFD_PIN_AS_GPIO0_OUT,
        .GPIO1PinMode = MCP251XFD_PIN_AS_GPIO1_IN,
        .INTsOutMode = MCP251XFD_PINS_PUSHPULL_OUT,
        .TXCANOutMode = MCP251XFD_PINS_PUSHPULL_OUT,
        .SysInterruptFlags = MCP251XFD_INT_RX_EVENT,
    };

    MCP251XFD_FIFO rx_fifo = {
        .Name = FLIPPER_CANBUS_RX_FIFO,
        .Size = MCP251XFD_FIFO_32_MESSAGE_DEEP,
        .Payload = MCP251XFD_PAYLOAD_8BYTE,
        .Direction = MCP251XFD_RECEIVE_FIFO,
        .Attempts = MCP251XFD_THREE_ATTEMPTS,
        .Priority = MCP251XFD_MESSAGE_TX_PRIORITY16,
        .ControlFlags = MCP251XFD_FIFO_NO_CONTROL_FLAGS,
        .InterruptFlags = MCP251XFD_FIFO_OVERFLOW_INT | MCP251XFD_FIFO_RECEIVE_FIFO_NOT_EMPTY_INT,
        .RAMInfos = NULL,
    };

    MCP251XFD_Filter standard_filter = {
        .Filter = MCP251XFD_FILTER0,
        .EnableFilter = true,
        .Match = MCP251XFD_MATCH_SID_EID,
        .PointTo = FLIPPER_CANBUS_RX_FIFO,
        .AcceptanceID = MCP251XFD_ACCEPT_ALL_MESSAGES,
        .AcceptanceMask = MCP251XFD_ACCEPT_ALL_MESSAGES,
        .ExtendedID = false,
    };
    MCP251XFD_Filter extended_filter = standard_filter;
    extended_filter.Filter = MCP251XFD_FILTER1;
    extended_filter.ExtendedID = true;

    eERRORRESULT error = Init_MCP251XFD(&worker->mcp, &config);
    if(error != ERR_OK) {
        FURI_LOG_E(TAG, "Init failed: %s", mcp251xfd_glue_error_to_string(error));
        furi_hal_spi_bus_handle_deinit(&worker->spi_handle);
        flipper_canbus_worker_report_error(worker, FlipperCanbusWorkerErrorMcpInit, error);
        return false;
    }

    error = MCP251XFD_ConfigureFIFO(&worker->mcp, &rx_fifo);
    if(error != ERR_OK) {
        FURI_LOG_E(TAG, "FIFO config failed: %s", mcp251xfd_glue_error_to_string(error));
        furi_hal_spi_bus_handle_deinit(&worker->spi_handle);
        flipper_canbus_worker_report_error(worker, FlipperCanbusWorkerErrorFifoConfig, error);
        return false;
    }

    error = MCP251XFD_ConfigureFilter(&worker->mcp, &standard_filter);
    if(error != ERR_OK) {
        FURI_LOG_E(
            TAG, "Standard filter config failed: %s", mcp251xfd_glue_error_to_string(error));
        furi_hal_spi_bus_handle_deinit(&worker->spi_handle);
        flipper_canbus_worker_report_error(
            worker, FlipperCanbusWorkerErrorStandardFilterConfig, error);
        return false;
    }

    error = MCP251XFD_ConfigureFilter(&worker->mcp, &extended_filter);
    if(error != ERR_OK) {
        FURI_LOG_E(
            TAG, "Extended filter config failed: %s", mcp251xfd_glue_error_to_string(error));
        furi_hal_spi_bus_handle_deinit(&worker->spi_handle);
        flipper_canbus_worker_report_error(
            worker, FlipperCanbusWorkerErrorExtendedFilterConfig, error);
        return false;
    }

    error = MCP251XFD_StartCAN20(&worker->mcp);
    if(error != ERR_OK) {
        FURI_LOG_E(TAG, "Start CAN2.0 failed: %s", mcp251xfd_glue_error_to_string(error));
        furi_hal_spi_bus_handle_deinit(&worker->spi_handle);
        flipper_canbus_worker_report_error(worker, FlipperCanbusWorkerErrorStartCan, error);
        return false;
    }

    return true;
}

static int32_t flipper_canbus_worker_thread(void* arg) {
    FlipperCanbusWorker* worker = arg;
    furi_assert(worker);

    if(!flipper_canbus_worker_configure_mcp251xfd(worker)) return 0;

    furi_hal_gpio_init(&gpio_ext_pa4, GpioModeInterruptFall, GpioPullUp, GpioSpeedVeryHigh);
    furi_hal_gpio_add_int_callback(&gpio_ext_pa4, flipper_canbus_worker_gpio_isr, worker);
    furi_hal_gpio_enable_int_callback(&gpio_ext_pa4);

    while(1) {
        uint32_t events =
            furi_thread_flags_wait(FlipperCanbusWorkerEventAll, FuriFlagWaitAny, FuriWaitForever);
        if(events & FlipperCanbusWorkerEventStop) break;

        if(events & FlipperCanbusWorkerEventMsgReceived) {
            bool stop_worker = false;
            while(true) {
                FlipperCanbusWorkerPollResult result =
                    flipper_canbus_worker_poll_mcp251xfd(worker);
                if((result == FlipperCanbusWorkerPollError) ||
                   (result == FlipperCanbusWorkerPollStop)) {
                    stop_worker = true;
                    break;
                }
                if(result == FlipperCanbusWorkerPollIdle) break;
            }
            if(stop_worker) break;
        }
    }

    furi_hal_gpio_disable_int_callback(&gpio_ext_pa4);
    furi_hal_gpio_remove_int_callback(&gpio_ext_pa4);
    MCP251XFD_RequestOperationMode(&worker->mcp, MCP251XFD_CONFIGURATION_MODE, true);
    furi_hal_spi_bus_handle_deinit(&worker->spi_handle);

    return 0;
}

FlipperCanbusWorker* flipper_canbus_worker_alloc(void) {
    FlipperCanbusWorker* worker = malloc(sizeof(FlipperCanbusWorker));

    worker->can_messages_mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    worker->thread = furi_thread_alloc_ex(TAG, 4096, flipper_canbus_worker_thread, worker);
    can_msgs_dict_init(worker->can_messages);

    return worker;
}

void flipper_canbus_worker_free(FlipperCanbusWorker* worker) {
    furi_assert(worker);
    can_msgs_dict_clear(worker->can_messages);
    furi_mutex_free(worker->can_messages_mutex);
    furi_thread_free(worker->thread);
    free(worker);
}

void flipper_canbus_worker_start(FlipperCanbusWorker* worker) {
    furi_assert(worker);
    if(furi_thread_get_state(worker->thread) != FuriThreadStateStopped) return;

    flipper_canbus_worker_lock(worker);
    can_msgs_dict_reset(worker->can_messages);
    flipper_canbus_worker_unlock(worker);
    furi_thread_start(worker->thread);
}

void flipper_canbus_worker_send_stop(FlipperCanbusWorker* worker) {
    furi_assert(worker);
    if(furi_thread_get_state(worker->thread) == FuriThreadStateStopped) return;

    furi_thread_flags_set(furi_thread_get_id(worker->thread), FlipperCanbusWorkerEventStop);
}

void flipper_canbus_worker_await_stop(FlipperCanbusWorker* worker) {
    furi_assert(worker);
    if(furi_thread_get_state(worker->thread) == FuriThreadStateStopped) return;

    furi_thread_join(worker->thread);
    flipper_canbus_worker_lock(worker);
    can_msgs_dict_reset(worker->can_messages);
    flipper_canbus_worker_unlock(worker);
}

void flipper_canbus_worker_set_error_callback(
    FlipperCanbusWorker* worker,
    FlipperCanbusWorkerErrorCallback cb,
    void* ctx) {
    worker->error_callback = cb;
    worker->error_callback_context = ctx;
}

uint32_t flipper_canbus_worker_get_count(FlipperCanbusWorker* worker) {
    flipper_canbus_worker_lock(worker);
    uint32_t count = can_msgs_dict_size(worker->can_messages);
    flipper_canbus_worker_unlock(worker);
    return count;
}

static void flipper_canbus_worker_sort_frames_by_id(FlipperCanbusFrame* frames, size_t count) {
    for(size_t i = 0; i < count; i++) {
        size_t min = i;
        for(size_t j = i + 1; j < count; j++) {
            if(frames[j].id < frames[min].id) min = j;
        }

        if(min == i) continue;
        FlipperCanbusFrame tmp = frames[i];
        frames[i] = frames[min];
        frames[min] = tmp;
    }
}

size_t flipper_canbus_worker_copy_snapshot(
    FlipperCanbusWorker* worker,
    FlipperCanbusFrame* frames,
    size_t frames_count) {
    furi_check(worker);
    furi_check(frames);

    flipper_canbus_worker_lock(worker);
    size_t count = 0;
    can_msgs_dict_it_t it;
    for(can_msgs_dict_it(it, worker->can_messages); !can_msgs_dict_end_p(it);
        can_msgs_dict_next(it)) {
        if(count >= frames_count) break;

        const can_msgs_dict_itref_t* item = can_msgs_dict_cref(it);
        frames[count].id = item->key;
        frames[count].count = item->value.count;
        frames[count].last_len = item->value.last_len;
        memcpy(frames[count].last_data, item->value.last_data, item->value.last_len);
        count++;
    }
    flipper_canbus_worker_unlock(worker);

    flipper_canbus_worker_sort_frames_by_id(frames, count);

    return count;
}

bool flipper_canbus_worker_get_frame(
    FlipperCanbusWorker* worker,
    uint32_t id,
    FlipperCanbusFrame* frame) {
    furi_check(worker);
    furi_check(frame);

    bool found = false;
    flipper_canbus_worker_lock(worker);
    do {
        FlipperCanbusMsg* msg = can_msgs_dict_get(worker->can_messages, id);
        if(!msg) break;

        frame->id = id;
        frame->count = msg->count;
        frame->last_len = msg->last_len;
        memcpy(frame->last_data, msg->last_data, msg->last_len);
        found = true;
    } while(0);
    flipper_canbus_worker_unlock(worker);
    return found;
}

void flipper_canbus_worker_get_frames(
    FlipperCanbusWorker* worker,
    const uint32_t* ids,
    FlipperCanbusFrame* frames,
    bool* found,
    size_t count) {
    furi_check(worker);
    furi_check(ids);
    furi_check(frames);
    furi_check(found);

    flipper_canbus_worker_lock(worker);
    for(size_t i = 0; i < count; i++) {
        FlipperCanbusMsg* msg = can_msgs_dict_get(worker->can_messages, ids[i]);
        if(msg) {
            frames[i].id = ids[i];
            frames[i].count = msg->count;
            frames[i].last_len = msg->last_len;
            memcpy(frames[i].last_data, msg->last_data, msg->last_len);
            found[i] = true;
        } else {
            found[i] = false;
        }
    }
    flipper_canbus_worker_unlock(worker);
}
