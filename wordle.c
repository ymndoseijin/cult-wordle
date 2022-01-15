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
#include <sys/timeb.h>
#include <stdint.h>

#define FIXED_NUM 13783815777230004908
#define HELP 6951207451432
#define VERSION 249805560270004805
#define CHETER 7569864722246490
#define OBSCURE 249805551112946930
#define RANDOM 7569865301282272
#define SEED 6951207846496

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
#define DICTIONARY_SIZE 1000000
#define WORD_SIZE 100

char *command_name;
char *output_message;

int next_num = 0;
int next_seed = 0;

int set_arg_seed = 0;
/* Default values */
int cheater = 0;
int obscured = 0;

int arg_random = 0;
int arg_seed = 0;

long force_num = 0;
char *path = "/usr/share/dict/words";

const unsigned long hash(const char *str) {
    unsigned long hash = 5381;  
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

size_t utf8len(char *s)
{
    size_t len = 0;
    for (; *s; ++s) if ((*s & 0xC0) != 0x80) ++len;
    return len;
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
    size_t word_len = 0;

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
                    if ((c & 0xC0) != 0x80) word_len++;
                }
            } else {
                if (force_num && (word_len != force_num))
                    ignore_word = 1;
                if (word_offset <= 2)
                    ignore_word = 1;
                if (!ignore_word) {
                    word[word_offset] = '\0';

                    dictionary[dictionary_offset] = malloc(sizeof(char)*word_offset+1);
                    strncpy(dictionary[dictionary_offset], word, word_offset+1);

                    if (dictionary_offset+1 >= DICTIONARY_SIZE) {
                        printf("dictionary_offset larger than dictionary size\n");
                        return 0;
                    }
                    dictionary_offset++;
                } else {
                    ignore_word = 0;
                }

                memset(word, 0, WORD_SIZE);
                memset(compare_buffer, 0, 2);
                word_offset = 0;
                word_len = 0;
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
        if (nrd < 0) {
            printf("for some reason, read file failed\n");
            return -1;
        } if (parse_buffer(&buffer, &word, &compare_buffer, word_offset, nrd) < 0) {
            printf("parse buffer failed\n");
            return -1;
        }
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
    int valid = 0;
    for (size_t i = 0; i < dictionary_offset; i++)
        if (strcmp(*(dictionary+i), word) == 0)
            valid = 1;
    
    return valid;
}

void help(void)
{
    printf("This is just a wordle clone, there isn't much to it at all\n");
    printf("usage: %s [option] ...\n", command_name);
    printf("       %s [option] ... dictionary-file\n\n", command_name);
    printf("  -h, --help shows this help\n");
    printf("  -v, --version outputs the \"version\"\n");
    printf("  -o, --obscure obscure final score letters\n");
    printf("  -c, --cheter CHETER!!\n");
    printf("  -n, --fixed-num [length] fixes word length in game, by default, off\n");
    printf("  -r, --random set seed to random mode, default daily seed\n");
    printf("  -s, --seed set [seed] seed to [seed], default daily seed\n");
}

void version(void)
{
    printf("%s built on %s\n", command_name, __DATE__);
    printf("No copyright at all.\n");
}


