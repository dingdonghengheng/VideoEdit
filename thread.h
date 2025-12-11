// thread.h - 线程处理模块接口

#ifndef THREAD_H
#define THREAD_H

#include <opencv2/core/core_c.h>

// 共享数据结构
typedef struct {
    IplImage *frame;           // 原始帧
    IplImage *processed_frame; // 处理后帧
    int frame_ready;           // 是否有待处理帧
    int should_exit;           // 是否应退出线程
} FrameProcessor;

// 函数声明
FrameProcessor* create_processor();
void destroy_processor(FrameProcessor *proc);
void* start_processing_thread(void *arg);
int get_processed_frame(FrameProcessor *proc, IplImage **out);

#endif
