// thread.c - 图像处理线程实现

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "thread.h"
#include <opencv2/imgproc/imgproc_c.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static FrameProcessor g_proc = {0};

// 初始化处理器
FrameProcessor* create_processor() {
    pthread_mutex_lock(&mutex);
    if (g_proc.frame == NULL) {
        // 初始化结构
        g_proc.frame = NULL;
        g_proc.processed_frame = NULL;
        g_proc.frame_ready = 0;
        g_proc.should_exit = 0;
    }
    pthread_mutex_unlock(&mutex);
    return &g_proc;
}

// 销毁资源
void destroy_processor(FrameProcessor *proc) {
    pthread_mutex_lock(&mutex);
    if (proc->frame) {
        cvReleaseImage(&proc->frame);
        proc->frame = NULL;
    }
    if (proc->processed_frame) {
        cvReleaseImage(&proc->processed_frame);
        proc->processed_frame = NULL;
    }
    pthread_mutex_unlock(&mutex);
}

// 处理线程主函数
void* start_processing_thread(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        if (g_proc.should_exit && !g_proc.frame_ready) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        if (g_proc.frame_ready && g_proc.frame) {
            CvSize size = cvGetSize(g_proc.frame);

            // 首次创建 processed_frame
            if (!g_proc.processed_frame) {
                g_proc.processed_frame = cvCreateImage(size, IPL_DEPTH_8U, 1);
            }

            cvCvtColor(g_proc.frame, g_proc.processed_frame, CV_BGR2GRAY);
            g_proc.frame_ready = 0;
        }

        pthread_mutex_unlock(&mutex);
        usleep(1000); // 小休避免忙等
    }
    printf("图像处理线程已退出。\n");
    return NULL;
}

// 获取处理后的帧（供主线程使用）
int get_processed_frame(FrameProcessor *proc, IplImage **out) {
    pthread_mutex_lock(&mutex);
    if (proc->processed_frame) {
        *out = cvCloneImage(proc->processed_frame);
        pthread_mutex_unlock(&mutex);
        return 1;
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}
