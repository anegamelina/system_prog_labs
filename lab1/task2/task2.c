#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef enum error{
    OK = 1,
    INPUT_ERROR = -2,
    FILE_ERROR = -3,
    MEMORY_ERROR = -4,
    PID_ERROR = -5,
    PROCESS_ERROR = -6
}error;

error xorN(char *filename, int N, uint64_t *res);

error mask(char *filename, char *mask, int *count);

error copyN(char *filename, int N);

error find_str(char **files, int num_of_files, char *str, char *found_in, char *flag_found);

error str_to_int(char *str, uint32_t *num);

error check_correct_N(char *N);

int main(int argc, char *argv[]) {
    error check;
    int i, N, count, num_of_files;
    char *answer, flag_found;
    uint64_t xor_res;

    if (argc < 3) {
        printf("Incorrect input. Try again.\n");
        return INPUT_ERROR;
    }

    if(strncmp(argv[argc - 1], "xor", 3) == 0) {
        N = strtol(argv[argc - 1] + 3, NULL, 10);
        if (N < 2 || N > 6) {
            printf("Incorrect input for xorN flag. N must be between 2 and 6.\n");
            return INPUT_ERROR;
        }
        for (i = 1; i < argc - 1; ++i) {
            check = xorN(argv[i], N, &xor_res);
            if (check == OK) {
                printf("XOR%d answer for file %s: %lu\n", N, argv[i], xor_res);
            } else {
                printf("Problem with files.\n");
                return FILE_ERROR;
            }
        }
    }
    else if(strcmp(argv[argc - 2], "mask") == 0) {
        for(i = 1; i < argc - 2; ++i) {
            check = mask(argv[i], argv[argc - 1], &count);
            if(check == OK){
                printf("Mask (%s) answer: %d\n", argv[i], count);
            }
            else if(check == FILE_ERROR){
                printf("Problem with files.\n");
                return FILE_ERROR;
            }
            else{
                printf("Incorrect input. Try again.\n");
                return INPUT_ERROR;
            }
        }
    }
    else if(strncmp(argv[argc - 1], "copy", 4) == 0){
        if(check_correct_N(argv[argc - 1]) != OK){
            printf("Incorrect number input.\n");
            return INPUT_ERROR;
        }
        else{
            N = strtol(argv[argc - 1] + 4, NULL, 10);
            if(N < 1 || N > 10){
                printf("Choose N in [1, 10]\n");
                return INPUT_ERROR;
            }

            for(i = 1; i < argc - 1; ++i){
                check = copyN(argv[i], N);
                if(check == OK){
                    printf("Successfully created %d copies for file %s.\n", N, argv[i]);
                }
                else if(check == FILE_ERROR){
                    printf("Problem with files.\n");
                    return FILE_ERROR;
                }
                else if(check == PID_ERROR){
                    printf("Problem with pid.\n");
                    return PID_ERROR;
                }
                else{
                    printf("Problem with child process.\n");
                    return PROCESS_ERROR;
                }
            }
        }
    }
    else if(strncmp(argv[argc - 2], "find", 4) == 0){
        answer = (char *) malloc(sizeof(char) * argc - 3);
        if(answer == NULL){
            printf("Problems with memory.\n");
            free(answer);
            return MEMORY_ERROR;
        }
        num_of_files = argc - 3;

        check = find_str((char **) argv + 1, num_of_files, argv[argc - 1], answer, &flag_found);

        if(check == OK){
            for(i = 0; i < argc - 3; ++i){
                if(flag_found && answer[i]){
                    printf("String was found in file %s\n", argv[i+1]);
                }
                else{
                    printf("String was not found in file %s\n", argv[i+1]);
                }
            }
        }
        free(answer);
    }
    else{
        printf("Incorrect input. Try again.\n");
        return INPUT_ERROR;
    }

    return OK;
}

error xorN(char *filename, int N, uint64_t *res){
    FILE *file;
    int i, cur_bit;
    uint64_t sum = 0, size_of_block, cur_block = 0, count = 0;
    uint8_t byte;

    file = fopen(filename, "rb"); // чтение в бинарном режиме

    if(!file){
        return FILE_ERROR;
    }

    size_of_block = 1ULL << N; // 2^N

    while(fread(&byte, 1, 1, file) == 1){
        for(i = 0; i < 8; ++i) {
            cur_bit = (byte >> i) & 1;

            cur_block = cur_block | (cur_bit << count);
            count++;

            if (count == size_of_block) {
                sum = sum ^ cur_block;
                cur_block = 0;
                count = 0;
            }
        }
    }

    if(count > 0){
        sum ^= cur_block;
    }

    *res = sum;
    fclose(file);
    return OK;
}

