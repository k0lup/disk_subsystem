#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

// ����������� �������� ��� �������������.
#define SEQUENCE_SIZE 2048

// ���������, ������������ ��������� ����� � ��������� �������.
const int tracks = 500; // ���������� ������� (���������) �� �����.
const int heads = 4; // ���������� ������������/������� �� �����.
const int sectors = 16; // ���������� �������� �� �����������.
const int rpm = 10000; // �������� �������� ����� (�������� � ������).
const int n = 1; // ���������� ���������������� ��������, �������������� � �������.
double t_max = 5000; // ������������ �������� ������� ����� ���������.
const double msPerMin = 60 * 1000; // ���������� ����������� � ������.
const double ts = 0.5; // ����� �������� ������� �� �������� ������� (��).
const double tr = msPerMin / (rpm * sectors); // ����� ������ ������ �������.
const double tw = tr + tr * sectors; // ����� ������ ������ �������.
const double tm = msPerMin * 5; // ����� ����� ������������� (5 �����).

// ����������� ��������� ��� �������� ���������� � �������.
typedef struct Request {
    int track;
    int sector;
    int head;
    double t;
    bool operation_type; // ��� �������� (0 - ������, 1 - ������).
} Request;

// ����������� ��������� ��� �������� ������������������ ��������.
typedef struct Sequence {
    Request** requests; // ��������� �� ������ ���������� �� �������.
    size_t length; // ������� ���������� ��������.
    size_t size; // ������������ ���������� ��������.
} Sequence;

// ����������� ��������� ��� �������� ������ �������������.
typedef struct ModelingData {
    int total_processed; // ����� ���������� ������������ ��������.
    int max_queue_len; // ������������ ����� ������� ��������.
    double* process_time; // ������ ������ ��������� ������� �������.
    double idle_time; // ����� ����� �������.
} ModelingData;

// ������� ��� �������� ������ �������.
Request* create_request(double time) {
    Request* req = malloc(sizeof(Request)); // ��������� ������ ��� ������.
    req->t = time + rand() % (int)t_max; // ��������� ������� ��������� �������.
    req->track = rand() % tracks; // ��������� ����� �������.
    req->head = rand() % heads; // ��������� ����� �������.
    req->sector = rand() % sectors; // ��������� ����� �������.
    req->operation_type = (bool)(rand() % 2); // ��������� ����� ���� ��������.
    return req;
}

// ������� ��� �������� ������������������ ��������.
Sequence* create_sequence() {
    Sequence* seq = malloc(sizeof(Sequence)); // ��������� ������ ��� ������������������.
    seq->length = 0; // ������������� ����� ������������������.
    seq->size = SEQUENCE_SIZE; // ������������� ������� ������������������.
    seq->requests = malloc(seq->size * sizeof(Request*)); // ��������� ������ ��� �������.
    Request* req = create_request(0);
    while (req->t < tm) { // ��������� �������� �� ���������� ������� �������������.
        if (seq->length == seq->size) { // ���������� ������� �������, ���� ��������� ��� ������.
            seq->size *= 2;
            seq->requests = realloc(seq->requests, seq->size * sizeof(Request*));
        }
        seq->requests[seq->length++] = req; // ���������� ������� � ������������������.
        req = create_request(req->t); // �������� ���������� �������.
    }
    return seq;
}

// ������� ��� �������� ��������� ������ �������������.
ModelingData* create_modeling_data(size_t length) {
    ModelingData* data = malloc(sizeof(ModelingData)); // ��������� ������ ��� ������ �������������.
    data->total_processed = 0; // ������������� ���������� ������������ ��������.
    data->process_time = malloc(length * sizeof(double)); // ��������� ������ ��� ����� ��������� ��������.
    data->max_queue_len = 0; // ������������� ������������ ����� �������.
    data->idle_time = 0; // ������������� ������� �������.
    return data;
}

// ������� ��� ������������ ������, ������� ������������������� ��������.
void free_sequence(Sequence* seq) {
    for (int i = 0; i < seq->length; i++) { // ������������ ������ ������� �������.
        free(seq->requests[i]);
    }
    free(seq); // ������������ ������, ������� �������������������.
}

