#include "scheduler.h"

// Создание новой пустой очереди
Queue* create_queue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));  // Выделяем память под очередь
    q->head = NULL;     // В начале очередь пуста
    q->tail = NULL;     // Указатель на последний элемент отсутствует
    q->size = 0;        // Размер очереди равен 0
    return q;
}

// Добавление процесса в конец очереди
void enqueue(Queue* q, Process* p) {
    p->next = NULL;     // Новый процесс будет последним, его next = NULL

    if (q->tail == NULL) {          // Если очередь пуста
        q->head = p;                // Новый процесс становится головой
        q->tail = p;                // И хвостом одновременно
    }
    else {
        q->tail->next = p;          // Присоединяем новый процесс к хвосту
        q->tail = p;                // Обновляем указатель на хвост
    }
    q->size++;          // Увеличиваем размер очереди
}

// Извлечение процесса из начала очереди
Process* dequeue(Queue* q) {
    if (q->head == NULL) return NULL;   // Если очередь пуста, возвращаем NULL

    Process* p = q->head;               // Запоминаем первый процесс
    q->head = q->head->next;            // Головой становится следующий процесс

    if (q->head == NULL) {              // Если очередь опустела
        q->tail = NULL;                 // Обнуляем указатель на хвост
    }

    p->next = NULL;     // Отсоединяем извлеченный процесс от очереди
    q->size--;          // Уменьшаем размер очереди
    return p;           // Возвращаем извлеченный процесс
}

// Просмотр первого процесса в очереди без извлечения
Process* peek(Queue* q) {
    return q->head;     // Возвращаем голову очереди
}

// Проверка очереди на пустоту
bool is_empty(Queue* q) {
    return q->head == NULL;     // Если голова NULL, очередь пуста
}

// Освобождение памяти очереди и всех процессов в ней
void free_queue(Queue* q) {
    while (!is_empty(q)) {      // Пока очередь не пуста
        Process* p = dequeue(q); // Извлекаем процесс
        free(p);                 // Освобождаем память процесса
    }
    free(q);        // Освобождаем память самой очереди
}

// Создание нового процесса с заданными параметрами
Process* create_process(int pid, const char* name, int arrival, int burst, int priority) {
    Process* p = (Process*)malloc(sizeof(Process));  // Выделяем память под процесс

    p->pid = pid;                   // Устанавливаем идентификатор
    strcpy(p->name, name);          // Копируем имя процесса
    p->arrival_time = arrival;      // Время появления
    p->burst_time = burst;          // Полное время выполнения
    p->remaining_time = burst;      // Оставшееся время = полному (пока не выполнялся)
    p->priority = priority;         // Приоритет
    p->waiting_time = 0;            // Время ожидания пока 0
    p->turnaround_time = 0;         // Время выполнения пока 0
    p->response_time = 0;           // Время ответа пока 0
    p->start_time = -1;             // -1 означает, что процесс еще не запускался
    p->completion_time = 0;         // Время завершения пока не определено
    p->completed = false;           // Процесс не завершен
    p->started = false;             // Процесс не запускался
    p->queue_level = 0;             // Уровень очереди по умолчанию 0
    p->next = NULL;                 // В начале процесса нет следующего

    return p;       // Возвращаем указатель на созданный процесс
}

// Вывод информации о процессе в консоль
void print_process(Process* p) {
    printf("PID: %d, Name: %s, Arrival: %d, Burst: %d, Priority: %d\n",
        p->pid, p->name, p->arrival_time, p->burst_time, p->priority);
}

