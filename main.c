#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

// Определение констант для моделирования.
#define SEQUENCE_SIZE 2048

// Константы, определяющие геометрию диска и параметры времени.
const int tracks = 500; // Количество дорожек (цилиндров) на диске.
const int heads = 4; // Количество поверхностей/головок на диске.
const int sectors = 16; // Количество секторов на поверхности.
const int rpm = 10000; // Скорость вращения диска (оборотов в минуту).
const int n = 1; // Количество последовательных секторов, обрабатываемых в запросе.
double t_max = 5000; // Максимальный интервал времени между запросами.
const double msPerMin = 60 * 1000; // Количество миллисекунд в минуте.
const double ts = 0.5; // Время перехода головки на соседнюю дорожку (мс).
const double tr = msPerMin / (rpm * sectors); // Время чтения одного сектора.
const double tw = tr + tr * sectors; // Время записи одного сектора.
const double tm = msPerMin * 5; // Общее время моделирования (5 минут).

// Определение структуры для хранения информации о запросе.
typedef struct Request {
    int track;
    int sector;
    int head;
    double t;
    bool operation_type; // Тип операции (0 - чтение, 1 - запись).
} Request;

// Определение структуры для хранения последовательности запросов.
typedef struct Sequence {
    Request** requests; // Указатель на массив указателей на запросы.
    size_t length; // Текущее количество запросов.
    size_t size; // Максимальное количество запросов.
} Sequence;

// Определение структуры для хранения данных моделирования.
typedef struct ModelingData {
    int total_processed; // Общее количество обработанных запросов.
    int max_queue_len; // Максимальная длина очереди запросов.
    double* process_time; // Массив времен обработки каждого запроса.
    double idle_time; // Общее время простоя.
} ModelingData;

// Функция для создания нового запроса.
Request* create_request(double time) {
    Request* req = malloc(sizeof(Request)); // Выделение памяти под запрос.
    req->t = time + rand() % (int)t_max; // Генерация времени появления запроса.
    req->track = rand() % tracks; // Случайный выбор дорожки.
    req->head = rand() % heads; // Случайный выбор головки.
    req->sector = rand() % sectors; // Случайный выбор сектора.
    req->operation_type = (bool)(rand() % 2); // Случайный выбор типа операции.
    return req;
}

// Функция для создания последовательности запросов.
Sequence* create_sequence() {
    Sequence* seq = malloc(sizeof(Sequence)); // Выделение памяти под последовательность.
    seq->length = 0; // Инициализация длины последовательности.
    seq->size = SEQUENCE_SIZE; // Инициализация размера последовательности.
    seq->requests = malloc(seq->size * sizeof(Request*)); // Выделение памяти под запросы.
    Request* req = create_request(0);
    while (req->t < tm) { // Генерация запросов до достижения времени моделирования.
        if (seq->length == seq->size) { // Увеличение размера массива, если достигнут его предел.
            seq->size *= 2;
            seq->requests = realloc(seq->requests, seq->size * sizeof(Request*));
        }
        seq->requests[seq->length++] = req; // Добавление запроса в последовательность.
        req = create_request(req->t); // Создание следующего запроса.
    }
    return seq;
}

// Функция для создания структуры данных моделирования.
ModelingData* create_modeling_data(size_t length) {
    ModelingData* data = malloc(sizeof(ModelingData)); // Выделение памяти под данные моделирования.
    data->total_processed = 0; // Инициализация количества обработанных запросов.
    data->process_time = malloc(length * sizeof(double)); // Выделение памяти под время обработки запросов.
    data->max_queue_len = 0; // Инициализация максимальной длины очереди.
    data->idle_time = 0; // Инициализация времени простоя.
    return data;
}

// Функция для освобождения памяти, занятой последовательностью запросов.
void free_sequence(Sequence* seq) {
    for (int i = 0; i < seq->length; i++) { // Освобождение памяти каждого запроса.
        free(seq->requests[i]);
    }
    free(seq); // Освобождение памяти, занятой последовательностью.
}

