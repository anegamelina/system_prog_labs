#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

typedef enum error{
    OK = 1,
    INPUT_ERROR = -1,
    MEMORY_ERROR = -2,
    FILE_ERROR = -3,
}error;

typedef struct User{
    char login[7];
    int pin;
    int sanctions;
}User;

int check_user_exist(User **users, int count, char *login, int pin);

error register_user(User **users, int *count, char *login, int pin);

error load_users(User **users, int *count, int *size);

error save_users(User **users, int count);

void print_time();

void print_date();

void print_howmuch(const char *time_str, char flag);

void sanctions(User *users, int user_count, const char *username, int number, int *sanctions);

int main(){
    User *users = NULL;
    int user_choice = 0, pin, is_logged_in = -1, sanctions_count = 1, users_count = 0, size = 1;
    char login[7], action[20];
    error check;
    int number;

    users = (User *) malloc(sizeof(User));
    if(!users){
        printf("Problems with memory.\n");
        return MEMORY_ERROR;
    }

    check = load_users(&users, &users_count, &size);

    if(check == FILE_ERROR){
        printf("Problems with file.\n");
        free(users);
        return FILE_ERROR;
    }
    else if(check == MEMORY_ERROR){
        printf("Problems with memory.\n");
        free(users);
        return MEMORY_ERROR;
    }


    while(1) {
        if(is_logged_in == -1) {
            printf("Choose the action: 1 - authorize, 2 - register, 3 - exit\n");
            scanf("%d", &user_choice);
            switch (user_choice) {
                case 1:
                    printf("Enter your login: ");
                    scanf("%s", login);
                    if(strlen(login) > 6){
                        printf("Login can't be longer than 6 characters.\n");
                        break;
                    }
                    printf("Enter your PIN-code: ");
                    scanf("%d", &pin);
                    is_logged_in = check_user_exist(&users, users_count, login, pin);
                    if(is_logged_in != -1){
                        sanctions_count = users[is_logged_in].sanctions;
                        printf("You have successfully logged in.\n");
                    }
                    else
                        printf("Incorrect login or PIN-code.\n");
                    break;

                case 2:
                    printf("Create a username(up to 6 characters): ");
                    scanf("%s", login);
                    if(strlen(login) > 6){
                        printf("Login can't be longer than 6 characters.\n");
                        break;
                    }
                    printf("Create a PIN-code:(from 0 to 100000): ");
                    scanf("%d", &pin);

                    if(pin < 0 || pin > 100000){
                        printf("PIN-code must be a number in [0, 100000].\n");
                        free(users);
                        break;
                    }
                    check = register_user(&users, &users_count, login, pin); // регистрация пользователя
                    printf("1 OK\n");

                    if(check == INPUT_ERROR){
                        printf("User with this login already exists.\n");
                        free(users);
                        return INPUT_ERROR;
                    }
                    else if(check == MEMORY_ERROR){
                        printf("Problems with memory.\n");
                        free(users);
                        return MEMORY_ERROR;
                    }
                    printf("You have successfully registered.\n");
                    check = save_users(&users, users_count);
                    if(check == FILE_ERROR){
                        printf("Problems with file.\n");
                        free(users);
                        return FILE_ERROR;
                    }
                    break;

                case 3:
                    printf("See you soon. Logging out of the system...\n");
                    free(users);
                    return OK;

                default:
                    printf("Incorrect choice input.\n");
                    break;
            }
        }
        else{
            if(sanctions_count == 0){
                printf("You have reached the limit of sanctions.\n");
                is_logged_in = 0;
                continue;
            }
            printf("Choose the action: ");
            scanf("%s", action);
            --sanctions_count;
            if(strcmp("Time", action) == 0){
                print_time();
            }
            else if(strcmp("Date", action) == 0){
                print_date();
            }
            else if(strcmp("Howmuch", action) == 0){
                char time_str[11];
                char flag;
                scanf("%10s %c", time_str, &flag);
                print_howmuch(time_str, flag);
            }
            else if(strcmp("Logout", action) == 0){
                is_logged_in = -1;
                printf("Logged out.\n");
                continue;
            }
            else if(strcmp("Sanctions", action) == 0){
                scanf("%6s %d", login, &number);
                sanctions(users, users_count, login, number, &sanctions_count);
            }
            else{
                printf("There is no such action.\n");
            }

            if(sanctions_count > 0)
                --sanctions_count;
        }
    }
    free(users);
    return OK;

}