// Создание планировщика определенного типа
Scheduler* create_scheduler(SchedulerType type, int time_quantum) {
    Scheduler* s = (Scheduler*)malloc(sizeof(Scheduler));  // Выделяем память

    s->type = type;                 // Устанавливаем тип планировщика
    s->time_quantum = time_quantum; // Квант времени (для RR)
    s->current_time = 0;            // Текущее время = 0
    s->current_process = NULL;      // Нет выполняемого процесса
    s->process_list = NULL;         // Массив процессов пуст
    s->process_count = 0;           // Количество процессов = 0
    s->context_switches = 0;        // Количество переключений = 0
    s->total_waiting_time = 0;      // Суммарное ожидание = 0
    s->total_turnaround_time = 0;   // Суммарное выполнение = 0
    s->total_response_time = 0;     // Суммарное время ответа = 0
    s->gantt_chart_pids = NULL;     // Диаграмма Ганта пуста
    s->gantt_chart_times = NULL;    // Времена для диаграммы пусты
    s->gantt_size = 0;              // Размер диаграммы = 0
    s->gantt_capacity = 0;          // Вместимость = 0

    s->ready_queue = create_queue();    // Создаем общую очередь готовности

    // Для приоритетного планирования создаем три очереди
    if (type == PRIORITY_QUEUE) {
        s->high_priority_queue = create_queue();    // Высокий приоритет
        s->medium_priority_queue = create_queue();  // Средний приоритет
        s->low_priority_queue = create_queue();     // Низкий приоритет
    }
    else {
        // Для других алгоритмов очереди не нужны
        s->high_priority_queue = NULL;
        s->medium_priority_queue = NULL;
        s->low_priority_queue = NULL;
    }

    return s;   // Возвращаем созданный планировщик
}

// Добавление процесса в список планировщика
void add_process(Scheduler* s, Process* p) {
    s->process_count++;     // Увеличиваем счетчик процессов
    // Перевыделяем память для массива процессов
    s->process_list = (Process**)realloc(s->process_list,
        sizeof(Process*) * s->process_count);
    // Добавляем процесс в конец массива
    s->process_list[s->process_count - 1] = p;
}

// Добавление записи в диаграмму Ганта
void add_to_gantt(Scheduler* s, int pid, int time) {
    // Если массив заполнен, увеличиваем его размер
    if (s->gantt_size >= s->gantt_capacity) {
        s->gantt_capacity = s->gantt_capacity == 0 ? 10 : s->gantt_capacity * 2;
        s->gantt_chart_pids = (int*)realloc(s->gantt_chart_pids,
            sizeof(int) * s->gantt_capacity);
        s->gantt_chart_times = (int*)realloc(s->gantt_chart_times,
            sizeof(int) * s->gantt_capacity);
    }
    // Добавляем PID и время в диаграмму
    s->gantt_chart_pids[s->gantt_size] = pid;
    s->gantt_chart_times[s->gantt_size] = time;
    s->gantt_size++;    // Увеличиваем размер диаграммы
}

// Сброс состояния процессов для повторного запуска
void reset_processes(Process** processes, int count) {
    for (int i = 0; i < count; i++) {
        processes[i]->remaining_time = processes[i]->burst_time;   // Сбрасываем время
        processes[i]->waiting_time = 0;         // Обнуляем ожидание
        processes[i]->turnaround_time = 0;      // Обнуляем время выполнения
        processes[i]->response_time = 0;        // Обнуляем время ответа
        processes[i]->start_time = -1;          // Сбрасываем время старта
        processes[i]->completion_time = 0;      // Сбрасываем время завершения
        processes[i]->completed = false;        // Сбрасываем флаг завершения
        processes[i]->started = false;          // Сбрасываем флаг запуска
    }
}

// Функция сравнения для сортировки процессов по времени выполнения (SJF)
int compare_burst(const void* a, const void* b) {
    Process* p1 = *(Process**)a;
    Process* p2 = *(Process**)b;
    if (p1->burst_time != p2->burst_time)
        return p1->burst_time - p2->burst_time;    // Сортируем по времени выполнения
    return p1->arrival_time - p2->arrival_time;    // При равенстве - по времени прибытия
}

