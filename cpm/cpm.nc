#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PATH_MAX_LEN 1024
#define VALUE_MAX_LEN 512
#define SOURCES_MAX_LEN 4096
#define MAX_SOURCES 256
#define CMD_MAX_LEN (PATH_MAX_LEN * 16)

#ifndef CPM_DEFAULT_STDLIB_DIR
#define CPM_DEFAULT_STDLIB_DIR ""
#endif

#define DEFAULT_NEOC_FLAGS "-I. -Ilib"
#define DEFAULT_SIZE_FLAGS "-O2"
#define VAL_FLAGS "-g -fno-omit-frame-pointer"
#define LEAK_FLAGS "-g -fno-omit-frame-pointer -fsanitize=address,leak"
#define LEAK_RUN_PREFIX "env ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:exitcode=99 LSAN_OPTIONS=exitcode=99"

static char g_exe_dir[PATH_MAX_LEN] = "";

struct Manifest {
    char name[VALUE_MAX_LEN];
    char version[VALUE_MAX_LEN];
    char src[PATH_MAX_LEN];
    char sources[SOURCES_MAX_LEN];
    char out[PATH_MAX_LEN];
    char neoc[VALUE_MAX_LEN];
    char neoc_flags[VALUE_MAX_LEN];
    char cc[VALUE_MAX_LEN];
    char cflags[VALUE_MAX_LEN];
    char linker[VALUE_MAX_LEN];
    char linker_flags[VALUE_MAX_LEN];
    char linker_script[PATH_MAX_LEN];
    char ldflags[VALUE_MAX_LEN];
    int jobs;
    int lowmem;
    int bare;
    int strip;
    int strip_sections;
};

struct SourceList {
    char path[MAX_SOURCES][PATH_MAX_LEN];
    int count;
};

struct BuildJob {
    char cmd[CMD_MAX_LEN];
    char generated_base[PATH_MAX_LEN];
    char c_path[PATH_MAX_LEN];
    char backup_path[PATH_MAX_LEN];
};

static int ensure_parent_dir(const char* path);
static int is_runtime_source(const char* src);