// Функция для вычисления времени до нового сектора.
int time_to_new_sector(int sector, double time) {
    int total_sectors = time / tr; // Вычисление общего количества секторов, пройденных за время.
    int curr_sector = total_sectors % sectors; // Определение текущего сектора.
    int diff = sector - curr_sector; // Разница между текущим и запрошенным секторами.
    double offset = time - total_sectors * tr; // Вычисление временного смещения.
    return diff >= 0 ? diff * tr - offset : (diff + sectors) * tr - offset; // Время до нового сектора.
}

// Функция для обработки запроса.
double process_request(Request* req, int currTrack, double t) {
    int track = req->track; // Номер трека запроса.
    // Вычисление времени поиска (подвода головки к нужному цилиндру).
    double tp = abs(track - currTrack) * ts;
    // Вычисление времени ожидания, пока нужный сектор не окажется под головкой.
    double to = time_to_new_sector(req->sector, t + tp);
    // Вычисление времени чтения или записи сектора.
    double trw = req->operation_type ? tw : tr;
    return tp + to + trw; // Общее время обработки запроса.
}
// Функция для нахождения минимального значения в массиве.
double arr_min(double* values, size_t len) {
    double min = INFINITY; // Инициализация минимума как бесконечность.
    for (size_t i = 0; i < len; i++) {
        min = fmin(values[i], min); // Обновление минимального значения.
    }
    return min; // Возврат наименьшего значения в массиве.
}

// Функция для нахождения максимального значения в массиве.
double arr_max(double* values, size_t len) {
    double max = -INFINITY; // Инициализация максимума как отрицательная бесконечность.
    for (size_t i = 0; i < len; i++) {
        max = fmax(values[i], max); // Обновление максимального значения.
    }
    return max; // Возврат наибольшего значения в массиве.
}

// Функция для вычисления среднего значения элементов массива.
double arr_mean(double* values, size_t len) {
    double sum = 0; // Сумма всех элементов.
    for (size_t i = 0; i < len; i++) {
        sum += values[i]; // Накопление суммы.
    }
    return sum / len; // Возврат среднего значения.
}

// Функция для вычисления среднеквадратического отклонения от среднего.
double arr_SD(double* values, size_t len, double mean) {
    double sd = 0; // Инициализация среднеквадратического отклонения.
    for (size_t i = 0; i < len; i++) {
        sd += pow(values[i] - mean, 2); // Суммирование квадратов отклонений от среднего.
    }
    return sqrt(sd / len); // Возврат среднеквадратического отклонения.
}