// Планировщик Round Robin (вытесняющий)
void schedule_rr(Scheduler* s) {
    Process** processes = s->process_list;    // Получаем список процессов
    int count = s->process_count;             // Количество процессов
    int time = 0;                             // Текущее время
    int completed = 0;                        // Количество завершенных процессов

    reset_processes(processes, count);        // Сбрасываем состояние процессов

    // Сортируем процессы по времени прибытия (пузырьковая сортировка)
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (processes[i]->arrival_time > processes[j]->arrival_time) {
                Process* temp = processes[i];
                processes[i] = processes[j];
                processes[j] = temp;
            }
        }
    }

    Queue* ready = create_queue();          // Создаем очередь готовности
    int idx = 0;                            // Индекс следующего прибывающего процесса
    Process* current = NULL;                // Текущий выполняемый процесс
    int remaining_quantum = s->time_quantum; // Оставшееся время кванта

    s->current_time = 0;                    // Сбрасываем время планировщика
    s->context_switches = 0;                // Сбрасываем счетчик переключений
    s->gantt_size = 0;                      // Сбрасываем диаграмму Ганта

    printf("\n    Циклическое планирование (Round Robin) (Квант = %d) \n", s->time_quantum);

    // Основной цикл планирования
    while (completed < count) {
        // Добавляем новые процессы в очередь готовности
        while (idx < count && processes[idx]->arrival_time <= time) {
            enqueue(ready, processes[idx]);
            printf("t=%d: Процесс %d (%s) прибыл\n", time, processes[idx]->pid, processes[idx]->name);
            idx++;
        }

        // Обработка текущего процесса
        if (current != NULL) {
            if (current->remaining_time == 0) {
                // Процесс завершился
                current->completion_time = time;
                current->turnaround_time = time - current->arrival_time;
                current->waiting_time = current->turnaround_time - current->burst_time;
                current->completed = true;
                completed++;
                s->total_waiting_time += current->waiting_time;
                s->total_turnaround_time += current->turnaround_time;
                s->total_response_time += current->response_time;

                printf("t=%d: Процесс %d (%s) завершён (Время выполнения=%d, Ожидание=%d)\n",
                    time, current->pid, current->name,
                    current->turnaround_time, current->waiting_time);

                current = NULL;
                remaining_quantum = s->time_quantum;

                // Добавляем разделитель в диаграмму Ганта
                if (s->gantt_size == 0 || s->gantt_chart_pids[s->gantt_size - 1] != -1) {
                    add_to_gantt(s, -1, time);
                }
                continue;
            }
            else if (remaining_quantum == 0) {
                // Квант времени истек - вытесняем процесс
                if (current->remaining_time > 0) {
                    enqueue(ready, current);    // Возвращаем процесс в очередь
                    printf("t=%d: Процесс %d (%s) вытеснен (осталось=%d)\n",
                        time, current->pid, current->name, current->remaining_time);
                    s->context_switches++;
                }
                current = NULL;
                remaining_quantum = s->time_quantum;

                // Добавляем разделитель в диаграмму Ганта
                if (s->gantt_size == 0 || s->gantt_chart_pids[s->gantt_size - 1] != -1) {
                    add_to_gantt(s, -1, time);
                }
            }
        }

        // Если нет текущего процесса, берем следующий из очереди
        if (current == NULL && !is_empty(ready)) {
            current = dequeue(ready);
            if (!current->started) {
                current->started = true;
                current->start_time = time;
                current->response_time = time - current->arrival_time;
            }
            remaining_quantum = s->time_quantum;
            printf("t=%d: Процесс %d (%s) начал выполнение\n",
                time, current->pid, current->name);
            add_to_gantt(s, current->pid, time);
        }

        // Выполняем процесс (1 единицу времени)
        if (current != NULL) {
            current->remaining_time--;
            remaining_quantum--;
            time++;
            s->current_time = time;
        }
        else {
            // Простой процессора (нет процессов для выполнения)
            if (idx < count) {
                time++;
                s->current_time = time;
                if (s->gantt_size == 0 || s->gantt_chart_pids[s->gantt_size - 1] != 0) {
                    add_to_gantt(s, 0, time - 1);    // IDLE = 0
                }
            }
            else {
                // Все процессы добавлены, но очередь пуста - завершаем
                break;
            }
        }

        // Проверяем новые процессы, прибывшие во время выполнения
        while (idx < count && processes[idx]->arrival_time <= time) {
            if (current != NULL && current->pid != processes[idx]->pid) {
                enqueue(ready, processes[idx]);
            }
            printf("t=%d: Процесс %d (%s) прибыл\n", time, processes[idx]->pid, processes[idx]->name);
            idx++;
        }
    }

    // Добавляем финальное время в диаграмму Ганта
    add_to_gantt(s, -1, time);
    s->current_time = time;

    free_queue(ready);  // Освобождаем очередь готовности
}

