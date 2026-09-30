#include "shinobi.h"

#ifndef _TASKS_H_
#define _TASKS_H_

// Probably should be moved to another header...
struct QueuedDat {
    char *basedir;
    char *filename;
    void **dest;
    int loaded_0x0c;
}
typedef QueuedDat;

typedef void (*TaskAction)(struct Task *task, void *state);

/* Only action/state and queuedItem_0x18 mean anything to the dispatcher in
 * 014a9c_tasks.c. field_0x08/0x0c are overlaid by each concrete task type
 * with its own meaning (phase_0x08, gdfs_0x0c in the asset-queue tasks);
 * field_0x10/0x14/0x1c are unused by every task type surveyed so far, so a
 * task struct that leaves them out is not missing anything. */
struct Task {
    TaskAction action;
    void *state;
    int field_0x08;
    void* field_0x0c;
    int field_0x10;
    int field_0x14;
    void* queuedItem_0x18;
    int field_0x1c;
}
typedef Task;

/**
 * @todo Should action be typed?
 */
int TaskSpawn_8c014ae8(Task *tasks, void *action, Task **created_task, void **create_state, size_t alloc_size);
void TaskKill_8c014b66(Task *task);
void TaskInitGroup_8c014a9c(Task *tasks, Sint32 count);
void TaskKillGroup_8c014ab4(Task *tasks);
void TaskSwitch_8c014b3e(Task *task, TaskAction action);
void TaskRunGroup_8c014b42(Task task[]);

#endif /* _TASKS_H_ */
