// ae - Aether command-line tool (Go-style interface)
// Usage: ae run file.ae, ae build file.ae, ae test, etc.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#define PATH_SEP "\\"
#define EXE_EXT ".exe"
#else
#include <unistd.h>
#include <sys/wait.h>
#define PATH_SEP "/"
#define EXE_EXT ""
#endif

#define AE_VERSION "0.4.0"

static char build_dir[512] = "build";
static char compiler_path[512];
static bool verbose = false;

void print_usage() {
    printf("Aether %s - Actor-based systems programming language\n\n", AE_VERSION);
    printf("Usage:\n");
    printf("  ae run <file>         Compile and run an Aether program\n");
    printf("  ae build <file>       Compile an Aether program to executable\n");
    printf("  ae compile <file>     Compile Aether to C code only\n");
    printf("  ae test               Run test suite\n");
    printf("  ae version            Show version information\n");
    printf("  ae help               Show this help message\n");
    printf("\n");
    printf("Options:\n");
    printf("  -v, --verbose         Show detailed compilation steps\n");
    printf("  -o <output>           Specify output file name\n");
    printf("\n");
    printf("Examples:\n");
    printf("  ae run hello.ae\n");
    printf("  ae build myapp.ae -o myapp\n");
    printf("  ae compile lib.ae\n");
}

int run_command(const char* cmd) {
    if (verbose) {
        printf("[Running] %s\n", cmd);
    }
    return system(cmd);
}

bool file_exists(const char* path) {
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path);
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
    return access(path, F_OK) == 0;
#endif
}

void find_compiler() {
    // Check if compiler exists in build directory
    snprintf(compiler_path, sizeof(compiler_path), "build%saetherc%s", PATH_SEP, EXE_EXT);
    if (file_exists(compiler_path)) {
        return;
    }
    
    // Check current directory
    snprintf(compiler_path, sizeof(compiler_path), "aetherc%s", EXE_EXT);
    if (file_exists(compiler_path)) {
        return;
    }
    
    // Check if installed in PATH
    if (system("aetherc --version > /dev/null 2>&1") == 0) {
        strcpy(compiler_path, "aetherc");
        return;
    }
    
    fprintf(stderr, "Error: Aether compiler not found\n");
    fprintf(stderr, "Run 'make compiler' to build it\n");
    exit(1);
}

char* get_basename(const char* path) {
    const char* base = strrchr(path, '/');
    if (!base) base = strrchr(path, '\\');
    if (!base) base = path; else base++;
    
    static char result[256];
    strncpy(result, base, sizeof(result) - 1);
    
    char* dot = strrchr(result, '.');
    if (dot) *dot = '\0';
    
    return result;
}

int cmd_run(const char* file) {
    char cmd[2048];
    char c_file[512];
    char exe_file[512];
    
    if (!file_exists(file)) {
        fprintf(stderr, "Error: File not found: %s\n", file);
        return 1;
    }
    
    printf("Compiling %s...\n", file);
    
    // Generate paths
    snprintf(c_file, sizeof(c_file), "%s%s_temp.c", build_dir, PATH_SEP);
    snprintf(exe_file, sizeof(exe_file), "%s%s_temp%s", build_dir, PATH_SEP, EXE_EXT);
    
    // Step 1: Compile .ae to .c
    snprintf(cmd, sizeof(cmd), "%s %s %s", compiler_path, file, c_file);
    if (run_command(cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        return 1;
    }
    
    // Step 2: Compile .c to executable
    printf("Building executable...\n");
    
#ifdef _WIN32
    // Windows: redirect stderr to suppress warnings
    snprintf(cmd, sizeof(cmd), 
        "gcc -O2 %s -o %s 2>nul",
        c_file, exe_file);
#else
    snprintf(cmd, sizeof(cmd), 
        "gcc -O2 %s -o %s 2>/dev/null",
        c_file, exe_file);
#endif
    
    if (run_command(cmd) != 0) {
        fprintf(stderr, "Build failed\n");
        return 1;
    }
    
    // Step 3: Run
    printf("Running...\n");
    printf("----------------------------------------\n");
    
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "%s", exe_file);
#else
    snprintf(cmd, sizeof(cmd), "./%s", exe_file);
#endif
    
    int result = run_command(cmd);
    printf("----------------------------------------\n");
    
    if (result != 0) {
        printf("Program exited with code %d\n", result);
    }
    
    return result;
}

