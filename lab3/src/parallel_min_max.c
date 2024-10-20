#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int active_child_processes = 0;
pid_t *child_pids;

void handle_alarm(int sig) {
    for (int i = 0; i < active_child_processes; i++) {
        kill(child_pids[i], SIGKILL);
    }
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;
  int timeout = -1;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {"timeout", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed < 0) {
                fprintf(stderr, "seed < 0\n");
                return 1;
              }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
                fprintf(stderr, "array_size < 0\n");
                return 1; 
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
                fprintf(stderr, "pnum <= 0\n");
                return 1;
            }
            break;
          case 3:
            with_files = true;
            break;
          case 4:
            timeout = atoi(optarg);
            if (timeout <= 0) {
                fprintf(stderr, "timeout <= 0\n");
                return 1;
            }
            break;
          default:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  child_pids = malloc(sizeof(pid_t) * pnum);

  int pipes[pnum][2];
  for (int i = 0; i < pnum; i++) {
      if (pipe(pipes[i]) == -1) {
          perror("pipe");
          exit(EXIT_FAILURE);
      }
  }

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      child_pids[i] = child_pid;
      active_child_processes += 1;
      if (child_pid == 0) {
        struct MinMax childMinMax;
        int start = array_size / pnum * i;
        int end = (i == pnum - 1) ? array_size : array_size / pnum * (i + 1);
        childMinMax = GetMinMax(array, start, end);
        // sleep(6);
        if (with_files) {
          char filename[16];
          sprintf(filename, "%d.txt", i); 
          FILE *file = fopen(filename, "w");
          fprintf(file, "%d %d\n", childMinMax.min, childMinMax.max);
          fclose(file);
        } else {
          close(pipes[i][0]);          
          write(pipes[i][1], &childMinMax.min, sizeof(childMinMax.min));
          write(pipes[i][1], &childMinMax.max, sizeof(childMinMax.max));
          close(pipes[i][1]); 
        }
        return 0;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }
  if (timeout > 0){
    signal(SIGALRM, handle_alarm);
    alarm(timeout);
  }

  while (active_child_processes > 0) {
    int status;
    pid_t child_pid = waitpid(-1, &status, WNOHANG);
    if (child_pid > 0) {
        active_child_processes -= 1;
        for (int i = 0; i < pnum; i++) {
            if (child_pids[i] == child_pid) {
                child_pids[i] = -1; 
                break;
            }
        }
    }
    }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      char filename[16];
      sprintf(filename, "%d.txt", i);
      FILE *file = fopen(filename, "r");
      if (file == NULL) {
          continue;
      }

      if (fscanf(file, "%d %d", &min, &max) != 2) {
          continue;
      }
      fclose(file);
      remove(filename);
    } else {
      close(pipes[i][1]);
      read(pipes[i][0], &min, sizeof(min));
      read(pipes[i][0], &max, sizeof(max));
      close(pipes[i][0]);
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}