error mask(char *filename, char *mask, int *count){
    uint32_t hex_mask, num;
    int res = 0;
    FILE *file;

    file = fopen(filename, "rb"); // чтение в бинарном режиме

    if(!file){
        return FILE_ERROR;
    }

    if(strlen(mask) > 4 || str_to_int(mask, &hex_mask) != OK){
        fclose(file);
        return INPUT_ERROR;
    }

    while(fread(&num, sizeof(uint32_t), 1, file) == 1) {
        if (hex_mask & num) {
            ++res;
        }
    }

    fclose(file);
    *count = res;
    return OK;

}

error str_to_int(char *str, uint32_t *num){
    char *endptr;
    unsigned long res;

    res = strtoul(str, &endptr, 16);

    if(res > UINT32_MAX) {
        return INPUT_ERROR;
    }

    if(*endptr != '\0') {
        return INPUT_ERROR;
    }

    *num = (uint32_t) res;

    return OK;
}

error copyN(char *filename, int N){
    FILE *file, *new_file;
    int i, len, symbol, status;
    char extension[10], name[256], new_name[270], *dot;
    error has_error = OK;
    pid_t pid;

    file = fopen(filename, "r");

    if(!file){
        return FILE_ERROR;
    }

    dot = strrchr(filename, '.');
    if(!dot){
        fclose(file);
        return FILE_ERROR;
    }

    strcpy(extension, dot);
    extension[strlen(extension)] = '\0';

    len = dot - filename;
    strncpy(name, filename, len);

    name[len] = '\0';

    for(i = 0; i < N; ++i){
        pid = fork();

        if(pid < 0){
            fclose(file);
            return PID_ERROR;
        }

        if(pid == 0){
            file = fopen(filename, "r");
            if(!file){
                exit(FILE_ERROR);
            }

            snprintf(new_name, sizeof(new_name), "%s (%d)%s", name, i + 1, extension);
            new_file = fopen(new_name, "w");

            if(!new_file){
                fclose(file);
                exit(FILE_ERROR);
            }

            while((symbol = fgetc(file)) != EOF){
                fputc(symbol, new_file);
            }

            fclose(new_file);
            fclose(file);
            exit(OK);
        }
    }

    for(i = 0; i < N; ++i){
        wait(&status);
        if(WIFEXITED(status) && WEXITSTATUS(status) != OK){
            has_error = PROCESS_ERROR;
        }
    }

    return has_error;
}

error check_correct_N(char *N){
    int i, len;
    len = strlen(N);

    for (i = 4; i < len; ++i) {
        if (!isdigit(N[i])) {
            return INPUT_ERROR;
        }
    }
    return OK;
}

error find_str(char **files, int num_of_files, char *str, char *found_in, char *flag_found){
    char *sh_mem, symbol;
    int i, ind = 0, str_size, shm_id;
    pid_t pid;

    str_size = strlen(str);

    shm_id = shmget(IPC_PRIVATE, (num_of_files + 1) * sizeof(char), IPC_CREAT | 0666);

    if(shm_id == -1){
        return MEMORY_ERROR;
    }

    sh_mem = (char *) shmat(shm_id, NULL, 0);
    if(sh_mem == (void *)-1){
        shmctl(shm_id, IPC_RMID, NULL);
        return MEMORY_ERROR;
    }

    memset(sh_mem, 0, num_of_files + 1);
    for (i = 0; i < num_of_files; ++i) {
        pid = fork();

        if (pid < 0) {
            shmdt(sh_mem);
            shmctl(shm_id, IPC_RMID, NULL);
            return PID_ERROR;
        }

        if (pid == 0) {
            FILE *file = fopen(files[i], "r");
            if (!file) {
                exit(FILE_ERROR);
            }

            while ((symbol = fgetc(file)) != EOF) {
                if (str[ind] == symbol) {
                    if (ind == str_size - 1) {
                        sh_mem[num_of_files] = 1;
                        sh_mem[i] = 1;
                        break;
                    }
                    ind++;
                } else {
                    fseek(file, -ind, SEEK_CUR);
                    ind = 0;
                }
            }

            fclose(file);

            exit(OK);
        }

    }

    for (i = 0; i < num_of_files; i++) {
        wait(NULL);
    }

    *flag_found = sh_mem[num_of_files];

    memcpy(found_in, sh_mem, num_of_files * sizeof(char));
    shmdt(sh_mem);
    shmctl(shm_id, IPC_RMID, NULL);

    return OK;
}