// ������� ��� ���������� ������� �� ������ �������.
int time_to_new_sector(int sector, double time) {
    int total_sectors = time / tr; // ���������� ������ ���������� ��������, ���������� �� �����.
    int curr_sector = total_sectors % sectors; // ����������� �������� �������.
    int diff = sector - curr_sector; // ������� ����� ������� � ����������� ���������.
    double offset = time - total_sectors * tr; // ���������� ���������� ��������.
    return diff >= 0 ? diff * tr - offset : (diff + sectors) * tr - offset; // ����� �� ������ �������.
}

// ������� ��� ��������� �������.
double process_request(Request* req, int currTrack, double t) {
    int track = req->track; // ����� ����� �������.
    // ���������� ������� ������ (������� ������� � ������� ��������).
    double tp = abs(track - currTrack) * ts;
    // ���������� ������� ��������, ���� ������ ������ �� �������� ��� ��������.
    double to = time_to_new_sector(req->sector, t + tp);
    // ���������� ������� ������ ��� ������ �������.
    double trw = req->operation_type ? tw : tr;
    return tp + to + trw; // ����� ����� ��������� �������.
}
// ������� ��� ���������� ������������ �������� � �������.
double arr_min(double* values, size_t len) {
    double min = INFINITY; // ������������� �������� ��� �������������.
    for (size_t i = 0; i < len; i++) {
        min = fmin(values[i], min); // ���������� ������������ ��������.
    }
    return min; // ������� ����������� �������� � �������.
}

// ������� ��� ���������� ������������� �������� � �������.
double arr_max(double* values, size_t len) {
    double max = -INFINITY; // ������������� ��������� ��� ������������� �������������.
    for (size_t i = 0; i < len; i++) {
        max = fmax(values[i], max); // ���������� ������������� ��������.
    }
    return max; // ������� ����������� �������� � �������.
}

// ������� ��� ���������� �������� �������� ��������� �������.
double arr_mean(double* values, size_t len) {
    double sum = 0; // ����� ���� ���������.
    for (size_t i = 0; i < len; i++) {
        sum += values[i]; // ���������� �����.
    }
    return sum / len; // ������� �������� ��������.
}

// ������� ��� ���������� ��������������������� ���������� �� ��������.
double arr_SD(double* values, size_t len, double mean) {
    double sd = 0; // ������������� ��������������������� ����������.
    for (size_t i = 0; i < len; i++) {
        sd += pow(values[i] - mean, 2); // ������������ ��������� ���������� �� ��������.
    }
    return sqrt(sd / len); // ������� ��������������������� ����������.
}

