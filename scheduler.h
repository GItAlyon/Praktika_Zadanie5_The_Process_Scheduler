#ifndef SCHEDULER_H
#define SCHEDULER_H
#define _CRT_SECURE_NO_WARNINGS

// Подключаем стандартные библиотеки
#include <stdio.h>      // Для ввода/вывода
#include <stdlib.h>     // Для malloc, free, realloc
#include <string.h>     // Для strcpy, strlen
#include <time.h>       // Для работы со временем
#include <stdbool.h>    // Для типа bool (true/false)

// Определения для Windows
#ifdef _WIN32
#include <windows.h>        // Для Windows API
#include <tlhelp32.h>       // Для работы с процессами Windows
#define sleep_ms(ms) Sleep(ms)  // Функция задержки для Windows
#else
#include <unistd.h>         // Для Linux/POSIX
#define sleep_ms(ms) usleep((ms) * 1000)  // Функция задержки для Linux
#endif

// Структура процесса - хранит всю информацию о процессе
typedef struct Process {
    int pid;                    // Уникальный идентификатор процесса
    char name[20];              // Имя процесса
    int arrival_time;           // Время появления процесса в системе
    int burst_time;             // Полное время выполнения процесса
    int remaining_time;         // Оставшееся время выполнения
    int priority;               // Приоритет (1 - высокий, 3 - низкий)
    int waiting_time;           // Время ожидания в очереди
    int turnaround_time;        // Полное время от появления до завершения
    int response_time;          // Время от появления до первого выполнения
    int start_time;             // Время начала первого выполнения
    int completion_time;        // Время завершения процесса
    bool completed;             // Флаг завершения процесса
    bool started;               // Флаг начала выполнения
    int queue_level;            // Уровень очереди (для многоуровневых очередей)
    struct Process* next;       // Указатель на следующий процесс в очереди
} Process;

// Структура очереди процессов (связанный список)
typedef struct Queue {
    Process* head;              // Указатель на первый элемент очереди
    Process* tail;              // Указатель на последний элемент очереди
    int size;                   // Количество элементов в очереди
} Queue;

// Перечисление типов алгоритмов планирования
typedef enum {
    PRIORITY_QUEUE,     // Приоритетное планирование с очередями
    ROUND_ROBIN,        // Циклическое планирование (Round Robin)
    SJF                 // Shortest Job First (самый короткий процесс)
} SchedulerType;

// Структура планировщика - основная структура управления
typedef struct Scheduler {
    SchedulerType type;                 // Тип планировщика
    Queue* ready_queue;                 // Общая очередь готовности
    Queue* high_priority_queue;         // Очередь высокого приоритета
    Queue* medium_priority_queue;       // Очередь среднего приоритета
    Queue* low_priority_queue;          // Очередь низкого приоритета
    int time_quantum;                   // Квант времени для RR
    int current_time;                   // Текущее время выполнения
    Process* current_process;           // Текущий выполняемый процесс
    Process** process_list;             // Массив указателей на все процессы
    int process_count;                  // Количество процессов
    int context_switches;               // Количество переключений контекста
    int total_waiting_time;             // Суммарное время ожидания
    int total_turnaround_time;          // Суммарное время выполнения
    int total_response_time;            // Суммарное время ответа
    int* gantt_chart_pids;              // Массив PID для диаграммы Ганта
    int* gantt_chart_times;             // Массив времен для диаграммы Ганта
    int gantt_size;                     // Текущий размер диаграммы
    int gantt_capacity;                 // Вместимость массивов диаграммы
} Scheduler;

// Функции для работы с очередями
Queue* create_queue();                  // Создание новой очереди
void enqueue(Queue* q, Process* p);     // Добавление процесса в очередь
Process* dequeue(Queue* q);             // Извлечение процесса из очереди
Process* peek(Queue* q);                // Просмотр первого процесса без извлечения
bool is_empty(Queue* q);                // Проверка очереди на пустоту
void free_queue(Queue* q);              // Освобождение памяти очереди

// Функции для работы с процессами
Process* create_process(int pid, const char* name, int arrival, int burst, int priority);
void print_process(Process* p);         // Вывод информации о процессе

// Основные функции планировщиков
Scheduler* create_scheduler(SchedulerType type, int time_quantum);  // Создание планировщика
void add_process(Scheduler* s, Process* p);     // Добавление процесса в планировщик
void schedule(Scheduler* s);                    // Запуск планирования
void print_gantt_chart(Scheduler* s);           // Вывод диаграммы Ганта
void print_statistics(Scheduler* s);            // Вывод статистики
void free_scheduler(Scheduler* s);              // Освобождение памяти планировщика

// Вспомогательные функции
void add_to_gantt(Scheduler* s, int pid, int time);     // Добавление в диаграмму Ганта
void reset_processes(Process** processes, int count);   // Сброс состояния процессов

#endif // SCHEDULER_H