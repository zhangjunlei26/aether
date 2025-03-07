#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 65536

// substitute_and_write:
// Searches the string 'src' for all occurrences of "builtin_print"
// and writes the output to 'out', replacing them with "printf".
void substitute_and_write(FILE *out, const char *src) {
    const char *needle = "builtin_print";
    size_t needle_len = strlen(needle);
    const char *p = src;
    const char *match;
    while ((match = strstr(p, needle)) != NULL) {
        // Write the segment before the match.
        fwrite(p, 1, match - p, out);
        // Write the replacement.
        fprintf(out, "printf");
        // Advance p past the match.
        p = match + needle_len;
    }
    // Write any remaining text.
    fputs(p, out);
}

// parse_func_signature:
// Parses "func" followed by optional spaces, a '(' then optional spaces,
// a ')' and returns. Exits on error.
void parse_func_signature(const char **p) {
    while (isspace(**p)) (*p)++;
    if (strncmp(*p, "func", 4) != 0) {
        fprintf(stderr, "Error: Expected 'func' after builtin_spawn\n");
        exit(1);
    }
    *p += 4;
    while (isspace(**p)) (*p)++;
    if (**p != '(') {
        fprintf(stderr, "Error: Expected '(' after 'func'\n");
        exit(1);
    }
    (*p)++; // skip '('
    while (isspace(**p)) (*p)++;
    if (**p != ')') {
        fprintf(stderr, "Error: Expected ')' after '(' in func signature\n");
        exit(1);
    }
    (*p)++; // skip ')'
}

// process_spawn_block:
// Processes a spawn block: extracts its contents, substitutes "builtin_print"
// with "printf", and outputs a new pthread function plus the pthread creation code.
static int spawn_counter = 0;
void process_spawn_block(const char **p, FILE *out) {
    while (isspace(**p)) (*p)++;
    parse_func_signature(p);
    while (isspace(**p)) (*p)++;
    if (**p != '{') {
        fprintf(stderr, "Error: Expected '{' after func() in spawn block\n");
        exit(1);
    }
    (*p)++; // skip '{'
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
    (*p)++; // skip '}'
    while (**p && **p != ';') (*p)++;
    if (**p == ';') (*p)++;

    spawn_counter++;
    char func_name[64];
    sprintf(func_name, "spawn_func_%d", spawn_counter);
    // Output the new function definition.
    fprintf(out, "\nvoid* %s(void* arg) {\n", func_name);
    substitute_and_write(out, block);
    fprintf(out, "\n    return NULL;\n}\n");
    // Output the pthread creation code.
    fprintf(out, "{ pthread_t thread; pthread_create(&thread, NULL, %s, NULL); pthread_detach(thread); }\n", func_name);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s input.aeth output.c\n", argv[0]);
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

    // Write standard headers.
    fprintf(fout, "#include <stdio.h>\n");
    fprintf(fout, "#include <stdlib.h>\n");
    fprintf(fout, "#include <pthread.h>\n\n");

    const char *p = source;
    while (*p) {
        // Skip lines starting with "import" or "module"
        if (strncmp(p, "import", 6) == 0 || strncmp(p, "module", 6) == 0) {
            while (*p && *p != '\n') p++;
            if (*p == '\n') {
                fputc('\n', fout);
                p++;
            }
            continue;
        }
        // Replace "func main()" with "int main()"
        if (strncmp(p, "func main()", 11) == 0) {
            fprintf(fout, "int main()");
            p += 11;
            continue;
        }
        // Replace "builtin_print" with "printf"
        if (strncmp(p, "builtin_print", 13) == 0) {
            fprintf(fout, "printf");
            p += 13;
            continue;
        }
        // Process "builtin_spawn"
        if (strncmp(p, "builtin_spawn", 13) == 0) {
            p += 13;
            while (*p && (isspace(*p) || *p == '(')) p++;
            process_spawn_block(&p, fout);
            continue;
        }
        // Copy character verbatim.
        fputc(*p, fout);
        p++;
    }

    free(source);
    fclose(fout);
    return 0;
}
