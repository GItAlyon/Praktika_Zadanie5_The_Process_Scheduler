#include "scheduler.h"

// Очистка экрана (работает в Windows и Linux)
void clear_screen() {
#ifdef _WIN32
    system("cls");      // Windows
#else
    system("clear");    // Linux/Mac
#endif
}

// Создание набора тестовых процессов с разными параметрами
Process** create_test_processes(int* count) {
    *count = 6;     // Создаем 6 процессов
    Process** processes = (Process**)malloc(sizeof(Process*) * (*count));

    // Процессы с разным временем прибытия и приоритетами
    processes[0] = create_process(1, "Firefox", 0, 8, 1);   // Высокий приоритет
    processes[1] = create_process(2, "Chrome", 1, 4, 2);    // Средний приоритет
    processes[2] = create_process(3, "Terminal", 2, 2, 3);  // Низкий приоритет
    processes[3] = create_process(4, "Editor", 3, 6, 1);    // Высокий приоритет
    processes[4] = create_process(5, "Music", 4, 3, 2);     // Средний приоритет
    processes[5] = create_process(6, "Download", 5, 10, 3); // Низкий приоритет

    return processes;
}

// Создание процессов, прибывающих одновременно (для демонстрации SJF)
Process** create_simultaneous_processes(int* count) {
    *count = 5;
    Process** processes = (Process**)malloc(sizeof(Process*) * (*count));

    // Все процессы прибывают в момент времени 0
    processes[0] = create_process(1, "Процесс_A", 0, 3, 1);
    processes[1] = create_process(2, "Процесс_B", 0, 6, 2);
    processes[2] = create_process(3, "Процесс_C", 0, 2, 3);
    processes[3] = create_process(4, "Процесс_D", 0, 5, 1);
    processes[4] = create_process(5, "Процесс_E", 0, 4, 2);

    return processes;
}

// Печать заголовка с рамкой
void print_header(const char* title) {
    printf("\n");
    printf("+------------------------------------------------------------------+\n");
    printf("| %-66s |\n", title);
    printf("+------------------------------------------------------------------+\n");
}

// Отображение главного меню
void show_menu() {
    clear_screen();     // Очищаем экран перед показом меню
    print_header("СИМУЛЯТОР ПЛАНИРОВЩИКА ПРОЦЕССОВ");
    printf("\n");
    printf("  +-----------------------------------------------------+\n");
    printf("  |  1. Запустить Round Robin (циклический)             |\n");
    printf("  |  2. Запустить приоритетное планирование             |\n");
    printf("  |  3. Запустить SJF (самый короткий процесс)          |\n");
    printf("  |  4. Сравнить все алгоритмы                          |\n");
#ifdef _WIN32
    // Эти пункты доступны только в Windows
    printf("  |  5. Показать реальные процессы Windows              |\n");
    printf("  |  6. Реальный мониторинг системы (аналог top)        |\n");
#endif
    printf("  |  0. Выход                                           |\n");
    printf("  +-----------------------------------------------------+\n");
    printf("\n  Выберите опцию: ");
}

// Запуск планировщика с отображением результатов
void run_scheduler_with_animation(Scheduler* s, const char* name) {
    printf("\n");
    print_header(name);         // Показываем заголовок
    schedule(s);                // Запускаем планирование
    print_gantt_chart(s);       // Показываем диаграмму Ганта
    print_statistics(s);        // Показываем статистику
    printf("\n  Нажмите Enter для продолжения...");
    getchar();  // Ждем нажатия Enter
}

