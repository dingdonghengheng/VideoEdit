// main.c - 主程序入口

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "thread.h"
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/videoio/videoio_c.h>

int main(int argc, char *argv[]) {
    const char *video_path = "input.mp4"; // 修改为你自己的路径
    CvCapture *capture;
    pthread_t processor_thread;
    FrameProcessor *proc;

    // 打开视频
    capture = cvCreateFileCapture(video_path);
    if (!capture) {
        fprintf(stderr, "错误：无法打开视频文件 '%s'\n", video_path);
        return -1;
    }

    // 创建处理器
    proc = create_processor();
    if (!proc) {
        fprintf(stderr, "无法创建处理器。\n");
        cvReleaseCapture(&capture);
        return -1;
    }

    // 创建处理线程
    if (pthread_create(&processor_thread, NULL, start_processing_thread, NULL)) {
        fprintf(stderr, "无法创建处理线程。\n");
        goto cleanup;
    }

    // 创建显示窗口
    cvNamedWindow("原始视频", CV_WINDOW_AUTOSIZE);
    cvNamedWindow("处理后视频", CV_WINDOW_AUTOSIZE);

    printf("开始播放视频，按 'q' 键退出...\n");

    IplImage *display_frame = NULL;

    while (1) {
        IplImage *frame = cvQueryFrame(capture);
        if (!frame) {
            printf("视频播放完毕或读取失败。\n");
            break;
        }

        // 显示原始帧
        cvShowImage("原始视频", frame);

        // 写入共享帧（线程安全）
        pthread_mutex_lock(&mutex);
        if (!proc->frame) {
            proc->frame = cvCloneImage(frame);
        } else {
            cvCopy(frame, proc->frame, NULL);
        }
        proc->frame_ready = 1;
        pthread_mutex_unlock(&mutex);

        // 获取处理结果并显示
        if (get_processed_frame(proc, &display_frame)) {
            cvShowImage("处理后视频", display_frame);
            cvReleaseImage(&display_frame);
            display_frame = NULL;
        }

        // 检查按键
        if (cvWaitKey(30) == 'q') {
            pthread_mutex_lock(&mutex);
            proc->should_exit = 1;
            pthread_mutex_unlock(&mutex);
            break;
        }
    }

    // 等待线程结束
    pthread_join(processor_thread, NULL);

cleanup:
    destroy_processor(proc);
    cvReleaseCapture(&capture);
    cvDestroyAllWindows();
    printf("程序已退出。\n");
    return 0;
}
