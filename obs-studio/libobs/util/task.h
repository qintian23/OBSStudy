#pragma once

#include "c99defs.h"

#ifdef __cplusplus
extern "C" {
#endif

struct os_task_queue;
typedef struct os_task_queue os_task_queue_t;

typedef void (*os_task_t)(void *param);

/// <summary>
/// 创建一个任务队列，开启处理任务线程
/// </summary>
/// <returns></returns>
EXPORT os_task_queue_t *os_task_queue_create();
/// <summary>
/// 向任务队列中加入一个任务
/// </summary>
/// <param name="tt"></param>
/// <param name="task"></param>
/// <param name="param"></param>
/// <returns></returns>
EXPORT bool os_task_queue_queue_task(os_task_queue_t *tt, os_task_t task,
				     void *param);
/// <summary>
/// 终止任务线程，释放相关资源
/// </summary>
/// <param name="tt"></param>
EXPORT void os_task_queue_destroy(os_task_queue_t *tt);
/// <summary>
/// 等待线程处理完现有的任务
/// </summary>
/// <param name="tt"></param>
/// <returns>是否处理完</returns>
EXPORT bool os_task_queue_wait(os_task_queue_t *tt);
/// <summary>
/// 判断任务线程是否启动
/// </summary>
/// <param name="tt"></param>
/// <returns></returns>
EXPORT bool os_task_queue_inside(os_task_queue_t *tt);

#ifdef __cplusplus
}
#endif
