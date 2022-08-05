#ifndef OS_PROJECT_TERMINAL_H
#define OS_PROJECT_TERMINAL_H
#include "../memory/memory.h"
#include "../utils/list.h"
#include <curses.h>
#include <menu.h>
#include <stdlib.h>
#include <string.h>

#define BOX_SIZE 2
static const int EXIT = -1;
static const long SECOND_IN_US = 1000000L;
extern list_t* process_log_list;
extern list_t* disk_log_list;
extern list_t* io_log_list;
extern sem_t log_mutex;
extern sem_t mem_mutex;
extern sem_t disk_mutex;
extern sem_t refresh_sem;
extern sem_t io_mutex;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define ENTER_KEY 10
#define BOX_OFFSET 2
#define TITLE_OFFSET 2


typedef struct {
    int is_proc;

    char* name;
    int remaining;
    int id;
    int pc;
} proc_log_info_t;

typedef struct {
    char* proc_name;
    int proc_id;
    int duration;
} io_log_info_t;

typedef struct {
    char *proc_name;
    int track;
    int is_read;
    int proc_id;
} disk_log_info_t;


typedef enum main_menu_choices {
    CREATE,
} main_menu_choices;

typedef struct menu_window_t {
    WINDOW* main_win;
    WINDOW* title_win;
    WINDOW* items_win;
    int height;
    int width;
    int begin_y;
    int begin_x;
} menu_window_t;

typedef struct menu_choice_t {
    const char* text;
    int value;
} menu_choice_t;

typedef struct menu_t {
    menu_choice_t* choices;
    int choices_count;
    const char* title;
    int height;
    int width;
    MENU* curses_menu;
    ITEM** items;
    menu_window_t* menu_window;
} menu_t;

typedef struct {
    int begin_y;
    int begin_x;
} coordinates_t;

typedef struct {
    WINDOW* main_window;
    WINDOW* text_window;
    WINDOW* title_window;
} io_window_t;

typedef struct {
    WINDOW* main_window;
    WINDOW* text_window;
    WINDOW* title_window;
} log_window_t;

#define MAIN_MENU(M)                                                           \
    M("Process create", CREATE)                                                \
    M("Exit", EXIT)

void print_item_index(menu_t* menu, const ITEM* item);
void menu_loop(menu_t* main_menu, void (*func)(int));

void init_screen();
void end_screen(void);

void start_menu_and_loop(menu_t* menu, void (*func)(int));
void print_welcome();
MENU* make_curses_menu(menu_t* menu);
menu_t* create_menu(int choices_count,
                    menu_choice_t choices[static choices_count],
                    const char* title);
const char* get_string_from_type(menu_t* menu, int type);
int get_type_from_string(menu_t* menu, const char* buffer);
int get_biggest_item(menu_choice_t choices[], int choice_count,
                     const char* title);

menu_window_t* init_menu_window(const menu_t* menu);
int begin_terminal();
void delete_menu(menu_t* menu);
void free_menu_window(menu_window_t* menu_window);

char* get_input_from_window(char* title, coordinates_t coordinates,
                            int buffer_size);
void print_with_window(char* string, char* title, int y, int x);

void process_log_init();
void disk_log_init();
void io_log_init();

void* refresh_process_log(void* _);
void* refresh_memory_log(void* _);
void* refresh_disk_log(void* _);
void* refresh_io_log(void* _);


log_window_t* init_io_log();
log_window_t* init_process_log();
log_window_t* init_memory_log();
log_window_t* init_disk_log();

/* Internal Terminal Function Prototypes */

/**
 * It returns a pointer to a process that has the
 * specified segment id. Otherwise, if there is not
 * exists such a process, then NULL is returned.
 *
 * @param sid the segment id
 *
 * @return a pointer to a process that has the
 *         specified segment id; otherwise, if
 *         there is not exists such a process,
 *         then NULL is returned.
 */
static process_t* get_process_sid(const int sid);

/**
 * It calculates the page in the segment that
 * is in use by the process. The in use is in
 * the meaning that the process is executing
 * an instruction in that page currently.
 *
 * @param segment the segment
 *
 * @return the page index (a non-negative value);
 *         otherwise, returns -1 indicating that
 *         no page in that segment is being used.
 */
static int page_inuse_index(const segment_t* segment);

#endif // OS_PROJECT_TERMINAL_H