static void die(const char* msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

static void die_errno(const char* msg)
{
    perror(msg);
    exit(1);
}

static int file_exists(const char* path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

static int dir_exists(const char* path)
{
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static int has_suffix(const char* s, const char* suffix)
{
    size_t len = strlen(s);
    size_t suffix_len = strlen(suffix);
    return len >= suffix_len && strcmp(s + len - suffix_len, suffix) == 0;
}

static char* trim(char* s)
{
    char* end;
    while(isspace((unsigned char)*s)) {
        s++;
    }
    end = s + strlen(s);
    while(end > s && isspace((unsigned char)end[-1])) {
        end--;
    }
    *end = '\0';
    return s;
}

static const char* base_name(const char* path)
{
    const char* slash = strrchr(path, '/');
    return slash == NULL ? path : slash + 1;
}

static void mkdir_one(const char* path)
{
    if(mkdir(path, 0777) != 0 && errno != EEXIST) {
        die_errno(path);
    }
}

static void mkdir_p(const char* path)
{
    char tmp[PATH_MAX_LEN];
    char* p;
    if(strlen(path) >= sizeof(tmp)) {
        die("cpm: path too long");
    }
    strcpy(tmp, path);
    for(p = tmp + 1; *p != '\0'; p++) {
        if(*p == '/') {
            *p = '\0';
            mkdir_one(tmp);
            *p = '/';
        }
    }
    mkdir_one(tmp);
}

static void write_file(const char* path, const char* text)
{
    FILE* fp = fopen(path, "w");
    if(fp == NULL) {
        die_errno(path);
    }
    fputs(text, fp);
    if(fclose(fp) != 0) {
        die_errno(path);
    }
}

static void copy_file(const char* src, const char* dst)
{
    FILE* in = fopen(src, "rb");
    FILE* out;
    char buf[8192];
    size_t n;

    if(in == NULL) {
        die_errno(src);
    }
    out = fopen(dst, "wb");
    if(out == NULL) {
        fclose(in);
        die_errno(dst);
    }
    while((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        if(fwrite(buf, 1, n, out) != n) {
            fclose(in);
            fclose(out);
            die_errno(dst);
        }
    }
    if(ferror(in)) {
        fclose(in);
        fclose(out);
        die_errno(src);
    }
    if(fclose(in) != 0) {
        fclose(out);
        die_errno(src);
    }
    if(fclose(out) != 0) {
        die_errno(dst);
    }
}

static int backup_generated_file(const char* generated, char* backup, size_t backup_size)
{
    const char* base;
    if(!file_exists(generated)) {
        backup[0] = '\0';
        return 0;
    }
    base = base_name(generated);
    snprintf(backup, backup_size, "target/debug/.cpm-save-%s", base);
    if(strlen(backup) >= backup_size - 1) {
        die("cpm: backup path too long");
    }
    unlink(backup);
    if(rename(generated, backup) != 0) {
        die_errno(generated);
    }
    return 1;
}

static void restore_generated_file(const char* generated, const char* backup)
{
    if(backup[0] == '\0') {
        return;
    }
    unlink(generated);
    if(rename(backup, generated) != 0) {
        die_errno(backup);
    }
}

static int move_generated_file(const char* generated, const char* dest, const char* backup)
{
    unlink(dest);
    if(rename(generated, dest) != 0) {
        restore_generated_file(generated, backup);
        perror(generated);
        return 1;
    }
    restore_generated_file(generated, backup);
    return 0;
}

static void join_path(char* out, size_t out_size, const char* dir, const char* name)
{
    if(dir[0] == '\0') {
        snprintf(out, out_size, "%s", name);
    }
    else {
        size_t len = strlen(dir);
        if(len > 0 && dir[len - 1] == '/') {
            snprintf(out, out_size, "%s%s", dir, name);
        }
        else {
            snprintf(out, out_size, "%s/%s", dir, name);
        }
    }
    if(strlen(out) >= out_size - 1) {
        die("cpm: path too long");
    }
}

static void dirname_of(char* out, size_t out_size, const char* path)
{
    const char* slash = strrchr(path, '/');
    size_t n;
    if(slash == NULL) {
        if(getcwd(out, out_size) == NULL) {
            die_errno("getcwd");
        }
        return;
    }
    n = (size_t)(slash - path);
    if(n == 0) {
        n = 1;
    }
    if(n >= out_size) {
        die("cpm: executable path too long");
    }
    memcpy(out, path, n);
    out[n] = '\0';
}

static void set_exe_dir(const char* argv0)
{
    char resolved[PATH_MAX_LEN];
    if(argv0 == NULL || argv0[0] == '\0') {
        if(getcwd(g_exe_dir, sizeof(g_exe_dir)) == NULL) {
            die_errno("getcwd");
        }
        return;
    }
    if(strchr(argv0, '/') != NULL) {
        if(realpath(argv0, resolved) != NULL) {
            dirname_of(g_exe_dir, sizeof(g_exe_dir), resolved);
        }
        else {
            dirname_of(g_exe_dir, sizeof(g_exe_dir), argv0);
        }
        return;
    }
    {
        const char* path_env = getenv("PATH");
        char path_buf[PATH_MAX_LEN * 8];
        char* p;
        if(path_env != NULL) {
            strncpy(path_buf, path_env, sizeof(path_buf) - 1);
            path_buf[sizeof(path_buf) - 1] = '\0';
            p = strtok(path_buf, ":");
            while(p != NULL) {
                char candidate[PATH_MAX_LEN];
                join_path(candidate, sizeof(candidate), p, argv0);
                if(access(candidate, X_OK) == 0) {
                    if(realpath(candidate, resolved) != NULL) {
                        dirname_of(g_exe_dir, sizeof(g_exe_dir), resolved);
                    }
                    else {
                        dirname_of(g_exe_dir, sizeof(g_exe_dir), candidate);
                    }
                    return;
                }
                p = strtok(NULL, ":");
            }
        }
    }
    if(getcwd(g_exe_dir, sizeof(g_exe_dir)) == NULL) {
        die_errno("getcwd");
    }
}

static int stdlib_dir_has_headers(const char* dir)
{
    char path[PATH_MAX_LEN];
    join_path(path, sizeof(path), dir, "neo-c.h");
    return file_exists(path);
}

static int find_stdlib_dir(char* out, size_t out_size)
{
    const char* env_dir = getenv("CPM_STDLIB_DIR");
    const char* candidates[] = {
        env_dir,
        CPM_DEFAULT_STDLIB_DIR,
        ".",
        "..",
        "../..",
        "/usr/local/include",
        "/usr/local/share/neo-c",
        "/usr/local/share/neo-c/neo-c",
        NULL
    };
    int i;

    if(env_dir != NULL && env_dir[0] != '\0' && stdlib_dir_has_headers(env_dir)) {
        strncpy(out, env_dir, out_size - 1);
        out[out_size - 1] = '\0';
        return 1;
    }
    if(CPM_DEFAULT_STDLIB_DIR[0] != '\0' && stdlib_dir_has_headers(CPM_DEFAULT_STDLIB_DIR)) {
        strncpy(out, CPM_DEFAULT_STDLIB_DIR, out_size - 1);
        out[out_size - 1] = '\0';
        return 1;
    }
    if(g_exe_dir[0] != '\0') {
        char path[PATH_MAX_LEN];
        join_path(path, sizeof(path), g_exe_dir, "..");
        if(stdlib_dir_has_headers(path)) {
            strncpy(out, path, out_size - 1);
            out[out_size - 1] = '\0';
            return 1;
        }
        join_path(path, sizeof(path), g_exe_dir, "../share/neo-c");
        if(stdlib_dir_has_headers(path)) {
            strncpy(out, path, out_size - 1);
            out[out_size - 1] = '\0';
            return 1;
        }
    }
    for(i = 2; candidates[i] != NULL; i++) {
        if(stdlib_dir_has_headers(candidates[i])) {
            strncpy(out, candidates[i], out_size - 1);
            out[out_size - 1] = '\0';
            return 1;
        }
    }
    return 0;
}

static void copy_if_exists(const char* src_dir, const char* name)
{
    char src[PATH_MAX_LEN];
    char dst[PATH_MAX_LEN];
    join_path(src, sizeof(src), src_dir, name);
    if(!file_exists(src)) {
        char src_name[PATH_MAX_LEN];
        snprintf(src_name, sizeof(src_name), "src/%s", name);
        if(strlen(src_name) >= sizeof(src_name) - 1) {
            die("cpm: path too long");
        }
        join_path(src, sizeof(src), src_dir, src_name);
    }
    if(!file_exists(src)) {
        return;
    }
    join_path(dst, sizeof(dst), "lib", name);
    copy_file(src, dst);
}

static void copy_standard_library(void)
{
    char stdlib_dir[PATH_MAX_LEN];
    if(!find_stdlib_dir(stdlib_dir, sizeof(stdlib_dir))) {
        die("cpm: neo-c standard library not found. set CPM_STDLIB_DIR=/path/to/neo-c");
    }
    mkdir_p("lib");
    copy_if_exists(stdlib_dir, "neo-c.h");
    copy_if_exists(stdlib_dir, "neo-c-str.nc");
    copy_if_exists(stdlib_dir, "neo-c-str.h");
    copy_if_exists(stdlib_dir, "neo-c-libc.h");
    copy_if_exists(stdlib_dir, "neo-c-net.h");
    copy_if_exists(stdlib_dir, "neo-c-pthread.h");
    if(!file_exists("lib/neo-c.h")) {
        die("cpm: failed to copy neo-c.h");
    }
    if(!file_exists("lib/neo-c-str.nc")) {
        die("cpm: failed to copy neo-c-str.nc");
    }
}

static void shell_quote(char* out, size_t out_size, const char* s)
{
    size_t n = 0;
    if(out_size < 3) {
        die("cpm: quote buffer too small");
    }
    out[n++] = '\'';
    while(*s != '\0') {
        if(*s == '\'') {
            const char* q = "'\\''";
            while(*q != '\0') {
                if(n + 1 >= out_size) {
                    die("cpm: command too long");
                }
                out[n++] = *q++;
            }
        }
        else {
            if(n + 1 >= out_size) {
                die("cpm: command too long");
            }
            out[n++] = *s;
        }
        s++;
    }
    if(n + 2 >= out_size) {
        die("cpm: command too long");
    }
    out[n++] = '\'';
    out[n] = '\0';
}

static int run_cmd(const char* cmd)
{
    int rc;
    puts(cmd);
    fflush(stdout);
    rc = system(cmd);
    if(rc != 0) {
        fprintf(stderr, "cpm: command failed: %s\n", cmd);
        return 1;
    }
    return 0;
}

static int parse_positive_int(const char* s)
{
    char* end = NULL;
    long value;
    if(s == NULL || s[0] == '\0') {
        return 0;
    }
    value = strtol(s, &end, 10);
    if(end == s || value <= 0 || value > 1024) {
        return 0;
    }
    return (int)value;
}

static int parse_bool_value(const char* s)
{
    if(s == NULL) {
        return 0;
    }
    return strcmp(s, "true") == 0 || strcmp(s, "1") == 0 ||
        strcmp(s, "yes") == 0 || strcmp(s, "on") == 0;
}

static int lowmem_enabled(const struct Manifest* m)
{
    const char* env_lowmem = getenv("CPM_LOWMEM");
    if(env_lowmem != NULL && env_lowmem[0] != '\0' && parse_bool_value(env_lowmem)) {
        return 1;
    }
    return m->lowmem;
}

static void build_extra_flags(char* out, size_t out_size, const char* extra_flags,
    const struct Manifest* m)
{
    const char* base = extra_flags == NULL ? "" : extra_flags;
    if(lowmem_enabled(m)) {
        snprintf(out, out_size, "%s -lowmem", base);
    }
    else {
        snprintf(out, out_size, "%s", base);
    }
    if(strlen(out) >= out_size - 1) {
        die("cpm: command too long");
    }
}

static int detect_default_jobs(void)
{
    long n = 1;
#ifdef _SC_NPROCESSORS_ONLN
    n = sysconf(_SC_NPROCESSORS_ONLN);
#endif
    if(n < 1) {
        n = 1;
    }
    if(n > MAX_SOURCES) {
        n = MAX_SOURCES;
    }
    return (int)n;
}

static int effective_jobs(const struct Manifest* m)
{
    int jobs = 0;
    const char* env_jobs = getenv("CPM_JOBS");
    if(lowmem_enabled(m)) {
        return 1;
    }
    if(env_jobs != NULL && env_jobs[0] != '\0') {
        jobs = parse_positive_int(env_jobs);
    }
    if(jobs <= 0 && m->jobs > 0) {
        jobs = m->jobs;
    }
    if(jobs <= 0) {
        jobs = detect_default_jobs();
    }
    if(jobs < 1) {
        jobs = 1;
    }
    if(jobs > MAX_SOURCES) {
        jobs = MAX_SOURCES;
    }
    return jobs;
}

static int run_parallel_jobs(struct BuildJob* jobs, int count, int max_jobs)
{
    int next = 0;
    int running = 0;
    int failed = 0;

    if(count <= 0) {
        return 0;
    }
    if(max_jobs <= 1) {
        int i;
        for(i = 0; i < count; i++) {
            if(run_cmd(jobs[i].cmd) != 0) {
                return 1;
            }
        }
        return 0;
    }

    printf("cpm: running %d build jobs with -j%d\n", count, max_jobs);
    fflush(stdout);
    while(next < count || running > 0) {
        while(next < count && running < max_jobs && !failed) {
            pid_t pid = fork();
            if(pid < 0) {
                perror("fork");
                failed = 1;
                break;
            }
            if(pid == 0) {
                _exit(run_cmd(jobs[next].cmd) == 0 ? 0 : 1);
            }
            next++;
            running++;
        }
        if(running > 0) {
            int status = 0;
            pid_t pid = wait(&status);
            if(pid < 0) {
                if(errno == EINTR) {
                    continue;
                }
                perror("wait");
                failed = 1;
                break;
            }
            running--;
            if(!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                failed = 1;
            }
        }
    }
    return failed ? 1 : 0;
}

static void unquote_value(char* dst, size_t dst_size, const char* src)
{
    size_t len;
    src = trim((char*)src);
    len = strlen(src);
    if(len >= 2 && src[0] == '"' && src[len - 1] == '"') {
        src++;
        len -= 2;
    }
    if(len >= dst_size) {
        len = dst_size - 1;
    }
    memcpy(dst, src, len);
    dst[len] = '\0';
}

static void manifest_defaults(struct Manifest* m)
{
    memset(m, 0, sizeof(*m));
    strcpy(m->name, "app");
    strcpy(m->version, "1.0.0");
    strcpy(m->src, "src/main.nc");
    strcpy(m->neoc, "neo-c");
    strcpy(m->neoc_flags, DEFAULT_NEOC_FLAGS);
    strcpy(m->cc, "clang");
    strcpy(m->cflags, "-Oz -ffreestanding -fno-asynchronous-unwind-tables -fno-ident -fno-stack-protector -fno-unwind-tables -nostdlib");
    strcpy(m->linker, "ld");
    strcpy(m->linker_flags, "-nostdlib -static -n --build-id=none");
    m->strip = 0;
}

static void manifest_finish(struct Manifest* m)
{
    const char* env_neoc = getenv("CPM_NEOC");
    if(env_neoc != NULL && env_neoc[0] != '\0') {
        strncpy(m->neoc, env_neoc, sizeof(m->neoc) - 1);
        m->neoc[sizeof(m->neoc) - 1] = '\0';
    }
    if(m->out[0] == '\0') {
        snprintf(m->out, sizeof(m->out), "target/debug/%s", m->name);
    }
}

static void read_manifest(struct Manifest* m)
{
    FILE* fp;
    char line[4096];
    char section[VALUE_MAX_LEN] = "";

    manifest_defaults(m);
    fp = fopen("Neo.toml", "r");
    if(fp == NULL) {
        die("cpm: Neo.toml not found");
    }
    while(fgets(line, sizeof(line), fp) != NULL) {
        char* s = trim(line);
        char* eq;

        if(*s == '\0' || *s == '#') {
            continue;
        }
        if(*s == '[') {
            char* r = strchr(s, ']');
            if(r != NULL) {
                *r = '\0';
                strncpy(section, s + 1, sizeof(section) - 1);
                section[sizeof(section) - 1] = '\0';
            }
            continue;
        }
        eq = strchr(s, '=');
        if(eq == NULL) {
            continue;
        }
        *eq = '\0';
        {
            char* key = trim(s);
            char* value = trim(eq + 1);
            if(strcmp(section, "package") == 0 && strcmp(key, "name") == 0) {
                unquote_value(m->name, sizeof(m->name), value);
            }
            else if(strcmp(section, "package") == 0 && strcmp(key, "version") == 0) {
                unquote_value(m->version, sizeof(m->version), value);
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "src") == 0) {
                unquote_value(m->src, sizeof(m->src), value);
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "sources") == 0) {
                unquote_value(m->sources, sizeof(m->sources), value);
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "out") == 0) {
                unquote_value(m->out, sizeof(m->out), value);
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "neoc") == 0) {
                unquote_value(m->neoc, sizeof(m->neoc), value);
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "neoc_flags") == 0) {
                unquote_value(m->neoc_flags, sizeof(m->neoc_flags), value);
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "cc") == 0) {
                unquote_value(m->cc, sizeof(m->cc), value);
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "cflags") == 0) {
                unquote_value(m->cflags, sizeof(m->cflags), value);
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "linker") == 0) {
                unquote_value(m->linker, sizeof(m->linker), value);
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "linker_flags") == 0) {
                unquote_value(m->linker_flags, sizeof(m->linker_flags), value);
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "linker_script") == 0) {
                unquote_value(m->linker_script, sizeof(m->linker_script), value);
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "ldflags") == 0) {
                unquote_value(m->ldflags, sizeof(m->ldflags), value);
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "jobs") == 0) {
                char unquoted[VALUE_MAX_LEN];
                unquote_value(unquoted, sizeof(unquoted), value);
                m->jobs = parse_positive_int(unquoted);
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "lowmem") == 0) {
                char unquoted[VALUE_MAX_LEN];
                unquote_value(unquoted, sizeof(unquoted), value);
                m->lowmem = parse_bool_value(unquoted);
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "bare") == 0) {
                char unquoted[VALUE_MAX_LEN];
                unquote_value(unquoted, sizeof(unquoted), value);
                m->bare = parse_bool_value(unquoted);
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "strip") == 0) {
                char unquoted[VALUE_MAX_LEN];
                unquote_value(unquoted, sizeof(unquoted), value);
                m->strip = strcmp(unquoted, "false") != 0 && strcmp(unquoted, "0") != 0;
            }
            else if(strcmp(section, "build") == 0 && strcmp(key, "strip_sections") == 0) {
                char unquoted[VALUE_MAX_LEN];
                unquote_value(unquoted, sizeof(unquoted), value);
                m->strip_sections = parse_bool_value(unquoted);
            }
        }
    }
    fclose(fp);
    manifest_finish(m);
}

