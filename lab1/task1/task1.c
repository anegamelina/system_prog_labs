#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

typedef enum error{
    OK = 1,
    INPUT_ERROR = -1,
    MEMORY_ERROR = -2,
    FILE_ERROR = -3,
}error;

typedef struct User{
    char login[7];
    unsigned int pin;
    int sanctions;
}User;

int check_user_exist(User **users, int count, char *login, int pin);

error register_user(User **users, int *count, char *login, int pin);

error load_users(User **users, int *count, int *size); // загрузка пользователей из файла в массив

error save_users(User **users, int count);

void print_time();

void print_date();

void print_howmuch(const char *time_str, char *flag);

int sanctions(User *users, int user_count, const char *username, int number);

int is_valid_login(const char *login);

int is_login_correct(const char *login);

int main(){
    User *users = NULL;
    int user_choice = 0, is_logged_in = -1, sanctions_count = 1, users_count = 0, size = 1;
    char login[7], action[20];
    unsigned int pin;
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
            while(scanf("%d", &user_choice) != 1 || user_choice < 1 || user_choice > 3){
                printf("Incorrect action input. Please choose 1, 2, or 3.\n");
                while(getchar() != '\n');
            }
            switch (user_choice) {
                case 1:
                    printf("Enter your login: ");
                    while(1) {
                        if (scanf("%7s", login) != 1) {
                            printf("Incorrect input. Please try again:\n");
                            while(getchar() != '\n');
                            continue;
                        }

                        if (!is_login_correct(login)) {
                            printf("Login must be up to 6 characters and contain at least one letter and one digit. Please try again:\n");
                            while(getchar() != '\n');
                            continue;
                        }
                        break;
                    }
                    printf("Enter your PIN-code: ");
                    while(scanf("%u", &pin) != 1){
                        printf("Incorrect PIN-code input. Please enter a valid PIN-code:\n");
                        while(getchar() != '\n');
                    }
                    is_logged_in = check_user_exist(&users, users_count, login, pin);
                    if(is_logged_in != -1){
                        sanctions_count = users[is_logged_in].sanctions;
                        printf("You have successfully logged in.\n");
                    }
                    else
                        printf("Incorrect login or PIN-code.\n");
                    break;

                case 2:
                    printf("Create a username (up to 6 characters, must contain at least one letter and one digit): ");
                    while(1) {
                        if (scanf("%7s", login) != 1) {
                            printf("Incorrect input. Please try again:\n");
                            while(getchar() != '\n');
                            continue;
                        }

                        if (!is_login_correct(login)) {
                            printf("Login must be up to 6 characters and contain at least one letter and one digit. Please try again:\n");
                            while(getchar() != '\n');
                            continue;
                        }
                        break;
                    }
                    printf("Create a PIN-code (from 0 to 100000): ");
                    while(scanf("%u", &pin) != 1 || pin < 0 || pin > 100000){
                        printf("PIN-code must be a number in [0, 100000]. Please try again:");
                        while(getchar() != '\n');
                    }
                    check = register_user(&users, &users_count, login, pin); // регистрация пользователя

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
                    printf("Incorrect action input.\n");
                    break;
            }
        }
        else {
            if (sanctions_count == 0) {
                    printf("You have reached the limit of sanctions for this user.\n");
                    is_logged_in = -1;
                    continue;
                }
            printf("Choose the action:\n");
            printf("Time - current time in format hh:mm:ss\n");
            printf("Date - current date in format dd.mm.yyyy\n");
            printf("Howmuch dd.mm.yyyy -flag - how much time has passed since the date(-s in seconds, -m in minutes, -h in hours, -y in years)\n");
            printf("Logout - log out to the authorization menu\n");
            printf("Sanctions username number_of_sanctions - set sanctions for the user\n");
            printf("Your choice:\n");

            scanf("%s", action);
            --sanctions_count;

            if (strcmp("Time", action) == 0) {
                print_time();
            } else if (strcmp("Date", action) == 0) {
                print_date();
            } else if (strcmp("Howmuch", action) == 0) {
                char time_str[11];
                char flag[3];
                if (scanf("%11s %3s", time_str, flag) != 2) {
                    printf("Incorrect input.\n");
                } else {
                    if(time_str[10] != '\0' || flag[2] != '\0' || flag[0] != '-'){
                        printf("Incorrect input.\n");
                    }
                    else {
                        print_howmuch(time_str, flag);
                    }
                }
            } else if (strcmp("Logout", action) == 0) {
                is_logged_in = -1;
                printf("Logged out.\n");
                continue;
            } else if (strcmp("Sanctions", action) == 0) {
                char username[7];
                int num;
                if (scanf("%7s %d", username, &num) == 2) {
                    int confirmation_code;
                    printf("Enter confirmation code to apply sanctions: ");
                    scanf("%d", &confirmation_code);

                    if (confirmation_code == 12345) {
                        if (sanctions(users, users_count, username, num)) {
                            printf("Sanctions applied to user %s. Limit: %d requests per session.\n", username,
                                   num);
                        } else {
                            printf("User not found.\n");
                        }
                    } else {
                        printf("Invalid confirmation code. Sanctions not applied.\n");
                    }
                }
                else {
                    printf("Incorrect input for Sanctions command.\n");
                }
            }
            else {
                printf("There is no such action.\n");
            }

            printf("Remaining requests: %d\n", sanctions_count);
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
    int sanctions;
    unsigned int pin;
    User *temp;
    file = fopen("users_info.txt", "r");
    if(!file)
        return FILE_ERROR;

    while(fscanf(file, "%6s %u %d", login, &pin, &sanctions) == 3){
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

int is_valid_login(const char *login) {
    int has_letter = 0;
    int has_digit = 0;

    for (int i = 0; login[i] != '\0'; i++) {
        if (!isalnum(login[i])) {
            return 0;
        }
        if (isalpha(login[i])) {
            has_letter = 1;
        }
        if (isdigit(login[i])) {
            has_digit = 1;
        }
    }

    return has_letter && has_digit;
}

int is_login_correct(const char *login) {
    if (strlen(login) > 6) {
        return 0;
    }
    if (!is_valid_login(login)) {
        return 0;
    }
    return 1;
}

void print_time(){ // запрос текущего времени в стандартном формате чч:мм:сс
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    printf("Current time: %02d:%02d:%02d\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
}

void print_date(){ // запрос текущей даты в стандартном формате дд.мм.гггг
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    printf("Current date: %02d.%02d.%04d\n", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
}

void print_howmuch(const char *time_str, char *flag){ // запрос прошедшего времени с указанной даты
    int day, month, year;
    double difference;

    if (sscanf(time_str, "%d.%d.%d", &day, &month, &year) != 3) {
        printf("Invalid date format. Use dd.mm.yyyy\n");
        return;
    }

    if (year < 1900) {
        printf("Invalid year.\n");
        return;
    }

    if (month < 1 || month > 12) {
        printf("Invalid month. Month must be between 1 and 12.\n");
        return;
    }

    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) { // если високосный год, то в феврале 29 дней
        days_in_month[1] = 29;
    }

    if (day < 1 || day > days_in_month[month - 1]) {
        printf("Invalid day. Day is out of range for the specified month and year.\n");
        return;
    }

    struct tm tm = {0};
    tm.tm_mday = day;
    tm.tm_mon = month - 1;
    tm.tm_year = year - 1900;

    time_t specified_time = mktime(&tm);
    if (specified_time == -1) {
        printf("Error converting date.\n");
        return;
    }

    time_t now = time(NULL);

    difference = difftime(now, specified_time);

    switch (flag[1]) {
        case 's':
            printf("Seconds passed: %.0f\n", difference);
            break;
        case 'm':
            printf("Minutes passed: %.0f\n", difference / 60);
            break;
        case 'h':
            printf("Hours passed: %.0f\n", difference / 3600);
            break;
        case 'y':
            printf("Years passed: %.0f\n", difference / (3600 * 24 * 365));
            break;
        default:
            printf("Invalid flag. Use -s, -m, -h, or -y.\n");
    }
}

int sanctions(User *users, int user_count, const char *username, int number) { // санкции
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].login, username) == 0) {
            users[i].sanctions = number;
            save_users(&users, user_count);
            return 1;
        }
    }
    return 0;
}