// Планировщик с приоритетными очередями (невытесняющий)
void schedule_priority_queue(Scheduler* s) {
    Process** processes = s->process_list;
    int count = s->process_count;
    int time = 0;
    int completed = 0;

    reset_processes(processes, count);  // Сбрасываем состояние процессов

    // Сортируем по времени прибытия
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (processes[i]->arrival_time > processes[j]->arrival_time) {
                Process* temp = processes[i];
                processes[i] = processes[j];
                processes[j] = temp;
            }
        }
    }

    // Создаем три очереди для разных приоритетов
    s->high_priority_queue = create_queue();
    s->medium_priority_queue = create_queue();
    s->low_priority_queue = create_queue();

    int idx = 0;
    s->current_time = 0;
    s->context_switches = 0;
    s->gantt_size = 0;

    printf("\n    Приоритетное планирование \n");
    printf("Уровни приоритета: 1 (Наивысший), 2 (Средний), 3 (Низкий)\n");

    while (completed < count) {
        // Добавляем новые процессы в соответствующие очереди по приоритету
        while (idx < count && processes[idx]->arrival_time <= time) {
            Process* p = processes[idx];
            if (p->priority == 1) {
                enqueue(s->high_priority_queue, p);
                printf("t=%d: Процесс %d (%s) добавлен в очередь ВЫСОКОГО приоритета\n",
                    time, p->pid, p->name);
            }
            else if (p->priority == 2) {
                enqueue(s->medium_priority_queue, p);
                printf("t=%d: Процесс %d (%s) добавлен в очередь СРЕДНЕГО приоритета\n",
                    time, p->pid, p->name);
            }
            else {
                enqueue(s->low_priority_queue, p);
                printf("t=%d: Процесс %d (%s) добавлен в очередь НИЗКОГО приоритета\n",
                    time, p->pid, p->name);
            }
            idx++;
        }

        // Выбираем процесс с наивысшим приоритетом (1 > 2 > 3)
        Process* current = NULL;
        Queue* selected_queue = NULL;
        char* queue_name = "";

        if (!is_empty(s->high_priority_queue)) {
            current = peek(s->high_priority_queue);
            selected_queue = s->high_priority_queue;
            queue_name = "HIGH";
        }
        else if (!is_empty(s->medium_priority_queue)) {
            current = peek(s->medium_priority_queue);
            selected_queue = s->medium_priority_queue;
            queue_name = "MEDIUM";
        }
        else if (!is_empty(s->low_priority_queue)) {
            current = peek(s->low_priority_queue);
            selected_queue = s->low_priority_queue;
            queue_name = "LOW";
        }

        if (current == NULL) {
            // Простой процессора
            if (idx < count) {
                time++;
                s->current_time = time;
                if (s->gantt_size == 0 || s->gantt_chart_pids[s->gantt_size - 1] != 0) {
                    add_to_gantt(s, 0, time - 1);
                }
                continue;
            }
            else {
                break;
            }
        }

        // Запускаем процесс (невытесняющий - выполняется до завершения)
        current = dequeue(selected_queue);

        if (!current->started) {
            current->started = true;
            current->start_time = time;
            current->response_time = time - current->arrival_time;
        }

        printf("t=%d: Процесс %d (%s) начал выполнение (%s приоритет)\n",
            time, current->pid, current->name, queue_name);

        add_to_gantt(s, current->pid, time);

        // Выполняем процесс до завершения (невытесняющий)
        int exec_time = current->remaining_time;
        current->remaining_time = 0;
        time += exec_time;
        s->current_time = time;

        // Процесс завершился
        current->completion_time = time;
        current->turnaround_time = time - current->arrival_time;
        current->waiting_time = current->turnaround_time - current->burst_time;
        current->completed = true;
        completed++;
        s->total_waiting_time += current->waiting_time;
        s->total_turnaround_time += current->turnaround_time;
        s->total_response_time += current->response_time;

        printf("t=%d: Process %d (%s) completed (TAT=%d, WT=%d)\n",
            time, current->pid, current->name,
            current->turnaround_time, current->waiting_time);

        // Добавляем разделитель в диаграмму Ганта
        add_to_gantt(s, -1, time);
        s->context_switches++;

        // Проверяем новые процессы, прибывшие во время выполнения
        while (idx < count && processes[idx]->arrival_time <= time) {
            Process* p = processes[idx];
            if (p->priority == 1) {
                enqueue(s->high_priority_queue, p);
            }
            else if (p->priority == 2) {
                enqueue(s->medium_priority_queue, p);
            }
            else {
                enqueue(s->low_priority_queue, p);
            }
            printf("t=%d: Process %d (%s) added to queue\n",
                time, p->pid, p->name);
            idx++;
        }
    }

    add_to_gantt(s, -1, time);
    s->current_time = time;
}

