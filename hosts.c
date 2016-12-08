#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define CR '\r'
#define LF '\n'
#define HOSTS_FILE "/etc/hosts"
#define MAX_FILE_SIZE 4096

typedef struct {
    char *domain;
    void *next;
} hosts_domain_t;

typedef struct {
    char           *ip;
    char           *annotation;
    int             domain_sum;
    hosts_domain_t *domain;
    void *next;
} hosts_line_t;

char *get_token(char *p, char *token_begin)
{
    char *token;

    if ((token = malloc(p - token_begin + 1)) == NULL) {
        printf("malloc mem error!\n");
        exit(1);
    }

    memcpy(token, token_begin, p - token_begin);
    token[p - token_begin] = '\0';

    return token;
}

hosts_domain_t *add_domain(hosts_line_t *hosts_line, 
    hosts_domain_t *hosts_domain, char *token) 
{
    hosts_domain_t *next_hosts_domain;

    if ((next_hosts_domain = malloc(sizeof(hosts_domain))) == NULL) {
        printf("malloc mem error!\n");
        exit(1);
    }

    if (hosts_line->domain == NULL) {
        hosts_line->domain = next_hosts_domain;
    } else {
        hosts_domain->next = next_hosts_domain;
    }

    hosts_domain = next_hosts_domain;
    hosts_domain->next = NULL;
    hosts_domain->domain = token;

    return hosts_domain;
}

hosts_line_t *init_hosts_line() {
    hosts_line_t *hosts_line;

    if ((hosts_line = malloc(sizeof(hosts_line_t))) == NULL) {
        printf("malloc mem error!\n");
        exit(1);
    }

    hosts_line->ip = NULL;
    hosts_line->annotation = NULL;
    hosts_line->domain_sum = 0;
    hosts_line->domain = NULL;

    return hosts_line;
}

hosts_line_t *load_hosts()
{
    int file_size;
    char *p, *end, *token_begin, *token, *annotation_begin;
    FILE *fp;
    struct stat st;
    hosts_line_t *hosts_line, *first_hosts_line, *next_hosts_line;
    hosts_domain_t *hosts_domain;

    hosts_domain = NULL;
    token_begin = NULL;
    annotation_begin = NULL;

    fp = fopen(HOSTS_FILE, "r");
    if (fp == NULL) {
        printf("cannot open file: %s\n", HOSTS_FILE);
        exit(1);
    }

    stat(HOSTS_FILE, &st);
    file_size = st.st_size;

    if (file_size > MAX_FILE_SIZE) {
        printf("file size is to big: %d\n", file_size);
        exit(1);
    }

    if ((p =  malloc(file_size + 2)) == NULL) {
        printf("malloc mem error!\n");
        exit(1);
    }

    p[file_size + 1] = LF;
    end = p + file_size;

    fread(p, file_size, 1, fp);
    fclose(fp);

    hosts_line = init_hosts_line();
    first_hosts_line = hosts_line;

    for (/* void */; p < end; p++) {
        switch (*p) {

        case '#':
            annotation_begin = p;
            while(*p != CR && *p != LF && p != end) {
                p++;
            }

            --p;
            break;

        case ' ':
        case '\t':
            if (token_begin) {
                token = get_token(p, token_begin);
                token_begin = NULL;
    
                if (hosts_line->ip == NULL) {
                    hosts_line->ip = token;
                } else {
                    hosts_domain = add_domain(hosts_line, hosts_domain, token);
                }
            }

            break;

        case CR:
        case LF:
            if (token_begin) {
                token = get_token(p, token_begin);
                hosts_domain = add_domain(hosts_line, hosts_domain, token);
                token_begin = NULL;
            }

            if (annotation_begin) {
                token = get_token(p, annotation_begin);
                annotation_begin = NULL;
                hosts_line->annotation = token;
            }

            while ((*p == CR || *p == LF) && p != end) {
                p++;
            }

            if (p < end) {
                next_hosts_line = init_hosts_line();
                hosts_line->next = next_hosts_line;
                hosts_line = next_hosts_line;
            }

            --p;

            break;
        default:
            if (token_begin == NULL) {
                token_begin = p;
            }
        }
    }

    return first_hosts_line;
}

int main(){
    hosts_line_t *hosts;
    hosts_domain_t *hosts_domain;

    hosts = load_hosts();

    while(hosts){
        if (hosts->ip) {
            printf("%-20s", hosts->ip);
            hosts_domain = hosts->domain;
            while(hosts_domain){
                printf(" %s", hosts_domain->domain);
                hosts_domain = hosts_domain->next;
            }
            printf("\n");
        }

      /*  
        if (hosts->annotation){
            printf("%s", hosts->annotation);
        }

        printf("\n");
      */
        hosts = hosts->next;
    }
}