int cmd_build(const char* file, const char* output) {
    char cmd[2048];
    char c_file[512];
    char exe_file[512];
    
    if (!file_exists(file)) {
        fprintf(stderr, "Error: File not found: %s\n", file);
        return 1;
    }
    
    const char* out_name = output ? output : get_basename(file);
    
    printf("Building %s -> %s...\n", file, out_name);
    
    // Generate paths
    snprintf(c_file, sizeof(c_file), "%s%s%s.c", build_dir, PATH_SEP, out_name);
    snprintf(exe_file, sizeof(exe_file), "%s%s%s%s", build_dir, PATH_SEP, out_name, EXE_EXT);
    
    // Step 1: Compile .ae to .c
    snprintf(cmd, sizeof(cmd), "%s %s %s", compiler_path, file, c_file);
    if (run_command(cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        return 1;
    }
    
    // Step 2: Compile .c to executable
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), 
        "gcc -O2 %s -o %s 2>nul",
        c_file, exe_file);
#else
    snprintf(cmd, sizeof(cmd), 
        "gcc -O2 %s -o %s 2>/dev/null",
        c_file, exe_file);
#endif
    
    if (run_command(cmd) != 0) {
        fprintf(stderr, "Build failed\n");
        return 1;
    }
    
    printf("Built: %s\n", exe_file);
    return 0;
}

int cmd_compile(const char* file, const char* output) {
    char cmd[1024];
    char c_file[512];
    
    if (!file_exists(file)) {
        fprintf(stderr, "Error: File not found: %s\n", file);
        return 1;
    }
    
    const char* out_name = output ? output : get_basename(file);
    snprintf(c_file, sizeof(c_file), "%s%s%s.c", build_dir, PATH_SEP, out_name);
    
    printf("Compiling %s -> %s...\n", file, c_file);
    
    snprintf(cmd, sizeof(cmd), "%s %s %s", compiler_path, file, c_file);
    if (run_command(cmd) != 0) {
        fprintf(stderr, "Compilation failed\n");
        return 1;
    }
    
    printf("Generated: %s\n", c_file);
    return 0;
}

int cmd_test() {
    printf("Running test suite...\n");
    return run_command("make test");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    find_compiler();
    
    const char* cmd = argv[1];
    const char* output = NULL;
    
    // Parse options
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output = argv[++i];
        }
    }
    
    // Commands
    if (strcmp(cmd, "run") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No input file specified\n");
            fprintf(stderr, "Usage: ae run <file>\n");
            return 1;
        }
        return cmd_run(argv[2]);
    }
    else if (strcmp(cmd, "build") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No input file specified\n");
            fprintf(stderr, "Usage: ae build <file> [-o output]\n");
            return 1;
        }
        return cmd_build(argv[2], output);
    }
    else if (strcmp(cmd, "compile") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: No input file specified\n");
            fprintf(stderr, "Usage: ae compile <file> [-o output]\n");
            return 1;
        }
        return cmd_compile(argv[2], output);
    }
    else if (strcmp(cmd, "test") == 0) {
        return cmd_test();
    }
    else if (strcmp(cmd, "version") == 0 || strcmp(cmd, "--version") == 0) {
        printf("Aether %s\n", AE_VERSION);
        return 0;
    }
    else if (strcmp(cmd, "help") == 0 || strcmp(cmd, "--help") == 0) {
        print_usage();
        return 0;
    }
    else {
        fprintf(stderr, "Error: Unknown command '%s'\n", cmd);
        fprintf(stderr, "Run 'ae help' for usage information\n");
        return 1;
    }
}