// Планировщик SJF (Shortest Job First - невытесняющий)
void schedule_sjf(Scheduler* s) {
    Process** processes = s->process_list;
    int count = s->process_count;
    int time = 0;
    int completed = 0;

    reset_processes(processes, count);  // Сбрасываем состояние процессов

    // Сортируем по времени прибытия
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (processes[i]->arrival_time > processes[j]->arrival_time) {
                Process* temp = processes[i];
                processes[i] = processes[j];
                processes[j] = temp;
            }
        }
    }

    s->current_time = 0;
    s->context_switches = 0;
    s->gantt_size = 0;

    printf("\n    Shortest Job First (SJF) Scheduling \n");

    while (completed < count) {
        // Находим доступные процессы (уже прибыли) и выбираем самый короткий
        Process* shortest = NULL;
        int shortest_burst = 999999;  // Большое число для поиска минимума

        for (int i = 0; i < count; i++) {
            if (!processes[i]->completed && processes[i]->arrival_time <= time) {
                if (processes[i]->remaining_time < shortest_burst) {
                    shortest_burst = processes[i]->remaining_time;
                    shortest = processes[i];
                }
            }
        }

        if (shortest == NULL) {
            // Простой процессора - нет доступных процессов
            time++;
            s->current_time = time;
            if (s->gantt_size == 0 || s->gantt_chart_pids[s->gantt_size - 1] != 0) {
                add_to_gantt(s, 0, time - 1);
            }
            continue;
        }

        // Запускаем самый короткий процесс (невытесняющий)
        if (!shortest->started) {
            shortest->started = true;
            shortest->start_time = time;
            shortest->response_time = time - shortest->arrival_time;
        }

        printf("t=%d: Process %d (%s) started (burst=%d)\n",
            time, shortest->pid, shortest->name, shortest->remaining_time);

        add_to_gantt(s, shortest->pid, time);

        // Выполняем процесс до завершения (невытесняющий)
        int exec_time = shortest->remaining_time;
        shortest->remaining_time = 0;
        time += exec_time;
        s->current_time = time;

        // Процесс завершился
        shortest->completion_time = time;
        shortest->turnaround_time = time - shortest->arrival_time;
        shortest->waiting_time = shortest->turnaround_time - shortest->burst_time;
        shortest->completed = true;
        completed++;
        s->total_waiting_time += shortest->waiting_time;
        s->total_turnaround_time += shortest->turnaround_time;
        s->total_response_time += shortest->response_time;

        printf("t=%d: Process %d (%s) completed (TAT=%d, WT=%d)\n",
            time, shortest->pid, shortest->name,
            shortest->turnaround_time, shortest->waiting_time);

        add_to_gantt(s, -1, time);
        s->context_switches++;
    }

    add_to_gantt(s, -1, time);
    s->current_time = time;
}

// Основная функция планирования - вызывает нужный алгоритм по типу
void schedule(Scheduler* s) {
    switch (s->type) {
    case ROUND_ROBIN:
        schedule_rr(s);
        break;
    case PRIORITY_QUEUE:
        schedule_priority_queue(s);
        break;
    case SJF:
        schedule_sjf(s);
        break;
    default:
        printf("Unknown scheduler type\n");
        break;
    }
}

