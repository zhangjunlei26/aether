#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define BUFFER_SIZE 65536

/* process_spawn_block
   Processes a spawn block: extracts the block content, generates a new pthread function,
   and outputs the pthread creation code.
*/
static int spawn_counter = 0;
void process_spawn_block(const char **p, FILE *out) {
    while (isspace(**p)) (*p)++;
    if (strncmp(*p, "func()", 6) != 0) {
        fprintf(stderr, "Error: Expected 'func()' after spawn\n");
        exit(1);
    }
    *p += 6;
    while (isspace(**p)) (*p)++;
    if (**p != '{') {
        fprintf(stderr, "Error: Expected '{' after func()\n");
        exit(1);
    }
    (*p)++;
    char block[BUFFER_SIZE];
    int i = 0;
    while (**p && **p != '}') {
        block[i++] = **p;
        (*p)++;
        if (i >= BUFFER_SIZE - 1) {
            fprintf(stderr, "Error: Spawn block too large\n");
            exit(1);
        }
    }
    if (**p != '}') {
        fprintf(stderr, "Error: Unmatched '{' in spawn block\n");
        exit(1);
    }
    block[i] = '\0';
    (*p)++;
    while (**p && **p != ';') (*p)++;
    if (**p == ';') (*p)++;
    spawn_counter++;
    char func_name[64];
    sprintf(func_name, "spawn_func_%d", spawn_counter);
    fprintf(out, "\nvoid* %s(void* arg) {\n", func_name);
    fprintf(out, "%s\n", block);
    fprintf(out, "    return NULL;\n}\n");
    fprintf(out, "{ pthread_t thread; pthread_create(&thread, NULL, %s, NULL); pthread_join(thread, NULL); }\n", func_name);
}

/* process_if_statement
   Processes an if statement by copying the entire if statement (including its block) verbatim.
   It handles nested braces.
*/
void process_if_statement(const char **p, FILE *out) {
    int brace_count = 0;
    const char *start = *p;
    while (**p && **p != '{') {
        (*p)++;
    }
    if (**p == '{') {
        brace_count = 1;
        (*p)++; 
        while (brace_count > 0 && **p) {
            if (**p == '{') brace_count++;
            else if (**p == '}') brace_count--;
            (*p)++;
        }
    }
    size_t len = *p - start;
    fwrite(start, 1, len, out);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s input.ae output.c\n", argv[0]);
        return 1;
    }
    const char* input_filename = argv[1];
    const char* output_filename = argv[2];
    FILE *fin = fopen(input_filename, "r");
    if (!fin) {
        perror("Error opening input file");
        return 1;
    }
    fseek(fin, 0, SEEK_END);
    long fsize = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    char *source = malloc(fsize + 1);
    if (!source) {
        perror("Memory allocation error");
        fclose(fin);
        return 1;
    }
    fread(source, 1, fsize, fin);
    fclose(fin);
    source[fsize] = '\0';
    FILE *fout = fopen(output_filename, "w");
    if (!fout) {
        perror("Error opening output file");
        free(source);
        return 1;
    }
    fprintf(fout, "#include <stdio.h>\n");
    fprintf(fout, "#include <stdlib.h>\n");
    fprintf(fout, "#include <pthread.h>\n\n");
    const char *p = source;
    while (*p) {
        if (strncmp(p, "import", 6) == 0 || strncmp(p, "module", 6) == 0) {
            while (*p && *p != '\n') p++;
            if (*p == '\n') { fputc('\n', fout); p++; }
            continue;
        }
        if (strncmp(p, "func main()", 11) == 0) {
            fprintf(fout, "int main()");
            p += 11;
            continue;
        }
        if (strncmp(p, "print", 5) == 0) {
            fprintf(fout, "printf");
            p += 5;
            continue;
        }
        if (strncmp(p, "spawn", 5) == 0) {
            p += 5;
            while (*p && (isspace(*p) || *p == '(')) p++;
            process_spawn_block(&p, fout);
            continue;
        }
        if (strncmp(p, "if", 2) == 0) {
            process_if_statement(&p, fout);
            continue;
        }
        fputc(*p, fout);
        p++;
    }
    free(source);
    fclose(fout);
    return 0;
}