// ������� ��� ������ ����������� �������������.
void print_results(Sequence* seq, ModelingData* md) {
    double minPT = arr_min(md->process_time, md->total_processed); // ���������� ����� ��������� �������.
    double maxPT = arr_max(md->process_time, md->total_processed); // ���������� ����� ��������� �������.
    double meanPT = arr_mean(md->process_time, md->total_processed); // ������� ����� ��������� �������.
    double sdPT = arr_SD(md->process_time, md->total_processed, meanPT); // �������������������� ���������� ������� ���������.

    // ����� ����������.
    printf("���������� ��������: %d �� %lu\n", md->total_processed, seq->length);
    printf("����������� ����� �������(��): %f\n", minPT);
    printf("������������ ����� �������(��): %f\n", maxPT);
    printf("������� ����� �������(��): %f\n", meanPT);
    printf("�������������������� ���������� �� �������� ������� �������(��): %f\n", sdPT);
    printf("������������ ����� �������: %i\n", md->max_queue_len);
    printf("����� �������(��): %f\n", md->idle_time);

    // ���������� � ����� ����������� ������������� ������� ������������ ��������.
    int histogram_index[8] = { 1, 10, 100, 500, 1000, 5000, 10000, INFINITY };
    int histogram_array[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    for (int i = 0; i < md->total_processed; i++) {
        double time = md->process_time[i];
        for (int j = 1; j < 8; j++) { // �������� �������������� ������� � ������������� ���������.
            if (time < histogram_index[j]) {
                histogram_array[j - 1]++;
                break;
            }
        }
    }

    printf("����������� ������������� ������� ������������:\n");
    for (int i = 0; i < 7; i++) {
        printf("|>%4i��", histogram_index[i]);
    }
    printf("\n");
    for (int i = 0; i < 7; i++) {
        printf("|%-7i", histogram_array[i]);
    }
    printf("\n\n");
}

// ������� ��� ����������� ���������� ������� �������, ������������� �� ������������� �������.
int get_last_request_index(Sequence* seq, int i, double t) {
    while (i + 1 < seq->length && seq->requests[i + 1]->t <= t) { // ������� �������� �� ��������� �������.
        i++;
    }
    return i; // ������� ������� ���������� �������.
}

// ������� ��� ������������ ������, ������� ������� �������������.
void free_modeling_data(ModelingData* data) {
    free(data->process_time); // ������������ ������, ������� ��������� ��������� ��������.
    free(data); // ������������ ������, ������� ���������� ������ �������������.
}

// ������� ��� ���������� ��������� FIFO (First In, First Out).
ModelingData* FIFO(Sequence* seq) {
    int track = 0; // ������� ������� ������� �����.
    int buffer_start = 0; // ������ ������ ��������.
    int buffer_end = -1; // ����� ������ ��������.
    double tmp_time = 0; // ��������� ����� ��� ������������ �������� �������.
    ModelingData* data = create_modeling_data(seq->length); // �������� ��������� ��� �������� ������ �������������.

    while (buffer_start < seq->length) { // ���� ���� ������� ��� ���������.
        buffer_end = get_last_request_index(seq, buffer_end, tmp_time); // ����������� ���������� ������� � ������� ��������� ���������.
        int queue = buffer_end - buffer_start; // ������ ������� ��������.
        if (queue > data->max_queue_len) data->max_queue_len = queue; // ���������� ������������ ����� �������.

        if (queue < 0) { // ���� � ������� ��� ��������.
            Request* request = seq->requests[buffer_start]; // ��������� ������ ��� ���������.
            data->idle_time += request->t - tmp_time; // ���������� ������� �������.
            tmp_time = request->t; // ���������� �������� �������.
            continue; // ������� � ���������� ���� �����.
        }

        Request* req = seq->requests[buffer_start]; // ����� ������� ��� ���������.
        double request_time = process_request(req, track, tmp_time); // ���������� �������, ������������ ��� ��������� �������.
        tmp_time += request_time; // ���������� �������� �������.
        if (tmp_time >= tm) { // ��������, �� ��������� �� ����� ����� �������������.
            buffer_end = get_last_request_index(seq, buffer_end, tmp_time); // ����������� ���������� ������� � ������� ��������� ���������.
            int queue = buffer_end - buffer_start; // ������ ������� ��������.
            if (queue > data->max_queue_len) data->max_queue_len = queue; // ���������� ������������ ����� �������.
            return data; // ������� ������ �������������.
        }

        track = req->track; // ���������� ������� �������.
        data->process_time[data->total_processed++] = tmp_time - req->t; // ������ ������� ��������� �������.
        buffer_start++; // ����������� � ������ ���������� ������� � ������.
    }
    return data; // ������� ������ ������������� ����� ��������� ���� ��������.
}

// ������� ��� ��������� �������� �� ������ ����� (������������ � qsort).
int cmp_requests_track(const void* a, const void* b) {
    Request* req1 = *(Request**)a; // �������������� void* � Request*.
    Request* req2 = *(Request**)b; // �������������� void* � Request*.
    // ����������� ���������� ��������� ������� ������.
    if (req1->track < req2->track) return -1;
    if (req1->track > req2->track) return 1;
    return 0;
}

// ������� ��� ��������� �������� �� ������ ����� � �������� ������� (������������ � qsort).
int cmp_requests_track_reverse(const void* a, const void* b) {
    Request* req1 = *(Request**)a; // �������������� void* � Request*.
    Request* req2 = *(Request**)b; // �������������� void* � Request*.
    // ����������� ���������� ��������� ������� ������ � �������� �������.
    if (req1->track < req2->track) return 1;
    if (req1->track > req2->track) return -1;
    return 0;
}

// �������-������� ��� ������ ������� ��������� � ����������� �� �����������.
void* cmp_requests(int direction) {
    return direction ? cmp_requests_track : cmp_requests_track_reverse;
}

// ������� ��� ���������� ��������� FSCAN (Full SCAN).
ModelingData* FSCAN(Sequence* seq) {
    int track = 0; // ������� ������� ������� �����.
    int direction = 1; // ����������� ������������ (1 - �����, 0 - ����).
    int buffer_start = 0; // ������ ������ ��������.
    int buffer_end = -1; // ����� ������ ��������.
    double tmp_time = 0; // ��������� ����� ��� ������������ �������� �������.
    ModelingData* data = create_modeling_data(seq->length); // �������� ��������� ��� �������� ������ �������������.
    while (buffer_start < seq->length) { // ���� ���� ������� ��� ���������.
        buffer_end = get_last_request_index(seq, buffer_end, tmp_time); // ����������� ���������� ������� � ������� ��������� ���������.
        int queue = buffer_end - buffer_start; // ������ ������� ��������.
        if (queue > data->max_queue_len)
            data->max_queue_len = queue; // ���������� ������������ ����� �������.

        if (queue < 0) { // ���� � ������� ��� ��������.
            Request* next = seq->requests[buffer_start]; // ��������� ������ ��� ���������.
            data->idle_time += next->t - tmp_time; // ���������� ������� �������.
            tmp_time = next->t; // ���������� �������� �������.
            continue; // ������� � ���������� ���� �����.
        }

        int len = buffer_end - buffer_start + 1; // ���������� ���������� �������� � ������.
        Request* requests[len]; // �������� ������� ��� �������� ��������� ��������.
        memcpy(requests, seq->requests + buffer_start, sizeof(requests)); // ����������� �������� � ������.

        // ���������� �������� � ������������ � ������� ������������ ������������.
        qsort(requests, len, sizeof(Request*), cmp_requests(direction));

        for (int i = 0; i < len; i++) { // ��������� ������� ������� � ��������������� �������.
            Request* req = requests[i];
            double treq = process_request(req, track, tmp_time); // ���������� �������, ������������ ��� ��������� �������.
            tmp_time += treq; // ���������� �������� �������.
            if (tmp_time >= tm) { // ��������, �� ��������� �� ����� ����� �������������.
                buffer_end = get_last_request_index(seq, buffer_end, tmp_time); // ����������� ���������� ������� � ������� ��������� ���������.
                int queue = buffer_end - buffer_start; // ������ ������� ��������.
                if (queue > data->max_queue_len) data->max_queue_len = queue; // ���������� ������������ ����� �������.
                return data; // ������� ������ �������������.
            }
            track = req->track; // ���������� ������� �������.
            data->process_time[data->total_processed++] = tmp_time - req->t; // ������ ������� ��������� �������.
        }

        buffer_start += len; // ����������� � ������ ���������� ������� � ������.
        direction = !direction; // ����� ����������� ������������.
    }
    return data; // ������� ������ ������������� ����� ��������� ���� ��������.
}

// �������� ������� ���������.
int main(void) {
    srand(time(0)); // ������������� ���������� ��������� �����.

    for (int i = 0; i < 3; i++) { // ���������� ���� ����� ������������� � ������� ���������� t_max.
        Sequence* seq = create_sequence(); // �������� ������������������ ��������.
        // ������������ �������� �� ��������� FIFO.
        ModelingData* FIFO_data = FIFO(seq);
        // ������������ �������� �� ��������� FSCAN.
        ModelingData* FSCAN_data = FSCAN(seq);
        // ����� ����������� ������������� ��� ������ ���������.
        printf("\nFIFO t_max = %f ��\n", t_max);
        print_results(seq, FIFO_data);
        printf("\nFSCAN t_max = %f ��\n", t_max);
        print_results(seq, FSCAN_data);
        t_max = t_max / 10; // ���������� t_max ��� ��������� ����� �������������.
        free_modeling_data(FIFO_data); // ������������ ������, ������� ������� FIFO.
        free_modeling_data(FSCAN_data); // ������������ ������, ������� ������� FSCAN.
        free_sequence(seq); // ������������ ������, ������� ������������������� ��������.
    }
    return 0; // ���������� ���������.
}
