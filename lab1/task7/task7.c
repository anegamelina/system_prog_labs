#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

typedef enum error{
    OK = 1,
    INPUT_ERROR = -1,
    DIR_ERROR = -2,
}error;

error make_list_files(char *directory);

int main(int argc, char *argv[]){
    int i;
    if(argc < 2){
        printf("Incorrect number of arguments.\n");
        return INPUT_ERROR;
    }

    for(i = 1; i < argc; ++i){
        printf("List of files for directory %s:\n", argv[i]);
        make_list_files(argv[i]);
        printf("\n");
    }

    return OK;
}

error make_list_files(char *directory){
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char filepath[BUFSIZ];

    dir = opendir(directory);
    if(dir == NULL)
        return DIR_ERROR;

    entry = readdir(dir);
    while(entry != NULL){

    }

}