int check_user_exist(User **users, int count, char *login, int pin){ // проверка на корректность данных при авторизации
    int i;
    for(i = 0; i < count; ++i){
        if(strcmp((*users)[i].login, login) == 0 && (*users)[i].pin == pin)
            return i; // возвращаем индекс пользователя
    }
    return -1;
}

error load_users(User **users, int *count, int *size){ // загрузка пользователей из файла в массив
    FILE *file;
    char login[7];
    int pin, sanctions;
    User *temp;
    file = fopen("users_info.txt", "r");
    if(!file)
        return FILE_ERROR;

    while(fscanf(file, "%6s %d %d", login, &pin, &sanctions) == 3){
        if(*size == *count) {
            *size *= 2;
            temp = (User *) realloc(*users, (*size) * sizeof(User));
            if (!temp) {
                fclose(file);
                return MEMORY_ERROR;
            }
            *users = temp;
        }

        strcpy((*users)[*count].login, login);
        (*users)[*count].pin = pin;
        (*users)[*count].sanctions = sanctions;
        ++(*count);

    }
    fclose(file);
    return OK;
}

error register_user(User **users, int *count, char *login, int pin){ // регистрация нового пользователя
    int i;
    User *temp = NULL;
    for(i = 0; i < *count; ++i){
        if(strcmp((*users)[i].login, login) == 0)
            return INPUT_ERROR;
    }

    temp = (User *) realloc(*users, (*count + 1) * sizeof(User));

    if(!temp) {
        return MEMORY_ERROR;
    }

    *users = temp;

    strcpy((*users)[*count].login, login);
    (*users)[*count].pin = pin;
    (*users)[*count].sanctions = -1; // по дефолту нет ограничений

    ++(*count);
    return OK;
}

error save_users(User **users, int count){
    FILE *file;
    int i;
    file = fopen("users_info.txt", "w");
    if(!file)
        return FILE_ERROR;

    for(i = 0; i < count; ++i)
        fprintf(file, "%s %d %d\n", (*users)[i].login, (*users)[i].pin, (*users)[i].sanctions);

    fclose(file);
    return OK;
}

void print_time(){
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    printf("Current time: %02d:%02d:%02d\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
}

void print_date(){
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    printf("Current date: %02d.%02d.%04d\n", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
}

void print_howmuch(const char *time_str, char flag){
    struct tm tm = {0};
    strptime(time_str, "%d.%m.%Y", &tm);
    time_t specified_time = mktime(&tm);
    time_t now = time(NULL);
    double diff = difftime(now, specified_time);

    switch (flag) {
        case 's':
            printf("Seconds passed: %.0f\n", diff);
            break;
        case 'm':
            printf("Minutes passed: %.0f\n", diff / 60);
            break;
        case 'h':
            printf("Hours passed: %.0f\n", diff / 3600);
            break;
        case 'y':
            printf("Years passed: %.0f\n", diff / (3600 * 24 * 365));
            break;
        default:
            printf("Invalid flag.\n");
    }
}

void sanctions(User *users, int user_count, const char *username, int number, int *sanctions) {
    if (number == 12345) {
        for (int i = 0; i < user_count; i++) {
            if (strcmp(users[i].login, username) == 0) {
                users[i].sanctions = number;
                *sanctions = number;
                printf("Sanctions applied to user %s.\n", username);
                save_users(&users, user_count); // сохраняем изменения в файл
                return;
            }
        }
        printf("User not found.\n");
    } else {
        printf("Invalid confirmation code.\n");
    }
}