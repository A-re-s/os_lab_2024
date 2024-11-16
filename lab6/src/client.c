#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdint.h>

#define MAX_SERVERS 100
#define MAX_IP_LENGTH 255

struct Server {
  char ip[255];
  int port;
};

struct Task {
    struct Server *server;
    uint64_t start;
    uint64_t end;
    uint64_t mod;
};

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a = a % mod;
  while (b > 0) {
    if (b % 2 == 1)
      result = (result + a) % mod;
    a = (a * 2) % mod;
    b /= 2;
  }

  return result % mod;
}


bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0)
    return false;

  *val = i;
  return true;
}

int read_servers(const char *filename, struct Server *servers, int max_servers) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Could not open file");
        return -1;
    }

    char line[512];
    int count = 0;

    while (fgets(line, sizeof(line), file) && count < max_servers) {
        line[strcspn(line, "\n")] = '\0';

        char *colon = strchr(line, ':');
        if (colon == NULL) {
            fprintf(stderr, "Invalid line format: %s\n", line);
            continue;
        }

        *colon = '\0';
        char *ip = line;
        int port = atoi(colon + 1);

        strncpy(servers[count].ip, ip, MAX_IP_LENGTH);
        servers[count].ip[MAX_IP_LENGTH - 1] = '\0';
        servers[count].port = port;

        count++;
    }

    fclose(file);
    return count;
}

void* compute_factorial_chunk(void *arg) {
    struct Task *task_data = (struct Task *)arg;
    struct Server *server = task_data->server;
    uint64_t begin = task_data->start;
    uint64_t end = task_data->end;
    uint64_t mod = task_data->mod;

    struct sockaddr_in server_addr;
    struct hostent *hostname = gethostbyname(server->ip);
    if (hostname == NULL) {
        fprintf(stderr, "gethostbyname failed with %s\n", server->ip);
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server->port);
    server_addr.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
        fprintf(stderr, "Socket creation failed!\n");
        return NULL;
    }

    if (connect(sck, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Connection failed\n");
        return NULL;
    }

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) < 0) {
        fprintf(stderr, "Send failed\n");
        close(sck);
        return NULL;
    }

    char response[sizeof(uint64_t)];
    if (recv(sck, response, sizeof(response), 0) < 0) {
        fprintf(stderr, "Receive failed\n");
        close(sck);
        return NULL;
    }

    uint64_t result = 0;
    memcpy(&result, response, sizeof(uint64_t));
    printf("Server %s:%d returned: %llu\n", server->ip, server->port, result);

    close(sck);
    return (void *)(uintptr_t)result;
}

int main(int argc, char **argv) {
    uint64_t k = -1;
    uint64_t mod = -1;
    char servers[255] = {'\0'};

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {{"k", required_argument, 0, 0},
                                           {"mod", required_argument, 0, 0},
                                           {"servers", required_argument, 0, 0},
                                           {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
        case 0: {
            switch (option_index) {
            case 0:
                ConvertStringToUI64(optarg, &k);
                break;
            case 1:
                ConvertStringToUI64(optarg, &mod);
                break;
            case 2:
                memcpy(servers, optarg, strlen(optarg));
                break;
            default:
                printf("Index %d is out of options\n", option_index);
            }
        } break;

        case '?':
            printf("Arguments error\n");
            break;
        default:
            fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    if (k == -1 || mod == -1 || !strlen(servers)) {
        fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n", argv[0]);
        return 1;
    }

    struct Server servers_list[MAX_SERVERS];
    unsigned int servers_num = read_servers(servers, servers_list, MAX_SERVERS);

    pthread_t threads[servers_num];
    uint64_t partial_results[servers_num];

    
    uint64_t chunk_size = k / servers_num;
    for (int i = 0; i < servers_num; i++) {
        struct Task *task_data = malloc(sizeof(struct Task));
        task_data->server = &servers_list[i];
        task_data->start = i * chunk_size + 1;
        task_data->end = (i == servers_num - 1) ? k : (i + 1) * chunk_size;
        task_data->mod = mod;

        pthread_create(&threads[i], NULL, compute_factorial_chunk, task_data);
    }

    uint64_t total_result = 1;
    for (int i = 0; i < servers_num; i++) {
        void *result;
        pthread_join(threads[i], &result);
        partial_results[i] = (uint64_t)(uintptr_t)result;
        total_result = MultModulo(total_result, partial_results[i], mod);
    }

    printf("Final result: %llu\n", total_result);

    return 0;
}