// Печать диаграммы Ганта (визуализация выполнения процессов)
void print_gantt_chart(Scheduler* s) {
    printf("\n    Диаграмма Ганта \n");

    if (s->gantt_size == 0) {
        printf("Нет данных\n");
        return;
    }

    // Собираем только блоки процессов (пропускаем разделители -1)
    int* clean_pids = NULL;
    int* clean_times = NULL;
    int clean_size = 0;

    for (int i = 0; i < s->gantt_size; i++) {
        if (s->gantt_chart_pids[i] != -1) {  // -1 это разделитель, пропускаем
            clean_pids = (int*)realloc(clean_pids, sizeof(int) * (clean_size + 1));
            clean_times = (int*)realloc(clean_times, sizeof(int) * (clean_size + 1));
            clean_pids[clean_size] = s->gantt_chart_pids[i];
            clean_times[clean_size] = s->gantt_chart_times[i];
            clean_size++;
        }
    }

    if (clean_size == 0) {
        printf("Нет данных\n");
        free(clean_pids);
        free(clean_times);
        return;
    }

    // Верхняя граница диаграммы
    printf(" ");
    for (int i = 0; i < clean_size - 1; i++) {
        int duration = clean_times[i + 1] - clean_times[i];
        for (int j = 0; j < duration * 2; j++) {
            printf("-");
        }
    }
    printf("\n");

    // Блоки процессов
    printf("|");
    for (int i = 0; i < clean_size - 1; i++) {
        int pid = clean_pids[i];
        int duration = clean_times[i + 1] - clean_times[i];
        int width = duration * 2;

        if (pid == 0) {
            printf("%*s", width, "IDLE");    // IDLE - простой процессора
        }
        else {
            char label[10];
            snprintf(label, sizeof(label), "P%d", pid);  // P1, P2, ...
            printf("%*s", width, label);
        }
        printf("|");
    }
    printf("\n");

    // Нижняя граница диаграммы
    printf(" ");
    for (int i = 0; i < clean_size - 1; i++) {
        int duration = clean_times[i + 1] - clean_times[i];
        for (int j = 0; j < duration * 2; j++) {
            printf("-");
        }
    }
    printf("\n");

    // Временные метки под диаграммой
    for (int i = 0; i < clean_size; i++) {
        int time = clean_times[i];
        if (i < clean_size - 1) {
            int duration = clean_times[i + 1] - clean_times[i];
            int width = duration * 2;
            char time_str[10];
            snprintf(time_str, sizeof(time_str), "%d", time);
            int len = strlen(time_str);

            // Если число не помещается в ширину, печатаем его с пробелом
            if (len > width) {
                printf("%s", time_str);
                printf(" ");
            }
            else {
                // Выравнивание по центру
                int padding = (width - len) / 2;
                for (int j = 0; j < padding; j++) printf(" ");
                printf("%s", time_str);
                for (int j = 0; j < width - padding - len; j++) printf(" ");
            }
        }
        else {
            printf("%d", time);  // Последняя метка
        }
    }
    printf("\n");

    free(clean_pids);
    free(clean_times);
}