static void source_list_add(struct SourceList* sources, const char* path)
{
    int i;
    for(i = 0; i < sources->count; i++) {
        if(strcmp(sources->path[i], path) == 0) {
            return;
        }
    }
    if(sources->count >= MAX_SOURCES) {
        die("cpm: too many source files");
    }
    strncpy(sources->path[sources->count], path, PATH_MAX_LEN - 1);
    sources->path[sources->count][PATH_MAX_LEN - 1] = '\0';
    sources->count++;
}

static void collect_sources_dir(struct SourceList* sources, const char* dir)
{
    DIR* dp;
    struct dirent* ent;

    dp = opendir(dir);
    if(dp == NULL) {
        return;
    }
    while((ent = readdir(dp)) != NULL) {
        char path[PATH_MAX_LEN];
        if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
        if(dir_exists(path)) {
            collect_sources_dir(sources, path);
        }
        else if(has_suffix(path, ".nc")) {
            source_list_add(sources, path);
        }
    }
    closedir(dp);
}

static void collect_sources(struct SourceList* sources, const char* main_src)
{
    memset(sources, 0, sizeof(*sources));
    source_list_add(sources, main_src);
    collect_sources_dir(sources, "src");
}

static void collect_manifest_sources(struct SourceList* sources, const struct Manifest* m)
{
    char buf[SOURCES_MAX_LEN];
    char* tok;

    if(m->sources[0] == '\0') {
        collect_sources(sources, m->src);
        return;
    }

    memset(sources, 0, sizeof(*sources));
    strncpy(buf, m->sources, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    tok = strtok(buf, " \t,");
    while(tok != NULL) {
        source_list_add(sources, tok);
        tok = strtok(NULL, " \t,");
    }
    if(sources->count == 0) {
        die("cpm: sources is empty");
    }
}

static void object_path(char* out, size_t out_size, const char* src, const struct Manifest* m)
{
    size_t i;
    size_t n;
    if(is_runtime_source(src)) {
        snprintf(out, out_size, "target/debug/neo-c-str.o");
        return;
    }
    if(strcmp(src, m->src) == 0) {
        snprintf(out, out_size, "target/debug/%s.o", m->name);
        return;
    }
    snprintf(out, out_size, "target/debug/%s", src);
    n = strlen(out);
    if(n >= 3 && strcmp(out + n - 3, ".nc") == 0) {
        out[n - 2] = 'o';
        out[n - 1] = '\0';
    }
    else if(n + 3 < out_size) {
        strcat(out, ".o");
    }
    for(i = strlen("target/debug/"); out[i] != '\0'; i++) {
        if(out[i] == '/') {
            out[i] = '_';
        }
    }
}

static void generated_c_basename(char* out, size_t out_size, const char* src)
{
    const char* base = base_name(src);
    size_t len = strlen(base);
    if(len + 1 >= out_size) {
        die("cpm: generated C path too long");
    }
    strncpy(out, base, out_size - 1);
    out[out_size - 1] = '\0';
    len = strlen(out);
    if(len >= 3 && strcmp(out + len - 3, ".nc") == 0) {
        out[len - 2] = 'c';
        out[len - 1] = '\0';
    }
    else if(len + 3 < out_size) {
        strcat(out, ".c");
    }
}

static int is_runtime_source(const char* src)
{
    return strcmp(base_name(src), "neo-c-str.nc") == 0;
}

static void generated_c_path(char* out, size_t out_size, const char* obj_path)
{
    size_t len;
    snprintf(out, out_size, "%s", obj_path);
    len = strlen(out);
    if(len >= 2 && strcmp(out + len - 2, ".o") == 0) {
        out[len - 1] = 'c';
    }
    else if(len + 3 < out_size) {
        strcat(out, ".c");
    }
    if(strlen(out) >= out_size - 1) {
        die("cpm: generated C path too long");
    }
}

static int should_wrap_with_common_header(const char* src)
{
    return file_exists("src/common.h")
        && strncmp(src, "src/", 4) == 0
        && has_suffix(src, ".nc");
}

static void prepare_source_path(char* out, size_t out_size, const char* src)
{
    if(!should_wrap_with_common_header(src)) {
        strncpy(out, src, out_size - 1);
        out[out_size - 1] = '\0';
        return;
    }

    snprintf(out, out_size, "target/debug/.cpm-src/%s", base_name(src));
    if(strlen(out) >= out_size - 1) {
        die("cpm: wrapper path too long");
    }
    ensure_parent_dir(out);
    {
        char text[PATH_MAX_LEN * 2];
        snprintf(text, sizeof(text),
            "#include \"../../../src/common.h\"\n"
            "#include \"../../../%s\"\n",
            src);
        if(strlen(text) >= sizeof(text) - 1) {
            die("cpm: wrapper source too long");
        }
        write_file(out, text);
    }
}

static void append_object_paths(char* cmd, size_t cmd_size, struct SourceList* sources,
    const struct Manifest* m)
{
    int i;
    for(i = 0; i < sources->count; i++) {
        char obj_path[PATH_MAX_LEN];
        char q_obj[PATH_MAX_LEN * 2];
        object_path(obj_path, sizeof(obj_path), sources->path[i], m);
        shell_quote(q_obj, sizeof(q_obj), obj_path);
        if(strlen(cmd) + strlen(q_obj) + 2 >= cmd_size) {
            die("cpm: command too long");
        }
        strcat(cmd, " ");
        strcat(cmd, q_obj);
    }
}

static int source_list_has_runtime(struct SourceList* sources)
{
    int i;
    for(i = 0; i < sources->count; i++) {
        if(is_runtime_source(sources->path[i])) {
            return 1;
        }
    }
    return 0;
}

static int source_list_has_duplicate_generated_names(struct SourceList* sources, int include_lib_runtime)
{
    char names[MAX_SOURCES + 1][PATH_MAX_LEN];
    int count = 0;
    int i;

    if(include_lib_runtime) {
        strcpy(names[count++], "neo-c-str.c");
    }
    for(i = 0; i < sources->count; i++) {
        char generated_base[PATH_MAX_LEN];
        int j;
        generated_c_basename(generated_base, sizeof(generated_base), sources->path[i]);
        for(j = 0; j < count; j++) {
            if(strcmp(names[j], generated_base) == 0) {
                return 1;
            }
        }
        strcpy(names[count++], generated_base);
    }
    return 0;
}

static void restore_build_jobs(struct BuildJob* jobs, int count)
{
    int i;
    for(i = 0; i < count; i++) {
        restore_generated_file(jobs[i].generated_base, jobs[i].backup_path);
    }
}

static int move_build_jobs(struct BuildJob* jobs, int count)
{
    int i;
    for(i = 0; i < count; i++) {
        if(move_generated_file(jobs[i].generated_base, jobs[i].c_path, jobs[i].backup_path) != 0) {
            return 1;
        }
    }
    return 0;
}

static int link_normal_output(const char* extra_flags, int optimize,
    const struct Manifest* m, struct SourceList* sources, const char* q_neoc)
{
    char q_out[PATH_MAX_LEN * 2];
    char cmd[CMD_MAX_LEN];
    shell_quote(q_out, sizeof(q_out), m->out);
    ensure_parent_dir(m->out);
    snprintf(cmd, sizeof(cmd), "%s %s %s %s -o %s",
        q_neoc, m->neoc_flags, extra_flags == NULL ? "" : extra_flags,
        optimize ? DEFAULT_SIZE_FLAGS : "", q_out);
    append_object_paths(cmd, sizeof(cmd), sources, m);
    if(file_exists("target/debug/neo-c-str.o") && !source_list_has_runtime(sources)) {
        char q_lib[PATH_MAX_LEN * 2];
        shell_quote(q_lib, sizeof(q_lib), "target/debug/neo-c-str.o");
        if(strlen(cmd) + strlen(q_lib) + 2 >= sizeof(cmd)) {
            die("cpm: command too long");
        }
        strcat(cmd, " ");
        strcat(cmd, q_lib);
    }
    if(m->ldflags[0] != '\0') {
        if(strlen(cmd) + strlen(m->ldflags) + 2 >= sizeof(cmd)) {
            die("cpm: command too long");
        }
        strcat(cmd, " ");
        strcat(cmd, m->ldflags);
    }
    if(run_cmd(cmd) != 0) {
        return 1;
    }
    if(m->strip && optimize) {
        char strip_cmd[PATH_MAX_LEN * 3];
        snprintf(strip_cmd, sizeof(strip_cmd), "strip -s%s %s",
            m->strip_sections ? " --strip-section-headers" : "", q_out);
        if(run_cmd(strip_cmd) != 0) {
            return 1;
        }
    }
    printf("built %s\n", m->out);
    return 0;
}

static int link_bare_output(int optimize, const struct Manifest* m,
    struct SourceList* sources, const char* q_out)
{
    char q_linker[PATH_MAX_LEN * 2];
    char cmd[CMD_MAX_LEN];
    shell_quote(q_linker, sizeof(q_linker), m->linker);
    snprintf(cmd, sizeof(cmd), "%s %s", q_linker, m->linker_flags);
    if(m->linker_script[0] != '\0') {
        char q_script[PATH_MAX_LEN * 2];
        shell_quote(q_script, sizeof(q_script), m->linker_script);
        if(strlen(cmd) + strlen(q_script) + 5 >= sizeof(cmd)) {
            die("cpm: command too long");
        }
        strcat(cmd, " -T ");
        strcat(cmd, q_script);
    }
    if(strlen(cmd) + strlen(q_out) + 5 >= sizeof(cmd)) {
        die("cpm: command too long");
    }
    strcat(cmd, " -o ");
    strcat(cmd, q_out);
    append_object_paths(cmd, sizeof(cmd), sources, m);
    if(m->ldflags[0] != '\0') {
        if(strlen(cmd) + strlen(m->ldflags) + 2 >= sizeof(cmd)) {
            die("cpm: command too long");
        }
        strcat(cmd, " ");
        strcat(cmd, m->ldflags);
    }
    if(run_cmd(cmd) != 0) {
        return 1;
    }
    if(m->strip && optimize) {
        char strip_cmd[PATH_MAX_LEN * 3];
        snprintf(strip_cmd, sizeof(strip_cmd), "strip -s%s %s",
            m->strip_sections ? " --strip-section-headers" : "", q_out);
        if(run_cmd(strip_cmd) != 0) {
            return 1;
        }
    }
    printf("built %s\n", m->out);
    return 0;
}

static int cmd_build_parallel_with_flags(const char* extra_flags, int optimize,
    const char* link_extra_flags, struct Manifest* m, struct SourceList* sources,
    const char* q_neoc)
{
    int include_lib_runtime = file_exists("lib/neo-c-str.nc");
    int max_jobs = effective_jobs(m);
    int job_count = 0;
    int i;
    struct BuildJob* jobs;

    if(max_jobs <= 1) {
        return -1;
    }
    if(source_list_has_duplicate_generated_names(sources, include_lib_runtime)) {
        puts("cpm: duplicate generated C basenames found; using serial build");
        return -1;
    }

    jobs = calloc((size_t)(sources->count + 1), sizeof(struct BuildJob));
    if(jobs == NULL) {
        die_errno("calloc");
    }

    if(include_lib_runtime) {
        char q_runtime_src[PATH_MAX_LEN * 2];
        char q_runtime_obj[PATH_MAX_LEN * 2];
        shell_quote(q_runtime_src, sizeof(q_runtime_src), "lib/neo-c-str.nc");
        shell_quote(q_runtime_obj, sizeof(q_runtime_obj), "target/debug/neo-c-str.o");
        strcpy(jobs[job_count].generated_base, "neo-c-str.c");
        strcpy(jobs[job_count].c_path, "target/debug/neo-c-str.c");
        backup_generated_file(jobs[job_count].generated_base,
            jobs[job_count].backup_path, sizeof(jobs[job_count].backup_path));
        snprintf(jobs[job_count].cmd, sizeof(jobs[job_count].cmd),
            "%s %s %s %s -c -uniq %s -o %s",
            q_neoc, m->neoc_flags, extra_flags == NULL ? "" : extra_flags,
            optimize ? DEFAULT_SIZE_FLAGS : "", q_runtime_src, q_runtime_obj);
        job_count++;
    }

    for(i = 0; i < sources->count; i++) {
        char obj_path_buf[PATH_MAX_LEN];
        char build_src[PATH_MAX_LEN];
        char q_src[PATH_MAX_LEN * 2];
        char q_obj[PATH_MAX_LEN * 2];

        object_path(obj_path_buf, sizeof(obj_path_buf), sources->path[i], m);
        generated_c_basename(jobs[job_count].generated_base,
            sizeof(jobs[job_count].generated_base), sources->path[i]);
        generated_c_path(jobs[job_count].c_path, sizeof(jobs[job_count].c_path),
            obj_path_buf);
        ensure_parent_dir(obj_path_buf);
        ensure_parent_dir(jobs[job_count].c_path);
        prepare_source_path(build_src, sizeof(build_src), sources->path[i]);
        shell_quote(q_src, sizeof(q_src), build_src);
        shell_quote(q_obj, sizeof(q_obj), obj_path_buf);
        backup_generated_file(jobs[job_count].generated_base,
            jobs[job_count].backup_path, sizeof(jobs[job_count].backup_path));
        if(is_runtime_source(sources->path[i])) {
            snprintf(jobs[job_count].cmd, sizeof(jobs[job_count].cmd),
                "%s %s %s %s -c -uniq %s -o %s",
                q_neoc, m->neoc_flags, extra_flags == NULL ? "" : extra_flags,
                optimize ? DEFAULT_SIZE_FLAGS : "", q_src, q_obj);
        }
        else {
            snprintf(jobs[job_count].cmd, sizeof(jobs[job_count].cmd),
                "%s %s %s %s -c %s -o %s",
                q_neoc, m->neoc_flags, extra_flags == NULL ? "" : extra_flags,
                optimize ? DEFAULT_SIZE_FLAGS : "", q_src, q_obj);
        }
        job_count++;
    }

    if(run_parallel_jobs(jobs, job_count, max_jobs) != 0) {
        restore_build_jobs(jobs, job_count);
        free(jobs);
        return 1;
    }
    if(move_build_jobs(jobs, job_count) != 0) {
        free(jobs);
        return 1;
    }
    free(jobs);
    return link_normal_output(link_extra_flags, optimize, m, sources, q_neoc);
}

static int cmd_bare_build_parallel_with_flags(const char* extra_flags, int optimize,
    struct Manifest* m, struct SourceList* sources, const char* q_out)
{
    int max_jobs = effective_jobs(m);
    int i;
    struct BuildJob* transpile_jobs;
    struct BuildJob* compile_jobs;
    char q_neoc[PATH_MAX_LEN * 2];
    char q_cc[PATH_MAX_LEN * 2];

    if(max_jobs <= 1) {
        return -1;
    }
    if(source_list_has_duplicate_generated_names(sources, 0)) {
        puts("cpm: duplicate generated C basenames found; using serial bare build");
        return -1;
    }

    transpile_jobs = calloc((size_t)sources->count, sizeof(struct BuildJob));
    compile_jobs = calloc((size_t)sources->count, sizeof(struct BuildJob));
    if(transpile_jobs == NULL || compile_jobs == NULL) {
        die_errno("calloc");
    }
    shell_quote(q_neoc, sizeof(q_neoc), m->neoc);
    shell_quote(q_cc, sizeof(q_cc), m->cc);

    for(i = 0; i < sources->count; i++) {
        char obj_path_buf[PATH_MAX_LEN];
        char build_src[PATH_MAX_LEN];
        char q_src[PATH_MAX_LEN * 2];
        char q_c_path[PATH_MAX_LEN * 2];
        char q_obj[PATH_MAX_LEN * 2];

        object_path(obj_path_buf, sizeof(obj_path_buf), sources->path[i], m);
        generated_c_basename(transpile_jobs[i].generated_base,
            sizeof(transpile_jobs[i].generated_base), sources->path[i]);
        generated_c_path(transpile_jobs[i].c_path, sizeof(transpile_jobs[i].c_path),
            obj_path_buf);
        strcpy(compile_jobs[i].generated_base, transpile_jobs[i].generated_base);
        strcpy(compile_jobs[i].c_path, transpile_jobs[i].c_path);
        ensure_parent_dir(obj_path_buf);
        ensure_parent_dir(transpile_jobs[i].c_path);
        prepare_source_path(build_src, sizeof(build_src), sources->path[i]);
        shell_quote(q_src, sizeof(q_src), build_src);
        shell_quote(q_c_path, sizeof(q_c_path), transpile_jobs[i].c_path);
        shell_quote(q_obj, sizeof(q_obj), obj_path_buf);
        backup_generated_file(transpile_jobs[i].generated_base,
            transpile_jobs[i].backup_path, sizeof(transpile_jobs[i].backup_path));
        snprintf(transpile_jobs[i].cmd, sizeof(transpile_jobs[i].cmd),
            "%s %s %s -bare -S %s",
            q_neoc, m->neoc_flags, extra_flags == NULL ? "" : extra_flags, q_src);
        snprintf(compile_jobs[i].cmd, sizeof(compile_jobs[i].cmd),
            "%s %s -c %s -o %s",
            q_cc, m->cflags, q_c_path, q_obj);
    }

    if(run_parallel_jobs(transpile_jobs, sources->count, max_jobs) != 0) {
        restore_build_jobs(transpile_jobs, sources->count);
        free(transpile_jobs);
        free(compile_jobs);
        return 1;
    }
    if(move_build_jobs(transpile_jobs, sources->count) != 0) {
        free(transpile_jobs);
        free(compile_jobs);
        return 1;
    }
    if(run_parallel_jobs(compile_jobs, sources->count, max_jobs) != 0) {
        free(transpile_jobs);
        free(compile_jobs);
        return 1;
    }
    free(transpile_jobs);
    free(compile_jobs);
    return link_bare_output(optimize, m, sources, q_out);
}

static int cmd_bare_build_with_flags(const char* extra_flags, int optimize,
    struct Manifest* m, struct SourceList* sources, const char* q_out)
{
    char q_neoc[PATH_MAX_LEN * 2];
    char q_cc[PATH_MAX_LEN * 2];
    char q_linker[PATH_MAX_LEN * 2];
    char cmd[PATH_MAX_LEN * 16];
    int i;

    shell_quote(q_neoc, sizeof(q_neoc), m->neoc);
    shell_quote(q_cc, sizeof(q_cc), m->cc);
    shell_quote(q_linker, sizeof(q_linker), m->linker);

    for(i = 0; i < sources->count; i++) {
        char obj_path_buf[PATH_MAX_LEN];
        char generated_base[PATH_MAX_LEN];
        char c_path_buf[PATH_MAX_LEN];
        char build_src[PATH_MAX_LEN];
        char q_src[PATH_MAX_LEN * 2];
        char q_generated_base[PATH_MAX_LEN * 2];
        char q_c_path[PATH_MAX_LEN * 2];
        char q_obj[PATH_MAX_LEN * 2];

        object_path(obj_path_buf, sizeof(obj_path_buf), sources->path[i], m);
        generated_c_basename(generated_base, sizeof(generated_base), sources->path[i]);
        generated_c_path(c_path_buf, sizeof(c_path_buf), obj_path_buf);
        ensure_parent_dir(obj_path_buf);
        ensure_parent_dir(c_path_buf);
        prepare_source_path(build_src, sizeof(build_src), sources->path[i]);
        shell_quote(q_src, sizeof(q_src), build_src);
        shell_quote(q_generated_base, sizeof(q_generated_base), generated_base);
        shell_quote(q_c_path, sizeof(q_c_path), c_path_buf);
        shell_quote(q_obj, sizeof(q_obj), obj_path_buf);

        snprintf(cmd, sizeof(cmd), "%s %s %s -bare -S %s",
            q_neoc, m->neoc_flags, extra_flags == NULL ? "" : extra_flags, q_src);
        if(run_cmd(cmd) != 0) {
            return 1;
        }
        snprintf(cmd, sizeof(cmd), "mv %s %s", q_generated_base, q_c_path);
        if(run_cmd(cmd) != 0) {
            return 1;
        }
        snprintf(cmd, sizeof(cmd), "%s %s -c %s -o %s",
            q_cc, m->cflags, q_c_path, q_obj);
        if(run_cmd(cmd) != 0) {
            return 1;
        }
    }

    snprintf(cmd, sizeof(cmd), "%s %s", q_linker, m->linker_flags);
    if(m->linker_script[0] != '\0') {
        char q_script[PATH_MAX_LEN * 2];
        shell_quote(q_script, sizeof(q_script), m->linker_script);
        if(strlen(cmd) + strlen(q_script) + 5 >= sizeof(cmd)) {
            die("cpm: command too long");
        }
        strcat(cmd, " -T ");
        strcat(cmd, q_script);
    }
    if(strlen(cmd) + strlen(q_out) + 5 >= sizeof(cmd)) {
        die("cpm: command too long");
    }
    strcat(cmd, " -o ");
    strcat(cmd, q_out);
    append_object_paths(cmd, sizeof(cmd), sources, m);
    if(m->ldflags[0] != '\0') {
        if(strlen(cmd) + strlen(m->ldflags) + 2 >= sizeof(cmd)) {
            die("cpm: command too long");
        }
        strcat(cmd, " ");
        strcat(cmd, m->ldflags);
    }
    if(run_cmd(cmd) != 0) {
        return 1;
    }
    if(m->strip && optimize) {
        char strip_cmd[PATH_MAX_LEN * 3];
        snprintf(strip_cmd, sizeof(strip_cmd), "strip -s%s %s",
            m->strip_sections ? " --strip-section-headers" : "", q_out);
        if(run_cmd(strip_cmd) != 0) {
            return 1;
        }
    }
    printf("built %s\n", m->out);
    return 0;
}

static void write_manifest(const char* name)
{
    char text[2048];
    snprintf(text, sizeof(text),
        "[package]\n"
        "name = \"%s\"\n"
        "version = \"1.0.0\"\n"
        "\n"
        "[build]\n"
        "src = \"src/main.nc\"\n"
        "out = \"target/debug/%s\"\n"
        "neoc = \"neo-c\"\n"
        "neoc_flags = \"-I. -Ilib\"\n"
        "ldflags = \"\"\n"
        "jobs = 0\n"
        "lowmem = false\n"
        "strip = false\n",
        name, name);
    write_file("Neo.toml", text);
}

static void write_main_source(void)
{
    write_file("src/main.nc",
        "#include <neo-c.h>\n"
        "\n"
        "int main(int argc, char** argv)\n"
        "{\n"
        "    puts(\"Hello from neo-c cpm\");\n"
        "    return 0;\n"
        "}\n");
}

static void write_common_header(void)
{
    write_file("src/common.h",
        "#ifndef CPM_COMMON_H\n"
        "#define CPM_COMMON_H\n"
        "\n"
        "#include <neo-c.h>\n"
        "\n"
        "/* Put declarations shared by src/*.nc files here. */\n"
        "\n"
        "#endif\n");
}

static int cmd_init(const char* name)
{
    if(file_exists("Neo.toml")) {
        die("cpm: Neo.toml already exists");
    }
    mkdir_p("src");
    copy_standard_library();
    write_manifest(name);
    if(!file_exists("src/main.nc")) {
        write_main_source();
    }
    if(!file_exists("src/common.h")) {
        write_common_header();
    }
    if(!file_exists(".gitignore")) {
        write_file(".gitignore", "target/\n");
    }
    printf("created package `%s`\n", name);
    return 0;
}

static int cmd_new(const char* name)
{
    if(file_exists(name)) {
        die("cpm: destination already exists");
    }
    mkdir_p(name);
    if(chdir(name) != 0) {
        die_errno(name);
    }
    return cmd_init(base_name(name));
}

static int ensure_parent_dir(const char* path)
{
    char dir[PATH_MAX_LEN];
    char* slash = strrchr(path, '/');
    size_t n;
    if(slash == NULL) {
        return 0;
    }
    n = (size_t)(slash - path);
    if(n >= sizeof(dir)) {
        die("cpm: output path too long");
    }
    memcpy(dir, path, n);
    dir[n] = '\0';
    mkdir_p(dir);
    return 0;
}

static int cmd_build_with_flags(const char* extra_flags, int optimize)
{
    struct Manifest m;
    struct SourceList sources;
    char q_neoc[PATH_MAX_LEN * 2];
    char q_out[PATH_MAX_LEN * 2];
    char cmd[PATH_MAX_LEN * 16];
    char neoc_extra_flags[VALUE_MAX_LEN * 2];
    int i;

    read_manifest(&m);
    build_extra_flags(neoc_extra_flags, sizeof(neoc_extra_flags), extra_flags, &m);
    mkdir_p("target/debug");
    collect_manifest_sources(&sources, &m);
    shell_quote(q_neoc, sizeof(q_neoc), m.neoc);

    if(m.bare) {
        int parallel_rc;
        shell_quote(q_out, sizeof(q_out), m.out);
        ensure_parent_dir(m.out);
        parallel_rc = cmd_bare_build_parallel_with_flags(neoc_extra_flags, optimize, &m, &sources, q_out);
        if(parallel_rc >= 0) {
            return parallel_rc;
        }
        return cmd_bare_build_with_flags(neoc_extra_flags, optimize, &m, &sources, q_out);
    }

    {
        int parallel_rc = cmd_build_parallel_with_flags(neoc_extra_flags, optimize,
            extra_flags, &m, &sources, q_neoc);
        if(parallel_rc >= 0) {
            return parallel_rc;
        }
    }

    if(file_exists("lib/neo-c-str.nc")) {
        char runtime_c_path[PATH_MAX_LEN];
        char runtime_backup_path[PATH_MAX_LEN];
        char q_runtime_src[PATH_MAX_LEN * 2];
        char q_runtime_obj[PATH_MAX_LEN * 2];
        shell_quote(q_runtime_src, sizeof(q_runtime_src), "lib/neo-c-str.nc");
        shell_quote(q_runtime_obj, sizeof(q_runtime_obj), "target/debug/neo-c-str.o");
        strcpy(runtime_c_path, "target/debug/neo-c-str.c");
        backup_generated_file("neo-c-str.c", runtime_backup_path, sizeof(runtime_backup_path));
        snprintf(cmd, sizeof(cmd), "%s %s %s %s -c -uniq %s -o %s",
            q_neoc, m.neoc_flags, neoc_extra_flags,
            optimize ? DEFAULT_SIZE_FLAGS : "", q_runtime_src, q_runtime_obj);
        if(run_cmd(cmd) != 0) {
            restore_generated_file("neo-c-str.c", runtime_backup_path);
            return 1;
        }
        if(move_generated_file("neo-c-str.c", runtime_c_path, runtime_backup_path) != 0) {
            return 1;
        }
    }

    for(i = 0; i < sources.count; i++) {
        char obj_path_buf[PATH_MAX_LEN];
        char generated_base[PATH_MAX_LEN];
        char c_path_buf[PATH_MAX_LEN];
        char backup_path[PATH_MAX_LEN];
        char build_src[PATH_MAX_LEN];
        char q_src[PATH_MAX_LEN * 2];
        char q_obj[PATH_MAX_LEN * 2];

        object_path(obj_path_buf, sizeof(obj_path_buf), sources.path[i], &m);
        generated_c_basename(generated_base, sizeof(generated_base), sources.path[i]);
        generated_c_path(c_path_buf, sizeof(c_path_buf), obj_path_buf);
        ensure_parent_dir(obj_path_buf);
        ensure_parent_dir(c_path_buf);
        prepare_source_path(build_src, sizeof(build_src), sources.path[i]);
        shell_quote(q_src, sizeof(q_src), build_src);
        shell_quote(q_obj, sizeof(q_obj), obj_path_buf);
        backup_generated_file(generated_base, backup_path, sizeof(backup_path));
        if(is_runtime_source(sources.path[i])) {
            snprintf(cmd, sizeof(cmd), "%s %s %s %s -c -uniq %s -o %s",
                q_neoc, m.neoc_flags, neoc_extra_flags,
                optimize ? DEFAULT_SIZE_FLAGS : "", q_src, q_obj);
        }
        else {
            snprintf(cmd, sizeof(cmd), "%s %s %s %s -c %s -o %s",
                q_neoc, m.neoc_flags, neoc_extra_flags,
                optimize ? DEFAULT_SIZE_FLAGS : "", q_src, q_obj);
        }
        if(run_cmd(cmd) != 0) {
            restore_generated_file(generated_base, backup_path);
            return 1;
        }
        if(move_generated_file(generated_base, c_path_buf, backup_path) != 0) {
            return 1;
        }
    }

    shell_quote(q_out, sizeof(q_out), m.out);
    ensure_parent_dir(m.out);
    snprintf(cmd, sizeof(cmd), "%s %s %s %s -o %s",
        q_neoc, m.neoc_flags, extra_flags == NULL ? "" : extra_flags,
        optimize ? DEFAULT_SIZE_FLAGS : "", q_out);
    append_object_paths(cmd, sizeof(cmd), &sources, &m);
    if(file_exists("target/debug/neo-c-str.o") && !source_list_has_runtime(&sources)) {
        char q_lib[PATH_MAX_LEN * 2];
        shell_quote(q_lib, sizeof(q_lib), "target/debug/neo-c-str.o");
        if(strlen(cmd) + strlen(q_lib) + 2 >= sizeof(cmd)) {
            die("cpm: command too long");
        }
        strcat(cmd, " ");
        strcat(cmd, q_lib);
    }
    if(m.ldflags[0] != '\0') {
        if(strlen(cmd) + strlen(m.ldflags) + 2 >= sizeof(cmd)) {
            die("cpm: command too long");
        }
        strcat(cmd, " ");
        strcat(cmd, m.ldflags);
    }
    if(run_cmd(cmd) != 0) {
        return 1;
    }
    if(m.strip && optimize) {
        char strip_cmd[PATH_MAX_LEN * 3];
        snprintf(strip_cmd, sizeof(strip_cmd), "strip -s%s %s",
            m.strip_sections ? " --strip-section-headers" : "", q_out);
        if(run_cmd(strip_cmd) != 0) {
            return 1;
        }
    }
    printf("built %s\n", m.out);
    return 0;
}

static int cmd_build(void)
{
    return cmd_build_with_flags(NULL, 1);
}

static int run_manifest_output(int argc, char** argv, const char* prefix)
{
    struct Manifest m;
    char q_out[PATH_MAX_LEN * 2];
    char cmd[PATH_MAX_LEN * 16];
    int i;

    read_manifest(&m);
    shell_quote(q_out, sizeof(q_out), m.out);
    if(prefix != NULL && prefix[0] != '\0') {
        snprintf(cmd, sizeof(cmd), "%s %s", prefix, q_out);
    }
    else {
        snprintf(cmd, sizeof(cmd), "%s", q_out);
    }
    for(i = 0; i < argc; i++) {
        char q_arg[PATH_MAX_LEN * 2];
        shell_quote(q_arg, sizeof(q_arg), argv[i]);
        if(strlen(cmd) + strlen(q_arg) + 2 >= sizeof(cmd)) {
            die("cpm: command too long");
        }
        strcat(cmd, " ");
        strcat(cmd, q_arg);
    }
    return run_cmd(cmd);
}

static int cmd_run(int argc, char** argv)
{
    if(cmd_build() != 0) {
        return 1;
    }
    return run_manifest_output(argc, argv, NULL);
}

static int cmd_val(int argc, char** argv)
{
    const char* valgrind = getenv("CPM_VALGRIND");
    char q_valgrind[PATH_MAX_LEN * 2];
    char prefix[PATH_MAX_LEN * 4];

    if(cmd_build_with_flags(VAL_FLAGS, 0) != 0) {
        return 1;
    }
    if(valgrind == NULL || valgrind[0] == '\0') {
        valgrind = "valgrind";
    }
    shell_quote(q_valgrind, sizeof(q_valgrind), valgrind);
    snprintf(prefix, sizeof(prefix),
        "%s --leak-check=full --show-leak-kinds=all "
        "--errors-for-leak-kinds=definite,possible --error-exitcode=99",
        q_valgrind);
    return run_manifest_output(argc, argv, prefix);
}

static int cmd_leak(int argc, char** argv)
{
    int rc;
    if(cmd_build_with_flags(LEAK_FLAGS, 0) != 0) {
        fputs("cpm: compiler leak sanitizer build failed\n", stderr);
        fputs("cpm: falling back to valgrind\n", stderr);
        return cmd_val(argc, argv);
    }
    rc = run_manifest_output(argc, argv, LEAK_RUN_PREFIX);
    if(rc != 0) {
        fputs("cpm: compiler leak sanitizer run failed\n", stderr);
        fputs("cpm: falling back to valgrind\n", stderr);
        return cmd_val(argc, argv);
    }
    return 0;
}

static int cmd_clean(void)
{
    return run_cmd("rm -rf target");
}

static int cmd_install(void)
{
    struct Manifest m;
    const char* destdir;
    char bindir[PATH_MAX_LEN];
    char q_out[PATH_MAX_LEN * 2];
    char q_bindir[PATH_MAX_LEN * 2];
    char cmd[PATH_MAX_LEN * 4];

    read_manifest(&m);
    if(!file_exists(m.out)) {
        if(cmd_build() != 0) {
            return 1;
        }
    }
    destdir = getenv("DESTDIR");
    if(destdir == NULL || destdir[0] == '\0') {
        destdir = "/usr/local";
    }
    snprintf(bindir, sizeof(bindir), "%s/bin", destdir);
    if(strlen(bindir) >= sizeof(bindir) - 1) {
        die("cpm: install path too long");
    }
    mkdir_p(bindir);
    shell_quote(q_out, sizeof(q_out), m.out);
    shell_quote(q_bindir, sizeof(q_bindir), bindir);
    snprintf(cmd, sizeof(cmd), "install -m 755 %s %s/%s", q_out, q_bindir, m.name);
    return run_cmd(cmd);
}

static void usage(void)
{
    fputs("usage: cpm <new|init|build|run|test|val|leak|clean|install> [args]\n", stderr);
}

int main(int argc, char** argv)
{
    if(argc < 2) {
        usage();
        return 2;
    }
    if(strcmp(argv[1], "new") == 0) {
        if(argc != 3) {
            usage();
            return 2;
        }
        return cmd_new(argv[2]);
    }
    if(strcmp(argv[1], "init") == 0) {
        return cmd_init(argc >= 3 ? argv[2] : base_name("."));
    }
    if(strcmp(argv[1], "build") == 0) {
        return cmd_build();
    }
    if(strcmp(argv[1], "run") == 0) {
        return cmd_run(argc - 2, argv + 2);
    }
    if(strcmp(argv[1], "test") == 0) {
        return cmd_run(0, NULL);
    }
    if(strcmp(argv[1], "val") == 0) {
        return cmd_val(argc - 2, argv + 2);
    }
    if(strcmp(argv[1], "leak") == 0) {
        return cmd_leak(argc - 2, argv + 2);
    }
    if(strcmp(argv[1], "clean") == 0) {
        return cmd_clean();
    }
    if(strcmp(argv[1], "install") == 0) {
        return cmd_install();
    }
    usage();
    return 2;
}
