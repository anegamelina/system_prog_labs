#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

typedef enum error{
    OK = 1,
    INPUT_ERROR = -1,
    DIR_ERROR = -2,
    MEMORY_ERROR = -3,
    STAT_ERROR = -4
}error;

typedef struct file_info{
    char *name;
    char type[16];
    unsigned long inode;
    unsigned long blocks;
}file_info;

error make_list_files(char *directory, file_info **files, int *num, int *size);

void free_all_files(file_info *files, int num);

int main(int argc, char *argv[]){
    int i, j = 0, num_of_files = 0, size = 1;
    file_info *files;
    error check;

    if(argc < 2){
        printf("Incorrect number of arguments.\n");
        return INPUT_ERROR;
    }

    files = (file_info *) malloc(sizeof(file_info));

    if(files == NULL){
        printf("Problems with memory.\n");
        return MEMORY_ERROR;
    }

    for(i = 1; i < argc; ++i){
        printf("List of files for directory %s:\n", argv[i]);
        check = make_list_files(argv[i], &files, &num_of_files, &size);
        if(check == DIR_ERROR){
            printf("Failed to open directory %s\n", argv[i]);
            free_all_files(files, num_of_files);
            return DIR_ERROR;
        }
        else if(check == MEMORY_ERROR){
            printf("Problems with memory.\n");
            free_all_files(files, num_of_files);
            return MEMORY_ERROR;
        }
        else if(check == STAT_ERROR){
            printf("Problems with stat.\n");
            free_all_files(files, num_of_files);
            return STAT_ERROR;
        }

        else{
            for(; j < num_of_files; ++j){
                printf("%s%-15s | Inode: %-8lu | Blocks: %-8lu\n", files[j].name, files[j].type, files[j].inode, files[j].blocks);
            }
        }
    }

    free_all_files(files, num_of_files);
    return OK;
}

error make_list_files(char *directory, file_info **files, int *num, int *size){
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char filepath[BUFSIZ];
    file_info *check;

    dir = opendir(directory);
    if(dir == NULL)
        return DIR_ERROR;

    while ((entry = readdir(dir)) != NULL){
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if(*size == *num){
            *size *= 2;
            check = (file_info *) realloc(*files, sizeof(file_info) * (*size));
            if(check == NULL) {
                return MEMORY_ERROR;
            }
            *files = check;
        }

        snprintf(filepath, sizeof(filepath), "%s/%s", directory, entry->d_name);
        if (stat(filepath, &file_stat) == -1) {
            return STAT_ERROR;
        }

        (*files)[*num].name = (char *) malloc((strlen(entry->d_name) + 1) * sizeof(char));

        if((*files)[*num].name == NULL){
            return MEMORY_ERROR;
        }

        strcpy((*files)[*num].name, entry->d_name);

        if (S_ISDIR(file_stat.st_mode)) {
            strcpy((*files)[*num].type,"(directory)");
        } else if (S_ISREG(file_stat.st_mode)) {
            strcpy((*files)[*num].type,"(file)");
        } else if (S_ISLNK(file_stat.st_mode)) {
            strcpy((*files)[*num].type,"(symbolic link)");
        } else {
            strcpy((*files)[*num].type,"(other)");
        }

        (*files)[*num].inode = file_stat.st_ino;
        (*files)[*num].blocks = file_stat.st_blocks;

        ++(*num);
    }

    closedir(dir);
    return OK;
}

void free_all_files(file_info *files, int num){
    int i;
    for(i = 0; i < num; ++i){
        free(files[i].name);
    }
    free(files);
}