int parse_argument(char *argument)
{
    if (next_num) {
        char *endptr = malloc(sizeof(char)*strlen(argument));
        force_num = strtol(argument, &endptr, 10);
        next_num = 0;
    } else if (next_seed) {
        char *endptr = malloc(sizeof(char)*strlen(argument));
        arg_seed = strtol(argument, &endptr, 10);
        next_seed = 0;
        set_arg_seed = 1;
    } else {
        /* long arguments, they have been manually hashed, not too hard */
        switch (hash(argument)) {
            case FIXED_NUM:
                next_num = 1;
                return 1;
            case HELP:
                help();
                return 0;
            case VERSION:
                version();
                return 0;
            case RANDOM:
                arg_random = 1;
                next_seed = 0;
                return 1;
            case SEED:
                arg_random = 0;
                next_seed = 1;
                return 1;
            case CHETER:
                cheater = 1;
                printf("CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER\n");
                return 1;
            case OBSCURE:
                obscured = 1;
                return 1;
            default:
                break;
        }                
        if (argument[0] == '-') {
            char *clone_buf = malloc(sizeof(char)*strlen(argument));
            strcpy(clone_buf, argument);
            int return_after = 0;
            clone_buf++;
            while (*clone_buf != '\0') {
                char cs = *clone_buf;
                switch (cs) {
                    case 'h':
                        help();
                        return 0;
                    case 'v':
                        version();
                        return 0;
                    case 'o':
                        obscured = 1;
                        break;
                    case 'n':
                        next_num = 1;
                        break;
                    case 'r':
                        arg_random = 1;
                        next_seed = 0;
                        break;
                    case 's':
                        arg_random = 0;
                        next_seed = 1;
                        break;
                    case 'c':
                        cheater = 1;
                        printf("CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER CHETER\n");
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
    }

    return 1;
}

int character_response(char c, int word_offset, char **message_ptr, char character_set[WORD_SIZE])
{
    char *message = *message_ptr;
    char character[2] = {c, '\0'};
    if (game_string[word_offset] == c) {
        valid_order(character, &message);
        
        if ((c & 0xC0) != 0x80) {
            if (obscured)
                valid_order("■ ", &output_message);
            else
                valid_order(character, &output_message);
        }
        return 1;
    } else {
        if (char_in_buf(character_set, c, WORD_SIZE) != 0) {
            valid_inside(character, &message);
            if ((c & 0xC0) != 0x80) {
                if (obscured)
                    valid_inside("■ ", &output_message);
                else
                    valid_inside(character, &output_message);
            }
            return 0;
        } else {
            invalid(character, &message);
            if ((c & 0xC0) != 0x80) {
                if (obscured)
                    invalid("■ ", &output_message);
                else
                    invalid(character, &output_message);
            }
            return 0;
        }
    }
}

int get_input(char **input_ptr)
{
    int valid = 0;
    char *input = *input_ptr;

    printf("> ");
    if (fgets(input, WORD_SIZE, stdin) != NULL) {
        filter_input_word(&input);
        int space_answer = strchr(input, ' ') ? 1 : 0;
        int size_answer = utf8len(input) == utf8len(game_string);
        int inside_answer = word_in_dict(input);
        /* printf("response from get_input is %i, %i, %i\n", space_answer, size_answer, inside_answer); */
        if (!space_answer && size_answer && inside_answer) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return -1;
    }
}

int main(int argc, char *argv[])
{
    command_name = argv[0];

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

    unsigned int seed;
    if (set_arg_seed) {
        seed = arg_seed;
    } else if (arg_random) {
        struct timeb time_now;
        ftime(&time_now);
        seed = time_now.time*1000+time_now.millitm;
    } else {
        struct timeb time_now;
        ftime(&time_now);
        struct tm *my_tm = localtime(&time_now.time);
        struct simple_date date = { my_tm->tm_year, my_tm->tm_mon, my_tm->tm_mday };
        union hacky seed_union;
        seed_union.date = date;
        seed = seed_union.hack;
    }
    
    srand(seed);
    game_string = choose_random_word();
    if (cheater)
        printf("%s\n", game_string);
    
    printf("Today's word is %lu letters long.\n", utf8len(game_string));
    char *input = malloc(sizeof(char)*WORD_SIZE);
    memset(input, 0, WORD_SIZE);
    printf("Give me a try.\n");


    int correct = 1;
    output_message = malloc(sizeof(char)*2048);
    memset(output_message, 0, sizeof(char)*2048);
    while (1) {
        char *message = malloc(sizeof(char)*2048);
        memset(message, 0, sizeof(char)*2048);
        int response = get_input(&input);
        if (response == 0) {
            strcpy(message, "Invalid message, sorry.\n");
            printf(message);
            free(message);
            continue;
        } else if (response == -1) {
            goto finish;
        }
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
        int skip_word = 0;
        int skip_input = 0;
        while (*clone_word != '\0') {
            char c = *clone_word;

            if (word_offset >= strlen(game_string)) {
                if (skip_word) {
                    char character[2] = {c, '\0'};
                    invalid(character, &message);
                }
                break;
            }

            int input_is_ascii = (c & 0x80) != 0x80;
            int input_is_utf8_start = ((c & 0xC0) != 0x80) && input;

            if (skip_input && (input_is_ascii || input_is_utf8_start)) {
                skip_input = 0;
                if (word_offset >= strlen(game_string)) break;
            }

            int word_is_ascii = (game_string[word_offset] & 0x80) == 0x80;
            int word_is_utf8_start = ((game_string[word_offset] & 0xC0) == 0x80) && word_is_ascii;

            if (skip_word && (word_is_ascii || word_is_utf8_start)) {
                skip_word = 0;
                char character[2] = {c, '\0'};
                invalid(character, &message);
                invalid("■ ", &output_message);
                clone_word++;
                c = *clone_word;
                if (c == '\0') break;
            }

            int both_unicode = ((c & 0x80) == 0x80) && ((game_string[word_offset] & 0x80) == 0x80);
            int both_normal = ((c & 0x80) != 0x80) && ((game_string[word_offset] & 0x80) != 0x80);


            /*
                might put this on an argument later, who knows
                
                printf("responding character (%c) %02x and (%c) %02x\n", c, c, game_string[word_offset], game_string[word_offset]);
                printf("response for both %i %i\n", both_unicode, both_normal);
                printf("response for just game_string %i\n", ((game_string[word_offset] & 0xC0) != 0x80));
            */

            if (both_unicode || both_normal) {

                correct = (character_response(c, word_offset, &message, character_set) == 0) ? 0 : correct;

                word_offset++;
                clone_word++;
            } else {
                correct = 0;
                if ((c & 0x80) == 0x80) {
                    if (!skip_input)
                        skip_input = 1;
                    char character[2] = {c, '\0'};
                    invalid(character, &message);
                    invalid("■ ", &output_message);
                    clone_word++;
                }
                
                if ((game_string[word_offset] & 0x80) == 0x80) {
                    if (!skip_word)
                        skip_word = 1;
                    word_offset++;
                }

            }
        }

        strcat(message, "\n");
        printf(message);
        free(message);

        if (correct != -1)
            strcat(output_message, "\n");
    
        if (correct == 1)
            break;
        
        correct = 1;
    }
    printf("That's it!\n");
finish:
    printf(output_message);
    free(output_message);
    printf("\nfinishing up\n");
    for (size_t i = 0; i < dictionary_offset; i++)
        free(dictionary[i]);
    close(fd);
    return 0;
}