// Функция для вывода результатов моделирования.
void print_results(Sequence* seq, ModelingData* md) {
    double minPT = arr_min(md->process_time, md->total_processed); // Наименьшее время обработки запроса.
    double maxPT = arr_max(md->process_time, md->total_processed); // Наибольшее время обработки запроса.
    double meanPT = arr_mean(md->process_time, md->total_processed); // Среднее время обработки запроса.
    double sdPT = arr_SD(md->process_time, md->total_processed, meanPT); // Среднеквадратическое отклонение времени обработки.

    // Вывод статистики.
    printf("Обработано запросов: %d из %lu\n", md->total_processed, seq->length);
    printf("Минимальное время запроса(мс): %f\n", minPT);
    printf("Максимальное время запроса(мс): %f\n", maxPT);
    printf("Среднее время запроса(мс): %f\n", meanPT);
    printf("Среднеквадратическое отклонение от среднего времени запроса(мс): %f\n", sdPT);
    printf("Максимальная длина очереди: %i\n", md->max_queue_len);
    printf("Время простоя(мс): %f\n", md->idle_time);

    // Подготовка и вывод гистограммы распределения времени обслуживания запросов.
    int histogram_index[8] = { 1, 10, 100, 500, 1000, 5000, 10000, INFINITY };
    int histogram_array[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    for (int i = 0; i < md->total_processed; i++) {
        double time = md->process_time[i];
        for (int j = 1; j < 8; j++) { // Проверка принадлежности времени к определенному интервалу.
            if (time < histogram_index[j]) {
                histogram_array[j - 1]++;
                break;
            }
        }
    }

    printf("Гистограмма распределения времени обслуживания:\n");
    for (int i = 0; i < 7; i++) {
        printf("|>%4iмс", histogram_index[i]);
    }
    printf("\n");
    for (int i = 0; i < 7; i++) {
        printf("|%-7i", histogram_array[i]);
    }
    printf("\n\n");
}

// Функция для определения последнего индекса запроса, обработанного до определенного времени.
int get_last_request_index(Sequence* seq, int i, double t) {
    while (i + 1 < seq->length && seq->requests[i + 1]->t <= t) { // Перебор запросов до заданного времени.
        i++;
    }
    return i; // Возврат индекса последнего запроса.
}

// Функция для освобождения памяти, занятой данными моделирования.
void free_modeling_data(ModelingData* data) {
    free(data->process_time); // Освобождение памяти, занятой временами обработки запросов.
    free(data); // Освобождение памяти, занятой структурой данных моделирования.
}

// Функция для реализации стратегии FIFO (First In, First Out).
ModelingData* FIFO(Sequence* seq) {
    int track = 0; // Текущая дорожка головки диска.
    int buffer_start = 0; // Начало буфера запросов.
    int buffer_end = -1; // Конец буфера запросов.
    double tmp_time = 0; // Временная метка для отслеживания текущего времени.
    ModelingData* data = create_modeling_data(seq->length); // Создание структуры для хранения данных моделирования.

    while (buffer_start < seq->length) { // Пока есть запросы для обработки.
        buffer_end = get_last_request_index(seq, buffer_end, tmp_time); // Определение последнего запроса в текущем временном интервале.
        int queue = buffer_end - buffer_start; // Размер очереди запросов.
        if (queue > data->max_queue_len) data->max_queue_len = queue; // Обновление максимальной длины очереди.

        if (queue < 0) { // Если в очереди нет запросов.
            Request* request = seq->requests[buffer_start]; // Следующий запрос для обработки.
            data->idle_time += request->t - tmp_time; // Обновление времени простоя.
            tmp_time = request->t; // Обновление текущего времени.
            continue; // Переход к следующему шагу цикла.
        }

        Request* req = seq->requests[buffer_start]; // Выбор запроса для обработки.
        double request_time = process_request(req, track, tmp_time); // Вычисление времени, необходимого для обработки запроса.
        tmp_time += request_time; // Обновление текущего времени.
        if (tmp_time >= tm) { // Проверка, не превышено ли общее время моделирования.
            buffer_end = get_last_request_index(seq, buffer_end, tmp_time); // Определение последнего запроса в текущем временном интервале.
            int queue = buffer_end - buffer_start; // Размер очереди запросов.
            if (queue > data->max_queue_len) data->max_queue_len = queue; // Обновление максимальной длины очереди.
            return data; // Возврат данных моделирования.
        }

        track = req->track; // Обновление текущей дорожки.
        data->process_time[data->total_processed++] = tmp_time - req->t; // Запись времени обработки запроса.
        buffer_start++; // Перемещение в начало следующего запроса в буфере.
    }
    return data; // Возврат данных моделирования после обработки всех запросов.
}

// Функция для сравнения запросов по номеру трека (используется в qsort).
int cmp_requests_track(const void* a, const void* b) {
    Request* req1 = *(Request**)a; // Преобразование void* в Request*.
    Request* req2 = *(Request**)b; // Преобразование void* в Request*.
    // Возвращение результата сравнения номеров треков.
    if (req1->track < req2->track) return -1;
    if (req1->track > req2->track) return 1;
    return 0;
}

// Функция для сравнения запросов по номеру трека в обратном порядке (используется в qsort).
int cmp_requests_track_reverse(const void* a, const void* b) {
    Request* req1 = *(Request**)a; // Преобразование void* в Request*.
    Request* req2 = *(Request**)b; // Преобразование void* в Request*.
    // Возвращение результата сравнения номеров треков в обратном порядке.
    if (req1->track < req2->track) return 1;
    if (req1->track > req2->track) return -1;
    return 0;
}

// Функция-обертка для выбора функции сравнения в зависимости от направления.
void* cmp_requests(int direction) {
    return direction ? cmp_requests_track : cmp_requests_track_reverse;
}

// Функция для реализации стратегии FSCAN (Full SCAN).
ModelingData* FSCAN(Sequence* seq) {
    int track = 0; // Текущая дорожка головки диска.
    int direction = 1; // Направление сканирования (1 - вверх, 0 - вниз).
    int buffer_start = 0; // Начало буфера запросов.
    int buffer_end = -1; // Конец буфера запросов.
    double tmp_time = 0; // Временная метка для отслеживания текущего времени.
    ModelingData* data = create_modeling_data(seq->length); // Создание структуры для хранения данных моделирования.
    while (buffer_start < seq->length) { // Пока есть запросы для обработки.
        buffer_end = get_last_request_index(seq, buffer_end, tmp_time); // Определение последнего запроса в текущем временном интервале.
        int queue = buffer_end - buffer_start; // Размер очереди запросов.
        if (queue > data->max_queue_len)
            data->max_queue_len = queue; // Обновление максимальной длины очереди.

        if (queue < 0) { // Если в очереди нет запросов.
            Request* next = seq->requests[buffer_start]; // Следующий запрос для обработки.
            data->idle_time += next->t - tmp_time; // Обновление времени простоя.
            tmp_time = next->t; // Обновление текущего времени.
            continue; // Переход к следующему шагу цикла.
        }

        int len = buffer_end - buffer_start + 1; // Вычисление количества запросов в буфере.
        Request* requests[len]; // Создание массива для хранения выбранных запросов.
        memcpy(requests, seq->requests + buffer_start, sizeof(requests)); // Копирование запросов в массив.

        // Сортировка запросов в соответствии с текущим направлением сканирования.
        qsort(requests, len, sizeof(Request*), cmp_requests(direction));

        for (int i = 0; i < len; i++) { // Обработка каждого запроса в отсортированном массиве.
            Request* req = requests[i];
            double treq = process_request(req, track, tmp_time); // Вычисление времени, необходимого для обработки запроса.
            tmp_time += treq; // Обновление текущего времени.
            if (tmp_time >= tm) { // Проверка, не превышено ли общее время моделирования.
                buffer_end = get_last_request_index(seq, buffer_end, tmp_time); // Определение последнего запроса в текущем временном интервале.
                int queue = buffer_end - buffer_start; // Размер очереди запросов.
                if (queue > data->max_queue_len) data->max_queue_len = queue; // Обновление максимальной длины очереди.
                return data; // Возврат данных моделирования.
            }
            track = req->track; // Обновление текущей дорожки.
            data->process_time[data->total_processed++] = tmp_time - req->t; // Запись времени обработки запроса.
        }

        buffer_start += len; // Перемещение в начало следующего запроса в буфере.
        direction = !direction; // Смена направления сканирования.
    }
    return data; // Возврат данных моделирования после обработки всех запросов.
}

// Основная функция программы.
int main(void) {
    srand(time(0)); // Инициализация генератора случайных чисел.

    for (int i = 0; i < 3; i++) { // Проведение трех серий моделирования с разными значениями t_max.
        Sequence* seq = create_sequence(); // Создание последовательности запросов.
        // Обслуживание запросов по стратегии FIFO.
        ModelingData* FIFO_data = FIFO(seq);
        // Обслуживание запросов по стратегии FSCAN.
        ModelingData* FSCAN_data = FSCAN(seq);
        // Вывод результатов моделирования для каждой стратегии.
        printf("\nFIFO t_max = %f мс\n", t_max);
        print_results(seq, FIFO_data);
        printf("\nFSCAN t_max = %f мс\n", t_max);
        print_results(seq, FSCAN_data);
        t_max = t_max / 10; // Уменьшение t_max для следующей серии моделирования.
        free_modeling_data(FIFO_data); // Освобождение памяти, занятой данными FIFO.
        free_modeling_data(FSCAN_data); // Освобождение памяти, занятой данными FSCAN.
        free_sequence(seq); // Освобождение памяти, занятой последовательностью запросов.
    }
    return 0; // Завершение программы.
}
