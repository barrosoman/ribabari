#include <curses.h>
#include <pthread.h>
#include <time.h>

#include "cpu.h"
#include "../terminal/terminal.h"

/**
 * A pointer to the kernel structure.
 */
kernel_t* kernel;

/* CPU Function Definition */

/**
 * It creates the CPU-Thread that
 * is going to run the CPU function
 * that is going to simulate the CPU
 * execution along the simulation.
 */
void cpu_init() {
    pthread_t cpu_id;
    pthread_attr_t cpu_attr;

    pthread_attr_init(&cpu_attr);
    pthread_attr_setscope(&cpu_attr, PTHREAD_SCOPE_SYSTEM);

    pthread_create(&cpu_id, NULL, (void *) cpu, NULL);
}

/**
 * It represents the function that
 * is going to simulate the CPU work
 * by executing instructions that are
 * selected by the scheduler.
 */
_Noreturn void cpu() {
    /* It waits the kernel to be initialized */
    while (!kernel);

    struct timespec start;
    struct timespec end;

    clock_gettime(CLOCK_REALTIME, &start);

    int no_process = 0;
    /* int i = 0; */
    while (1) {
        /* It checks if there is no scheduled proc */
        if (!kernel->scheduler.scheduled_proc) {
            if (!no_process) {
                list_add(log_list, (void*) "No process, is running, then some will be scheduled.\n");
                /* mvprintw(i % 4, 0, "No process is running, then some will be scheduled.\n"); */
                refresh();
                no_process = 1;
                /* i++; */
            }

            /* Schedule the first process */
            schedule_process(&kernel->scheduler, NONE);
        }
        /* There is some process running */
        else {
            no_process = 0;
            do {
                clock_gettime(CLOCK_REALTIME, &end);
                const int elapsed = (end.tv_sec - start.tv_sec) * 1000000000L
                                     + (end.tv_nsec - start.tv_nsec);

                if (elapsed >= 1000000000L) {
                    start = end;

                    /* It calculates the physical memory address from the virtual memory address */
                    const int pc = FETCH_INSTR_ADDR(kernel->scheduler.scheduled_proc);
                    const int page_number = PAGE_NUMBER(pc);
                    const int page_offset = PAGE_OFFSET(pc);

                    /* Fetch the instruction from the page stored in the memory */
                    segment_t* seg = segment_find(&kernel->seg_table, kernel->scheduler.scheduled_proc->seg_id);
                    instr_t instr = seg->page_table[page_number].code[page_offset];

                    char* buffer = (char*) malloc(50);
                    sprintf(buffer,
                            "Process %s, remaining: %d, pc: %d, sid: %d.\n",
                           kernel->scheduler.scheduled_proc->name,
                           (kernel->scheduler.scheduled_proc->remaining),
                           pc,
                           seg->id);
                    list_add(log_list, (void*) buffer);

                    /* move(i % 4, 0); */
                    /* clrtoeol(); */
                    /* printw(buffer); */
                    /* refresh(); */
                    /* i++; */

                    /* Evaluate the current instruction to be executed by the process */
                    eval(kernel->scheduler.scheduled_proc, &instr);
                }
            } while (
                kernel->scheduler.scheduled_proc != NULL &&
                kernel->scheduler.scheduled_proc->remaining > 0 &&
                kernel->scheduler.scheduled_proc->pc < kernel->scheduler.scheduled_proc->code_len);

            if (kernel->scheduler.scheduled_proc == NULL)
                continue;

            /* The process has finished its execution, then it must be terminated */
            if (kernel->scheduler.scheduled_proc->pc >= kernel->scheduler.scheduled_proc->code_len)
                sysCall(PROCESS_FINISH, kernel->scheduler.scheduled_proc);
            else
                /* Interrupt the current process to schedule another one, since the latter */
                /* has completed its quantum time (or time slice) */
                sysCall(PROCESS_INTERRUPT, (void *) QUANTUM_COMPLETED);
        }
    }
}

