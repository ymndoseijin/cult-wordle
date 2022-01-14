#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <uchar.h>

#define FIXED_NUM 13783815777230004908
#define HELP 6951207451432

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\x1B[0m"

#define BUFFER_SIZE 4096
#define DICTIONARY_SIZE 500000
#define WORD_SIZE 100

int next_num = 0;
int cheater = 0;
int obscured = 0;
long force_num = 0;
/* Default value */
char *path = "/usr/share/dict/words";
char *output_message;

const unsigned long hash(const char *str) {
    unsigned long hash = 5381;  
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

char apostrophe[2] = {'\'','s'};
char *dictionary[DICTIONARY_SIZE];
char *game_string;
size_t dictionary_offset;

struct simple_date {
    uint16_t year;
    uint8_t month;
    uint8_t day;
};

union hacky {
    struct simple_date date;
    unsigned int hack;
};  

void filter_input_word(char **ptr)
{
    char *word = *ptr;
    char *clone_buf = &word[0];
    while (*clone_buf != '\0') {
        char cs = *clone_buf;
        if (cs == '\n') {
            *clone_buf = '\0';
            break;
        } else {
            *clone_buf = (char)tolower(cs);
        }
        clone_buf++;
    }
}    

int char_in_buf(char *buf, char c, size_t size) 
{
    int return_value = 0;
    char *clone_buf = &buf[0];
    while (*clone_buf != '\0') {
        char cs = *clone_buf;
        if (cs == c)
            return_value = 1;
        clone_buf++;
    }
    return return_value;
}

int parse_buffer(char (*buffer_ptr)[BUFFER_SIZE], char (*word_ptr)[WORD_SIZE], char (*compare_ptr)[2], size_t word_offset, ssize_t nrd)
{
    char *buffer = *buffer_ptr;    
    char *word = *word_ptr;    
    char *compare_buffer = *compare_ptr;    

    char *clone_buffer = &buffer[0];

    while (clone_buffer < buffer + nrd) {
        char c = *clone_buffer;
        int ignore_word;

        if (c != '\0') {
            if (c != '\n') {
                compare_buffer[0] = compare_buffer[1];
                compare_buffer[1] = c;

                if (strncmp(compare_buffer, apostrophe, 2) == 0 || isupper(c)) {
                    ignore_word = 1;
                }

                if (!ignore_word) {
                    word[word_offset] = c;
                    word_offset++;
                }
            } else {
                if (force_num && word_offset != force_num)
                    ignore_word = 1;
                if (word_offset <= 2)
                    ignore_word = 1;
                if (!ignore_word) {
                    word[word_offset] = '\0';

                    dictionary[dictionary_offset] = malloc(sizeof(char)*word_offset+1);
                    strncpy(dictionary[dictionary_offset], word, word_offset+1);

                    if (dictionary_offset+1 >= DICTIONARY_SIZE) {
                        return -1;
                    }
                    dictionary_offset++;
                } else {
                    ignore_word = 0;
                }

                memset(word, 0, WORD_SIZE);
                memset(compare_buffer, 0, 2);
                word_offset = 0;
            }
        }
        clone_buffer++;
    }

    return 0;
}

int fill_buffer(int fd)
{
    char word[WORD_SIZE];
    char compare_buffer[2];
    memset(word, 0, WORD_SIZE);
    memset(compare_buffer, 0, 2);

    size_t word_offset = 0;

    char buffer[BUFFER_SIZE];
    ssize_t nrd;
    while ((nrd = read(fd, buffer, BUFFER_SIZE))) {
        if (nrd < 0)
            return -1;
        if (parse_buffer(&buffer, &word, &compare_buffer, word_offset, nrd) < 0)
            return -1;
    }
    return 0;
}

char *choose_random_word(void)
{
    long unsigned int loop_count = dictionary_offset/RAND_MAX;
    long unsigned int offset;

    if (loop_count >= RAND_MAX)
        exit(-1); /* fuck off */

    if (loop_count != 0) {
        long unsigned int selector = (loop_count != 0) ? rand() % loop_count : 0;
        long unsigned int remainder = dictionary_offset-RAND_MAX;
        offset = selector*RAND_MAX+(rand() % remainder);
    } else {
        offset = rand() % dictionary_offset;
    }
    
    return *(dictionary+offset);
}

int get_input(char **input_ptr)
{
    int valid = 0;
    char *input = *input_ptr;

    while (!valid) {
        printf("> ");
        if (fgets(input, WORD_SIZE, stdin) != NULL) {
            filter_input_word(&input);
            int space_answer = strchr(input, ' ') ? 1 : 0;
            int size_answer = strlen(input) != strlen(game_string);
            if (!(space_answer || size_answer))
                valid = 1;
        } else {
            return -1;
        }
    }
    return 0;
}
        
void valid_order(char *c, char **message_ptr)
{
    char *message = *message_ptr;
    strcat(message, KGRN);
    strcat(message, c);
    strcat(message, RESET);
}

void valid_inside(char *c, char **message_ptr)
{
    char *message = *message_ptr;
    strcat(message, KYEL);
    strcat(message, c);
    strcat(message, RESET);
}

void invalid(char *c, char **message_ptr)
{
    char *message = *message_ptr;
    strcat(message, KRED);
    strcat(message, c);
    strcat(message, RESET);
}

int word_in_dict(char *word)
{
    return 1;
    int valid = 0;
    for (size_t i = 0; i < dictionary_offset; i++)
        if (strcmp(*(dictionary+i), word) == 0)
            valid = 1;
    
    return valid;
}

void help(void)
{
    printf("This is just a wordle clone, there isn't much to it at all\n");
}

void version(void)
{
    printf("people who do version control are idiots\n");
}

int parse_argument(char *argument)
{
    if (!next_num) {
        switch (hash(argument)) {
            case FIXED_NUM:
                next_num = 1;
                return 1;
                break;
            case HELP:
                help();
                return 0;
            default:
                break;
        }                
        if (argument[0] == '-') {
            char *clone_buf = malloc(sizeof(char)*strlen(argument));
            strcpy(clone_buf, argument);
            int return_after = 0;
            while (*clone_buf != '\0') {
                char cs = *clone_buf;
                switch (cs) {
                    case 'h':
                        help();
                        return 0;
                    case 'v':
                        version();
                        break;
                    case 'o':
                        obscured = 1;
                        break;
                    case 'n':
                        next_num = 1;
                        break;
                    case 'c':
                        cheater = 1;
                        printf("CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER\n");
                        break;
                    case '-':
                        break;
                    default:
                        printf("Invalid argument %c.\n", cs);
                        return -1;
                }
                clone_buf++;
            }
            if (return_after == 1)
                return 1;
        } else {
            path = argument;
        }
    } else {
        char *endptr = malloc(sizeof(char)*strlen(argument));
        force_num = strtol(argument, &endptr, 10);
        next_num = 0;
    }

    return 1;
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("not enough arguments!");
        return -1;
    }

    for (int i = 1; i < argc; i++) {
        int response = parse_argument(argv[i]);
        if (response == 0)
            return 0;
        else if (response == -1)
            return -1;
    }

    int fd = open(path, O_RDONLY);

    if (fill_buffer(fd) < 0)
        return -1;

    printf("The game has about %lu words.\n", dictionary_offset);
    if (dictionary_offset == 0) {
        printf("The game can't be played!\n");
        return -1;
    }

    struct timespec ts;    
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm *my_tm = localtime(&ts.tv_sec);
    struct simple_date date = { my_tm->tm_year, my_tm->tm_mon, my_tm->tm_mday };
    union hacky seed;
    seed.date = date;
    
    srand(seed.hack);
    game_string = choose_random_word();
    if (cheater)
        printf("%s\n", game_string);
    
    printf("Today's word is %lu letters long.\n", strlen(game_string));
    char *input = malloc(sizeof(char)*WORD_SIZE);
    memset(input, 0, WORD_SIZE);
    printf("Give me a try.\n");

    if (get_input(&input) < 0) {
        goto finish;
    }

    int correct = 1;
    char *output_message = malloc(sizeof(char)*2048);
    memset(output_message, 0, sizeof(char)*2048);
    while (1) {
        char *message = malloc(sizeof(char)*2048);
        memset(message, 0, sizeof(char)*2048);
        if (word_in_dict(input)) {
            char *clone_input = &game_string[0];
            char character_set[WORD_SIZE];
            size_t cs_offset = 0;

            while (*clone_input != '\0') {
                char c = *clone_input;
                int will_add = char_in_buf(character_set, c, WORD_SIZE);

                if (!will_add) {
                    character_set[cs_offset] = c;
                    cs_offset++;
                }

                clone_input++;
            }

            size_t word_offset = 0;
            char *clone_word = &input[0];
            while (*clone_word != '\0') {
                char c = *clone_word;

                if (word_offset < strlen(game_string)) {
                    char character[2] = {c, '\0'};
                    if (game_string[word_offset] == c) {
                        valid_order(character, &message);
                        valid_order("■", &output_message);
                    } else {
                        if (char_in_buf(character_set, c, WORD_SIZE) != 0) {
                            correct = 0;
                            valid_inside(character, &message);
                            valid_inside("■", &output_message);
                        } else {
                            correct = 0;
                            invalid(character, &message);
                            invalid("■", &output_message);
                        }
                    }
                }
                word_offset++;
                clone_word++;
            }
        } else {
            strcpy(message, "Invalid message, sorry.");
            correct = 0;
        }

        strcat(message, "\n");
        printf(message);
        free(message);
        strcat(output_message, "\n");
    
        if (correct)
            break;
        
        correct = 1;

        if (get_input(&input) < 0) {
            goto finish;
        }
    }
    printf("That's it!\n");
finish:
    strcat(output_message, "\n");
    printf(output_message);
    free(output_message);
    printf("\nfinishing up\n");
    for (size_t i = 0; i < dictionary_offset; i++)
        free(dictionary[i]);
    close(fd);
    return 0;
}
