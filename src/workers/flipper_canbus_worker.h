#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FLIPPER_CANBUS_MAX_IDS        256U
#define FLIPPER_CANBUS_MAX_FRAME_SIZE 8U

typedef struct FlipperCanbusWorker FlipperCanbusWorker;

typedef enum {
    FlipperCanbusWorkerErrorGetFifoStatus,
    FlipperCanbusWorkerErrorReceiveMessage,
    FlipperCanbusWorkerErrorMcpInit,
    FlipperCanbusWorkerErrorFifoConfig,
    FlipperCanbusWorkerErrorStandardFilterConfig,
    FlipperCanbusWorkerErrorExtendedFilterConfig,
    FlipperCanbusWorkerErrorStartCan,
    FlipperCanbusWorkerErrorCount,
} FlipperCanbusWorkerErrorResult;

typedef void (*FlipperCanbusWorkerErrorCallback)(
    void* context,
    FlipperCanbusWorkerErrorResult error,
    const char* driver_error);

typedef struct {
    uint32_t id;
    uint32_t count;
    uint32_t last_len;
    uint8_t last_data[FLIPPER_CANBUS_MAX_FRAME_SIZE];
} FlipperCanbusFrame;

FlipperCanbusWorker* flipper_canbus_worker_alloc(void);
void flipper_canbus_worker_free(FlipperCanbusWorker* worker);

void flipper_canbus_worker_start(FlipperCanbusWorker* worker);
void flipper_canbus_worker_send_stop(FlipperCanbusWorker* worker);
void flipper_canbus_worker_await_stop(FlipperCanbusWorker* worker);

void flipper_canbus_worker_set_error_callback(
    FlipperCanbusWorker* worker,
    FlipperCanbusWorkerErrorCallback cb,
    void* ctx);

uint32_t flipper_canbus_worker_get_count(FlipperCanbusWorker* worker);
size_t flipper_canbus_worker_copy_snapshot(
    FlipperCanbusWorker* worker,
    FlipperCanbusFrame* frames,
    size_t frames_count);
bool flipper_canbus_worker_get_frame(
    FlipperCanbusWorker* worker,
    uint32_t id,
    FlipperCanbusFrame* frame);
void flipper_canbus_worker_get_frames(
    FlipperCanbusWorker* worker,
    const uint32_t* ids,
    FlipperCanbusFrame* frames,
    bool* found,
    size_t count);