// Печать статистики выполнения
void print_statistics(Scheduler* s) {
    printf("\n    Статистика \n");
    printf("Всего процессов: %d\n", s->process_count);
    printf("Переключений контекста: %d\n", s->context_switches);
    printf("Общее время выполнения: %d\n", s->current_time);
    printf("\n");

    // Шапка таблицы
    printf("Процесс | Прибытие | Время | Приоритет | Ожидание | Выполнение | Ответ\n");
    printf("--------|----------|-------|-----------|----------|------------|-------\n");

    // Вывод данных по каждому процессу
    for (int i = 0; i < s->process_count; i++) {
        Process* p = s->process_list[i];
        printf("P%-6d | %-7d | %-5d | %-8d | %-7d | %-10d | %-8d\n",
            p->pid, p->arrival_time, p->burst_time, p->priority,
            p->waiting_time, p->turnaround_time, p->response_time);
    }

    // Вычисляем реальную загрузку CPU
    float total_burst = 0;
    for (int i = 0; i < s->process_count; i++) {
        total_burst += s->process_list[i]->burst_time;
    }
    float cpu_utilization = s->current_time > 0 ? (total_burst / s->current_time) * 100 : 0;

    // Вывод средних значений
    printf("\nСреднее время ожидания: %.2f\n",
        s->process_count > 0 ? (float)s->total_waiting_time / s->process_count : 0);
    printf("Среднее время выполнения: %.2f\n",
        s->process_count > 0 ? (float)s->total_turnaround_time / s->process_count : 0);
    printf("Среднее время ответа: %.2f\n",
        s->process_count > 0 ? (float)s->total_response_time / s->process_count : 0);
    printf("Загрузка CPU: %.1f%%\n", cpu_utilization);
}

// Освобождение всей памяти, занятой планировщиком
void free_scheduler(Scheduler* s) {
    // Освобождаем каждый процесс
    if (s->process_list) {
        for (int i = 0; i < s->process_count; i++) {
            free(s->process_list[i]);
        }
        free(s->process_list);
    }
    // Освобождаем диаграмму Ганта
    if (s->gantt_chart_pids) free(s->gantt_chart_pids);
    if (s->gantt_chart_times) free(s->gantt_chart_times);
    // Освобождаем очереди
    free_queue(s->ready_queue);
    if (s->high_priority_queue) free_queue(s->high_priority_queue);
    if (s->medium_priority_queue) free_queue(s->medium_priority_queue);
    if (s->low_priority_queue) free_queue(s->low_priority_queue);
    free(s);    // Освобождаем сам планировщик
}

// ФУНКЦИИ ДЛЯ РАБОТЫ С РЕАЛЬНЫМИ ПРОЦЕССАМИ WINDOWS

#ifdef _WIN32

// Получить список реальных процессов Windows (аналог tasklist)
void get_real_processes() {
    // Создаем snapshot всех процессов в системе
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        printf("Ошибка создания снапшота процессов\n");
        return;
    }

    // Используем Wide-версию для корректной работы с Unicode
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);

    printf("\n    РЕАЛЬНЫЕ ПРОЦЕССЫ WINDOWS \n");
    printf("+----------+----------------------------------+\n");
    printf("| PID      | Имя процесса                     |\n");
    printf("+----------+----------------------------------+\n");

    int count = 0;
    // Проходим по всем процессам
    if (Process32FirstW(snapshot, &pe)) {
        do {
            // Конвертируем имя из Unicode в ANSI для вывода
            char process_name[256];
            int len = WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, -1,
                process_name, sizeof(process_name), NULL, NULL);
            if (len > 0) {
                process_name[len - 1] = '\0';
            }
            else {
                strcpy(process_name, "???");
            }

            printf("| %-8d | %-32s |\n", pe.th32ProcessID, process_name);
            count++;
        } while (Process32NextW(snapshot, &pe));
    }
    printf("+----------+----------------------------------+\n");
    printf("Всего процессов: %d\n", count);

    CloseHandle(snapshot);  // Закрываем хендл snapshot'а
}