// Интерактивный режим - показ всех алгоритмов
void interactive_mode() {
    clear_screen();
    print_header("ИНТЕРАКТИВНЫЙ РЕЖИМ");
    printf("\n  Этот режим покажет пошаговое выполнение\n");
    printf("  различных алгоритмов планирования.\n\n");

    // Создаем тестовые процессы
    int count;
    Process** processes = create_test_processes(&count);

    // Показываем информацию о тестовых процессах
    printf("  Тестовые процессы:\n");
    printf("  +-----+----------+---------+-------+----------+\n");
    printf("  | PID | Имя      | Прибытие| Время | Приоритет|\n");
    printf("  +-----+----------+---------+-------+----------+\n");
    for (int i = 0; i < count; i++) {
        printf("  | %-3d | %-8s | %-7d | %-5d | %-8d |\n",
            processes[i]->pid,
            processes[i]->name,
            processes[i]->arrival_time,
            processes[i]->burst_time,
            processes[i]->priority);
    }
    printf("  +-----+----------+---------+-------+----------+\n");
    printf("\n  Нажмите Enter для запуска симуляции...");
    getchar();

    // Запускаем Round Robin
    Process** p1 = create_test_processes(&count);
    Scheduler* rr = create_scheduler(ROUND_ROBIN, 2);
    for (int i = 0; i < count; i++) add_process(rr, p1[i]);
    run_scheduler_with_animation(rr, "ЦИКЛИЧЕСКИЙ ПЛАНИРОВЩИК (Квант=2)");

    // Запускаем Priority Queue
    Process** p2 = create_test_processes(&count);
    Scheduler* pq = create_scheduler(PRIORITY_QUEUE, 0);
    for (int i = 0; i < count; i++) add_process(pq, p2[i]);
    run_scheduler_with_animation(pq, "ПРИОРИТЕТНОЕ ПЛАНИРОВАНИЕ");

    // Запускаем SJF
    Process** p3 = create_test_processes(&count);
    Scheduler* sjf = create_scheduler(SJF, 0);
    for (int i = 0; i < count; i++) add_process(sjf, p3[i]);
    run_scheduler_with_animation(sjf, "ПЛАНИРОВЩИК SJF (Кратчайший процесс)");

    // Освобождаем память
    for (int i = 0; i < count; i++) {
        free(p1[i]);
        free(p2[i]);
        free(p3[i]);
    }
    free(p1);
    free(p2);
    free(p3);
    free(processes);
}

// Главная функция - точка входа в программу
int main() {
#ifdef _WIN32
    SetConsoleCP(1251);         
    SetConsoleOutputCP(1251);  
#endif

    int choice;

    do {
        show_menu();                    // Показываем меню
        scanf("%d", &choice);           // Читаем выбор пользователя
        getchar();                      // Очищаем буфер ввода

        switch (choice) {
        case 1: {   // Запуск Round Robin
            int count;
            Process** processes = create_test_processes(&count);
            Scheduler* s = create_scheduler(ROUND_ROBIN, 2);
            for (int i = 0; i < count; i++) add_process(s, processes[i]);
            run_scheduler_with_animation(s, "ЦИКЛИЧЕСКИЙ ПЛАНИРОВЩИК (Квант=2)");
            free_scheduler(s);
            break;
        }
        case 2: {   // Запуск приоритетного планирования
            int count;
            Process** processes = create_test_processes(&count);
            Scheduler* s = create_scheduler(PRIORITY_QUEUE, 0);
            for (int i = 0; i < count; i++) add_process(s, processes[i]);
            run_scheduler_with_animation(s, "ПРИОРИТЕТНОЕ ПЛАНИРОВАНИЕ");
            free_scheduler(s);
            break;
        }
        case 3: {   // Запуск SJF
            int count;
            Process** processes = create_test_processes(&count);
            Scheduler* s = create_scheduler(SJF, 0);
            for (int i = 0; i < count; i++) add_process(s, processes[i]);
            run_scheduler_with_animation(s, "ПЛАНИРОВЩИК SJF (Кратчайший процесс)");
            free_scheduler(s);
            break;
        }
        case 4:   // Запуск всех планировщиков
            interactive_mode();
            break;
#ifdef _WIN32
        case 5: {   // Показать реальные процессы Windows
            clear_screen();
            print_header("РЕАЛЬНЫЕ ПРОЦЕССЫ WINDOWS");
            get_real_processes();
            printf("\n  Нажмите Enter для продолжения...");
            getchar();
            break;
        }
        case 6: {   // Реальный мониторинг системы (аналог top)
            clear_screen();
            print_header("РЕАЛЬНЫЙ МОНИТОРИНГ СИСТЕМЫ");
            printf("\n  Внимание! Это реальный мониторинг Windows.\n");
            printf("  Нажмите Enter для запуска...");
            getchar();
            real_time_monitor();
            break;
        }
#endif
        case 0:   // Выход из программы
            printf("\n  До свидания!\n\n");
            break;
        default:   // Неверный выбор
            printf("\n  Неверный выбор! Нажмите Enter для продолжения...");
            getchar();
        }
    } while (choice != 0);  // Повторяем, пока пользователь не выберет выход

    return 0;
}