// Получение загрузки CPU (в процентах)
float get_cpu_usage() {
    // Статические переменные для хранения предыдущих значений
    static FILETIME prev_idle = { 0, 0 };
    static FILETIME prev_kernel = { 0, 0 };
    static FILETIME prev_user = { 0, 0 };
    static int first_call = 1;  // Флаг первого вызова

    FILETIME idle, kernel, user;
    GetSystemTimes(&idle, &kernel, &user);  // Получаем текущие времена системы

    // При первом вызове просто запоминаем значения
    if (first_call) {
        prev_idle = idle;
        prev_kernel = kernel;
        prev_user = user;
        first_call = 0;
        return 0.0f;
    }

    // Конвертируем FILETIME в 64-битные числа
    ULARGE_INTEGER idle_now, kernel_now, user_now;
    ULARGE_INTEGER idle_prev, kernel_prev, user_prev;

    idle_now.LowPart = idle.dwLowDateTime;
    idle_now.HighPart = idle.dwHighDateTime;
    kernel_now.LowPart = kernel.dwLowDateTime;
    kernel_now.HighPart = kernel.dwHighDateTime;
    user_now.LowPart = user.dwLowDateTime;
    user_now.HighPart = user.dwHighDateTime;

    idle_prev.LowPart = prev_idle.dwLowDateTime;
    idle_prev.HighPart = prev_idle.dwHighDateTime;
    kernel_prev.LowPart = prev_kernel.dwLowDateTime;
    kernel_prev.HighPart = prev_kernel.dwHighDateTime;
    user_prev.LowPart = prev_user.dwLowDateTime;
    user_prev.HighPart = prev_user.dwHighDateTime;

    // Вычисляем разницу между текущим и предыдущим состоянием
    unsigned long long idle_diff = idle_now.QuadPart - idle_prev.QuadPart;
    unsigned long long kernel_diff = kernel_now.QuadPart - kernel_prev.QuadPart;
    unsigned long long user_diff = user_now.QuadPart - user_prev.QuadPart;
    unsigned long long total_diff = kernel_diff + user_diff;

    // Обновляем предыдущие значения
    prev_idle = idle;
    prev_kernel = kernel;
    prev_user = user;

    if (total_diff == 0) return 0.0f;  // Защита от деления на ноль

    // Загрузка CPU = (все время - время простоя) / общее время * 100
    float usage = 100.0f - (float)((long long)idle_diff * 100.0 / total_diff);

    // Ограничиваем значения в пределах 0-100%
    if (usage < 0) usage = 0;
    if (usage > 100) usage = 100;

    return usage;
}

// Функция для реального мониторинга системы (аналог top)
void real_time_monitor() {
    printf("\n    РЕАЛЬНЫЙ МОНИТОРИНГ СИСТЕМЫ \n");
    printf("Нажмите Ctrl+C для выхода\n\n");

    // Бесконечный цикл обновления
    while (1) {
        system("cls");  // Очищаем экран

        // Заголовок
        printf("+------------------------------------------------------------------+\n");
        printf("| РЕАЛЬНЫЙ МОНИТОРИНГ СИСТЕМЫ (обновление каждую секунду)          |\n");
        printf("+------------------------------------------------------------------+\n\n");

        // Загрузка CPU
        float cpu = get_cpu_usage();
        printf("Загрузка CPU: %.1f%%\n", cpu);

        // График загрузки CPU (горизонтальная гистограмма)
        printf("CPU: ");
        int bars = (int)(cpu / 2);  // 1 символ = 2% загрузки
        for (int i = 0; i < 50; i++) {
            if (i < bars) printf("#");
            else printf(" ");
        }
        printf(" %.1f%%\n\n", cpu);

        // Получаем список процессов для отображения
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32W pe;
            pe.dwSize = sizeof(PROCESSENTRY32W);

            printf("+----------+----------------------------------+\n");
            printf("| PID      | Имя процесса                     |\n");
            printf("+----------+----------------------------------+\n");

            int count = 0;
            int total = 0;
            if (Process32FirstW(snapshot, &pe)) {
                do {
                    total++;
                    // Показываем только первые 20 процессов (для читаемости)
                    if (count < 20) {
                        char process_name[256];
                        int len = WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, -1,
                            process_name, sizeof(process_name), NULL, NULL);
                        if (len > 0) {
                            process_name[len - 1] = '\0';
                        }
                        else {
                            strcpy(process_name, "???");
                        }
                        printf("| %-8d | %-32s |\n", pe.th32ProcessID, process_name);
                        count++;
                    }
                } while (Process32NextW(snapshot, &pe));
            }
            printf("+----------+----------------------------------+\n");
            printf("Показано %d процессов (всего: %d)\n", count, total);

            CloseHandle(snapshot);
        }

        printf("\nНажмите Ctrl+C для выхода...\n");
        Sleep(1000);  // Ждем 1 секунду перед обновлением
    }
}

#endif // _WIN32