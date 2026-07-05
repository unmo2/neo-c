#ifdef CCPP_BARE
typedef unsigned long size_t;
typedef long time_t;
typedef struct __neo_FILE FILE;
extern int* __errno_location(void);
#define errno (*__errno_location())
extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;
struct utsname {
    char sysname[65];
    char nodename[65];
    char release[65];
    char version[65];
    char machine[65];
    char domainname[65];
};
struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    long tm_gmtoff;
    const char* tm_zone;
};
#ifndef NULL
#define NULL ((void*)0)
#endif
#ifndef EOF
#define EOF (-1)
#endif
#ifndef INT_MAX
#define INT_MAX 2147483647
#endif
#ifndef UINT_MAX
#define UINT_MAX 4294967295U
#endif
#ifndef LONG_MAX
#define LONG_MAX 9223372036854775807L
#endif
#ifndef ULONG_MAX
#define ULONG_MAX 18446744073709551615UL
#endif
#ifndef LLONG_MAX
#define LLONG_MAX 9223372036854775807LL
#endif
#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif
#ifndef ERANGE
#define ERANGE 34
#endif
#define true 1
#define false 0
typedef int bool;
void exit(int status);
void perror(const char* s);
void* malloc(size_t size);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);
void free(void* ptr);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, size_t n);
char* strcat(char* dst, const char* src);
size_t strlen(const char* s);
void* memcpy(void* dst, const void* src, size_t n);
void* memmove(void* dst, const void* src, size_t n);
void* memset(void* dst, int c, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
void* memchr(const void* s, int c, size_t n);
char* strchr(const char* s, int c);
char* strrchr(const char* s, int c);
char* strstr(const char* haystack, const char* needle);
long strtol(const char* nptr, char** endptr, int base);
unsigned long strtoul(const char* nptr, char** endptr, int base);
unsigned long long strtoull(const char* nptr, char** endptr, int base);
int snprintf(char* out, size_t out_size, const char* fmt, ...);
int fprintf(FILE* f, const char* fmt, ...);
int fputs(const char* s, FILE* f);
int fputc(int c, FILE* f);
int fgetc(FILE* f);
int ungetc(int c, FILE* f);
int fclose(FILE* f);
FILE* fopen(const char* path, const char* mode);
char* fgets(char* s, int size, FILE* f);
int fflush(FILE* f);
char* getenv(const char* name);
time_t time(time_t* t);
struct tm* localtime(const time_t* t);
int uname(struct utsname* buf);
static int isspace(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f'; }
static int isalpha(int c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
static int isdigit(int c) { return c >= '0' && c <= '9'; }
static int isalnum(int c) { return isalpha(c) || isdigit(c); }
static int isxdigit(int c) { return isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }
static int toupper(int c) { return (c >= 'a' && c <= 'z') ? c - ('a' - 'A') : c; }
static int tolower(int c) { return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c; }
#else
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <time.h>
#endif

#define PP_RESCAN_MAX 1024

typedef struct {
    char *name;
    char *value; // replacement text for both object/function macros
    bool is_func;
    char **params; // parameter names if is_func
    size_t nparams;
    char *var_param; // name for GNU-style named variadic macros (e.g., args...)
    bool is_variadic;
    bool self_ref;
} Macro;

typedef struct MacroHashNode {
    size_t macro_index;
    struct MacroHashNode *next;
} MacroHashNode;

typedef struct {
    Macro *items;
    size_t len;
    size_t cap;
    MacroHashNode **buckets;
    size_t nbuckets;
} MacroTable;

typedef struct {
    char *name;
    char *sig;
} MacroSnapItem;

typedef struct MacroSnapshot {
    MacroSnapItem *items;
    size_t len;
    size_t cap;
} MacroSnapshot;

static bool macro_is_vararg_token(const Macro *m, const char *tok) {
    if (!m || !m->is_variadic || !tok) return false;
    if (strcmp(tok, "__VA_ARGS__") == 0) return true;
    return m->var_param && strcmp(tok, m->var_param) == 0;
}

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

static void *xrealloc(void *p, size_t n) {
    void *q = realloc(p, n);
    if (!q && n != 0) die("realloc");
    return q;
}

static char *xstrdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char *)malloc(n);
    if (!p) die("malloc");
    memcpy(p, s, n);
    return p;
}

static bool file_exists(const char *path) {
    if (!path || !*path) return false;
    FILE *f = fopen(path, "r");
    if (f) { fclose(f); return true; }
    return false;
}

static void mtable_init(MacroTable *t) {
    t->items = NULL;
    t->len = 0;
    t->cap = 0;
    t->buckets = NULL;
    t->nbuckets = 0;
}

static size_t macro_hash_name(const char *name) {
    size_t hash = 1469598103934665603ull;
    const unsigned char *p = (const unsigned char *)name;
    while (*p) {
        hash ^= (size_t)*p++;
        hash *= 1099511628211ull;
    }
    return hash;
}

static size_t macro_hash_name_n(const char *name, size_t len) {
    size_t hash = 1469598103934665603ull;
    const unsigned char *p = (const unsigned char *)name;
    for (size_t i = 0; i < len; ++i) {
        hash ^= (size_t)p[i];
        hash *= 1099511628211ull;
    }
    return hash;
}

static size_t mtable_bucket_index(const MacroTable *t, const char *name) {
    return t->nbuckets ? (macro_hash_name(name) % t->nbuckets) : 0;
}

static size_t mtable_bucket_index_n(const MacroTable *t, const char *name, size_t len) {
    return t->nbuckets ? (macro_hash_name_n(name, len) % t->nbuckets) : 0;
}

static void mtable_rehash(MacroTable *t, size_t nbuckets) {
    MacroHashNode **new_buckets = (MacroHashNode **)calloc(nbuckets, sizeof(MacroHashNode *));
    if (!new_buckets) die("calloc");

    if (t->buckets) {
        for (size_t i = 0; i < t->nbuckets; ++i) {
            MacroHashNode *node = t->buckets[i];
            while (node) {
                MacroHashNode *next = node->next;
                size_t bucket = macro_hash_name(t->items[node->macro_index].name) % nbuckets;
                node->next = new_buckets[bucket];
                new_buckets[bucket] = node;
                node = next;
            }
        }
        free(t->buckets);
    }

    t->buckets = new_buckets;
    t->nbuckets = nbuckets;
}

static void mtable_maybe_grow_buckets(MacroTable *t, size_t next_len) {
    size_t target = t->nbuckets;
    if (target == 0) {
        target = 64;
    }
    while (next_len * 4 >= target * 3) {
        target *= 2;
    }
    if (target != t->nbuckets) {
        mtable_rehash(t, target);
    }
}

static MacroHashNode **mtable_find_slot(MacroTable *t, const char *name) {
    if (!t->buckets || t->nbuckets == 0) {
        return NULL;
    }

    MacroHashNode **slot = &t->buckets[mtable_bucket_index(t, name)];
    while (*slot) {
        if (strcmp(t->items[(*slot)->macro_index].name, name) == 0) {
            return slot;
        }
        slot = &(*slot)->next;
    }
    return NULL;
}

static void mtable_index_insert(MacroTable *t, size_t macro_index) {
    mtable_maybe_grow_buckets(t, t->len);

    size_t bucket = mtable_bucket_index(t, t->items[macro_index].name);
    MacroHashNode *node = (MacroHashNode *)malloc(sizeof(MacroHashNode));
    if (!node) die("malloc");
    node->macro_index = macro_index;
    node->next = t->buckets[bucket];
    t->buckets[bucket] = node;
}

static void mtable_index_remove(MacroTable *t, const char *name) {
    MacroHashNode **slot = mtable_find_slot(t, name);
    if (!slot) {
        return;
    }

    MacroHashNode *node = *slot;
    *slot = node->next;
    free(node);
}

static void mtable_index_update(MacroTable *t, const char *name, size_t macro_index) {
    MacroHashNode **slot = mtable_find_slot(t, name);
    if (slot) {
        (*slot)->macro_index = macro_index;
    }
}

static size_t mtable_find_index(const MacroTable *t, const char *name) {
    if (!t->buckets || t->nbuckets == 0) {
        return (size_t)-1;
    }

    MacroHashNode *node = t->buckets[mtable_bucket_index(t, name)];
    while (node) {
        if (strcmp(t->items[node->macro_index].name, name) == 0) {
            return node->macro_index;
        }
        node = node->next;
    }
    return (size_t)-1;
}

static size_t mtable_find_index_n(const MacroTable *t, const char *name, size_t len) {
    if (!t->buckets || t->nbuckets == 0) {
        return (size_t)-1;
    }

    MacroHashNode *node = t->buckets[mtable_bucket_index_n(t, name, len)];
    while (node) {
        const char *entry_name = t->items[node->macro_index].name;
        if (strncmp(entry_name, name, len) == 0 && entry_name[len] == '\0') {
            return node->macro_index;
        }
        node = node->next;
    }
    return (size_t)-1;
}

static Macro *mtable_append(MacroTable *t, const char *name) {
    if (t->len == t->cap) {
        size_t ncap = t->cap ? t->cap * 2 : 8;
        Macro *nitems = (Macro *)xrealloc(t->items, ncap * sizeof(Macro));
        t->items = nitems;
        t->cap = ncap;
    }

    size_t index = t->len++;
    Macro *m = &t->items[index];
    m->name = xstrdup(name);
    m->value = NULL;
    m->is_func = false;
    m->params = NULL;
    m->nparams = 0;
    m->var_param = NULL;
    m->is_variadic = false;
    m->self_ref = false;

    mtable_index_insert(t, index);
    return m;
}

static void mtable_free(MacroTable *t) {
    if (!t) return;
    for (size_t i = 0; i < t->len; ++i) {
        free(t->items[i].name);
        free(t->items[i].value);
        if (t->items[i].params) {
            for (size_t j=0;j<t->items[i].nparams;++j) free(t->items[i].params[j]);
            free(t->items[i].params);
        }
        free(t->items[i].var_param);
    }
    free(t->items);
    if (t->buckets) {
        for (size_t i = 0; i < t->nbuckets; ++i) {
            MacroHashNode *node = t->buckets[i];
            while (node) {
                MacroHashNode *next = node->next;
                free(node);
                node = next;
            }
        }
    }
    free(t->buckets);
}

static void mtable_unset(MacroTable *t, const char *name) {
    size_t index = mtable_find_index(t, name);
    if (index == (size_t)-1) {
        return;
    }

    mtable_index_remove(t, name);

    free(t->items[index].name);
    free(t->items[index].value);
    if (t->items[index].params) {
        for (size_t j = 0; j < t->items[index].nparams; ++j) free(t->items[index].params[j]);
        free(t->items[index].params);
    }
    free(t->items[index].var_param);

    size_t last = t->len - 1;
    if (index != last) {
        t->items[index] = t->items[last];
        mtable_index_update(t, t->items[index].name, index);
    }
    t->len--;
}

static Macro *mtable_find(MacroTable *t, const char *name) {
    size_t index = mtable_find_index(t, name);
    return index == (size_t)-1 ? NULL : &t->items[index];
}

static Macro *mtable_find_n(MacroTable *t, const char *name, size_t len) {
    size_t index = mtable_find_index_n(t, name, len);
    return index == (size_t)-1 ? NULL : &t->items[index];
}

static bool macro_value_contains_ident(const char *val, const char *name) {
    if (!val || !name || !*name) return false;
    enum { S_CODE, S_DQ, S_SQ, S_SL, S_BL } st = S_CODE;
    int bl_depth = 0;
    for (size_t i = 0; val[i]; ++i) {
        char c = val[i];
        if (st == S_CODE) {
            if (c == '"') { st = S_DQ; continue; }
            if (c == '\'') { st = S_SQ; continue; }
            if (c == '/' && val[i+1] == '/') { st = S_SL; i++; continue; }
            if (c == '/' && val[i+1] == '*') { st = S_BL; bl_depth = 1; i++; continue; }
            if (isalpha((unsigned char)c) || c == '_') {
                char id[256]; size_t j = 0; size_t k = i;
                while (val[k] && (isalnum((unsigned char)val[k]) || val[k] == '_')) {
                    if (j + 1 < sizeof id) id[j++] = val[k];
                    k++;
                }
                id[j] = '\0';
                if (strcmp(id, name) == 0) return true;
                i = k - 1;
            }
        } else if (st == S_DQ) {
            if (c == '"' && val[i-1] != '\\') st = S_CODE;
        } else if (st == S_SQ) {
            if (c == '\'' && val[i-1] != '\\') st = S_CODE;
        } else if (st == S_SL) {
            if (c == '\n' || c == '\r') st = S_CODE;
        } else if (st == S_BL) {
            if (c == '/' && val[i+1] == '*') { bl_depth++; i++; continue; }
            if (c == '*' && val[i+1] == '/') {
                bl_depth--; i++;
                if (bl_depth <= 0) { bl_depth = 0; st = S_CODE; }
                continue;
            }
        }
    }
    return false;
}

static void mtable_set_obj(MacroTable *t, const char *name, const char *value) {
    // If exists, replace
    Macro *m = mtable_find(t, name);
    if (m) {
        free(m->value);
        if (m->params) { for (size_t j=0;j<m->nparams;++j) free(m->params[j]); free(m->params); m->params=NULL; m->nparams=0; }
        if (m->var_param) { free(m->var_param); m->var_param = NULL; }
        m->is_func = false;
        m->value = xstrdup(value ? value : "");
        m->self_ref = macro_value_contains_ident(m->value, name);
        return;
    }
    m = mtable_append(t, name);
    m->value = xstrdup(value ? value : "");
    m->self_ref = macro_value_contains_ident(m->value, name);
}

static const char *mtable_get(const MacroTable *t, const char *name) {
    size_t index = mtable_find_index(t, name);
    if (index == (size_t)-1 || t->items[index].is_func) {
        return NULL;
    }
    return t->items[index].value;
}

typedef struct {
    bool initialized;
    bool ok;
    struct utsname uts;
} HostInfoCache;

static HostInfoCache g_host_info = {0};

static void init_host_info(void) {
    if (g_host_info.initialized) return;
    g_host_info.initialized = true;
    g_host_info.ok = uname(&g_host_info.uts) == 0;
}

static int uname_has(const char *token) {
    if (!token || !*token) return 0;
    init_host_info();
    if (!g_host_info.ok) return 0;
    return strstr(g_host_info.uts.sysname, token) != NULL ||
           strstr(g_host_info.uts.nodename, token) != NULL ||
           strstr(g_host_info.uts.release, token) != NULL ||
           strstr(g_host_info.uts.version, token) != NULL ||
           strstr(g_host_info.uts.machine, token) != NULL;
}

static void set_host_macros(MacroTable *t) {
    int is64 = (sizeof(void*) == 8);
    union { unsigned int u; unsigned char b[4]; } endu;
    endu.u = 0x01020304u;
    int is_little = (endu.b[0] == 0x04);

    // Common byte/bit and size hints
    mtable_set_obj(t, "__CHAR_BIT__", "8");
    mtable_set_obj(t, "__ORDER_LITTLE_ENDIAN__", "1234");
    mtable_set_obj(t, "__ORDER_BIG_ENDIAN__", "4321");
    mtable_set_obj(t, "__ORDER_PDP_ENDIAN__", "3412");
    mtable_set_obj(t, "__BYTE_ORDER__", is_little ? "1234" : "4321");
    if (is_little) mtable_set_obj(t, "__LITTLE_ENDIAN__", "1"); else mtable_set_obj(t, "__BIG_ENDIAN__", "1");
    const char *psz = is64 ? "8" : "4";
    mtable_set_obj(t, "__SIZEOF_POINTER__", psz);
    mtable_set_obj(t, "__SIZEOF_SIZE_T__", psz);
    mtable_set_obj(t, "__SIZEOF_SHORT__", "2");
    mtable_set_obj(t, "__SIZEOF_INT__", "4");
    mtable_set_obj(t, "__SIZEOF_LONG_LONG__", "8");
    mtable_set_obj(t, "__SIZEOF_LONG__", is64 ? "8" : "4");
    const char *i8 = "signed char";
    const char *u8 = "unsigned char";
    const char *i16 = "short int";
    const char *u16 = "unsigned short int";
    const char *i32 = "int";
    const char *u32 = "unsigned int";
    const char *i64 = is64 ? "long int" : "long long int";
    const char *u64 = is64 ? "unsigned long int" : "unsigned long long int";
    const char *iptr = is64 ? "long int" : "int";
    const char *uptr = is64 ? "unsigned long int" : "unsigned int";
    if (!mtable_get(t, "__CHAR16_TYPE__")) mtable_set_obj(t, "__CHAR16_TYPE__", u16);
    if (!mtable_get(t, "__CHAR32_TYPE__")) mtable_set_obj(t, "__CHAR32_TYPE__", u32);
    if (!mtable_get(t, "__INT8_TYPE__")) mtable_set_obj(t, "__INT8_TYPE__", i8);
    if (!mtable_get(t, "__UINT8_TYPE__")) mtable_set_obj(t, "__UINT8_TYPE__", u8);
    if (!mtable_get(t, "__INT16_TYPE__")) mtable_set_obj(t, "__INT16_TYPE__", i16);
    if (!mtable_get(t, "__UINT16_TYPE__")) mtable_set_obj(t, "__UINT16_TYPE__", u16);
    if (!mtable_get(t, "__INT32_TYPE__")) mtable_set_obj(t, "__INT32_TYPE__", i32);
    if (!mtable_get(t, "__UINT32_TYPE__")) mtable_set_obj(t, "__UINT32_TYPE__", u32);
    if (!mtable_get(t, "__INT64_TYPE__")) mtable_set_obj(t, "__INT64_TYPE__", i64);
    if (!mtable_get(t, "__UINT64_TYPE__")) mtable_set_obj(t, "__UINT64_TYPE__", u64);
    if (!mtable_get(t, "__INTPTR_TYPE__")) mtable_set_obj(t, "__INTPTR_TYPE__", iptr);
    if (!mtable_get(t, "__UINTPTR_TYPE__")) mtable_set_obj(t, "__UINTPTR_TYPE__", uptr);
    if (!mtable_get(t, "__INTMAX_TYPE__")) mtable_set_obj(t, "__INTMAX_TYPE__", i64);
    if (!mtable_get(t, "__UINTMAX_TYPE__")) mtable_set_obj(t, "__UINTMAX_TYPE__", u64);
    if (!mtable_get(t, "__INT_LEAST8_TYPE__")) mtable_set_obj(t, "__INT_LEAST8_TYPE__", i8);
    if (!mtable_get(t, "__UINT_LEAST8_TYPE__")) mtable_set_obj(t, "__UINT_LEAST8_TYPE__", u8);
    if (!mtable_get(t, "__INT_LEAST16_TYPE__")) mtable_set_obj(t, "__INT_LEAST16_TYPE__", i16);
    if (!mtable_get(t, "__UINT_LEAST16_TYPE__")) mtable_set_obj(t, "__UINT_LEAST16_TYPE__", u16);
    if (!mtable_get(t, "__INT_LEAST32_TYPE__")) mtable_set_obj(t, "__INT_LEAST32_TYPE__", i32);
    if (!mtable_get(t, "__UINT_LEAST32_TYPE__")) mtable_set_obj(t, "__UINT_LEAST32_TYPE__", u32);
    if (!mtable_get(t, "__INT_LEAST64_TYPE__")) mtable_set_obj(t, "__INT_LEAST64_TYPE__", i64);
    if (!mtable_get(t, "__UINT_LEAST64_TYPE__")) mtable_set_obj(t, "__UINT_LEAST64_TYPE__", u64);
    if (!mtable_get(t, "__INT_FAST8_TYPE__")) mtable_set_obj(t, "__INT_FAST8_TYPE__", i32);
    if (!mtable_get(t, "__UINT_FAST8_TYPE__")) mtable_set_obj(t, "__UINT_FAST8_TYPE__", u32);
    if (!mtable_get(t, "__INT_FAST16_TYPE__")) mtable_set_obj(t, "__INT_FAST16_TYPE__", i32);
    if (!mtable_get(t, "__UINT_FAST16_TYPE__")) mtable_set_obj(t, "__UINT_FAST16_TYPE__", u32);
    if (!mtable_get(t, "__INT_FAST32_TYPE__")) mtable_set_obj(t, "__INT_FAST32_TYPE__", i32);
    if (!mtable_get(t, "__UINT_FAST32_TYPE__")) mtable_set_obj(t, "__UINT_FAST32_TYPE__", u32);
    if (!mtable_get(t, "__INT_FAST64_TYPE__")) mtable_set_obj(t, "__INT_FAST64_TYPE__", i64);
    if (!mtable_get(t, "__UINT_FAST64_TYPE__")) mtable_set_obj(t, "__UINT_FAST64_TYPE__", u64);

    if (!mtable_get(t, "__INT_MAX__")) {
        char buf[64];
        snprintf(buf, sizeof buf, "%d", INT_MAX);
        mtable_set_obj(t, "__INT_MAX__", buf);
    }
    if (!mtable_get(t, "__FLT_MIN__")) mtable_set_obj(t, "__FLT_MIN__", "1.17549435e-38F");
    if (!mtable_get(t, "__DBL_MIN__")) mtable_set_obj(t, "__DBL_MIN__", "2.2250738585072014e-308");
    if (!mtable_get(t, "__LDBL_MIN__")) mtable_set_obj(t, "__LDBL_MIN__", "3.36210314311209350626e-4932L");

    int is_android = uname_has("Android") || getenv("ANDROID_ROOT") != NULL;
    int is_linux = uname_has("Linux");
    int is_mac = uname_has("Darwin");
    int is_bsd = uname_has("FreeBSD") || uname_has("OpenBSD") || uname_has("NetBSD") || uname_has("DragonFly");
    init_host_info();
    int is_unix = is_linux || is_android || is_mac || is_bsd || g_host_info.ok;

    if (is_linux || is_android) {
    mtable_set_obj(t, "__linux__", "1");
    mtable_set_obj(t, "linux", "1");
    mtable_set_obj(t, "__gnu_linux__", "1");
    mtable_set_obj(t, "__LINUX__", "1");
    mtable_set_obj(t, "__unix__", "1");
    mtable_set_obj(t, "__unix", "1");
    mtable_set_obj(t, "__ELF__", "1");
    mtable_set_obj(t, "__WCHAR_TYPE__", "int");
    mtable_set_obj(t, "__gwchar_t", "wchar_t");
    mtable_set_obj(t, "__WCHAR_TYPE__", "int");
    mtable_set_obj(t, "__WINT_TYPE__", "unsigned int");
    mtable_set_obj(t, "__SIZE_TYPE__", "unsigned long");
    mtable_set_obj(t, "__SIZE_TYPE__", "unsigned long");
    mtable_set_obj(t, "__PTRDIFF_TYPE__", "long");
    mtable_set_obj(t, "__WCHAR_MAX__", "0x7fffffff");
    mtable_set_obj(t, "__WCHAR_MINU__", "-0x7fffffff-1");
    if (is64) mtable_set_obj(t, "__LP64__", "1");
    mtable_set_obj(t, "_GNU_SOURCE", "1");
    mtable_set_obj(t, "_DEFAULT_SOURCE", "1");
    if (is_android) {
        mtable_set_obj(t, "__ANDROID__", "1");
    }
    }
    else if (is_mac) {
    mtable_set_obj(t, "__APPLE__", "1");
    mtable_set_obj(t, "__MACH__", "1");
    mtable_set_obj(t, "__MAC__", "1");
    if (is64) mtable_set_obj(t, "__LP64__", "1");
    mtable_set_obj(t, "_DARWIN_C_SOURCE", "1");
    mtable_set_obj(t, "_POSIX_C_SOURCE", "200809L");
    mtable_set_obj(t, "__DARWIN_C_LEVEL", "200809L");
    mtable_set_obj(t, "__DARWIN_NBBY", "8");
    mtable_set_obj(t, "__DARWIN_NFDBITS", "32");
    mtable_set_obj(t, "__DARWIN_NULL", "0");
    mtable_set_obj(t, "_CACHED_RUNES", "256");
    mtable_set_obj(t, "__WCHAR_TYPE__", "int");
    mtable_set_obj(t, "__WINT_TYPE__", "unsigned int");
    mtable_set_obj(t, "__SIZE_TYPE__", "unsigned long");
    mtable_set_obj(t, "__SIZE_TYPE__", "unsigned long");
    mtable_set_obj(t, "__PTRDIFF_TYPE__", "long");
    mtable_set_obj(t, "__WCHAR_MAX__", "0x7fffffff");
    mtable_set_obj(t, "__WCHAR_MINU__", "-0x7fffffff-1");
    }
    else if (is_bsd) {
    mtable_set_obj(t, "__unix__", "1");
    mtable_set_obj(t, "__unix", "1");
    if (is64) mtable_set_obj(t, "__LP64__", "1");
    }
    else if (is_unix) {
        mtable_set_obj(t, "__unix__", "1");
        mtable_set_obj(t, "__unix", "1");
        if (is64) mtable_set_obj(t, "__LP64__", "1");
    }

    int x86_64 = uname_has("x86_64");
    int aarch64 = uname_has("aarch64") || uname_has("arm64");
    int i386 = uname_has("i386");
    int riscv = uname_has("riscv64");

    if(x86_64) {
        mtable_set_obj(t, "__x86_64__", "1");
        mtable_set_obj(t, "__x86_64__", "1");
        mtable_set_obj(t, "__amd64__", "1");
        mtable_set_obj(t, "__LP64__", "1");
        mtable_set_obj(t, "__WORDSIZE", "64");
    }
    else if(aarch64) {
        mtable_set_obj(t, "__aarch64__", "1");
        mtable_set_obj(t, "__arm64__", "1");
        mtable_set_obj(t, "__LP64__", "1");
        mtable_set_obj(t, "__WORDSIZE", "64");
    }
    else if(i386) {
        mtable_set_obj(t, "__i386__", "1");
        mtable_set_obj(t, "__WORDSIZE", "32");
    }
    else if(riscv) {
        mtable_set_obj(t, "__riscv", "1");
        mtable_set_obj(t, "__riscv_xlen", is64 ? "64" : "32");
        if (is64) {
            mtable_set_obj(t, "__LP64__", "1");
            mtable_set_obj(t, "__WORDSIZE", "64");
        }
        else {
            mtable_set_obj(t, "__WORDSIZE", "32");
        }
    }
}

static void set_datetime_macros(MacroTable *t) {
    char datebuf[32];
    char timebuf[16];
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    static const char *months[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    if (tm_now && tm_now->tm_mon >= 0 && tm_now->tm_mon < 12) {
        snprintf(datebuf, sizeof(datebuf), "\"%s %2d %04d\"",
            months[tm_now->tm_mon], tm_now->tm_mday, tm_now->tm_year + 1900);
        snprintf(timebuf, sizeof(timebuf), "\"%02d:%02d:%02d\"",
            tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
    }
    else {
        snprintf(datebuf, sizeof(datebuf), "\"??? ?? ????\"");
        snprintf(timebuf, sizeof(timebuf), "\"??:??:??\"");
    }

    mtable_set_obj(t, "__DATE__", datebuf);
    mtable_set_obj(t, "__TIME__", timebuf);
}

struct PPOpts {
    const char **incdirs;
    size_t nincdirs;
    const char **sysincdirs;
    size_t nsysincdirs;
    const char **once_paths;
    size_t nonce_paths;
    const char **cmd_defs; size_t ncmd_defs;
    const char **cmd_undefs; size_t ncmd_undefs;
    const char **cmd_includes; size_t ncmd_includes;
    // Track currently-open include chain to avoid cycles (A->B->A...)
    const char **open_stack;
    size_t nopen_stack;
    // Whether to keep comments in the output (like gcc -C). Currently, comments
    // are preserved by default; this flag exists to accept the -C option
    // without error and for potential future use.
    bool keep_comments;
};

typedef struct PPOpts PPOpts;
static bool process_define(MacroTable *t, char *line);
static bool parse_ident(const char *p, size_t *adv, char *buf, size_t bufsz);
static void mtable_set_func(MacroTable *t, const char *name, char **params, size_t nparams,
                            bool variadic, char *var_param, const char *value);

static void apply_predefined_macros(MacroTable *t, const PPOpts *opts) {
    // Baseline C macros
    mtable_set_obj(t, "__STDC__", "1");
    mtable_set_obj(t, "__STDC_VERSION__", "201112L");
    mtable_set_obj(t, "__STDC_NO_VLA__", "1");
    mtable_set_obj(t, "__STDC_HOSTED__", "1");
    set_datetime_macros(t);
//    if (!mtable_get(t, "true")) mtable_set_obj(t, "true", "1");
//    if (!mtable_get(t, "false")) mtable_set_obj(t, "false", "0");
    if (!mtable_get(t, "__bool_true_false_are_defined")) mtable_set_obj(t, "__bool_true_false_are_defined", "1");
    if (!mtable_get(t, "__ATOMIC_RELAXED")) mtable_set_obj(t, "__ATOMIC_RELAXED", "0");
    if (!mtable_get(t, "__ATOMIC_CONSUME")) mtable_set_obj(t, "__ATOMIC_CONSUME", "1");
    if (!mtable_get(t, "__ATOMIC_ACQUIRE")) mtable_set_obj(t, "__ATOMIC_ACQUIRE", "2");
    if (!mtable_get(t, "__ATOMIC_RELEASE")) mtable_set_obj(t, "__ATOMIC_RELEASE", "3");
    if (!mtable_get(t, "__ATOMIC_ACQ_REL")) mtable_set_obj(t, "__ATOMIC_ACQ_REL", "4");
    if (!mtable_get(t, "__ATOMIC_SEQ_CST")) mtable_set_obj(t, "__ATOMIC_SEQ_CST", "5");
    // Provide common compiler-identification macros so system headers don't complain
    mtable_set_obj(t, "__GNUC__", "3");
    mtable_set_obj(t, "__GNUC_MINOR__", "2");
    mtable_set_obj(t, "__clang__", "1");
    mtable_set_obj(t, "__NEO_C__", "1");
    mtable_set_obj(t, "__BEGIN_DECLS", "");
    mtable_set_obj(t, "__END_DECLS", "");
    // Common feature-test helpers frequently used in system headers
    process_define(t, "#define __GNUC_PREREQ(ma,mi) (((__GNUC__)*100 + (__GNUC_MINOR__)) >= ((ma)*100 + (mi)))");
    process_define(t, "#define __has_include(x) 0");
    process_define(t, "#define __has_include_next(x) 0");
    // Detect host OS/arch/endianness using runtime hints when possible
    set_host_macros(t);
    // Provide conservative fallbacks for feature/attribute checks
    process_define(t, "#define __has_feature(x) 0");
    process_define(t, "#define __has_extension(x) 0");
    process_define(t, "#define __has_attribute(x) 0");
    process_define(t, "#define __has_builtin(x) 0");
    process_define(t, "#define __has_c_attribute(x) 0");
    // Common POSIX access() mode constants as fallbacks
    process_define(t, "#define F_OK 0");
    process_define(t, "#define X_OK 1");
    process_define(t, "#define W_OK 2");
    process_define(t, "#define R_OK 4");
    // Suppress SIMD decl attribute markers from glibc headers for compatibility.
    process_define(t, "#define __SIMD_DECL(function)");
    // OpenSSL headers rely on these attribute/visibility helpers. Provide
    // no-op fallbacks so unknown tokens don't leak into neo-c parsing.
    if (!mtable_get(t, "ossl_inline")) mtable_set_obj(t, "ossl_inline", "");
    if (!mtable_get(t, "ossl_unused")) mtable_set_obj(t, "ossl_unused", "");
    if (!mtable_get(t, "ossl_noreturn")) mtable_set_obj(t, "ossl_noreturn", "");
    if (!mtable_get(t, "OPENSSL_EXPORT")) mtable_set_obj(t, "OPENSSL_EXPORT", "");
    if (!mtable_get(t, "OPENSSL_EXTERN")) mtable_set_obj(t, "OPENSSL_EXTERN", "extern");
    if (!mtable_get(t, "__owur")) mtable_set_obj(t, "__owur", "");
    if (!mtable_get(t, "ossl_ssize_t")) mtable_set_obj(t, "ossl_ssize_t", "ssize_t");

    if (opts) {
        for (size_t di=0; di<opts->ncmd_defs; ++di) {
            const char *d = opts->cmd_defs[di];
            const char *eq = strchr(d, '=');
            if (eq) {
                char name[256]; size_t nlen = (size_t)(eq - d); if (nlen >= sizeof name) nlen = sizeof name - 1; memcpy(name, d, nlen); name[nlen]='\0';
                const char *val = eq+1;
                mtable_set_obj(t, name, val);
            } else {
                mtable_set_obj(t, d, "1");
            }
        }
        for (size_t ui=0; ui<opts->ncmd_undefs; ++ui) {
            mtable_unset(t, opts->cmd_undefs[ui]);
        }
    }
}

static bool starts_with(const char *s, const char *prefix) {
    size_t a = strlen(s), b = strlen(prefix);
    return a >= b && memcmp(s, prefix, b) == 0;
}

static bool is_system_header_path(const char *path) {
    if (!path || !*path) return false;
    return strstr(path, "/usr/include") != NULL ||
           strstr(path, "/usr/local/include") != NULL ||
           strstr(path, "/opt/homebrew/include") != NULL ||
           strstr(path, "/usr/lib/clang") != NULL ||
           strstr(path, "MacOSX.sdk/usr/include") != NULL;
}

static bool parse_define_name(const char *line, char *namebuf, size_t namebuf_sz) {
    if (!line || !namebuf || namebuf_sz == 0) return false;
    const char *p = line;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '#') {
        p++;
        while (*p && isspace((unsigned char)*p)) p++;
    }
    if (starts_with(p, "define")) p += 6;
    while (*p && isspace((unsigned char)*p)) p++;
    size_t adv = 0;
    return parse_ident(p, &adv, namebuf, namebuf_sz);
}

static char *path_join(const char *a, const char *b);

static bool add_sysinc_if_header(PPOpts *opts, char *incdir, const char *header) {
    if (!opts || !incdir || !header) { free(incdir); return false; }
    char *probe = path_join(incdir, header);
    bool ok = file_exists(probe);
    free(probe);
    if (ok) {
        opts->sysincdirs = (const char**)xrealloc((void*)opts->sysincdirs, sizeof(char*)*(opts->nsysincdirs+1));
        opts->sysincdirs[opts->nsysincdirs++] = incdir;
        return true;
    }
    free(incdir);
    return false;
}

static bool add_best_clang_resource_include(PPOpts *opts, const char *base) {
    if (!opts || !base) return false;
    // Probe common clang resource version directories without directory enumeration.
    for (int major = 30; major >= 3; --major) {
        char vbuf[32];
        snprintf(vbuf, sizeof vbuf, "%d", major);
        {
            char *verdir = path_join(base, vbuf);
            char *incdir = path_join(verdir, "include");
            free(verdir);
            if (add_sysinc_if_header(opts, incdir, "stdatomic.h")) return true;
        }
        for (int minor = 0; minor <= 2; ++minor) {
            for (int patch = 0; patch <= 9; ++patch) {
                snprintf(vbuf, sizeof vbuf, "%d.%d.%d", major, minor, patch);
                char *verdir = path_join(base, vbuf);
                char *incdir = path_join(verdir, "include");
                free(verdir);
                if (add_sysinc_if_header(opts, incdir, "stdatomic.h")) return true;
            }
        }
    }
    return false;
}

static bool add_gcc_internal_includes(PPOpts *opts, const char *gcc_base) {
    if (!opts || !gcc_base) return false;
    const char *triples[] = {
        "x86_64-linux-gnu",
        "x86_64-redhat-linux",
        "x86_64-pc-linux-gnu",
        "aarch64-linux-gnu",
        "i386-linux-gnu",
        "arm-linux-gnueabihf",
        "arm-linux-gnueabi",
        "powerpc64le-linux-gnu",
        "riscv64-linux-gnu",
        "s390x-linux-gnu"
    };
    for (size_t ti = 0; ti < sizeof(triples)/sizeof(triples[0]); ++ti) {
        char *tbase = path_join(gcc_base, triples[ti]);
        bool found = false;
        for (int major = 20; major >= 4; --major) {
            char vbuf[16];
            snprintf(vbuf, sizeof vbuf, "%d", major);
            char *vdir = path_join(tbase, vbuf);
            char *incdir = path_join(vdir, "include");
            if (add_sysinc_if_header(opts, incdir, "stdatomic.h")) {
                char *incfixed = path_join(vdir, "include-fixed");
                add_sysinc_if_header(opts, incfixed, "limits.h");
                found = true;
                free(vdir);
                break;
            }
            free(vdir);
        }
        free(tbase);
        if (found) return true;
    }
    return false;
}

static bool is_escaped(const char *s, size_t i) {
    size_t n = 0;
    while (i > 0 && s[i-1] == '\\') { n++; i--; }
    return (n % 2) != 0;
}

static void scan_paren_state(const char *s, int *paren_depth, int *bl_depth) {
    enum { PS_CODE, PS_DQ, PS_SQ, PS_BL } st = (*bl_depth > 0) ? PS_BL : PS_CODE;
    for (size_t i = 0; s && s[i]; ++i) {
        char c = s[i];
        if (st == PS_CODE) {
            if (c == '"') { st = PS_DQ; continue; }
            if (c == '\'') { st = PS_SQ; continue; }
            if (c == '/' && s[i+1] == '/') break;
            if (c == '/' && s[i+1] == '*') { st = PS_BL; (*bl_depth)++; i++; continue; }
            if (c == '(') { (*paren_depth)++; continue; }
            if (c == ')' && *paren_depth > 0) { (*paren_depth)--; continue; }
        } else if (st == PS_DQ) {
            if (c == '"' && !is_escaped(s, i)) st = PS_CODE;
        } else if (st == PS_SQ) {
            if (c == '\'' && !is_escaped(s, i)) st = PS_CODE;
        } else if (st == PS_BL) {
            if (c == '/' && s[i+1] == '*') { (*bl_depth)++; i++; continue; }
            if (c == '*' && s[i+1] == '/') { (*bl_depth)--; i++; if (*bl_depth <= 0) { *bl_depth = 0; st = PS_CODE; } continue; }
        }
    }
}

static void scan_comment_depth(const char *s, int *depth) {
    enum { SD_CODE, SD_DQ, SD_SQ, SD_BL, SD_SL } st = (*depth > 0) ? SD_BL : SD_CODE;
    for (size_t i = 0; s && s[i]; ++i) {
        char c = s[i];
        if (st == SD_CODE) {
            if (c == '"') { st = SD_DQ; continue; }
            if (c == '\'') { st = SD_SQ; continue; }
            if (c == '/' && s[i+1] == '/') { st = SD_SL; break; }
            if (c == '/' && s[i+1] == '*') { (*depth)++; st = SD_BL; i++; continue; }
        } else if (st == SD_DQ) {
            if (c == '"' && !is_escaped(s, i)) st = SD_CODE;
        } else if (st == SD_SQ) {
            if (c == '\'' && !is_escaped(s, i)) st = SD_CODE;
        } else if (st == SD_BL) {
            if (c == '/' && s[i+1] == '*') { (*depth)++; i++; continue; }
            if (c == '*' && s[i+1] == '/') { (*depth)--; i++; if (*depth <= 0) { *depth = 0; st = SD_CODE; } continue; }
        }
    }
}

static bool starts_c_include_block(const char *s) {
    const char *p = s ? s : "";
    while (*p && isspace((unsigned char)*p)) p++;
    if (strncmp(p, "c_include", 9) != 0) return false;
    if (isalnum((unsigned char)p[9]) || p[9] == '_') return false;
    p += 9;
    while (*p && isspace((unsigned char)*p)) p++;
    return *p == '{';
}

static void scan_c_include_brace_depth(const char *s, int *depth) {
    enum { CB_CODE, CB_DQ, CB_SQ, CB_BL, CB_SL } st = CB_CODE;
    for (size_t i = 0; s && s[i]; ++i) {
        char c = s[i];
        if (st == CB_CODE) {
            if (c == '"') { st = CB_DQ; continue; }
            if (c == '\'') { st = CB_SQ; continue; }
            if (c == '/' && s[i+1] == '/') { st = CB_SL; break; }
            if (c == '/' && s[i+1] == '*') { st = CB_BL; i++; continue; }
            if (c == '{') { (*depth)++; continue; }
            if (c == '}' && *depth > 0) { (*depth)--; continue; }
        } else if (st == CB_DQ) {
            if (c == '"' && !is_escaped(s, i)) st = CB_CODE;
        } else if (st == CB_SQ) {
            if (c == '\'' && !is_escaped(s, i)) st = CB_CODE;
        } else if (st == CB_BL) {
            if (c == '*' && s[i+1] == '/') { st = CB_CODE; i++; continue; }
        }
    }
}

static bool only_trailing_space_or_comments(const char *p) {
    const char *q = p ? p : "";
    while (1) {
        while (*q && isspace((unsigned char)*q)) q++;
        if (*q == '\0') return true;
        if (q[0] == '/' && q[1] == '/') return true;
        if (q[0] == '/' && q[1] == '*') {
            q += 2;
            while (*q && !(q[0] == '*' && q[1] == '/')) q++;
            if (*q == '\0') return true;
            q += 2;
            continue;
        }
        return false;
    }
}

static void pp_directive_error(const char *path, long line, const char *msg);
static void pp_directive_warning(const char *path, long line, const char *msg);

static void parse_line_control_payload(const char *payload, bool allow_linemarker_flags,
                                       const char *path, long err_line,
                                       long *logical_line_out, bool *has_fname_out, char **fname_out) {
    const char *lp = payload ? payload : "";
    while (*lp && isspace((unsigned char)*lp)) lp++;
    if (!isdigit((unsigned char)*lp)) {
        pp_directive_error(path, err_line, "invalid #line directive");
    }

    errno = 0;
    char *endp = NULL;
    long logical_line = strtol(lp, &endp, 10);
    if (endp == lp || errno == ERANGE || logical_line <= 0) {
        pp_directive_error(path, err_line, "invalid #line directive");
    }

    bool has_fname = false;
    char *parsed_fname = NULL;
    lp = endp;
    while (*lp && isspace((unsigned char)*lp)) lp++;
    if (*lp == '"') {
        const char *scan = lp + 1;
        size_t fname_len = 0;
        while (*scan && *scan != '"') {
            if (*scan == '\\' && scan[1] != '\0') scan++;
            fname_len++;
            scan++;
        }
        if (*scan != '"') {
            pp_directive_error(path, err_line, "invalid #line directive");
        }
        parsed_fname = (char*)xrealloc(NULL, fname_len + 1);
        size_t wi = 0;
        lp++;
        while (*lp && *lp != '"') {
            if (*lp == '\\' && lp[1] != '\0') lp++;
            parsed_fname[wi++] = *lp;
            lp++;
        }
        parsed_fname[wi] = '\0';
        if (*lp != '"') {
            pp_directive_error(path, err_line, "invalid #line directive");
        }
        lp++;
        if (allow_linemarker_flags) {
            while (1) {
                while (*lp && isspace((unsigned char)*lp)) lp++;
                if (*lp == '\0') break;
                if (lp[0] == '/' && lp[1] == '/') break;
                if (lp[0] == '/' && lp[1] == '*') {
                    if (only_trailing_space_or_comments(lp)) break;
                    pp_directive_error(path, err_line, "invalid #line directive");
                }
                if (!isdigit((unsigned char)*lp)) {
                    pp_directive_error(path, err_line, "invalid #line directive");
                }
                while (isdigit((unsigned char)*lp)) lp++;
            }
        } else if (!only_trailing_space_or_comments(lp)) {
            pp_directive_error(path, err_line, "invalid #line directive");
        }
        has_fname = true;
    } else if (!only_trailing_space_or_comments(lp)) {
        pp_directive_error(path, err_line, "invalid #line directive");
    }

    *logical_line_out = logical_line;
    *has_fname_out = has_fname;
    *fname_out = parsed_fname;
}

static long g_cur_line = 0;

static void pp_directive_error(const char *path, long line, const char *msg) {
    long logical_line = line > 0 ? line : 1;
    long real_line = g_cur_line > 0 ? g_cur_line : logical_line;
    if (path && *path) {
        fprintf(stderr, "%s %ld(%ld): [error] %s\n", path, logical_line, real_line, msg ? msg : "preprocessor error");
    } else {
        fprintf(stderr, "error: %s\n", msg ? msg : "preprocessor error");
    }
    exit(1);
}

static void pp_directive_warning(const char *path, long line, const char *msg) {
    long logical_line = line > 0 ? line : 1;
    long real_line = g_cur_line > 0 ? g_cur_line : logical_line;
    if (path && *path) {
        fprintf(stderr, "%s %ld(%ld): [warning] %s\n", path, logical_line, real_line, msg ? msg : "preprocessor warning");
    } else {
        fprintf(stderr, "warning: %s\n", msg ? msg : "preprocessor warning");
    }
}

static void strip_comments_inplace(char *s) {
    if (!s) return;
    enum { SC_CODE, SC_DQ, SC_SQ, SC_BL } st = SC_CODE;
    int bl_depth = 0;
    size_t r = 0, w = 0;
    while (s[r]) {
        char c = s[r];
        if (st == SC_CODE) {
            if (c == '"') { s[w++] = c; st = SC_DQ; r++; continue; }
            if (c == '\'') { s[w++] = c; st = SC_SQ; r++; continue; }
            if (c == '/' && s[r+1] == '/') {
                if (w > 0 && !isspace((unsigned char)s[w-1])) s[w++] = ' ';
                break;
            }
            if (c == '/' && s[r+1] == '*') {
                if (w > 0 && !isspace((unsigned char)s[w-1])) s[w++] = ' ';
                st = SC_BL; bl_depth = 1; r += 2; continue;
            }
            s[w++] = c; r++; continue;
        } else if (st == SC_DQ) {
            s[w++] = c;
            if (c == '"' && s[r-1] != '\\') st = SC_CODE;
            r++;
        } else if (st == SC_SQ) {
            s[w++] = c;
            if (c == '\'' && s[r-1] != '\\') st = SC_CODE;
            r++;
        } else if (st == SC_BL) {
            if (c == '/' && s[r+1] == '*') { bl_depth++; r += 2; continue; }
            if (c == '*' && s[r+1] == '/') {
                bl_depth--; r += 2;
                if (bl_depth <= 0) { bl_depth = 0; st = SC_CODE; }
                continue;
            }
            r++;
        }
    }
    s[w] = '\0';
}

static void strip_comments_multiline_inplace(char *s) {
    if (!s) return;
    enum { SC_CODE, SC_DQ, SC_SQ, SC_SL, SC_BL } st = SC_CODE;
    int bl_depth = 0;
    size_t r = 0, w = 0;
    while (s[r]) {
        char c = s[r];
        if (st == SC_CODE) {
            if (c == '"') { s[w++] = c; st = SC_DQ; r++; continue; }
            if (c == '\'') { s[w++] = c; st = SC_SQ; r++; continue; }
            if (c == '/' && s[r+1] == '/') {
                if (w > 0 && !isspace((unsigned char)s[w-1])) s[w++] = ' ';
                st = SC_SL; r += 2; continue;
            }
            if (c == '/' && s[r+1] == '*') {
                if (w > 0 && !isspace((unsigned char)s[w-1])) s[w++] = ' ';
                st = SC_BL; bl_depth = 1; r += 2; continue;
            }
            s[w++] = c; r++; continue;
        } else if (st == SC_DQ) {
            s[w++] = c;
            if (c == '"' && !is_escaped(s, r)) st = SC_CODE;
            r++;
        } else if (st == SC_SQ) {
            s[w++] = c;
            if (c == '\'' && !is_escaped(s, r)) st = SC_CODE;
            r++;
        } else if (st == SC_SL) {
            if (c == '\n' || c == '\r') {
                s[w++] = c;
                st = SC_CODE;
            }
            r++;
        } else if (st == SC_BL) {
            if (c == '/' && s[r+1] == '*') { bl_depth++; r += 2; continue; }
            if (c == '*' && s[r+1] == '/') {
                bl_depth--; r += 2;
                if (bl_depth <= 0) { bl_depth = 0; st = SC_CODE; }
                continue;
            }
            if (c == '\n' || c == '\r') s[w++] = c;
            r++;
        }
    }
    s[w] = '\0';
}

static void free_macro_params(char **params, size_t nparams) {
    if (!params) return;
    for (size_t i = 0; i < nparams; ++i) free(params[i]);
    free(params);
}

static void skip_pp_space_and_comments(const char *s, size_t *i) {
    while (1) {
        while (s[*i] && isspace((unsigned char)s[*i])) (*i)++;
        if (s[*i] == '/' && s[*i + 1] == '/') {
            *i += 2;
            while (s[*i] && s[*i] != '\n' && s[*i] != '\r') (*i)++;
            continue;
        }
        if (s[*i] == '/' && s[*i + 1] == '*') {
            *i += 2;
            while (s[*i] && !(s[*i] == '*' && s[*i + 1] == '/')) (*i)++;
            if (s[*i]) *i += 2;
            continue;
        }
        break;
    }
}

static size_t match_pp_punctuator_len(const char *s) {
    static const char *puncts[] = {
        "%:%:", "<<=", ">>=", "...", "->", "++", "--", "<<", ">>",
        "<=", ">=", "==", "!=", "&&", "||", "*=", "/=", "%=", "+=",
        "-=", "&=", "^=", "|=", "##", "<:", ":>", "<%", "%>", "%:"
    };
    for (size_t i = 0; i < sizeof(puncts) / sizeof(puncts[0]); ++i) {
        size_t n = strlen(puncts[i]);
        if (strncmp(s, puncts[i], n) == 0) return n;
    }
    return 1;
}

static bool next_pp_token_span(const char *s, size_t *idx, size_t *beg, size_t *end) {
    size_t i = *idx;
    skip_pp_space_and_comments(s, &i);
    if (!s[i]) {
        *idx = i;
        return false;
    }

    size_t b = i;
    char c = s[i];
    if (isalpha((unsigned char)c) || c == '_') {
        i++;
        while (s[i] && (isalnum((unsigned char)s[i]) || s[i] == '_')) i++;
    }
    else if (isdigit((unsigned char)c) || (c == '.' && isdigit((unsigned char)s[i + 1]))) {
        i++;
        while (s[i]) {
            char d = s[i];
            if (isalnum((unsigned char)d) || d == '_' || d == '.') {
                i++;
                continue;
            }
            if ((d == '+' || d == '-') &&
                (s[i - 1] == 'e' || s[i - 1] == 'E' || s[i - 1] == 'p' || s[i - 1] == 'P')) {
                i++;
                continue;
            }
            break;
        }
    }
    else if (c == '"' || c == '\'') {
        char q = c;
        i++;
        while (s[i]) {
            if (s[i] == '\\' && s[i + 1]) { i += 2; continue; }
            if (s[i] == q) { i++; break; }
            i++;
        }
    }
    else {
        i += match_pp_punctuator_len(s + i);
    }

    *beg = b;
    *end = i;
    *idx = i;
    return true;
}

static bool macro_replacement_tokens_equal(const char *a, const char *b) {
    size_t ia = 0;
    size_t ib = 0;
    const char *sa = a ? a : "";
    const char *sb = b ? b : "";

    while (1) {
        size_t ba = 0, ea = 0, bb = 0, eb = 0;
        bool ha = next_pp_token_span(sa, &ia, &ba, &ea);
        bool hb = next_pp_token_span(sb, &ib, &bb, &eb);
        if (!ha || !hb) return ha == hb;
        if ((ea - ba) != (eb - bb)) return false;
        if (strncmp(sa + ba, sb + bb, ea - ba) != 0) return false;
    }
}

static bool macro_var_param_equal(const char *a, const char *b) {
    if (!a && !b) return true;
    if (!a || !b) return false;
    return strcmp(a, b) == 0;
}

static bool macro_def_compatible(const Macro *m, bool is_func, char **params, size_t nparams,
                                 bool variadic, const char *var_param, const char *value) {
    if (!m) return true;
    if (m->is_func != is_func) return false;
    if (is_func) {
        if (m->nparams != nparams) return false;
        if (m->is_variadic != variadic) return false;
        for (size_t i = 0; i < nparams; ++i) {
            if (!m->params || !m->params[i] || !params || !params[i]) return false;
            if (strcmp(m->params[i], params[i]) != 0) return false;
        }
        if (!macro_var_param_equal(m->var_param, var_param)) return false;
    }
    return macro_replacement_tokens_equal(m->value ? m->value : "", value ? value : "");
}

static bool process_define(MacroTable *t, char *line) {
    // Parse: (optional) '#' + spaces + 'define' + spaces + NAME[(params)] [VALUE...]
    // Accept both "#define ..." and "#   define ..." and even just "define ..." (internal use).
    char *p = line;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '#') {
        p++;
        while (*p && isspace((unsigned char)*p)) p++;
    }
    if (starts_with(p, "define")) p += 6;
    while (*p && isspace((unsigned char)*p)) p++;

    char namebuf[256];
    size_t ni = 0;
    if (!(isalpha((unsigned char)*p) || *p == '_')) return false;
    while (*p && (isalnum((unsigned char)*p) || *p == '_')) {
        if (ni + 1 < sizeof(namebuf)) namebuf[ni++] = *p;
        p++;
    }
    namebuf[ni] = '\0';
    if (strcmp(namebuf, "_Static_assert") == 0) return true;
    if (strcmp(namebuf, "__SIMD_DECL") == 0 && mtable_find(t, "__SIMD_DECL")) return true;

    char *after_name = p;
    if (*after_name == '(') {
        p = after_name + 1;
        char **params = NULL;
        size_t np = 0, pc = 0;
        bool variadic = false;
        char *var_param = NULL;
        bool parse_ok = true;
        bool done = false;

        while (1) {
            while (*p && isspace((unsigned char)*p)) p++;
            if (*p == ')') {
                p++;
                done = true;
                break;
            }

            if (p[0] == '.' && p[1] == '.' && p[2] == '.') {
                variadic = true;
                p += 3;
                while (*p && isspace((unsigned char)*p)) p++;
                if (*p != ')') { parse_ok = false; break; }
                p++;
                done = true;
                break;
            }

            char id[256];
            size_t j = 0;
            if (!(isalpha((unsigned char)*p) || *p == '_')) { parse_ok = false; break; }
            while (*p && (isalnum((unsigned char)*p) || *p == '_')) {
                if (j + 1 < sizeof(id)) id[j++] = *p;
                p++;
            }
            id[j] = '\0';

            const char *p2 = p;
            while (*p2 && isspace((unsigned char)*p2)) p2++;
            if (p2[0] == '.' && p2[1] == '.' && p2[2] == '.') {
                variadic = true;
                free(var_param);
                var_param = xstrdup(id);
                p = (char *)p2 + 3;
                while (*p && isspace((unsigned char)*p)) p++;
                if (*p != ')') { parse_ok = false; break; }
                p++;
                done = true;
                break;
            }

            if (np == pc) {
                pc = pc ? pc * 2 : 4;
                params = (char **)xrealloc(params, pc * sizeof(char *));
            }
            params[np++] = xstrdup(id);
            p = (char *)p2;
            if (*p == ',') { p++; continue; }
            if (*p == ')') {
                p++;
                done = true;
                break;
            }
            parse_ok = false;
            break;
        }

        if (!parse_ok || !done) {
            free_macro_params(params, np);
            free(var_param);
            return false;
        }

        while (*p && isspace((unsigned char)*p)) p++;
        char *valbuf = xstrdup(p);
        strip_comments_inplace(valbuf);
        size_t L = strlen(valbuf);
        while (L && isspace((unsigned char)valbuf[L - 1])) valbuf[--L] = '\0';

        Macro *m = mtable_find(t, namebuf);
        if (m) {
            bool ok = macro_def_compatible(m, true, params, np, variadic, var_param, valbuf);
            free_macro_params(params, np);
            free(var_param);
            free(valbuf);
            return ok;
        }

        mtable_set_func(t, namebuf, params, np, variadic, var_param, valbuf);
        free(valbuf);
        return true;
    }

    p = after_name;
    while (*p && isspace((unsigned char)*p)) p++;
    char *valbuf = xstrdup(p);
    strip_comments_inplace(valbuf);
    size_t L = strlen(valbuf);
    while (L && isspace((unsigned char)valbuf[L - 1])) valbuf[--L] = '\0';

    Macro *m = mtable_find(t, namebuf);
    if (m) {
        bool ok = macro_def_compatible(m, false, NULL, 0, false, NULL, valbuf);
        free(valbuf);
        return ok;
    }

    mtable_set_obj(t, namebuf, valbuf);
    free(valbuf);
    return true;
}

static void mtable_set_func(MacroTable *t, const char *name, char **params, size_t nparams,
                            bool variadic, char *var_param, const char *value) {
    Macro *m = mtable_find(t, name);
    if (!m) {
        m = mtable_append(t, name);
    } else {
        free(m->value);
        if (m->params) { for (size_t j=0;j<m->nparams;++j) free(m->params[j]); free(m->params); }
        if (m->var_param) { free(m->var_param); }
    }
    m->is_func = true;
    m->value = xstrdup(value ? value : "");
    m->params = params;
    m->nparams = nparams;
    m->var_param = var_param;
    m->is_variadic = variadic;
    m->self_ref = macro_value_contains_ident(m->value, name);
}

typedef struct {
    char *buf;
    size_t len;
    size_t cap;
} Str;

static void sb_init(Str *s) { s->buf = NULL; s->len = 0; s->cap = 0; }
static void sb_free(Str *s) { free(s->buf); }
static void sb_reserve(Str *s, size_t need) {
    if (need <= s->cap) return;
    size_t ncap = s->cap ? s->cap : 128;
    while (need > ncap) ncap *= 2;
    s->buf = (char *)xrealloc(s->buf, ncap);
    s->cap = ncap;
}
static void sb_putc(Str *s, char c) {
    sb_reserve(s, s->len + 2);
    s->buf[s->len++] = c; s->buf[s->len] = '\0';
}
static void sb_putn(Str *s, const char *t, size_t n) {
    if (n == 0) return;
    sb_reserve(s, s->len + n + 1);
    memcpy(s->buf + s->len, t, n);
    s->len += n;
    s->buf[s->len] = '\0';
}
static void sb_puts(Str *s, const char *t) {
    size_t n = strlen(t);
    sb_putn(s, t, n);
}

static void normalize_digraph_tokens_inplace(Str *line) {
    if (!line || !line->buf || !*line->buf) return;

    enum { S_CODE, S_DQ, S_SQ, S_SL, S_BL } st = S_CODE;
    int bl_depth = 0;
    const char *s = line->buf;
    Str out;
    sb_init(&out);

    for (size_t i = 0; s[i]; ) {
        char c = s[i];

        if (st == S_CODE) {
            if (c == '"') { st = S_DQ; sb_putc(&out, c); i++; continue; }
            if (c == '\'') { st = S_SQ; sb_putc(&out, c); i++; continue; }
            if (c == '/' && s[i+1] == '/') { st = S_SL; sb_puts(&out, "//"); i += 2; continue; }
            if (c == '/' && s[i+1] == '*') { st = S_BL; bl_depth = 1; sb_puts(&out, "/*"); i += 2; continue; }

            if (c == '%' && s[i+1] == ':' && s[i+2] == '%' && s[i+3] == ':') {
                sb_puts(&out, "##");
                i += 4;
                continue;
            }
            if (c == '%' && s[i+1] == ':') {
                sb_putc(&out, '#');
                i += 2;
                continue;
            }
            if (c == '<' && s[i+1] == ':') {
                sb_putc(&out, '[');
                i += 2;
                continue;
            }
            if (c == ':' && s[i+1] == '>') {
                sb_putc(&out, ']');
                i += 2;
                continue;
            }
            if (c == '<' && s[i+1] == '%') {
                sb_putc(&out, '{');
                i += 2;
                continue;
            }
            if (c == '%' && s[i+1] == '>') {
                size_t p = i;
                while (p > 0 && isspace((unsigned char)s[p-1])) p--;
                if (p > 0 && s[p-1] == '*') {
                    // neo-c ownership suffix "*%>" must stay intact.
                    sb_puts(&out, "%>");
                } else {
                    sb_putc(&out, '}');
                }
                i += 2;
                continue;
            }

            sb_putc(&out, c);
            i++;
        } else if (st == S_DQ) {
            sb_putc(&out, c);
            if (c == '"' && (i == 0 || s[i-1] != '\\')) st = S_CODE;
            i++;
        } else if (st == S_SQ) {
            sb_putc(&out, c);
            if (c == '\'' && (i == 0 || s[i-1] != '\\')) st = S_CODE;
            i++;
        } else if (st == S_SL) {
            sb_putc(&out, c);
            if (c == '\n' || c == '\r') st = S_CODE;
            i++;
        } else {
            if (c == '/' && s[i+1] == '*') { bl_depth++; sb_puts(&out, "/*"); i += 2; continue; }
            if (c == '*' && s[i+1] == '/') {
                bl_depth--;
                sb_puts(&out, "*/");
                i += 2;
                if (bl_depth <= 0) { bl_depth = 0; st = S_CODE; }
                continue;
            }
            sb_putc(&out, c);
            i++;
        }
    }

    sb_free(line);
    *line = out;
}

static void normalize_trigraphs_inplace(Str *line) {
    if (!line || !line->buf || !*line->buf) return;

    const char *s = line->buf;
    Str out;
    sb_init(&out);

    for (size_t i = 0; s[i]; ) {
        if (s[i] == '?' && s[i+1] == '?') {
            char repl = '\0';
            switch (s[i+2]) {
                case '=': repl = '#'; break;
                case '(': repl = '['; break;
                case '/': repl = '\\'; break;
                case ')': repl = ']'; break;
                case '\'': repl = '^'; break;
                case '<': repl = '{'; break;
                case '!': repl = '|'; break;
                case '>': repl = '}'; break;
                case '-': repl = '~'; break;
                default: break;
            }
            if (repl != '\0') {
                sb_putc(&out, repl);
                i += 3;
                continue;
            }
        }
        sb_putc(&out, s[i]);
        i++;
    }

    sb_free(line);
    *line = out;
}

static void sb_put_escaped_cstr(Str *s, const char *t) {
    sb_putc(s, '"');
    if (t) {
        for (size_t i = 0; t[i]; ++i) {
            unsigned char c = (unsigned char)t[i];
            if (c == '\\' || c == '"') { sb_putc(s, '\\'); sb_putc(s, (char)c); }
            else if (c == '\n') sb_puts(s, "\\n");
            else if (c == '\r') sb_puts(s, "\\r");
            else if (c == '\t') sb_puts(s, "\\t");
            else sb_putc(s, (char)c);
        }
    }
    sb_putc(s, '"');
}

static FILE *open_in_search(const char *name, bool quoted,
                            const char *curdir, const PPOpts *opts,
                            char **outpath);
static FILE *open_in_search_next(const char *name,
                                 const char *this_path,
                                 const PPOpts *opts,
                                 char **outpath);


static bool g_keep_comments = true; // default; set per-invocation in preprocess
// Persist block comment depth across lines
static int g_block_comment_depth = 0;
// Tracks block-comment nesting that started on non-emitted lines (for example,
// directive lines). Those comments must stay stripped even when keep_comments
// is enabled, otherwise continuation lines can leak as raw tokens.
static int g_hidden_block_comment_depth = 0;
static const char *g_cur_file = NULL;
static int g_expand_pass = 0;
static int g_preprocess_depth = 0;
static bool g_injected_xen_types = false;
static Str g_macro_call_out = {0};
// Context for evaluating __has_include in #if expressions
static const PPOpts *g_ifexpr_opts = NULL;
static const char *g_ifexpr_curdir = NULL;
static const char *g_ifexpr_this_path = NULL;
static bool g_ifexpr_builtin_error = false;

static void update_hidden_comment_depth(int before, int after) {
    if (after > before) {
        g_hidden_block_comment_depth += (after - before);
    } else if (after < before) {
        int dec = before - after;
        if (dec > g_hidden_block_comment_depth) dec = g_hidden_block_comment_depth;
        g_hidden_block_comment_depth -= dec;
    }
    if (g_hidden_block_comment_depth < 0) g_hidden_block_comment_depth = 0;
}

static int eval_has_include_name(const char *name, bool quoted, bool next) {
    if (!name || !*name) return 0;
    if (!g_ifexpr_opts) return 0;
    char *opened_path = NULL;
    FILE *f = NULL;
    if (next) {
        f = open_in_search_next(name, g_ifexpr_this_path, g_ifexpr_opts, &opened_path);
    } else {
        f = open_in_search(name, quoted, g_ifexpr_curdir, g_ifexpr_opts, &opened_path);
    }
    if (f) fclose(f);
    if (opened_path) free(opened_path);
    return f ? 1 : 0;
}

static const char *skip_pp_space(const char *p) {
    while (*p && isspace((unsigned char)*p)) p++;
    return p;
}

static bool parse_header_name_operand(const char *text, bool *quoted, char *name, size_t name_sz) {
    const char *p = skip_pp_space(text ? text : "");
    bool is_quoted = false;
    size_t ni = 0;

    if (*p == '"') {
        is_quoted = true;
        p++;
        while (*p && *p != '"') {
            if (*p == '\\' && p[1] != '\0') p++;
            if (ni + 1 < name_sz) name[ni++] = *p;
            p++;
        }
        if (*p != '"') return false;
        p++;
    } else if (*p == '<') {
        is_quoted = false;
        p++;
        while (*p && *p != '>') {
            if (ni + 1 < name_sz) name[ni++] = *p;
            p++;
        }
        if (*p != '>') return false;
        p++;
    } else {
        return false;
    }

    p = skip_pp_space(p);
    if (*p != '\0' || ni == 0) return false;
    name[ni] = '\0';
    if (quoted) *quoted = is_quoted;
    return true;
}

static bool resolve_has_include_operand(const MacroTable *t, const char *operand,
                                        bool *quoted, char *name, size_t name_sz) {
    char cur[1024];
    const char *s = skip_pp_space(operand ? operand : "");
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) n--;
    if (n == 0 || n >= sizeof(cur)) return false;
    memcpy(cur, s, n);
    cur[n] = '\0';

    for (int pass = 0; pass < PP_RESCAN_MAX; ++pass) {
        if (parse_header_name_operand(cur, quoted, name, name_sz)) {
            return true;
        }

        const char *p = skip_pp_space(cur);
        if (!(isalpha((unsigned char)*p) || *p == '_')) return false;
        const char *q = p + 1;
        while (*q && (isalnum((unsigned char)*q) || *q == '_')) q++;
        const char *trail = skip_pp_space(q);
        if (*trail != '\0') return false;

        size_t ilen = (size_t)(q - p);
        if (ilen == 0 || ilen >= 256) return false;
        char ident[256];
        memcpy(ident, p, ilen);
        ident[ilen] = '\0';

        const char *mv = t ? mtable_get((MacroTable*)t, ident) : NULL;
        if (!mv) return false;

        const char *m = skip_pp_space(mv);
        size_t mlen = strlen(m);
        while (mlen > 0 && isspace((unsigned char)m[mlen - 1])) mlen--;
        if (mlen == 0 || mlen >= sizeof(cur)) return false;
        memcpy(cur, m, mlen);
        cur[mlen] = '\0';
    }
    return false;
}

static bool eval_has_include_call(const MacroTable *t, const char *line, size_t start, bool next, size_t *end, int *value) {
    size_t p = start;
    while (line[p] && isspace((unsigned char)line[p])) p++;
    if (line[p] != '(') return false;
    p++;
    size_t arg_start = p;
    int depth = 1;
    while (line[p]) {
        if (line[p] == '(') {
            depth++;
        } else if (line[p] == ')') {
            depth--;
            if (depth == 0) break;
        }
        p++;
    }
    if (depth != 0 || line[p] != ')') return false;

    size_t arg_len = p - arg_start;
    char *arg = (char*)malloc(arg_len + 1);
    if (!arg) die("malloc");
    memcpy(arg, line + arg_start, arg_len);
    arg[arg_len] = '\0';

    bool quoted = false;
    char name[512];
    bool ok = resolve_has_include_operand(t, arg, &quoted, name, sizeof(name));
    free(arg);
    if (!ok) return false;

    p++;
    if (end) *end = p;
    if (value) *value = eval_has_include_name(name, quoted, next);
    return true;
}

static bool eval_has_attribute_call(const char *line, size_t start, size_t *end, int *value) {
    size_t p = start;
    while (line[p] && isspace((unsigned char)line[p])) p++;
    if (line[p] != '(') return false;
    p++;
    while (line[p] && isspace((unsigned char)line[p])) p++;

    if (!(isalpha((unsigned char)line[p]) || line[p] == '_')) return false;
    while (line[p] && (isalnum((unsigned char)line[p]) || line[p] == '_')) p++;

    while (line[p] && isspace((unsigned char)line[p])) p++;
    if (line[p] != ')') return false;
    p++;
    if (end) *end = p;
    if (value) *value = 0;
    return true;
}

static void sb_put_stringized_argument(Str *out, const char *raw);

#define IDENT_EQ(ptr, len, lit) ((len) == sizeof(lit) - 1 && memcmp((ptr), (lit), sizeof(lit) - 1) == 0)

static void expand_into_str_mode(const MacroTable *t, const char *line, Str *out, bool protect_defined) {
    enum { CODE, STR_DQ, STR_SQ, SL_COMMENT, BL_COMMENT } state = CODE;
    int bl_depth = g_block_comment_depth; // continue block comments across lines
    int hidden_bl_depth = g_hidden_block_comment_depth;
    if (bl_depth > 0) state = BL_COMMENT;
    for (size_t i = 0; line[i] != '\0'; ) {
        char c = line[i];
        if (c == '\n') {
            sb_putc(out, c);
            g_cur_line++;
            if (state == SL_COMMENT) state = CODE;
            i++;
            continue;
        }
        if (state == CODE) {
            // handle comments
            if (c == '/' && line[i+1] == '/') {
                if (g_keep_comments) { state = SL_COMMENT; sb_puts(out, "//"); i += 2; continue; }
                // strip rest of line
                break;
            }
            if (c == '/' && line[i+1] == '*') {
                state = BL_COMMENT; bl_depth += 1; i += 2;
                if (g_keep_comments && hidden_bl_depth == 0) { sb_puts(out, "/*"); }
                continue;
            }
            if (c == '"') { state = STR_DQ; sb_putc(out, c); i++; continue; }
            if (c == '\'') { state = STR_SQ; sb_putc(out, c); i++; continue; }
            if (isalpha((unsigned char)c) || c == '_') {
                const char *ident = line + i;
                size_t j = i;
                while (line[j] && (isalnum((unsigned char)line[j]) || line[j] == '_')) j++;
                size_t ident_len = j - i;
                if (IDENT_EQ(ident, ident_len, "__LINE__")) {
                    char numbuf[32];
                    snprintf(numbuf, sizeof numbuf, "%ld", g_cur_line);
                    sb_puts(out, numbuf);
                    i = j;
                    continue;
                }
                if (IDENT_EQ(ident, ident_len, "__FILE__")) {
                    sb_put_escaped_cstr(out, g_cur_file ? g_cur_file : "");
                    i = j;
                    continue;
                }
                if (IDENT_EQ(ident, ident_len, "true") || IDENT_EQ(ident, ident_len, "false")) {
                    sb_putn(out, ident, ident_len);
                    i = j;
                    continue;
                }
                if (protect_defined && IDENT_EQ(ident, ident_len, "defined")) {
                    sb_puts(out, "defined");
                    size_t p = j;
                    while (line[p] && isspace((unsigned char)line[p])) { sb_putc(out, line[p]); p++; }
                    if (line[p] == '(') {
                        sb_putc(out, line[p++]);
                        while (line[p] && isspace((unsigned char)line[p])) { sb_putc(out, line[p]); p++; }
                        if (isalpha((unsigned char)line[p]) || line[p] == '_') {
                            while (line[p] && (isalnum((unsigned char)line[p]) || line[p] == '_')) {
                                sb_putc(out, line[p]); p++;
                            }
                            while (line[p] && isspace((unsigned char)line[p])) { sb_putc(out, line[p]); p++; }
                            if (line[p] == ')') { sb_putc(out, line[p]); p++; }
                        }
                        i = p;
                        continue;
                    } else if (isalpha((unsigned char)line[p]) || line[p] == '_') {
                        while (line[p] && (isalnum((unsigned char)line[p]) || line[p] == '_')) {
                            sb_putc(out, line[p]); p++;
                        }
                        i = p;
                        continue;
                    }
                    i = j;
                    continue;
                }
                if (protect_defined && (IDENT_EQ(ident, ident_len, "__has_include") || IDENT_EQ(ident, ident_len, "__has_include_next"))) {
                    if (IDENT_EQ(ident, ident_len, "__has_include_next") &&
                        g_ifexpr_this_path && strstr(g_ifexpr_this_path, "stdatomic.h") != NULL) {
                        size_t p2 = j;
                        while (line[p2] && isspace((unsigned char)line[p2])) p2++;
                        if (line[p2] != '(') {
                            g_ifexpr_builtin_error = true;
                            sb_puts(out, "0");
                            i = j;
                            continue;
                        }
                        int depth = 0;
                        size_t q = p2;
                        while (line[q]) {
                            if (line[q] == '(') depth++;
                            else if (line[q] == ')') {
                                depth--;
                                if (depth == 0) { q++; break; }
                            }
                            q++;
                        }
                        if (depth != 0) {
                            g_ifexpr_builtin_error = true;
                            q = strlen(line);
                        }
                        sb_puts(out, "0");
                        i = q;
                        continue;
                    }
                    size_t endp = 0;
                    int val = 0;
                    bool next = IDENT_EQ(ident, ident_len, "__has_include_next");
                    size_t p2 = j;
                    while (line[p2] && isspace((unsigned char)line[p2])) p2++;
                    if (line[p2] != '(') {
                        g_ifexpr_builtin_error = true;
                        sb_puts(out, "0");
                        i = j;
                        continue;
                    }
                    if (eval_has_include_call(t, line, j, next, &endp, &val)) {
                        sb_puts(out, val ? "1" : "0");
                        i = endp;
                        continue;
                    }
                    // __has_include / __has_include_next used with call syntax
                    // but malformed operand should be diagnosed as invalid #if.
                    if (line[p2] == '(') {
                        g_ifexpr_builtin_error = true;
                        int depth = 0;
                        size_t q = p2;
                        while (line[q]) {
                            if (line[q] == '(') depth++;
                            else if (line[q] == ')') {
                                depth--;
                                if (depth == 0) { q++; break; }
                            }
                            q++;
                        }
                        if (depth != 0) q = strlen(line);
                        sb_puts(out, "0");
                        i = q;
                        continue;
                    }
                }
                if (protect_defined && IDENT_EQ(ident, ident_len, "__has_attribute")) {
                    size_t endp = 0;
                    int val = 0;
                    if (eval_has_attribute_call(line, j, &endp, &val)) {
                        sb_puts(out, val ? "1" : "0");
                        i = endp;
                        continue;
                    }
                }
                // Handle _Pragma("...") operator by consuming it and suppressing output,
                // since non-"once" pragmas are dropped elsewhere anyway.
                if (IDENT_EQ(ident, ident_len, "_Pragma")) {
                    size_t j2 = j; while (line[j2] && isspace((unsigned char)line[j2])) j2++;
                    if (line[j2] == '(') {
                        size_t p = j2 + 1;
                        // skip spaces
                        while (line[p] && isspace((unsigned char)line[p])) p++;
                        bool ok = false;
                        if (line[p] == '"') {
                            // consume string literal, handling escapes
                            p++;
                            while (line[p] != '\0') {
                                if (line[p] == '"' && !is_escaped(line, p)) { p++; ok = true; break; }
                                p++;
                            }
                            // skip trailing spaces
                            while (line[p] && isspace((unsigned char)line[p])) p++;
                            if (ok && line[p] == ')') {
                                // successfully consumed _Pragma("...") — drop it
                                i = p + 1;
                                continue;
                            }
                        }
                        // If malformed, fall through and emit as normal text
                    }
                }
                Macro *m = mtable_find_n((MacroTable*)t, ident, ident_len);
                const Macro *fm = NULL;
                const char *rep = NULL;
                if (m) {
                    if (m->is_func) fm = m;
                    else rep = m->value;
                }
                size_t j2 = j; while (line[j2] && isspace((unsigned char)line[j2])) j2++;
                if (fm && line[j2] == '(') {
                    if (g_expand_pass > 0 && fm->self_ref) {
                        sb_putn(out, ident, ident_len);
                        i = j;
                        continue;
                    }
                    // parse arguments
                    size_t p = j2 + 1; int depthP = 1, depthB = 0, depthC = 0;
                    enum {AC_CODE, AC_DQ, AC_SQ, AC_SL, AC_BL} ast = AC_CODE;
                    int ac_bl_depth = 0;
                    Str cur; sb_init(&cur);
                    char **args_raw = NULL;
                    char **args_raw_full = NULL;
                    size_t argc=0, acc=0;
                    while (line[p] != '\0' && (depthP > 0 || depthB > 0 || depthC > 0)) {
                        char ch = line[p];
                        if (ast == AC_CODE) {
                            if (ch == '"') { ast = AC_DQ; sb_putc(&cur, ch); p++; continue; }
                            if (ch == '\'') { ast = AC_SQ; sb_putc(&cur, ch); p++; continue; }
                            if (ch == '/' && line[p+1]=='/') {
                                if (g_keep_comments) { ast = AC_SL; sb_puts(&cur, "//"); p+=2; continue; }
                                // strip till end of line
                                while (line[p] && line[p] != '\n' && line[p] != '\r') p++;
                                continue;
                            }
                            if (ch == '/' && line[p+1]=='*') {
                                ast = AC_BL;
                                ac_bl_depth = 1;
                                if (g_keep_comments) sb_puts(&cur, "/*");
                                p+=2;
                                continue;
                            }
                            if (ch == '(') { depthP++; sb_putc(&cur,ch); p++; continue; }
                            if (ch == ')') {
                                depthP--; if (depthP==0 && depthB==0 && depthC==0) { // finish last arg
                                    char *astr_full = xstrdup(cur.buf ? cur.buf : "");
                                    // trim cur
                                    while (cur.len>0 && isspace((unsigned char)cur.buf[cur.len-1])) cur.buf[--cur.len]='\0';
                                    size_t lead=0; while (lead<cur.len && isspace((unsigned char)cur.buf[lead])) lead++;
                                    char *astr = xstrdup(cur.buf?cur.buf+lead:"");
                                    if (argc==acc) {
                                        acc=acc?acc*2:4;
                                        args_raw = xrealloc(args_raw, acc*sizeof(char*));
                                        args_raw_full = xrealloc(args_raw_full, acc*sizeof(char*));
                                    }
                                    args_raw[argc] = astr;
                                    args_raw_full[argc] = astr_full;
                                    argc++;
                                    p++; break;
                                } else { sb_putc(&cur,ch); p++; continue; }
                            }
                            if (ch == '[') { depthB++; sb_putc(&cur,ch); p++; continue; }
                            if (ch == ']') { if (depthB>0) depthB--; sb_putc(&cur,ch); p++; continue; }
                            if (ch == '{') { depthC++; sb_putc(&cur,ch); p++; continue; }
                            if (ch == '}') { if (depthC>0) depthC--; sb_putc(&cur,ch); p++; continue; }
                            if (ch == ',' && depthP==1 && depthB==0 && depthC==0) {
                                // split arg
                                char *astr_full = xstrdup(cur.buf ? cur.buf : "");
                                while (cur.len>0 && isspace((unsigned char)cur.buf[cur.len-1])) cur.buf[--cur.len]='\0';
                                size_t lead=0; while (lead<cur.len && isspace((unsigned char)cur.buf[lead])) lead++;
                                char *astr = xstrdup(cur.buf?cur.buf+lead:"");
                                if (argc==acc) {
                                    acc=acc?acc*2:4;
                                    args_raw = xrealloc(args_raw, acc*sizeof(char*));
                                    args_raw_full = xrealloc(args_raw_full, acc*sizeof(char*));
                                }
                                args_raw[argc] = astr;
                                args_raw_full[argc] = astr_full;
                                argc++;
                                cur.len = 0; if (cur.buf) cur.buf[0]='\0';
                                p++; continue;
                            }
                            sb_putc(&cur, ch); p++;
                        } else if (ast == AC_DQ) {
                            sb_putc(&cur, ch);
                            if (ch == '"' && !is_escaped(line, p)) ast = AC_CODE;
                            p++;
                        } else if (ast == AC_SQ) {
                            sb_putc(&cur, ch);
                            if (ch == '\'' && !is_escaped(line, p)) ast = AC_CODE;
                            p++;
                        } else if (ast == AC_SL) {
                            if (g_keep_comments) sb_putc(&cur, ch); p++;
                        } else if (ast == AC_BL) {
                            if (ch == '/' && line[p+1] == '*') {
                                if (g_keep_comments) sb_puts(&cur, "/*");
                                ac_bl_depth++; p += 2; continue;
                            }
                            if (ch == '*' && line[p+1] == '/') {
                                if (g_keep_comments) sb_puts(&cur, "*/");
                                p += 2; ac_bl_depth--;
                                if (ac_bl_depth <= 0) { ac_bl_depth = 0; ast = AC_CODE; }
                                continue;
                            }
                            if (g_keep_comments) sb_putc(&cur, ch);
                            p++;
                        }
                    }
                    // prescan/expand args (object-like & nested function-like calls on that text)
                    char **args = NULL; size_t acc2 = argc ? argc : 1; args = xrealloc(NULL, acc2*sizeof(char*));
                    for (size_t ai=0; ai<argc; ++ai) {
                        Str ex; sb_init(&ex);
                        long saved_line = g_cur_line;
                        int saved_pass = g_expand_pass;
                        int saved_bl = g_block_comment_depth;
                        g_cur_line = saved_line;
                        g_expand_pass = saved_pass;
                        expand_into_str_mode(t, args_raw[ai], &ex, protect_defined);
                        int arg_bl_depth = g_block_comment_depth;
                        // Rescan to fully expand nested macros inside arguments.
                        for (int pass = 0; pass < PP_RESCAN_MAX; ++pass) {
                            Str next; sb_init(&next);
                            g_cur_line = saved_line;
                            g_expand_pass = saved_pass + pass + 1;
                            g_block_comment_depth = saved_bl;
                            expand_into_str_mode(t, ex.buf ? ex.buf : "", &next, protect_defined);
                            g_block_comment_depth = arg_bl_depth;
                            bool same = ((ex.buf == NULL && next.buf == NULL) ||
                                         (ex.buf && next.buf && strcmp(ex.buf, next.buf) == 0));
                            sb_free(&ex);
                            ex = next;
                            if (same) break;
                        }
                        g_cur_line = saved_line;
                        g_expand_pass = saved_pass;
                        g_block_comment_depth = arg_bl_depth;
                        args[ai] = xstrdup(ex.buf ? ex.buf : "");
                        sb_free(&ex);
                    }
                    // prepare joined varargs strings
                    size_t var_start = fm->nparams;
                    Str join_raw; sb_init(&join_raw);
                    Str join_exp; sb_init(&join_exp);
                    if (fm->is_variadic && argc > var_start) {
                        for (size_t vi=var_start; vi<argc; ++vi) {
                            if (vi>var_start) { sb_puts(&join_raw, ","); sb_puts(&join_exp, ","); }
                            sb_puts(&join_raw, args_raw_full[vi] ? args_raw_full[vi] : "");
                            sb_puts(&join_exp, args[vi] ? args[vi] : "");
                        }
                    }
                    const char *joined_raw = join_raw.buf ? join_raw.buf : "";
                    const char *joined_exp = join_exp.buf ? join_exp.buf : "";
                    // substitute params into function body with support for # and ##
                    // substitute params into function body
                    const char *body = fm->value ? fm->value : "";
                    enum {RB_CODE, RB_DQ, RB_SQ, RB_SL, RB_BL} rb = RB_CODE;
                    int rb_bl_depth = 0;
                    // a small buffer to hold token we are about to paste
                    for (size_t bi=0; body[bi] != '\0'; ) {
                        char bc = body[bi];
                        if (rb == RB_CODE) {
                            if (bc=='"') { rb=RB_DQ; sb_putc(out, bc); bi++; continue; }
                            if (bc=='\'') { rb=RB_SQ; sb_putc(out, bc); bi++; continue; }
                            if (bc=='/' && body[bi+1]=='/') { rb=RB_SL; sb_puts(out, "//"); bi+=2; continue; }
                            if (bc=='/' && body[bi+1]=='*') { rb=RB_BL; rb_bl_depth = 1; sb_puts(out, "/*"); bi+=2; continue; }
                            if (bc=='#') {
                                if (body[bi+1]=='#') {
                                    // token pasting: remove trailing spaces from out, then append next token (param or ident) without spaces
                                    // skip '##'
                                    bi += 2; // eat ##
                                    // skip spaces
                                    while (isspace((unsigned char)body[bi])) bi++;
                                    // capture next token
                                    if (isalpha((unsigned char)body[bi]) || body[bi]=='_') {
                                        char tok[256]; size_t tk=0; size_t bj=bi;
                                        while (body[bj] && (isalnum((unsigned char)body[bj]) || body[bj]=='_')) { if (tk+1<sizeof tok) tok[tk++]=body[bj]; bj++; }
                                        tok[tk]='\0';
                                        // trim trailing spaces in out
                                        while (out->len>0 && isspace((unsigned char)out->buf[out->len-1])) out->buf[--out->len]='\0';
                                        // see if tok is parameter
                                        size_t pi_idx = (size_t)-1;
                                        for (size_t pi=0; pi<fm->nparams; ++pi) { if (strcmp(fm->params[pi], tok)==0) { pi_idx = pi; break; } }
                                        const char *text = NULL;
                                        if (macro_is_vararg_token(fm, tok)) {
                                            text = joined_raw;
                                        } else if (pi_idx != (size_t)-1 && pi_idx < argc) {
                                            text = args_raw[pi_idx];
                                        } else {
                                            text = tok;
                                        }
                                        // if empty, remove trailing comma
                                        if ((!text || *text=='\0')) {
                                            while (out->len>0 && isspace((unsigned char)out->buf[out->len-1])) out->buf[--out->len]='\0';
                                            if (out->len>0 && out->buf[out->len-1]==',') out->buf[--out->len]='\0';
                                        }
                                        sb_puts(out, text ? text : "");
                                        bi = bj;
                                        continue;
                                    } else {
                                        // paste with single char; if at end, just stop
                                        while (out->len>0 && isspace((unsigned char)out->buf[out->len-1])) out->buf[--out->len]='\0';
                                        if (body[bi] == '\0') {
                                            break; // avoid advancing past terminator
                                        }
                                        sb_putc(out, body[bi]);
                                        bi++;
                                        continue;
                                    }
                                } else {
                                    // stringize: # param
                                    bi++; while (isspace((unsigned char)body[bi])) bi++;
                                    if (isalpha((unsigned char)body[bi]) || body[bi]=='_') {
                                        char tok[256]; size_t tk=0; size_t bj=bi;
                                        while (body[bj] && (isalnum((unsigned char)body[bj]) || body[bj]=='_')) { if (tk+1<sizeof tok) tok[tk++]=body[bj]; bj++; }
                                        tok[tk]='\0';
                                        size_t pi_idx = (size_t)-1;
                                        for (size_t pi=0; pi<fm->nparams; ++pi) { if (strcmp(fm->params[pi], tok)==0) { pi_idx = pi; break; } }
                                        if (macro_is_vararg_token(fm, tok)) {
                                            // stringize joined raw varargs
                                            sb_put_stringized_argument(out, joined_raw);
                                            bi = bj; continue;
                                        } else if (pi_idx != (size_t)-1 && pi_idx < argc) {
                                            // stringize raw args
                                            sb_put_stringized_argument(out, args_raw_full[pi_idx]);
                                            bi = bj; continue;
                                        }
                                        // not a param; output as literal
                                        sb_putc(out, '#');
                                        sb_putn(out, body + bi, bj - bi);
                                        bi = bj; continue;
                                    } else {
                                        // stringize fallback: if nothing follows '#', emit '#' and stop
                                        sb_putc(out, '#');
                                        if (body[bi] == '\0') { break; }
                                        sb_putc(out, body[bi]);
                                        bi++;
                                        continue;
                                    }
                                }
                            }
                            if (isalpha((unsigned char)bc) || bc=='_') {
                                char idb[256]; size_t kk=0; size_t bj=bi;
                                while (body[bj] && (isalnum((unsigned char)body[bj]) || body[bj]=='_')) { if (kk+1<sizeof idb) idb[kk++]=body[bj]; bj++; }
                                idb[kk]='\0';
                                // If this token is immediately followed by ## (ignoring spaces),
                                // it participates in token pasting and must use raw macro args.
                                size_t look = bj;
                                while (body[look] && isspace((unsigned char)body[look])) look++;
                                bool paste_right = (body[look] == '#' && body[look + 1] == '#');
                                // Handle _Pragma("...") inside macro bodies: consume and drop
                                if (strcmp(idb, "_Pragma") == 0) {
                                    size_t j2 = bj; while (body[j2] && isspace((unsigned char)body[j2])) j2++;
                                    if (body[j2] == '(') {
                                        size_t p2 = j2 + 1; while (body[p2] && isspace((unsigned char)body[p2])) p2++;
                                        bool okp = false;
                                        if (body[p2] == '"') {
                                            p2++;
                                            while (body[p2] != '\0') {
                                                if (body[p2] == '"' && body[p2-1] != '\\') { p2++; okp = true; break; }
                                                p2++;
                                            }
                                            while (body[p2] && isspace((unsigned char)body[p2])) p2++;
                                            if (okp && body[p2] == ')') { bi = p2 + 1; continue; }
                                        }
                                    }
                                    // fall through to emit normally if malformed
                                }
                                // match parameter
                                size_t pi_idx = (size_t)-1;
                                for (size_t pi=0; pi<fm->nparams; ++pi) { if (strcmp(fm->params[pi], idb)==0) { pi_idx = pi; break; } }
                                if (macro_is_vararg_token(fm, idb)) {
                                    sb_puts(out, paste_right ? joined_raw : joined_exp);
                                } else if (pi_idx != (size_t)-1 && pi_idx < argc) {
                                    sb_puts(out, paste_right ? args_raw[pi_idx] : args[pi_idx]);
                                } else {
                                    sb_putn(out, body + bi, bj - bi);
                                }
                                bi = bj; continue;
                            }
                            sb_putc(out, bc); bi++;
                        } else if (rb == RB_DQ) { sb_putc(out, bc); if (bc=='"' && body[bi-1] != '\\') rb=RB_CODE; bi++; }
                        else if (rb == RB_SQ) { sb_putc(out, bc); if (bc=='\'' && body[bi-1] != '\\') rb=RB_CODE; bi++; }
                        else if (rb == RB_SL) { sb_putc(out, bc); bi++; }
                        else if (rb == RB_BL) {
                            if (bc=='/' && body[bi+1]=='*') { rb_bl_depth++; sb_puts(out, "/*"); bi+=2; continue; }
                            if (bc=='*' && body[bi+1]=='/') {
                                rb_bl_depth--; sb_puts(out, "*/"); bi+=2;
                                if (rb_bl_depth <= 0) { rb_bl_depth = 0; rb = RB_CODE; }
                                continue;
                            }
                            sb_putc(out, bc); bi++;
                        }
                    }
                    // cleanup args
                    for (size_t ai=0; ai<argc; ++ai) { free(args[ai]); free(args_raw[ai]); free(args_raw_full[ai]); }
                    free(args); free(args_raw); free(args_raw_full);
                    sb_free(&join_raw); sb_free(&join_exp);
                    i = p; // advance past arguments
                    continue;
                } else if (rep) {
                    if (g_expand_pass > 0 && m && m->self_ref) {
                        sb_putn(out, ident, ident_len);
                        i = j;
                    } else {
                        sb_puts(out, rep);
                        i = j;
                    }
                } else {
                    // not a macro; copy original slice
                    sb_putn(out, ident, ident_len);
                    i = j;
                }
                continue;
            }
            sb_putc(out, c); i++;
        } else if (state == STR_DQ) {
            sb_putc(out, c);
            if (c == '"' && !is_escaped(line, i)) state = CODE;
            i++;
        } else if (state == STR_SQ) {
            sb_putc(out, c);
            if (c == '\'' && !is_escaped(line, i)) state = CODE;
            i++;
        } else if (state == SL_COMMENT) {
            // copy until end of line (or drop if stripping)
            if (g_keep_comments) sb_putc(out, c);
            i++;
        } else if (state == BL_COMMENT) {
            if (c == '/' && line[i+1] == '*') {
                if (hidden_bl_depth > 0) hidden_bl_depth++;
                if (g_keep_comments && hidden_bl_depth == 0) sb_puts(out, "/*");
                bl_depth++;
                i += 2;
                continue;
            }
            if (c == '*' && line[i+1] == '/') {
                if (g_keep_comments && hidden_bl_depth == 0) sb_puts(out, "*/");
                i += 2;
                bl_depth--;
                if (hidden_bl_depth > 0) hidden_bl_depth--;
                if (bl_depth <= 0) { state = CODE; }
                continue;
            }
            if (g_keep_comments && hidden_bl_depth == 0) sb_putc(out, c);
            i++;
        }
    }
    // persist block comment depth across lines
    g_block_comment_depth = bl_depth;
    g_hidden_block_comment_depth = hidden_bl_depth;
}

#undef IDENT_EQ

static void sb_put_stringized_argument(Str *out, const char *raw) {
    const char *s = raw ? raw : "";
    enum { SG_CODE, SG_DQ, SG_SQ, SG_SL, SG_BL } st = SG_CODE;
    bool pending_space = false;
    bool emitted = false;
    int bl_depth = 0;

    sb_putc(out, '"');
    for (size_t i = 0; s[i] != '\0';) {
        unsigned char ch = (unsigned char)s[i];
        if (st == SG_CODE) {
            if (s[i] == '/' && s[i+1] == '/') {
                if (emitted) pending_space = true;
                st = SG_SL;
                i += 2;
                continue;
            }
            if (s[i] == '/' && s[i+1] == '*') {
                if (emitted) pending_space = true;
                st = SG_BL;
                bl_depth = 1;
                i += 2;
                continue;
            }
            if (isspace(ch)) {
                if (emitted) pending_space = true;
                i++;
                continue;
            }
            if (pending_space) {
                sb_putc(out, ' ');
                pending_space = false;
                emitted = true;
            }
            if (s[i] == '\\' || s[i] == '"') sb_putc(out, '\\');
            sb_putc(out, s[i]);
            emitted = true;
            if (s[i] == '"') st = SG_DQ;
            else if (s[i] == '\'') st = SG_SQ;
            i++;
            continue;
        }
        if (st == SG_DQ) {
            if (s[i] == '\\' || s[i] == '"') sb_putc(out, '\\');
            sb_putc(out, s[i]);
            emitted = true;
            if (s[i] == '"' && !is_escaped(s, i)) st = SG_CODE;
            i++;
            continue;
        }
        if (st == SG_SQ) {
            if (s[i] == '\\' || s[i] == '"') sb_putc(out, '\\');
            sb_putc(out, s[i]);
            emitted = true;
            if (s[i] == '\'' && !is_escaped(s, i)) st = SG_CODE;
            i++;
            continue;
        }
        if (st == SG_SL) {
            if (s[i] == '\n' || s[i] == '\r') st = SG_CODE;
            i++;
            continue;
        }
        if (st == SG_BL) {
            if (s[i] == '/' && s[i+1] == '*') {
                bl_depth++;
                i += 2;
                continue;
            }
            if (s[i] == '*' && s[i+1] == '/') {
                bl_depth--;
                i += 2;
                if (bl_depth <= 0) {
                    bl_depth = 0;
                    st = SG_CODE;
                }
                continue;
            }
            i++;
            continue;
        }
    }
    sb_putc(out, '"');
}

static void expand_into_str(const MacroTable *t, const char *line, Str *out) {
    expand_into_str_mode(t, line, out, false);
}

static void expand_into_str_ifexpr(const MacroTable *t, const char *line, Str *out) {
    expand_into_str_mode(t, line, out, true);
}

static void replace_ident_token(Str *line, const char *token, const char *repl) {
    if (!line || !line->buf || !token || !*token) return;
    enum { S_CODE, S_DQ, S_SQ, S_SL, S_BL } st = S_CODE;
    int bl_depth = 0;
    Str out; sb_init(&out);
    const char *s = line->buf;
    for (size_t i = 0; s[i]; ) {
        char c = s[i];
        if (st == S_CODE) {
            if (c == '"') { st = S_DQ; sb_putc(&out, c); i++; continue; }
            if (c == '\'') { st = S_SQ; sb_putc(&out, c); i++; continue; }
            if (c == '/' && s[i+1] == '/') { st = S_SL; sb_puts(&out, "//"); i += 2; continue; }
            if (c == '/' && s[i+1] == '*') { st = S_BL; bl_depth = 1; sb_puts(&out, "/*"); i += 2; continue; }
            if (isalpha((unsigned char)c) || c == '_') {
                char id[256]; size_t j = 0; size_t k = i;
                while (s[k] && (isalnum((unsigned char)s[k]) || s[k] == '_')) {
                    if (j + 1 < sizeof id) id[j++] = s[k];
                    k++;
                }
                id[j] = '\0';
                if (strcmp(id, token) == 0) {
                    if (repl && *repl) sb_puts(&out, repl);
                    i = k;
                    continue;
                }
                while (i < k) { sb_putc(&out, s[i++]); }
                continue;
            }
            sb_putc(&out, c); i++;
        } else if (st == S_DQ) {
            sb_putc(&out, c);
            if (c == '"' && s[i-1] != '\\') st = S_CODE;
            i++;
        } else if (st == S_SQ) {
            sb_putc(&out, c);
            if (c == '\'' && s[i-1] != '\\') st = S_CODE;
            i++;
        } else if (st == S_SL) {
            sb_putc(&out, c); i++;
        } else if (st == S_BL) {
            if (c == '/' && s[i+1] == '*') { bl_depth++; sb_puts(&out, "/*"); i += 2; continue; }
            if (c == '*' && s[i+1] == '/') {
                bl_depth--; sb_puts(&out, "*/"); i += 2;
                if (bl_depth <= 0) { bl_depth = 0; st = S_CODE; }
                continue;
            }
            sb_putc(&out, c); i++;
        }
    }
    sb_free(line);
    *line = out;
}

static void rewrite_complex_postfix(Str *line) {
    if (!line || !line->buf) return;
    enum { S_CODE, S_DQ, S_SQ, S_SL, S_BL } st = S_CODE;
    int bl_depth = 0;
    Str out; sb_init(&out);
    const char *s = line->buf;
    for (size_t i = 0; s[i]; ) {
        char c = s[i];
        if (st == S_CODE) {
            if (c == '"') { st = S_DQ; sb_putc(&out, c); i++; continue; }
            if (c == '\'') { st = S_SQ; sb_putc(&out, c); i++; continue; }
            if (c == '/' && s[i+1] == '/') { st = S_SL; sb_puts(&out, "//"); i += 2; continue; }
            if (c == '/' && s[i+1] == '*') { st = S_BL; bl_depth = 1; sb_puts(&out, "/*"); i += 2; continue; }
            if (isalpha((unsigned char)c) || c == '_') {
                char id[64]; size_t j = 0; size_t k = i;
                while (s[k] && (isalnum((unsigned char)s[k]) || s[k] == '_')) {
                    if (j + 1 < sizeof id) id[j++] = s[k];
                    k++;
                }
                id[j] = '\0';
                if (strcmp(id, "long") == 0) {
                    size_t p = k;
                    while (s[p] && isspace((unsigned char)s[p])) p++;
                    if (isalpha((unsigned char)s[p]) || s[p] == '_') {
                        char id2[64]; size_t j2 = 0; size_t k2 = p;
                        while (s[k2] && (isalnum((unsigned char)s[k2]) || s[k2] == '_')) {
                            if (j2 + 1 < sizeof id2) id2[j2++] = s[k2];
                            k2++;
                        }
                        id2[j2] = '\0';
                        if (strcmp(id2, "double") == 0) {
                            size_t p2 = k2;
                            while (s[p2] && isspace((unsigned char)s[p2])) p2++;
                            if (isalpha((unsigned char)s[p2]) || s[p2] == '_') {
                                char id3[64]; size_t j3 = 0; size_t k3 = p2;
                                while (s[k3] && (isalnum((unsigned char)s[k3]) || s[k3] == '_')) {
                                    if (j3 + 1 < sizeof id3) id3[j3++] = s[k3];
                                    k3++;
                                }
                                id3[j3] = '\0';
                                if (strcmp(id3, "_Complex") == 0) {
                                    sb_puts(&out, "_Complex long double");
                                    i = k3;
                                    continue;
                                }
                            }
                        }
                    }
                } else if (strcmp(id, "double") == 0 || strcmp(id, "float") == 0) {
                    size_t p = k;
                    while (s[p] && isspace((unsigned char)s[p])) p++;
                    if (isalpha((unsigned char)s[p]) || s[p] == '_') {
                        char id2[64]; size_t j2 = 0; size_t k2 = p;
                        while (s[k2] && (isalnum((unsigned char)s[k2]) || s[k2] == '_')) {
                            if (j2 + 1 < sizeof id2) id2[j2++] = s[k2];
                            k2++;
                        }
                        id2[j2] = '\0';
                        if (strcmp(id2, "_Complex") == 0) {
                            sb_puts(&out, "_Complex ");
                            sb_puts(&out, id);
                            i = k2;
                            continue;
                        }
                    }
                }
                while (i < k) { sb_putc(&out, s[i++]); }
                continue;
            }
            sb_putc(&out, c); i++;
        } else if (st == S_DQ) {
            sb_putc(&out, c);
            if (c == '"' && s[i-1] != '\\') st = S_CODE;
            i++;
        } else if (st == S_SQ) {
            sb_putc(&out, c);
            if (c == '\'' && s[i-1] != '\\') st = S_CODE;
            i++;
        } else if (st == S_SL) {
            sb_putc(&out, c); i++;
        } else if (st == S_BL) {
            if (c == '/' && s[i+1] == '*') { bl_depth++; sb_puts(&out, "/*"); i += 2; continue; }
            if (c == '*' && s[i+1] == '/') {
                bl_depth--; sb_puts(&out, "*/"); i += 2;
                if (bl_depth <= 0) { bl_depth = 0; st = S_CODE; }
                continue;
            }
            sb_putc(&out, c); i++;
        }
    }
    sb_free(line);
    *line = out;
}

static void rewrite_empty_array_dim(Str *line) {
    if (!line || !line->buf) return;
    enum { S_CODE, S_DQ, S_SQ, S_SL, S_BL } st = S_CODE;
    int bl_depth = 0;
    Str out; sb_init(&out);
    const char *s = line->buf;
    for (size_t i = 0; s[i]; ) {
        char c = s[i];
        if (st == S_CODE) {
            if (c == '"') { st = S_DQ; sb_putc(&out, c); i++; continue; }
            if (c == '\'') { st = S_SQ; sb_putc(&out, c); i++; continue; }
            if (c == '/' && s[i+1] == '/') { st = S_SL; sb_puts(&out, "//"); i += 2; continue; }
            if (c == '/' && s[i+1] == '*') { st = S_BL; bl_depth = 1; sb_puts(&out, "/*"); i += 2; continue; }
            if (c == '[' && s[i+1] == ']' && s[i+2] == '[') {
                sb_puts(&out, "[1][");
                i += 3;
                continue;
            }
            sb_putc(&out, c); i++;
        } else if (st == S_DQ) {
            sb_putc(&out, c);
            if (c == '"' && s[i-1] != '\\') st = S_CODE;
            i++;
        } else if (st == S_SQ) {
            sb_putc(&out, c);
            if (c == '\'' && s[i-1] != '\\') st = S_CODE;
            i++;
        } else if (st == S_SL) {
            sb_putc(&out, c); i++;
        } else if (st == S_BL) {
            if (c == '/' && s[i+1] == '*') { bl_depth++; sb_puts(&out, "/*"); i += 2; continue; }
            if (c == '*' && s[i+1] == '/') {
                bl_depth--; sb_puts(&out, "*/"); i += 2;
                if (bl_depth <= 0) { bl_depth = 0; st = S_CODE; }
                continue;
            }
            sb_putc(&out, c); i++;
        }
    }
    sb_free(line);
    *line = out;
}

static void strip_struct_tag(Str *line, const char *tag) {
    if (!line || !line->buf || !tag || !*tag) return;
    enum { S_CODE, S_DQ, S_SQ, S_SL, S_BL } st = S_CODE;
    int bl_depth = 0;
    Str out; sb_init(&out);
    const char *s = line->buf;
    for (size_t i = 0; s[i]; ) {
        char c = s[i];
        if (st == S_CODE) {
            if (c == '"') { st = S_DQ; sb_putc(&out, c); i++; continue; }
            if (c == '\'') { st = S_SQ; sb_putc(&out, c); i++; continue; }
            if (c == '/' && s[i+1] == '/') { st = S_SL; sb_puts(&out, "//"); i += 2; continue; }
            if (c == '/' && s[i+1] == '*') { st = S_BL; bl_depth = 1; sb_puts(&out, "/*"); i += 2; continue; }
            if (isalpha((unsigned char)c) || c == '_') {
                char id[256]; size_t j = 0; size_t k = i;
                while (s[k] && (isalnum((unsigned char)s[k]) || s[k] == '_')) {
                    if (j + 1 < sizeof id) id[j++] = s[k];
                    k++;
                }
                id[j] = '\0';
                if (strcmp(id, "struct") == 0) {
                    size_t p = k;
                    size_t ws_start = p;
                    while (s[p] && isspace((unsigned char)s[p])) p++;
                    if (isalpha((unsigned char)s[p]) || s[p] == '_') {
                        char id2[256]; size_t j2 = 0; size_t k2 = p;
                        while (s[k2] && (isalnum((unsigned char)s[k2]) || s[k2] == '_')) {
                            if (j2 + 1 < sizeof id2) id2[j2++] = s[k2];
                            k2++;
                        }
                        id2[j2] = '\0';
                        if (strcmp(id2, tag) == 0) {
                            sb_puts(&out, "struct");
                            if (p > ws_start) sb_putc(&out, ' ');
                            i = k2;
                            continue;
                        }
                    }
                }
                while (i < k) { sb_putc(&out, s[i++]); }
                continue;
            }
            sb_putc(&out, c); i++;
        } else if (st == S_DQ) {
            sb_putc(&out, c);
            if (c == '"' && s[i-1] != '\\') st = S_CODE;
            i++;
        } else if (st == S_SQ) {
            sb_putc(&out, c);
            if (c == '\'' && s[i-1] != '\\') st = S_CODE;
            i++;
        } else if (st == S_SL) {
            sb_putc(&out, c); i++;
        } else if (st == S_BL) {
            if (c == '/' && s[i+1] == '*') { bl_depth++; sb_puts(&out, "/*"); i += 2; continue; }
            if (c == '*' && s[i+1] == '/') {
                bl_depth--; sb_puts(&out, "*/"); i += 2;
                if (bl_depth <= 0) { bl_depth = 0; st = S_CODE; }
                continue;
            }
            sb_putc(&out, c); i++;
        }
    }
    sb_free(line);
    *line = out;
}

static void strip_extension_tokens(Str *line) {
    if (!line || !line->buf) return;
    enum { S_CODE, S_DQ, S_SQ, S_SL, S_BL } st = S_CODE;
    int bl_depth = 0;
    Str out; sb_init(&out);
    const char *s = line->buf;
    for (size_t i = 0; s[i]; ) {
        char c = s[i];
        if (st == S_CODE) {
            if (c == '"') { st = S_DQ; sb_putc(&out, c); i++; continue; }
            if (c == '\'') { st = S_SQ; sb_putc(&out, c); i++; continue; }
            if (c == '/' && s[i+1] == '/') { st = S_SL; sb_puts(&out, "//"); i += 2; continue; }
            if (c == '/' && s[i+1] == '*') { st = S_BL; bl_depth = 1; sb_puts(&out, "/*"); i += 2; continue; }
            if (isalpha((unsigned char)c) || c == '_') {
                char id[256]; size_t j = 0; size_t k = i;
                while (s[k] && (isalnum((unsigned char)s[k]) || s[k] == '_')) {
                    if (j + 1 < sizeof id) id[j++] = s[k];
                    k++;
                }
                id[j] = '\0';
                if (strcmp(id, "__extension__") == 0) {
                    size_t next = k;
                    while (s[next] && isspace((unsigned char)s[next])) next++;
                    if (out.len > 0 && !isspace((unsigned char)out.buf[out.len - 1]) &&
                        (isalnum((unsigned char)s[next]) || s[next] == '_')) {
                        sb_putc(&out, ' ');
                    }
                    i = next;
                    continue;
                }
                while (i < k) { sb_putc(&out, s[i++]); }
                continue;
            }
            sb_putc(&out, c); i++;
        } else if (st == S_DQ) {
            sb_putc(&out, c);
            if (c == '"' && s[i-1] != '\\') st = S_CODE;
            i++;
        } else if (st == S_SQ) {
            sb_putc(&out, c);
            if (c == '\'' && s[i-1] != '\\') st = S_CODE;
            i++;
        } else if (st == S_SL) {
            sb_putc(&out, c); i++;
        } else if (st == S_BL) {
            if (c == '/' && s[i+1] == '*') { bl_depth++; sb_puts(&out, "/*"); i += 2; continue; }
            if (c == '*' && s[i+1] == '/') {
                bl_depth--; sb_puts(&out, "*/"); i += 2;
                if (bl_depth <= 0) { bl_depth = 0; st = S_CODE; }
                continue;
            }
            sb_putc(&out, c); i++;
        }
    }
    sb_free(line);
    *line = out;
}

static void rewrite_anonymous_typedef_tag(Str *line) {
    if (!line || !line->buf) return;
    const char *s = line->buf;
    const char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (!starts_with(p, "typedef")) return;
    p += strlen("typedef");
    if (isalnum((unsigned char)*p) || *p == '_') return;
    while (*p && isspace((unsigned char)*p)) p++;
    const char *kind_start = p;
    size_t kind_len = 0;
    if (starts_with(p, "struct")) { kind_len = strlen("struct"); }
    else if (starts_with(p, "union")) { kind_len = strlen("union"); }
    else if (starts_with(p, "enum")) { kind_len = strlen("enum"); }
    else { return; }
    p += kind_len;
    if (isalnum((unsigned char)*p) || *p == '_') return;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p != '{') return; // already has a tag or not anonymous
    const char *brace_start = p;
    enum { S_CODE, S_DQ, S_SQ, S_SL, S_BL } st = S_CODE;
    int depth = 0;
    int bl_depth = 0;
    const char *q = p;
    for (; *q; ++q) {
        char c = *q;
        if (st == S_CODE) {
            if (c == '"') { st = S_DQ; continue; }
            if (c == '\'') { st = S_SQ; continue; }
            if (c == '/' && q[1] == '/') { st = S_SL; q++; continue; }
            if (c == '/' && q[1] == '*') { st = S_BL; bl_depth = 1; q++; continue; }
            if (c == '{') { depth++; continue; }
            if (c == '}') {
                depth--;
                if (depth == 0) { q++; break; }
            }
        } else if (st == S_DQ) {
            if (c == '"' && q[-1] != '\\') st = S_CODE;
        } else if (st == S_SQ) {
            if (c == '\'' && q[-1] != '\\') st = S_CODE;
        } else if (st == S_SL) {
            if (c == '\n' || c == '\r') st = S_CODE;
        } else if (st == S_BL) {
            if (c == '/' && q[1] == '*') { bl_depth++; q++; continue; }
            if (c == '*' && q[1] == '/') {
                bl_depth--; q++;
                if (bl_depth <= 0) { bl_depth = 0; st = S_CODE; }
                continue;
            }
        }
    }
    if (depth != 0) return;
    const char *after = q;
    while (*after && isspace((unsigned char)*after)) after++;
    if (!(isalpha((unsigned char)*after) || *after == '_')) return;
    char name[256]; size_t ni = 0;
    while (*after && (isalnum((unsigned char)*after) || *after == '_')) {
        if (ni + 1 < sizeof name) name[ni++] = *after;
        after++;
    }
    name[ni] = '\0';
    while (*after && isspace((unsigned char)*after)) after++;
    if (*after != ';') return;
    const char *prefix = s;
    const char *kind_end = kind_start + kind_len;
    Str out; sb_init(&out);
    for (const char *x = prefix; x < kind_end; ++x) sb_putc(&out, *x);
    sb_putc(&out, ' ');
    sb_puts(&out, name);
    sb_putc(&out, ' ');
    sb_puts(&out, brace_start);
    sb_free(line);
    *line = out;
}

static char *lskip(char *p) {
    while (*p && isspace((unsigned char)*p)) p++;
    return p;
}

static const char *clskip(const char *p) {
    while (*p && isspace((unsigned char)*p)) p++;
    return p;
}

static bool parse_ident(const char *p, size_t *adv, char *buf, size_t bufsz) {
    size_t i = 0;
    if (!(isalpha((unsigned char)p[0]) || p[0] == '_')) return false;
    size_t j = 0;
    while (p[i] && (isalnum((unsigned char)p[i]) || p[i] == '_')) {
        if (j + 1 < bufsz) buf[j++] = p[i];
        i++;
    }
    buf[j] = '\0';
    if (adv) *adv = i;
    return true;
}

typedef enum {
    DEF_TOK_HASH,
    DEF_TOK_HASHHASH,
    DEF_TOK_IDENT,
    DEF_TOK_OTHER
} DefineTokKind;

typedef struct {
    DefineTokKind kind;
    char ident[256];
} DefineTok;

static void define_tok_push(DefineTok **items, size_t *len, size_t *cap,
                            DefineTokKind kind, const char *ident) {
    if (*len == *cap) {
        size_t ncap = *cap ? (*cap * 2) : 16;
        *items = (DefineTok*)xrealloc(*items, ncap * sizeof(DefineTok));
        *cap = ncap;
    }
    (*items)[*len].kind = kind;
    (*items)[*len].ident[0] = '\0';
    if (ident) {
        strncpy((*items)[*len].ident, ident, sizeof((*items)[*len].ident) - 1);
        (*items)[*len].ident[sizeof((*items)[*len].ident) - 1] = '\0';
    }
    (*len)++;
}

static bool define_param_name_match(const char *ident, char **params, size_t nparams,
                                    bool variadic, const char *var_param) {
    if (!ident || !*ident) return false;
    for (size_t i = 0; i < nparams; ++i) {
        if (params[i] && strcmp(params[i], ident) == 0) return true;
    }
    if (variadic) {
        if (var_param && strcmp(var_param, ident) == 0) return true;
        if (strcmp(ident, "__VA_ARGS__") == 0) return true;
    }
    return false;
}

static bool parse_define_signature(const char *p, bool *is_func,
                                   char ***params, size_t *nparams,
                                   bool *variadic, char **var_param,
                                   const char **repl_start) {
    if (is_func) *is_func = false;
    if (params) *params = NULL;
    if (nparams) *nparams = 0;
    if (variadic) *variadic = false;
    if (var_param) *var_param = NULL;
    if (repl_start) *repl_start = p;

    const char *q = clskip(p ? p : "");
    size_t adv = 0;
    char namebuf[256];
    if (!parse_ident(q, &adv, namebuf, sizeof(namebuf))) {
        return false;
    }
    q += adv;

    if (*q == '(') {
        if (is_func) *is_func = true;
        q++;
        char **pp = NULL;
        size_t np = 0;
        size_t pc = 0;
        bool var = false;
        char *vname = NULL;
        while (1) {
            while (*q && isspace((unsigned char)*q)) q++;
            if (*q == ')') {
                q++;
                break;
            }
            if (q[0] == '.' && q[1] == '.' && q[2] == '.') {
                var = true;
                q += 3;
                while (*q && isspace((unsigned char)*q)) q++;
                if (*q == ')') q++;
                break;
            }
            char id[256];
            size_t j = 0;
            if (!(isalpha((unsigned char)*q) || *q == '_')) {
                if (pp) { for (size_t i = 0; i < np; ++i) free(pp[i]); free(pp); }
                if (vname) free(vname);
                return false;
            }
            while (*q && (isalnum((unsigned char)*q) || *q == '_')) {
                if (j + 1 < sizeof(id)) id[j++] = *q;
                q++;
            }
            id[j] = '\0';

            const char *q2 = q;
            while (*q2 && isspace((unsigned char)*q2)) q2++;
            if (q2[0] == '.' && q2[1] == '.' && q2[2] == '.') {
                var = true;
                if (vname) free(vname);
                vname = xstrdup(id);
                q = q2 + 3;
                while (*q && isspace((unsigned char)*q)) q++;
                if (*q == ')') q++;
                break;
            }

            if (np == pc) {
                pc = pc ? pc * 2 : 4;
                pp = (char**)xrealloc(pp, pc * sizeof(char*));
            }
            pp[np++] = xstrdup(id);
            q = q2;
            if (*q == ',') {
                q++;
                continue;
            }
            if (*q == ')') {
                q++;
                break;
            }
            if (pp) { for (size_t i = 0; i < np; ++i) free(pp[i]); free(pp); }
            if (vname) free(vname);
            return false;
        }

        if (params) *params = pp;
        else if (pp) { for (size_t i = 0; i < np; ++i) free(pp[i]); free(pp); }
        if (nparams) *nparams = np;
        if (variadic) *variadic = var;
        if (var_param) *var_param = vname;
        else if (vname) free(vname);
    }

    while (*q && isspace((unsigned char)*q)) q++;
    if (repl_start) *repl_start = q;
    return true;
}

static bool validate_define_replacement_list(const char *repl, bool is_func,
                                             char **params, size_t nparams,
                                             bool variadic, const char *var_param) {
    const char *s = repl ? repl : "";
    DefineTok *toks = NULL;
    size_t ntoks = 0;
    size_t cap = 0;

    for (size_t i = 0; s[i]; ) {
        char c = s[i];
        if (isspace((unsigned char)c)) {
            i++;
            continue;
        }
        if (c == '/' && s[i + 1] == '/') {
            break;
        }
        if (c == '/' && s[i + 1] == '*') {
            i += 2;
            while (s[i] && !(s[i] == '*' && s[i + 1] == '/')) i++;
            if (s[i]) i += 2;
            continue;
        }
        if (c == '"') {
            i++;
            while (s[i]) {
                if (s[i] == '\\' && s[i + 1]) { i += 2; continue; }
                if (s[i] == '"') { i++; break; }
                i++;
            }
            define_tok_push(&toks, &ntoks, &cap, DEF_TOK_OTHER, NULL);
            continue;
        }
        if (c == '\'') {
            i++;
            while (s[i]) {
                if (s[i] == '\\' && s[i + 1]) { i += 2; continue; }
                if (s[i] == '\'') { i++; break; }
                i++;
            }
            define_tok_push(&toks, &ntoks, &cap, DEF_TOK_OTHER, NULL);
            continue;
        }
        if (c == '#') {
            if (s[i + 1] == '#') {
                define_tok_push(&toks, &ntoks, &cap, DEF_TOK_HASHHASH, NULL);
                i += 2;
            } else {
                define_tok_push(&toks, &ntoks, &cap, DEF_TOK_HASH, NULL);
                i++;
            }
            continue;
        }
        if (isalpha((unsigned char)c) || c == '_') {
            char id[256];
            size_t j = 0;
            size_t k = i;
            while (s[k] && (isalnum((unsigned char)s[k]) || s[k] == '_')) {
                if (j + 1 < sizeof(id)) id[j++] = s[k];
                k++;
            }
            id[j] = '\0';
            define_tok_push(&toks, &ntoks, &cap, DEF_TOK_IDENT, id);
            i = k;
            continue;
        }

        define_tok_push(&toks, &ntoks, &cap, DEF_TOK_OTHER, NULL);
        i++;
    }

    bool ok = true;
    for (size_t i = 0; i < ntoks; ++i) {
        if (toks[i].kind == DEF_TOK_HASHHASH) {
            if (i == 0 || i + 1 >= ntoks) {
                ok = false;
                break;
            }
            if (toks[i - 1].kind == DEF_TOK_HASH || toks[i - 1].kind == DEF_TOK_HASHHASH) {
                ok = false;
                break;
            }
            if (toks[i + 1].kind == DEF_TOK_HASH || toks[i + 1].kind == DEF_TOK_HASHHASH) {
                ok = false;
                break;
            }
        }
        if (is_func && toks[i].kind == DEF_TOK_HASH) {
            if (i + 1 >= ntoks || toks[i + 1].kind != DEF_TOK_IDENT) {
                ok = false;
                break;
            }
            if (!define_param_name_match(toks[i + 1].ident, params, nparams, variadic, var_param)) {
                ok = false;
                break;
            }
        }
    }

    free(toks);
    return ok;
}

static bool validate_define_directive(const char *p) {
    bool is_func = false;
    char **params = NULL;
    size_t nparams = 0;
    bool variadic = false;
    char *var_param = NULL;
    const char *repl_start = NULL;

    bool sig_ok = parse_define_signature(p, &is_func, &params, &nparams,
                                         &variadic, &var_param, &repl_start);
    if (!sig_ok) {
        if (params) { for (size_t i = 0; i < nparams; ++i) free(params[i]); free(params); }
        if (var_param) free(var_param);
        return false;
    }

    bool valid = validate_define_replacement_list(repl_start, is_func, params, nparams,
                                                  variadic, var_param);
    if (params) { for (size_t i = 0; i < nparams; ++i) free(params[i]); free(params); }
    if (var_param) free(var_param);
    return valid;
}

static bool parse_module_signature(const char *p, char *namebuf, size_t namebuf_sz,
                                   bool *is_func, char ***params, size_t *nparams,
                                   bool *variadic, char **var_param, const char **after_sig) {
    if (is_func) *is_func = false;
    if (params) *params = NULL;
    if (nparams) *nparams = 0;
    if (variadic) *variadic = false;
    if (var_param) *var_param = NULL;
    if (after_sig) *after_sig = p;

    const char *q = clskip(p);
    size_t adv = 0;
    if (!parse_ident(q, &adv, namebuf, namebuf_sz)) return false;
    q += adv;

    const char *r = clskip(q);
    if (*r == '(') {
        if (is_func) *is_func = true;
        char **pp = NULL; size_t np = 0, pc = 0;
        bool var = false;
        char *vname = NULL;
        const char *s = r + 1;
        bool ok = true;
        while (1) {
            while (*s && isspace((unsigned char)*s)) s++;
            if (*s == ')') { s++; break; }
            if (s[0]=='.' && s[1]=='.' && s[2]=='.') {
                var = true;
                s += 3;
                while (*s && isspace((unsigned char)*s)) s++;
                if (*s == ')') s++;
                break;
            }
            char id[256]; size_t j=0;
            if (!(isalpha((unsigned char)*s) || *s=='_')) { ok = false; break; }
            while (*s && (isalnum((unsigned char)*s) || *s=='_')) { if (j+1<sizeof id) id[j++]=*s; s++; }
            id[j]='\0';
            const char *s2 = s;
            while (*s2 && isspace((unsigned char)*s2)) s2++;
            if (s2[0]=='.' && s2[1]=='.' && s2[2]=='.') {
                var = true;
                if (vname) free(vname);
                vname = xstrdup(id);
                s = s2 + 3;
                while (*s && isspace((unsigned char)*s)) s++;
                if (*s == ')') s++;
                break;
            }
            if (np == pc) { pc = pc? pc*2:4; pp = xrealloc(pp, pc*sizeof(char*)); }
            pp[np++] = xstrdup(id);
            s = s2;
            if (*s == ',') { s++; continue; }
            if (*s == ')') { s++; break; }
            ok = false;
            break;
        }
        if (!ok) {
            if (pp) { for (size_t i=0; i<np; ++i) free(pp[i]); free(pp); }
            if (vname) free(vname);
            return false;
        }
        if (params) *params = pp;
        else if (pp) { for (size_t i=0; i<np; ++i) free(pp[i]); free(pp); }
        if (nparams) *nparams = np;
        if (variadic) *variadic = var;
        if (var_param) *var_param = vname;
        else if (vname) free(vname);
        q = s;
    } else {
        q = r;
    }
    if (after_sig) *after_sig = q;
    return true;
}

static const char *find_first_open_brace(const char *s) {
    enum { FB_CODE, FB_DQ, FB_SQ, FB_SL, FB_BL } st = FB_CODE;
    int bl_depth = 0;
    for (size_t i = 0; s && s[i]; ++i) {
        char c = s[i];
        if (st == FB_CODE) {
            if (c == '"') { st = FB_DQ; continue; }
            if (c == '\'') { st = FB_SQ; continue; }
            if (c == '/' && s[i+1] == '/') { st = FB_SL; break; }
            if (c == '/' && s[i+1] == '*') { st = FB_BL; bl_depth = 1; i++; continue; }
            if (c == '{') return s + i;
        } else if (st == FB_DQ) {
            if (c == '"' && !is_escaped(s, i)) st = FB_CODE;
        } else if (st == FB_SQ) {
            if (c == '\'' && !is_escaped(s, i)) st = FB_CODE;
        } else if (st == FB_SL) {
            break;
        } else if (st == FB_BL) {
            if (c == '/' && s[i+1] == '*') { bl_depth++; i++; continue; }
            if (c == '*' && s[i+1] == '/') {
                bl_depth--; i++;
                if (bl_depth <= 0) { bl_depth = 0; st = FB_CODE; }
                continue;
            }
        }
    }
    return NULL;
}

// Expression evaluator for #if: supports numbers, identifiers (numeric macros -> value else 0), defined,
// unary ! ~ + -, arithmetic * / + -, relational < <= > >=, equality == !=, logical && ||, ?:, parentheses
typedef struct {
    const char *s;
    bool err;
} Expr;

static void ex_mark_error(Expr *e) {
    if (e) e->err = true;
}

static const char *ex_skip(const char *s) {
    while (1) {
        while (*s && isspace((unsigned char)*s)) s++;
        if (s[0] == '/' && s[1] == '*') {
            s += 2;
            while (*s && !(s[0] == '*' && s[1] == '/')) s++;
            if (*s) s += 2;
            continue;
        }
        if (s[0] == '/' && s[1] == '/') {
            while (*s && *s != '\n' && *s != '\r') s++;
            continue;
        }
        // Be tolerant to stray block-comment closers in some system headers.
        if (s[0] == '*' && s[1] == '/') {
            s += 2;
            continue;
        }
        break;
    }
    return s;
}
typedef enum {
    EX_RANK_INT = 1,
    EX_RANK_LONG = 2,
    EX_RANK_LLONG = 3
} ExRank;

typedef struct {
    unsigned long long u;
    bool is_unsigned;
    ExRank rank;
} ExValue;

static ExValue ex_parse_expr(const MacroTable *t, Expr *e);
static void ex_usual_arith(ExValue *a, ExValue *b);

static ExValue ex_make_signed_rank(long long v, ExRank rank) {
    ExValue r;
    r.u = (unsigned long long)v;
    r.is_unsigned = false;
    r.rank = rank;
    return r;
}

static ExValue ex_make_unsigned_rank(unsigned long long v, ExRank rank) {
    ExValue r;
    r.u = v;
    r.is_unsigned = true;
    r.rank = rank;
    return r;
}

static ExValue ex_make_signed(long long v) {
    return ex_make_signed_rank(v, EX_RANK_INT);
}

static ExValue ex_make_unsigned(unsigned long long v) {
    return ex_make_unsigned_rank(v, EX_RANK_INT);
}

static long long ex_as_signed(ExValue v) {
    return (long long)v.u;
}

static bool ex_truthy(ExValue v) {
    return v.u != 0ULL;
}

static ExValue ex_cast_value(ExValue v, bool as_unsigned, ExRank rank) {
    if (as_unsigned) return ex_make_unsigned_rank(v.u, rank);
    return ex_make_signed_rank((long long)v.u, rank);
}

static void ex_usual_arith(ExValue *a, ExValue *b) {
    if (!a || !b) return;
    bool as_unsigned = a->is_unsigned || b->is_unsigned;
    ExRank rank = EX_RANK_LLONG;
    *a = ex_cast_value(*a, as_unsigned, rank);
    *b = ex_cast_value(*b, as_unsigned, rank);
}

static ExValue ex_select_cond_value(bool cond, ExValue vt, ExValue vf) {
    ex_usual_arith(&vt, &vf);
    return cond ? vt : vf;
}

static bool parse_int_suffix(const char *p, const char **out_end, bool *out_unsigned, int *out_lcount) {
    const char *q = p ? p : "";
    if (!(*q == 'u' || *q == 'U' || *q == 'l' || *q == 'L')) {
        if (out_end) *out_end = q;
        if (out_unsigned) *out_unsigned = false;
        if (out_lcount) *out_lcount = 0;
        return true;
    }

    bool got_u = false;
    int got_l = 0; // 0:none, 1:l/L, 2:ll/LL
    for (int i = 0; i < 2; ++i) {
        if (!got_u && (*q == 'u' || *q == 'U')) {
            got_u = true;
            q++;
            continue;
        }
        if (got_l == 0 && (*q == 'l' || *q == 'L')) {
            char c = *q;
            if (q[1] == c) {
                got_l = 2;
                q += 2;
            } else if (q[1] == 'l' || q[1] == 'L') {
                return false; // mixed-case long-long suffix is invalid
            } else {
                got_l = 1;
                q++;
            }
            continue;
        }
        break;
    }

    if (*q == 'u' || *q == 'U' || *q == 'l' || *q == 'L') {
        return false;
    }
    if (out_end) *out_end = q;
    if (out_unsigned) *out_unsigned = got_u;
    if (out_lcount) *out_lcount = got_l;
    return true;
}

static bool ex_is_decimal_literal(const char *s) {
    if (!s || !*s) return false;
    if (s[0] == '0') {
        if (s[1] == 'x' || s[1] == 'X' || s[1] == 'b' || s[1] == 'B') return false;
        if (s[1] >= '0' && s[1] <= '9') return false;
    }
    return true;
}

static ExValue parse_int_literal(const char *s, const char **out_end) {
    char *digits_end = NULL;
    errno = 0;
    unsigned long long u = strtoull(s, &digits_end, 0);
    bool overflow = (errno == ERANGE);
    const char *p = digits_end ? digits_end : s;
    const char *suffix_end = p;
    bool suffix_unsigned = false;
    int lcount = 0;
    if (parse_int_suffix(p, &suffix_end, &suffix_unsigned, &lcount)) {
        p = suffix_end;
    }
    if (out_end) *out_end = p;

    bool is_decimal = ex_is_decimal_literal(s);
    ExRank rank = EX_RANK_LLONG;
    bool is_unsigned = suffix_unsigned;

    if (suffix_unsigned) {
        if (lcount == 2) {
            rank = EX_RANK_LLONG;
        } else if (lcount == 1) {
            rank = (overflow || u > (unsigned long long)ULONG_MAX) ? EX_RANK_LLONG : EX_RANK_LONG;
        } else {
            if (!overflow && u <= (unsigned long long)UINT_MAX) rank = EX_RANK_INT;
            else if (!overflow && u <= (unsigned long long)ULONG_MAX) rank = EX_RANK_LONG;
            else rank = EX_RANK_LLONG;
        }
    } else if (is_decimal) {
        if (lcount == 2) {
            rank = EX_RANK_LLONG;
            if (overflow || u > (unsigned long long)LLONG_MAX) is_unsigned = true;
        } else if (lcount == 1) {
            if (!overflow && u <= (unsigned long long)LONG_MAX) rank = EX_RANK_LONG;
            else if (!overflow && u <= (unsigned long long)LLONG_MAX) rank = EX_RANK_LLONG;
            else { rank = EX_RANK_LLONG; is_unsigned = true; }
        } else {
            if (!overflow && u <= (unsigned long long)INT_MAX) rank = EX_RANK_INT;
            else if (!overflow && u <= (unsigned long long)LONG_MAX) rank = EX_RANK_LONG;
            else if (!overflow && u <= (unsigned long long)LLONG_MAX) rank = EX_RANK_LLONG;
            else { rank = EX_RANK_LLONG; is_unsigned = true; }
        }
    } else {
        // Octal/hex/bin constants include unsigned candidates in selection.
        if (lcount == 2) {
            rank = EX_RANK_LLONG;
            if (overflow || u > (unsigned long long)LLONG_MAX) is_unsigned = true;
        } else if (lcount == 1) {
            if (!overflow && u <= (unsigned long long)LONG_MAX) { rank = EX_RANK_LONG; is_unsigned = false; }
            else if (!overflow && u <= (unsigned long long)ULONG_MAX) { rank = EX_RANK_LONG; is_unsigned = true; }
            else if (!overflow && u <= (unsigned long long)LLONG_MAX) { rank = EX_RANK_LLONG; is_unsigned = false; }
            else { rank = EX_RANK_LLONG; is_unsigned = true; }
        } else {
            if (!overflow && u <= (unsigned long long)INT_MAX) { rank = EX_RANK_INT; is_unsigned = false; }
            else if (!overflow && u <= (unsigned long long)UINT_MAX) { rank = EX_RANK_INT; is_unsigned = true; }
            else if (!overflow && u <= (unsigned long long)LONG_MAX) { rank = EX_RANK_LONG; is_unsigned = false; }
            else if (!overflow && u <= (unsigned long long)ULONG_MAX) { rank = EX_RANK_LONG; is_unsigned = true; }
            else if (!overflow && u <= (unsigned long long)LLONG_MAX) { rank = EX_RANK_LLONG; is_unsigned = false; }
            else { rank = EX_RANK_LLONG; is_unsigned = true; }
        }
    }

    return is_unsigned ? ex_make_unsigned_rank(u, rank) : ex_make_signed_rank((long long)u, rank);
}

static int ex_hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static bool ex_parse_escape(const char **pp, long *out_ch) {
    const char *p = *pp;
    if (*p != '\\') return false;
    p++;
    if (*p == '\0') return false;
    long ch = 0;
    switch (*p) {
    case 'a': ch = '\a'; p++; break;
    case 'b': ch = '\b'; p++; break;
    case 'f': ch = '\f'; p++; break;
    case 'n': ch = '\n'; p++; break;
    case 'r': ch = '\r'; p++; break;
    case 't': ch = '\t'; p++; break;
    case 'v': ch = '\v'; p++; break;
    case '\\': ch = '\\'; p++; break;
    case '\'': ch = '\''; p++; break;
    case '"': ch = '"'; p++; break;
    case '?': ch = '\?'; p++; break;
    case 'x': {
        p++;
        int d = ex_hex_digit(*p);
        if (d < 0) return false;
        ch = 0;
        while ((d = ex_hex_digit(*p)) >= 0) {
            ch = (ch << 4) + d;
            p++;
        }
        break;
    }
    default:
        if (*p >= '0' && *p <= '7') {
            ch = 0;
            int count = 0;
            while (count < 3 && *p >= '0' && *p <= '7') {
                ch = (ch << 3) + (*p - '0');
                p++;
                count++;
            }
        } else {
            ch = (unsigned char)*p;
            p++;
        }
        break;
    }
    *pp = p;
    if (out_ch) *out_ch = ch;
    return true;
}

static bool ex_parse_char_literal(const char *s, const char **out_end, long *out_val) {
    const char *p = s;
    if ((*p == 'L' || *p == 'u' || *p == 'U') && p[1] == '\'') {
        p++;
    }
    if (*p != '\'') return false;
    p++;
    if (*p == '\0' || *p == '\n' || *p == '\r') return false;

    long v = 0;
    int n = 0;
    while (*p && *p != '\'') {
        long ch = 0;
        if (*p == '\\') {
            if (!ex_parse_escape(&p, &ch)) return false;
        } else {
            ch = (unsigned char)*p;
            p++;
        }
        v = (v << 8) | (ch & 0xFF);
        n++;
    }
    if (*p != '\'' || n == 0) return false;
    p++;
    if (out_end) *out_end = p;
    if (out_val) *out_val = v;
    return true;
}

static ExValue ex_value_of_ident(const MacroTable *t, const char *ident) {
    if (strcmp(ident, "true") == 0) return ex_make_signed(1);
    if (strcmp(ident, "false") == 0) return ex_make_signed(0);
    const char *v = mtable_get(t, ident);
    if (!v) return ex_make_signed(0);
    const char *end = NULL;
    ExValue n = parse_int_literal(v, &end);
    if (end && *end == '\0') return n;
    return ex_make_signed(0);
}

static bool ex_is_builtin_like_ident(const char *ident) {
    if (!ident) return false;
    if (starts_with(ident, "__has_")) return true;
    if (starts_with(ident, "__is_")) return true;
    if (starts_with(ident, "__building_")) return true;
    return false;
}

static ExValue ex_parse_primary(const MacroTable *t, Expr *e) {
    e->s = ex_skip(e->s);
    if (*e->s == '(') {
        e->s++;
        ExValue v = ex_parse_expr(t, e);
        e->s = ex_skip(e->s);
        if (*e->s == ')') {
            e->s++;
        } else {
            ex_mark_error(e);
        }
        return v;
    }
    if (isdigit((unsigned char)*e->s)) {
        const char *end = NULL;
        ExValue v = parse_int_literal(e->s, &end);
        e->s = end ? end : e->s;
        return v;
    }
    {
        const char *end = NULL;
        long v = 0;
        if (ex_parse_char_literal(e->s, &end, &v)) {
            e->s = end ? end : e->s;
            return ex_make_signed(v);
        }
    }
    char id[256]; size_t a=0;
    if (parse_ident(e->s, &a, id, sizeof id)) {
        e->s += a;
        if (strcmp(id, "defined") == 0) {
            const char *p = ex_skip(e->s);
            long v = 0;
            if (*p == '(') {
                bool has_close = false;
                p++; p = ex_skip(p);
                char did[256]; size_t da = 0;
                if (parse_ident(p, &da, did, sizeof did)) {
                    p += da;
                    p = ex_skip(p);
                    if (*p == ')') { p++; has_close = true; }
                    v = mtable_get(t, did) ? 1 : 0;
                }
                if (!has_close) ex_mark_error(e);
            } else {
                char did[256]; size_t da = 0;
                if (parse_ident(p, &da, did, sizeof did)) {
                    p += da;
                    v = mtable_get(t, did) ? 1 : 0;
                } else {
                    ex_mark_error(e);
                }
            }
            e->s = p;
            return ex_make_signed(v);
        }
        ExValue v = ex_value_of_ident(t, id);
        const char *p = ex_skip(e->s);
        if (*p == '(') {
            int depth = 0;
            const char *q = p;
            for (; *q; ++q) {
                char ch = *q;
                if (ch == '(') { depth++; continue; }
                if (ch == ')') {
                    depth--;
                    if (depth == 0) {
                        q++;
                        break;
                    }
                }
            }
            if (depth != 0) {
                ex_mark_error(e);
                e->s = q;
            } else if (ex_is_builtin_like_ident(id)) {
                // Accept compiler builtin-like predicates used in system
                // headers as 0 for compatibility when not explicitly handled.
                e->s = q;
                v = ex_make_signed(0);
            } else {
                // In #if expressions, generic identifier(...) token
                // sequences are invalid after macro expansion.
                ex_mark_error(e);
                e->s = p;
            }
        }
        return v;
    }
    ex_mark_error(e);
    return ex_make_signed(0);
}

static ExValue ex_parse_unary(const MacroTable *t, Expr *e) {
    e->s = ex_skip(e->s);
    if (*e->s == '!') {
        e->s++;
        ExValue v = ex_parse_unary(t, e);
        return ex_make_signed(ex_truthy(v) ? 0 : 1);
    }
    if (*e->s == '~') {
        e->s++;
        ExValue v = ex_parse_unary(t, e);
        if (v.is_unsigned) return ex_make_unsigned_rank(~v.u, v.rank);
        return ex_make_signed_rank(~ex_as_signed(v), v.rank);
    }
    if (*e->s == '+') {
        e->s++;
        return ex_parse_unary(t, e);
    }
    if (*e->s == '-') {
        e->s++;
        ExValue v = ex_parse_unary(t, e);
        if (v.is_unsigned) return ex_make_unsigned_rank(0ULL - v.u, v.rank);
        return ex_make_signed_rank(-ex_as_signed(v), v.rank);
    }
    return ex_parse_primary(t, e);
}

static ExValue ex_parse_mul(const MacroTable *t, Expr *e) {
    ExValue v = ex_parse_unary(t, e);
    while (1) {
        const char *s = ex_skip(e->s);
        if (*s == '*') {
            e->s = s + 1;
            ExValue r = ex_parse_unary(t, e);
            ExValue l = v;
            ex_usual_arith(&l, &r);
            if (l.is_unsigned) v = ex_make_unsigned_rank(l.u * r.u, l.rank);
            else v = ex_make_signed_rank(ex_as_signed(l) * ex_as_signed(r), l.rank);
            continue;
        }
        if (*s == '/') {
            e->s = s + 1;
            ExValue r = ex_parse_unary(t, e);
            ExValue l = v;
            ex_usual_arith(&l, &r);
            if (r.u == 0ULL) v = l;
            else if (l.is_unsigned) v = ex_make_unsigned_rank(l.u / r.u, l.rank);
            else v = ex_make_signed_rank(ex_as_signed(l) / ex_as_signed(r), l.rank);
            continue;
        }
        if (*s == '%') {
            e->s = s + 1;
            ExValue r = ex_parse_unary(t, e);
            ExValue l = v;
            ex_usual_arith(&l, &r);
            if (r.u == 0ULL) v = l;
            else if (l.is_unsigned) v = ex_make_unsigned_rank(l.u % r.u, l.rank);
            else v = ex_make_signed_rank(ex_as_signed(l) % ex_as_signed(r), l.rank);
            continue;
        }
        e->s = s; break;
    }
    return v;
}

static ExValue ex_parse_add(const MacroTable *t, Expr *e) {
    ExValue v = ex_parse_mul(t, e);
    while (1) {
        const char *s = ex_skip(e->s);
        if (*s == '+') {
            e->s = s + 1;
            ExValue r = ex_parse_mul(t, e);
            ExValue l = v;
            ex_usual_arith(&l, &r);
            if (l.is_unsigned) v = ex_make_unsigned_rank(l.u + r.u, l.rank);
            else v = ex_make_signed_rank(ex_as_signed(l) + ex_as_signed(r), l.rank);
            continue;
        }
        if (*s == '-') {
            e->s = s + 1;
            ExValue r = ex_parse_mul(t, e);
            ExValue l = v;
            ex_usual_arith(&l, &r);
            if (l.is_unsigned) v = ex_make_unsigned_rank(l.u - r.u, l.rank);
            else v = ex_make_signed_rank(ex_as_signed(l) - ex_as_signed(r), l.rank);
            continue;
        }
        e->s = s; break;
    }
    return v;
}

static ExValue ex_parse_shift(const MacroTable *t, Expr *e) {
    ExValue v = ex_parse_add(t, e);
    const unsigned long long SHIFT_WIDTH = 64ULL;
    while (1) {
        const char *s = ex_skip(e->s);
        if (s[0] == '<' && s[1] == '<') {
            e->s = s + 2;
            ExValue r = ex_parse_add(t, e);
            ExValue l = v;
            bool zero_result = false;
            bool use_right = false;
            unsigned int sh = 0;
            if (r.is_unsigned) {
                if (r.u >= SHIFT_WIDTH) zero_result = true;
                else sh = (unsigned int)r.u;
            } else {
                long long sr = ex_as_signed(r);
                if (sr < 0) {
                    unsigned long long mag = (unsigned long long)(-(sr + 1LL)) + 1ULL;
                    if (mag >= SHIFT_WIDTH) zero_result = true;
                    else { sh = (unsigned int)mag; use_right = true; }
                } else if ((unsigned long long)sr >= SHIFT_WIDTH) {
                    zero_result = true;
                } else {
                    sh = (unsigned int)sr;
                }
            }
            if (zero_result) {
                v = l.is_unsigned ? ex_make_unsigned_rank(0ULL, l.rank) : ex_make_signed_rank(0LL, l.rank);
            } else if (use_right) {
                if (l.is_unsigned) v = ex_make_unsigned_rank(l.u >> sh, l.rank);
                else v = ex_make_signed_rank(ex_as_signed(l) >> sh, l.rank);
            } else if (l.is_unsigned) {
                v = ex_make_unsigned_rank(l.u << sh, l.rank);
            } else {
                v = ex_make_signed_rank(ex_as_signed(l) << sh, l.rank);
            }
            continue;
        }
        if (s[0] == '>' && s[1] == '>') {
            e->s = s + 2;
            ExValue r = ex_parse_add(t, e);
            ExValue l = v;
            bool zero_result = false;
            bool use_left = false;
            unsigned int sh = 0;
            if (r.is_unsigned) {
                if (r.u >= SHIFT_WIDTH) zero_result = true;
                else sh = (unsigned int)r.u;
            } else {
                long long sr = ex_as_signed(r);
                if (sr < 0) {
                    unsigned long long mag = (unsigned long long)(-(sr + 1LL)) + 1ULL;
                    if (mag >= SHIFT_WIDTH) zero_result = true;
                    else { sh = (unsigned int)mag; use_left = true; }
                } else if ((unsigned long long)sr >= SHIFT_WIDTH) {
                    zero_result = true;
                } else {
                    sh = (unsigned int)sr;
                }
            }
            if (zero_result) {
                v = l.is_unsigned ? ex_make_unsigned_rank(0ULL, l.rank) : ex_make_signed_rank(0LL, l.rank);
            } else if (use_left) {
                if (l.is_unsigned) v = ex_make_unsigned_rank(l.u << sh, l.rank);
                else v = ex_make_signed_rank(ex_as_signed(l) << sh, l.rank);
            } else if (l.is_unsigned) {
                v = ex_make_unsigned_rank(l.u >> sh, l.rank);
            } else {
                v = ex_make_signed_rank(ex_as_signed(l) >> sh, l.rank);
            }
            continue;
        }
        e->s = s; break;
    }
    return v;
}

static ExValue ex_parse_rel(const MacroTable *t, Expr *e) {
    ExValue v = ex_parse_shift(t, e);
    while (1) {
        const char *s = ex_skip(e->s);
        if (s[0] == '<' && s[1] == '=') {
            e->s = s + 2;
            ExValue r = ex_parse_shift(t, e);
            ExValue l = v;
            ex_usual_arith(&l, &r);
            bool ok = l.is_unsigned ? (l.u <= r.u) : (ex_as_signed(l) <= ex_as_signed(r));
            v = ex_make_signed(ok ? 1 : 0);
            continue;
        }
        if (s[0] == '>' && s[1] == '=') {
            e->s = s + 2;
            ExValue r = ex_parse_shift(t, e);
            ExValue l = v;
            ex_usual_arith(&l, &r);
            bool ok = l.is_unsigned ? (l.u >= r.u) : (ex_as_signed(l) >= ex_as_signed(r));
            v = ex_make_signed(ok ? 1 : 0);
            continue;
        }
        if (s[0] == '<') {
            e->s = s + 1;
            ExValue r = ex_parse_shift(t, e);
            ExValue l = v;
            ex_usual_arith(&l, &r);
            bool ok = l.is_unsigned ? (l.u < r.u) : (ex_as_signed(l) < ex_as_signed(r));
            v = ex_make_signed(ok ? 1 : 0);
            continue;
        }
        if (s[0] == '>') {
            e->s = s + 1;
            ExValue r = ex_parse_shift(t, e);
            ExValue l = v;
            ex_usual_arith(&l, &r);
            bool ok = l.is_unsigned ? (l.u > r.u) : (ex_as_signed(l) > ex_as_signed(r));
            v = ex_make_signed(ok ? 1 : 0);
            continue;
        }
        e->s = s; break;
    }
    return v;
}

static ExValue ex_parse_eq(const MacroTable *t, Expr *e) {
    ExValue v = ex_parse_rel(t, e);
    while (1) {
        const char *s = ex_skip(e->s);
        if (s[0] == '=' && s[1] == '=') {
            e->s = s + 2;
            ExValue r = ex_parse_rel(t, e);
            ExValue l = v;
            ex_usual_arith(&l, &r);
            bool ok = l.is_unsigned ? (l.u == r.u) : (ex_as_signed(l) == ex_as_signed(r));
            v = ex_make_signed(ok ? 1 : 0);
            continue;
        }
        if (s[0] == '!' && s[1] == '=') {
            e->s = s + 2;
            ExValue r = ex_parse_rel(t, e);
            ExValue l = v;
            ex_usual_arith(&l, &r);
            bool ok = l.is_unsigned ? (l.u != r.u) : (ex_as_signed(l) != ex_as_signed(r));
            v = ex_make_signed(ok ? 1 : 0);
            continue;
        }
        e->s = s; break;
    }
    return v;
}

static ExValue ex_parse_band(const MacroTable *t, Expr *e) {
    ExValue v = ex_parse_eq(t, e);
    while (1) {
        const char *s = ex_skip(e->s);
        if (*s == '&' && s[1] != '&') {
            e->s = s + 1;
            ExValue r = ex_parse_eq(t, e);
            ExValue l = v;
            ex_usual_arith(&l, &r);
            if (l.is_unsigned) v = ex_make_unsigned_rank(l.u & r.u, l.rank);
            else v = ex_make_signed_rank(ex_as_signed(l) & ex_as_signed(r), l.rank);
            continue;
        }
        e->s = s;
        break;
    }
    return v;
}

static ExValue ex_parse_bxor(const MacroTable *t, Expr *e) {
    ExValue v = ex_parse_band(t, e);
    while (1) {
        const char *s = ex_skip(e->s);
        if (*s == '^') {
            e->s = s + 1;
            ExValue r = ex_parse_band(t, e);
            ExValue l = v;
            ex_usual_arith(&l, &r);
            if (l.is_unsigned) v = ex_make_unsigned_rank(l.u ^ r.u, l.rank);
            else v = ex_make_signed_rank(ex_as_signed(l) ^ ex_as_signed(r), l.rank);
            continue;
        }
        e->s = s;
        break;
    }
    return v;
}

static ExValue ex_parse_bor(const MacroTable *t, Expr *e) {
    ExValue v = ex_parse_bxor(t, e);
    while (1) {
        const char *s = ex_skip(e->s);
        if (*s == '|' && s[1] != '|') {
            e->s = s + 1;
            ExValue r = ex_parse_bxor(t, e);
            ExValue l = v;
            ex_usual_arith(&l, &r);
            if (l.is_unsigned) v = ex_make_unsigned_rank(l.u | r.u, l.rank);
            else v = ex_make_signed_rank(ex_as_signed(l) | ex_as_signed(r), l.rank);
            continue;
        }
        e->s = s;
        break;
    }
    return v;
}

static ExValue ex_parse_land(const MacroTable *t, Expr *e) {
    ExValue v = ex_parse_bor(t, e);
    while (1) {
        const char *s = ex_skip(e->s);
        if (s[0] == '&' && s[1] == '&') {
            e->s = s + 2;
            ExValue r = ex_parse_bor(t, e);
            v = ex_make_signed((ex_truthy(v) && ex_truthy(r)) ? 1 : 0);
            continue;
        }
        e->s = s;
        break;
    }
    return v;
}

static ExValue ex_parse_lor(const MacroTable *t, Expr *e) {
    ExValue v = ex_parse_land(t, e);
    while (1) {
        const char *s = ex_skip(e->s);
        if (s[0] == '|' && s[1] == '|') {
            e->s = s + 2;
            ExValue r = ex_parse_land(t, e);
            v = ex_make_signed((ex_truthy(v) || ex_truthy(r)) ? 1 : 0);
            continue;
        }
        e->s = s;
        break;
    }
    return v;
}

static ExValue ex_parse_cond(const MacroTable *t, Expr *e) {
    ExValue v = ex_parse_lor(t, e);
    const char *s = ex_skip(e->s);
    if (*s == '?') {
        e->s = s + 1;
        ExValue vt = ex_parse_cond(t, e);
        s = ex_skip(e->s);
        if (*s == ':') {
            e->s = s + 1;
            ExValue vf = ex_parse_cond(t, e);
            v = ex_select_cond_value(ex_truthy(v), vt, vf);
        }
        else ex_mark_error(e);
    }
    return v;
}

static ExValue ex_parse_expr(const MacroTable *t, Expr *e) { return ex_parse_cond(t, e); }

typedef struct { bool parent_active; bool current_active; bool taken; bool saw_else; } IfCtx;


static char *path_join(const char *a, const char *b) {
    size_t la = strlen(a), lb = strlen(b);
    bool need_slash = (la > 0 && a[la-1] != '/');
    size_t n = la + (need_slash?1:0) + lb + 1;
    char *p = (char*)malloc(n);
    if (!p) die("malloc");
    memcpy(p, a, la);
    size_t i = la;
    if (need_slash) p[i++] = '/';
    memcpy(p+i, b, lb); i += lb; p[i] = '\0';
    return p;
}

static char *dirname_dup(const char *path) {
    const char *slash = strrchr(path, '/');
    if (!slash) return xstrdup(".");
    size_t n = (size_t)(slash - path);
    char *d = (char*)malloc(n+1);
    if (!d) die("malloc");
    memcpy(d, path, n); d[n] = '\0';
    return d;
}

static FILE *open_in_search(const char *name, bool quoted,
                            const char *curdir, const PPOpts *opts,
                            char **outpath) {
    if (quoted && curdir) {
        char *p = path_join(curdir, name);
        FILE *f = fopen(p, "r");
        if (f) { if (outpath) *outpath = p; else free(p); return f; }
        free(p);
    }
    for (size_t i=0; i<(opts?opts->nincdirs:0); ++i) {
        char *p = path_join(opts->incdirs[i], name);
        FILE *f = fopen(p, "r");
        if (f) { if (outpath) *outpath = p; else free(p); return f; }
        free(p);
    }
    for (size_t i=0; i<(opts?opts->nsysincdirs:0); ++i) {
        char *p = path_join(opts->sysincdirs[i], name);
        FILE *f = fopen(p, "r");
        if (f) { if (outpath) *outpath = p; else free(p); return f; }
        free(p);
    }
    return NULL;
}

static FILE *open_in_search_next(const char *name,
                                 const char *this_path,
                                 const PPOpts *opts,
                                 char **outpath) {
    // Determine the index of the directory where this_path was found, by
    // reconstructing candidate full paths and comparing exactly. Then continue
    // search from the next entry in that list.
    long start_inc = 0, start_sys = 0;
    bool found_this = false;
    if (this_path && opts) {
        for (size_t i=0; i<opts->nincdirs; ++i) {
            char *cand = path_join(opts->incdirs[i], name);
            bool eq = (strcmp(cand, this_path) == 0);
            free(cand);
            if (eq) { start_inc = (long)i + 1; found_this = true; break; }
        }
        if (!found_this) {
            for (size_t j=0; j<opts->nsysincdirs; ++j) {
                char *cand = path_join(opts->sysincdirs[j], name);
                bool eq = (strcmp(cand, this_path) == 0);
                free(cand);
                if (eq) { start_sys = (long)j + 1; found_this = true; break; }
            }
        }
        // If we can't locate the current header in search paths, avoid
        // spuriously treating __has_include_next / #include_next as found.
        if (!found_this) return NULL;
    }
    if (opts) {
        for (long i=start_inc; i<(long)opts->nincdirs; ++i) {
            char *p = path_join(opts->incdirs[i], name);
            if (this_path && strcmp(p, this_path) == 0) { free(p); continue; }
            FILE *f = fopen(p, "r");
            if (f) { if (outpath) *outpath = p; else free(p); return f; }
            free(p);
        }
        for (long j=start_sys; j<(long)opts->nsysincdirs; ++j) {
            char *p = path_join(opts->sysincdirs[j], name);
            if (this_path && strcmp(p, this_path) == 0) { free(p); continue; }
            FILE *f = fopen(p, "r");
            if (f) { if (outpath) *outpath = p; else free(p); return f; }
            free(p);
        }
    }
    return NULL;
}

// Read one logical line (without trailing \n). Returns true if a line was read.
static bool has_line_splice_suffix(const Str *line) {
    if (!line || !line->buf || line->len == 0) return false;
    if (line->buf[line->len - 1] == '\\') return true;
    if (line->len >= 3 &&
        line->buf[line->len - 3] == '?' &&
        line->buf[line->len - 2] == '?' &&
        line->buf[line->len - 1] == '/') {
        return true;
    }
    return false;
}

static void remove_line_splice_suffix(Str *line) {
    if (!line || !line->buf || line->len == 0) return;
    if (line->buf[line->len - 1] == '\\') {
        line->buf[--line->len] = '\0';
        return;
    }
    if (line->len >= 3 &&
        line->buf[line->len - 3] == '?' &&
        line->buf[line->len - 2] == '?' &&
        line->buf[line->len - 1] == '/') {
        line->len -= 3;
        line->buf[line->len] = '\0';
    }
}

static bool read_line(FILE *in, Str *line) {
    sb_init(line);
    int ch;
    bool got = false;
    while ((ch = fgetc(in)) != EOF) {
        got = true;
        if (ch == '\f') {
            // Treat form-feed as whitespace and drop it.
            continue;
        }
        if (ch == '\r') {
            int ch2 = fgetc(in);
            if (ch2 != '\n' && ch2 != EOF) ungetc(ch2, in);
            // handle line splicing if trailing backslash present
            if (has_line_splice_suffix(line)) { remove_line_splice_suffix(line); continue; }
            break;
        }
        if (ch == '\n') {
            if (has_line_splice_suffix(line)) { remove_line_splice_suffix(line); continue; }
            break;
        }
        sb_putc(line, (char)ch);
    }
    return got;
}

static bool collect_module_body(FILE *in, const char *start, long *line_no, Str *body) {
    enum { MB_CODE, MB_DQ, MB_SQ, MB_SL, MB_BL } st = MB_CODE;
    int bl_depth = 0;
    int brace_depth = 1;
    const char *s = start ? start : "";
    Str cur; bool has_cur = false; sb_init(&cur);
    while (1) {
        for (size_t i = 0; s[i]; ++i) {
            char c = s[i];
            if (st == MB_CODE) {
                if (c == '"') { st = MB_DQ; sb_putc(body, c); continue; }
                if (c == '\'') { st = MB_SQ; sb_putc(body, c); continue; }
                if (c == '/' && s[i+1] == '/') {
                    st = MB_SL;
                    sb_putc(body, c);
                    if (s[i+1]) { sb_putc(body, s[i+1]); i++; }
                    continue;
                }
                if (c == '/' && s[i+1] == '*') {
                    st = MB_BL; bl_depth = 1;
                    sb_putc(body, c);
                    if (s[i+1]) { sb_putc(body, s[i+1]); i++; }
                    continue;
                }
                if (c == '{') { brace_depth++; sb_putc(body, c); continue; }
                if (c == '}') {
                    brace_depth--;
                    if (brace_depth == 0) {
                        if (has_cur) sb_free(&cur);
                        return true;
                    }
                    sb_putc(body, c);
                    continue;
                }
                sb_putc(body, c);
            } else if (st == MB_DQ) {
                sb_putc(body, c);
                if (c == '"' && !is_escaped(s, i)) st = MB_CODE;
            } else if (st == MB_SQ) {
                sb_putc(body, c);
                if (c == '\'' && !is_escaped(s, i)) st = MB_CODE;
            } else if (st == MB_SL) {
                sb_putc(body, c);
            } else if (st == MB_BL) {
                sb_putc(body, c);
                if (c == '/' && s[i+1] == '*') { bl_depth++; sb_putc(body, s[++i]); continue; }
                if (c == '*' && s[i+1] == '/') {
                    bl_depth--; sb_putc(body, s[++i]);
                    if (bl_depth <= 0) { bl_depth = 0; st = MB_CODE; }
                }
            }
        }
        if (st == MB_SL) st = MB_CODE;
        if (has_cur) { sb_free(&cur); has_cur = false; }
        if (!read_line(in, &cur)) return false;
        has_cur = true;
        if (line_no) { (*line_no)++; g_cur_line = *line_no; }
        sb_putc(body, '\n');
        s = cur.buf ? cur.buf : "";
    }
}

static bool line_ends_with_ident(const char *s, const char *ident) {
    if (!s || !ident) return false;
    size_t slen = strlen(s);
    while (slen > 0 && isspace((unsigned char)s[slen - 1])) slen--;
    size_t ilen = strlen(ident);
    if (slen < ilen) return false;
    if (strncmp(s + slen - ilen, ident, ilen) != 0) return false;
    if (slen > ilen) {
        char prev = s[slen - ilen - 1];
        if (isalnum((unsigned char)prev) || prev == '_') return false;
    }
    return true;
}


static bool line_ends_with_func_macro(const char *s, const MacroTable *t) {
    if (!s || !t) return false;
    enum { LM_CODE, LM_DQ, LM_SQ, LM_SL, LM_BL } st = (g_block_comment_depth > 0) ? LM_BL : LM_CODE;
    int bl_depth = g_block_comment_depth;
    const char *last = NULL;
    size_t last_len = 0;
    int last_tok = 0; // 0 none, 1 ident, 2 other

    for (size_t i = 0; s[i]; ++i) {
        char c = s[i];
        if (st == LM_CODE) {
            if (c == '"') { st = LM_DQ; last_tok = 2; continue; }
            if (c == '\'') { st = LM_SQ; last_tok = 2; continue; }
            if (c == '/' && s[i+1] == '/') { st = LM_SL; break; }
            if (c == '/' && s[i+1] == '*') { st = LM_BL; bl_depth++; i++; continue; }
            if (isspace((unsigned char)c)) continue;
            if (isalpha((unsigned char)c) || c == '_') {
                size_t j = i + 1;
                while (s[j] && (isalnum((unsigned char)s[j]) || s[j] == '_')) j++;
                last = s + i;
                last_len = j - i;
                last_tok = 1;
                i = j - 1;
                continue;
            }
            last_tok = 2;
            continue;
        } else if (st == LM_DQ) {
            if (c == '"' && !is_escaped(s, i)) st = LM_CODE;
        } else if (st == LM_SQ) {
            if (c == '\'' && !is_escaped(s, i)) st = LM_CODE;
        } else if (st == LM_SL) {
            break;
        } else if (st == LM_BL) {
            if (c == '/' && s[i+1] == '*') { bl_depth++; i++; continue; }
            if (c == '*' && s[i+1] == '/') {
                bl_depth--; i++;
                if (bl_depth <= 0) { bl_depth = 0; st = LM_CODE; }
                continue;
            }
        }
    }

    if (last_tok != 1 || !last || last_len == 0) return false;
    char name[256];
    size_t n = last_len < sizeof name - 1 ? last_len : sizeof name - 1;
    memcpy(name, last, n);
    name[n] = '\0';
    Macro *m = mtable_find((MacroTable*)t, name);
    return m && m->is_func;
}

static long count_lines_in_str(const char *s) {
    if (!s || !*s) return 1;
    long n = 1;
    for (const char *p = s; *p; ++p) {
        if (*p == '\n') n++;
    }
    return n;
}

static void emit_line_directive(FILE *out, long line, const char *path) {
    if (!out || !path || !*path) return;
    if (line < 1) line = 1;
    fputs("#line ", out);
    fprintf(out, "%ld \"", line);
    for (const char *p = path; *p; ++p) {
        if (*p == '\\' || *p == '"') fputc('\\', out);
        fputc(*p, out);
    }
    fputs("\"\n", out);
}

static bool maybe_inject_xen_types(FILE *out, const char *this_path,
                                   const char *curdir, const PPOpts *opts) {
    if (!out || g_injected_xen_types) return false;
    if (!this_path || !strstr(this_path, "/xen/")) return false;
    char *opened = NULL;
    FILE *fxen = open_in_search("xen/interface/xen.h", false, curdir, opts, &opened);
    if (fxen) {
        fclose(fxen);
        free(opened);
        return false;
    }
    fxen = open_in_search("xen/xen.h", false, curdir, opts, &opened);
    if (fxen) {
        fclose(fxen);
        free(opened);
        return false;
    }
    fputs("/* ccpp: xen typedefs */\n", out);
    fputs("typedef unsigned short domid_t;\n", out);
    fputs("typedef unsigned int grant_ref_t;\n", out);
    fputs("typedef unsigned long xen_pfn_t;\n", out);
    g_injected_xen_types = true;
    return true;
}

static void preprocess_file(const char *path, FILE *in, const char *curdir, const PPOpts *opts, FILE *out, MacroTable *ptbl);

static void preprocess(FILE *in, FILE *out, const PPOpts *opts, const char *curdir, const char *this_path, MacroTable *ptbl)
{
    MacroTable tbl_local;
    MacroTable *tbl = ptbl;
    if (!tbl) { tbl = &tbl_local; mtable_init(tbl); }
    if (g_preprocess_depth == 0) g_injected_xen_types = false;
    g_preprocess_depth++;
    // Set comment handling for this run
    bool prev_keep_comments = g_keep_comments;
    int prev_block_comment_depth = g_block_comment_depth;
    int prev_hidden_block_comment_depth = g_hidden_block_comment_depth;
    g_keep_comments = opts && opts->keep_comments;
    if (g_keep_comments && this_path) {
        if (strstr(this_path, "/usr/include") ||
            strstr(this_path, "/usr/local/include") ||
            strstr(this_path, "/opt/homebrew/include") ||
            strstr(this_path, "/usr/lib/clang") ||
            strstr(this_path, "MacOSX.sdk/usr/include")) {
            g_keep_comments = false;
        }
    }
    g_block_comment_depth = 0;
    g_hidden_block_comment_depth = 0;
    const PPOpts *prev_ifexpr_opts = g_ifexpr_opts;
    const char *prev_ifexpr_curdir = g_ifexpr_curdir;
    const char *prev_ifexpr_this_path = g_ifexpr_this_path;
    g_ifexpr_opts = opts;
    g_ifexpr_curdir = curdir;
    g_ifexpr_this_path = this_path;
    const char *prev_file = g_cur_file;
    char *line_file_override = NULL;
    long prev_line = g_cur_line;
    g_cur_file = this_path;
    g_cur_line = 0;

    if (this_path && *this_path) {
        emit_line_directive(out, 1, this_path);
    }
    if (maybe_inject_xen_types(out, this_path, curdir, opts)) {
        if (this_path && *this_path) {
            emit_line_directive(out, 1, this_path);
        }
    }

    // Apply predefined + command-line macros for top-level preprocess only
    if (!ptbl) {
        apply_predefined_macros(tbl, opts);
    }
    // Push current file onto include stack (to detect cycles)
    bool pushed_this = false;
    if (this_path && *this_path && opts) {
        PPOpts *mopts = (PPOpts*)opts; // cast away const locally
        mopts->open_stack = (const char**)xrealloc((void*)mopts->open_stack, sizeof(char*)*(mopts->nopen_stack+1));
        mopts->open_stack[mopts->nopen_stack++] = xstrdup(this_path);
        pushed_this = true;
    }

    Str line;
    Str prev; bool has_prev = false; sb_init(&prev);
    // Heuristic to help Come/"using C" environments: inject missing libc prototypes
    bool pending_using_c = false;
    bool injected_libc_protos = false;
    bool injected_libc_typedefs = false;
    bool skipping_sockaddr_union = false;
    bool sockaddr_union_const = false;
    bool in_c_include_block = false;
    int c_include_brace_depth = 0;
    IfCtx *istk = NULL; size_t ilen = 0, icap = 0;
    long line_no = 0;
    bool need_line_directive = false;
    Str lookahead; bool has_lookahead = false; sb_init(&lookahead);
    while (1) {
        if (has_lookahead) {
            line = lookahead;
            has_lookahead = false;
            sb_init(&lookahead);
        } else {
            if (!read_line(in, &line)) break;
        }
        line_no++;
        g_cur_line = line_no;
        char *raw = line.buf ? line.buf : "";
        if (line_ends_with_ident(raw, "__attribute_deprecated_msg__")) {
            Str next; sb_init(&next);
            if (read_line(in, &next)) {
                const char *q = next.buf ? next.buf : "";
                while (*q && isspace((unsigned char)*q)) q++;
                if (*q == '(') {
                    sb_putc(&line, '\n');
                    sb_puts(&line, next.buf ? next.buf : "");
                    sb_free(&next);
                    line_no++;
                } else {
                    lookahead = next;
                    has_lookahead = true;
                }
            } else {
                sb_free(&next);
            }
        }
        raw = line.buf ? line.buf : "";
        if (in_c_include_block) {
            if (has_prev) {
                if (prev.buf) fputs(prev.buf, out);
                fputc('\n', out);
                sb_free(&prev); sb_init(&prev); has_prev = false;
            }
            fputs(raw, out);
            fputc('\n', out);
            scan_c_include_brace_depth(raw, &c_include_brace_depth);
            in_c_include_block = c_include_brace_depth > 0;
            sb_free(&line);
            continue;
        }
        normalize_trigraphs_inplace(&line);
        raw = line.buf ? line.buf : "";
        normalize_digraph_tokens_inplace(&line);
        raw = line.buf ? line.buf : "";
        bool need_line_directive_after = false;

        if (!has_lookahead && line_ends_with_func_macro(raw, tbl)) {
            Str next; sb_init(&next);
            if (read_line(in, &next)) {
                const char *q = next.buf ? next.buf : "";
                while (*q && isspace((unsigned char)*q)) q++;
                if (*q == '(') {
                    sb_putc(&line, '\n');
                    sb_puts(&line, next.buf ? next.buf : "");
                    sb_free(&next);
                    line_no++;
                } else {
                    lookahead = next;
                    has_lookahead = true;
                }
            } else {
                sb_free(&next);
            }
        }

        char *p = lskip(raw);
        bool active = (ilen == 0) ? true : istk[ilen-1].current_active;
        bool in_block_comment = (g_block_comment_depth > 0);

        bool is_directive_line = (!in_block_comment && *p == '#');
        if (is_directive_line) {
            p++; p = lskip(p);
            char *directive_body = p;
            char kw[32]; size_t adv = 0; parse_ident(p, &adv, kw, sizeof kw); p += adv; p = lskip(p);
            bool emit_directive_line = false;
            bool emitted_line_marker = false;
            bool comment_depth_handled = false;
            int directive_bl_start = g_block_comment_depth;
            if (adv == 0 && isdigit((unsigned char)*directive_body)) {
                if (active) {
                    long logical_line = 0;
                    bool has_fname = false;
                    char *parsed_fname = NULL;
                    long physical_line_no = line_no;
                    parse_line_control_payload(directive_body, true, this_path, physical_line_no,
                                               &logical_line, &has_fname, &parsed_fname);

                    if (has_fname) {
                        free(line_file_override);
                        line_file_override = xstrdup(parsed_fname ? parsed_fname : "");
                        g_cur_file = line_file_override;
                    }

                    // The next source line is numbered as logical_line.
                    line_no = logical_line - 1;
                    g_cur_line = line_no;

                    if (has_prev) {
                        if (prev.buf) fputs(prev.buf, out);
                        fputc('\n', out);
                        sb_free(&prev); sb_init(&prev); has_prev = false;
                    }
                    fprintf(out, "#line %ld", logical_line);
                    if (has_fname) {
                        Str q; sb_init(&q);
                        sb_putc(&q, ' ');
                        sb_put_escaped_cstr(&q, parsed_fname ? parsed_fname : "");
                        if (q.buf) fputs(q.buf, out);
                        sb_free(&q);
                    }
                    fputc('\n', out);
                    if (parsed_fname) free(parsed_fname);
                    emit_directive_line = true;
                    emitted_line_marker = true;
                }
            } else if (strcmp(kw, "define") == 0) {
                if (active) {
                    if (!validate_define_directive(p)) {
                        pp_directive_error(this_path, line_no, "invalid #define directive");
                    }
                    if (!process_define(tbl, raw)) {
                        if (is_system_header_path(this_path)) {
                            char namebuf[256];
                            if (parse_define_name(raw, namebuf, sizeof(namebuf))) {
                                mtable_unset(tbl, namebuf);
                                (void)process_define(tbl, raw);
                            }
                        }
                        else {
                            pp_directive_error(this_path, line_no, "incompatible macro redefinition");
                        }
                    }
                }
            } else if (strcmp(kw, "undef") == 0) {
                if (active) {
                    char id[256]; size_t a=0; parse_ident(p, &a, id, sizeof id);
                    if (a == 0 || !only_trailing_space_or_comments(p + a)) {
                        pp_directive_error(this_path, line_no, "invalid #undef directive");
                    }
                    bool is_darwin = mtable_get(tbl, "__APPLE__") != NULL;
                    if (is_darwin && (
                            strcmp(id, "__DARWIN_NFDBITS") == 0 ||
                            strcmp(id, "__DARWIN_NBBY") == 0 ||
                            strcmp(id, "_DARWIN_C_SOURCE") == 0 ||
                            strcmp(id, "_POSIX_C_SOURCE") == 0 ||
                            strcmp(id, "_CACHED_RUNES") == 0)) {
                        // keep predefined Darwin compatibility macros
                    } else {
                        mtable_unset(tbl, id);
                    }
                }
            } else if (strcmp(kw, "module") == 0) {
                char namebuf[256];
                bool is_func = false;
                char **params = NULL;
                size_t nparams = 0;
                bool variadic = false;
                char *var_param = NULL;
                const char *after_sig = NULL;
                bool ok_sig = parse_module_signature(p, namebuf, sizeof namebuf, &is_func,
                                                     &params, &nparams, &variadic,
                                                     &var_param, &after_sig);
                if (ok_sig) {
                    const char *brace = find_first_open_brace(after_sig);
                    Str brace_line; bool brace_owned = false; sb_init(&brace_line);
                    while (!brace) {
                        if (!read_line(in, &brace_line)) break;
                        brace_owned = true;
                        line_no++;
                        g_cur_line = line_no;
                        brace = find_first_open_brace(brace_line.buf ? brace_line.buf : "");
                        if (!brace) { sb_free(&brace_line); brace_owned = false; }
                    }
                    if (brace) {
                        Str body; sb_init(&body);
                        const char *start = brace + 1;
                        bool ok_body = collect_module_body(in, start, &line_no, &body);
                        if (brace_owned) sb_free(&brace_line);
                        if (ok_body) {
                            if (body.buf) {
                                strip_comments_multiline_inplace(body.buf);
                                size_t L = strlen(body.buf);
                                while (L && isspace((unsigned char)body.buf[L-1])) body.buf[--L] = '\0';
                                body.len = L;
                            }
                            if (active) {
                                if (is_func) {
                                    mtable_set_func(tbl, namebuf, params, nparams, variadic, var_param,
                                                    body.buf ? body.buf : "");
                                } else {
                                    mtable_set_obj(tbl, namebuf, body.buf ? body.buf : "");
                                    if (params) { for (size_t i=0; i<nparams; ++i) free(params[i]); free(params); }
                                    if (var_param) free(var_param);
                                }
                            } else {
                                if (params) { for (size_t i=0; i<nparams; ++i) free(params[i]); free(params); }
                                if (var_param) free(var_param);
                            }
                        } else {
                            if (params) { for (size_t i=0; i<nparams; ++i) free(params[i]); free(params); }
                            if (var_param) free(var_param);
                        }
                        sb_free(&body);
                    } else {
                        if (brace_owned) sb_free(&brace_line);
                        if (params) { for (size_t i=0; i<nparams; ++i) free(params[i]); free(params); }
                        if (var_param) free(var_param);
                    }
                }
            } else if (strcmp(kw, "include") == 0 || strcmp(kw, "include_next") == 0) {
                if (active) {
                    // Flush buffered previous line before including
                    if (has_prev) {
                        if (prev.buf) fputs(prev.buf, out);
                        fputc('\n', out);
                        sb_free(&prev); sb_init(&prev); has_prev = false;
                    }
                    // Parse header name and validate directive form.
                    // For #include MACRO form, macro-expand first and then parse
                    // as header-name.
                    const char *hp = p;
                    while (*hp && isspace((unsigned char)*hp)) hp++;
                    Str include_expanded;
                    bool use_include_expanded = false;
                    if (*hp != '"' && *hp != '<') {
                        use_include_expanded = true;
                        sb_init(&include_expanded);
                        expand_into_str(tbl, hp, &include_expanded);
                        for (int pass = 0; pass < PP_RESCAN_MAX; ++pass) {
                            Str next;
                            sb_init(&next);
                            expand_into_str(tbl, include_expanded.buf ? include_expanded.buf : "", &next);
                            bool same = ((include_expanded.buf == NULL && next.buf == NULL) ||
                                         (include_expanded.buf && next.buf && strcmp(include_expanded.buf, next.buf) == 0));
                            sb_free(&include_expanded);
                            include_expanded = next;
                            if (same) break;
                        }
                        hp = include_expanded.buf ? include_expanded.buf : "";
                        while (*hp && isspace((unsigned char)*hp)) hp++;
                    }
                    bool quoted = false;
                    bool include_form_ok = false;
                    char header[512];
                    size_t hi = 0;
                    if (*hp == '"') {
                        quoted = true;
                        include_form_ok = true;
                        hp++;
                        while (*hp && *hp != '"') {
                            if (hi + 1 < sizeof header) header[hi++] = *hp;
                            hp++;
                        }
                        if (*hp != '"') include_form_ok = false;
                        else hp++;
                    } else if (*hp == '<') {
                        include_form_ok = true;
                        hp++;
                        while (*hp && *hp != '>') {
                            if (hi + 1 < sizeof header) header[hi++] = *hp;
                            hp++;
                        }
                        if (*hp != '>') include_form_ok = false;
                        else hp++;
                    }
                    header[hi]='\0';
                    if (!include_form_ok || hi == 0 || !only_trailing_space_or_comments(hp)) {
                        if (use_include_expanded) sb_free(&include_expanded);
                        pp_directive_error(this_path, line_no, "invalid #include directive");
                    }
                    char *opened_path=NULL;
                    FILE *f = NULL;
                    if (strcmp(kw, "include") == 0) {
                        f = open_in_search(header, quoted, curdir, opts, &opened_path);
                    } else {
                        f = open_in_search_next(header, this_path, opts, &opened_path);
                    }
                    if (f) {
                        // check pragma once cache
                        bool skip = false;
                        for (size_t oi=0; oi<(opts?opts->nonce_paths:0); ++oi) {
                            if (strcmp(opts->once_paths[oi], opened_path)==0) { skip = true; break; }
                        }
                        // also skip if this path is already in the current include stack (cycle)
                        if (!skip && opts && opts->nopen_stack > 0) {
                            for (size_t si=0; si<opts->nopen_stack; ++si) {
                                if (strcmp(opts->open_stack[si], opened_path) == 0) { skip = true; break; }
                            }
                        }
                        if (!skip) {
                            char *ndir = dirname_dup(opened_path);
                            preprocess_file(opened_path, f, ndir, opts, out, tbl);
                            fclose(f);
                            free(ndir);
                            if (this_path && *this_path) {
                                emit_line_directive(out, line_no + 1, this_path);
                                emitted_line_marker = true;
                            }
                        }
                        free(opened_path);
                    } else {
                        if (use_include_expanded) sb_free(&include_expanded);
                        bool strict_missing = false;
                        const char *strict_env = getenv("CCPP_STRICT_MISSING_INCLUDE");
                        if (strict_env && *strict_env) strict_missing = true;
                        if (!strict_missing && !quoted) {
                            const char *suppress_w = getenv("CCPP_SUPPRESS_WARNINGS");
                            if (!(suppress_w && *suppress_w)) {
                                fprintf(stderr, "warning: %s %ld(%ld): include file not found: <%s>\n",
                                        this_path ? this_path : "<stdin>",
                                        line_no > 0 ? line_no : 1,
                                        g_cur_line > 0 ? g_cur_line : (line_no > 0 ? line_no : 1),
                                        header);
                            }
                        } else {
                            pp_directive_error(this_path, line_no, "include file not found");
                        }
                    }
                    if (use_include_expanded) sb_free(&include_expanded);
                }
            } else if (strcmp(kw, "line") == 0) {
                if (active) {
                    long physical_line_no = line_no;
                    Str line_expanded; sb_init(&line_expanded);
                    expand_into_str(tbl, p, &line_expanded);
                    for (int pass = 0; pass < PP_RESCAN_MAX; ++pass) {
                        Str next;
                        sb_init(&next);
                        expand_into_str(tbl, line_expanded.buf ? line_expanded.buf : "", &next);
                        bool same = ((line_expanded.buf == NULL && next.buf == NULL) ||
                                     (line_expanded.buf && next.buf && strcmp(line_expanded.buf, next.buf) == 0));
                        sb_free(&line_expanded);
                        line_expanded = next;
                        if (same) break;
                    }

                    long logical_line = 0;
                    bool has_fname = false;
                    char *parsed_fname = NULL;
                    parse_line_control_payload(line_expanded.buf ? line_expanded.buf : "",
                                               false, this_path, physical_line_no,
                                               &logical_line, &has_fname, &parsed_fname);
                    sb_free(&line_expanded);

                    if (has_fname) {
                        free(line_file_override);
                        line_file_override = xstrdup(parsed_fname ? parsed_fname : "");
                        g_cur_file = line_file_override;
                    }

                    // The next source line is numbered as logical_line.
                    line_no = logical_line - 1;
                    g_cur_line = line_no;

                    if (has_prev) {
                        if (prev.buf) fputs(prev.buf, out);
                        fputc('\n', out);
                        sb_free(&prev); sb_init(&prev); has_prev = false;
                    }
                    fprintf(out, "#line %ld", logical_line);
                    if (has_fname) {
                        Str q; sb_init(&q);
                        sb_putc(&q, ' ');
                        sb_put_escaped_cstr(&q, parsed_fname ? parsed_fname : "");
                        if (q.buf) fputs(q.buf, out);
                        sb_free(&q);
                    }
                    fputc('\n', out);
                    if (parsed_fname) free(parsed_fname);
                    emit_directive_line = true;
                    emitted_line_marker = true;
                }
            } else if (strcmp(kw, "warning") == 0) {
                if (active) {
                    const char *suppress_w = getenv("CCPP_SUPPRESS_WARNINGS");
                    bool in_sys = false;
                    if (this_path) {
                        if (strstr(this_path, "/usr/include") || strstr(this_path, "MacOSX.sdk/usr/include")) in_sys = true;
                    }
                    if (!(suppress_w && *suppress_w) && !in_sys) {
                        pp_directive_warning(this_path, line_no, (p && *p) ? p : "#warning");
                    }
                }
            } else if (strcmp(kw, "error") == 0) {
                if (active) {
                    const char *relax_env = getenv("CCPP_RELAX_ERRORS");
                    const char *suppress_demoted = getenv("CCPP_SUPPRESS_DEMOTED_ERRORS");
                    bool in_sys = false;
                    if (this_path) {
                        if (strstr(this_path, "/usr/include") || strstr(this_path, "MacOSX.sdk/usr/include")) in_sys = true;
                    }
                    // In system headers, suppress demoted errors by default (no stderr noise).
                    if (in_sys) {
                        (void)suppress_demoted; // no-op, intentionally quiet for system headers
                        (void)relax_env;
                        // do nothing
                    } else if (relax_env) {
                        if (!suppress_demoted) {
                            pp_directive_warning(this_path, line_no, (p && *p) ? p : "error(demoted)");
                        }
                    } else {
                        pp_directive_error(this_path, line_no, (p && *p) ? p : "#error");
                    }
                }
            } else if (strcmp(kw, "ifdef") == 0) {
                char id[256]; size_t a=0; parse_ident(p, &a, id, sizeof id);
                if (a == 0 || !only_trailing_space_or_comments(p + a)) {
                    pp_directive_error(this_path, line_no, "invalid #ifdef directive");
                }
                bool cond = mtable_get(tbl, id) != NULL;
                if (ilen == icap) { icap = icap? icap*2:8; istk = xrealloc(istk, icap*sizeof(IfCtx)); }
                bool parent = active;
                bool curr = parent && cond;
                istk[ilen++] = (IfCtx){ parent, curr, curr, false };
            } else if (strcmp(kw, "ifndef") == 0) {
                char id[256]; size_t a=0; parse_ident(p, &a, id, sizeof id);
                if (a == 0 || !only_trailing_space_or_comments(p + a)) {
                    pp_directive_error(this_path, line_no, "invalid #ifndef directive");
                }
                bool cond = mtable_get(tbl, id) == NULL;
                if (ilen == icap) { icap = icap? icap*2:8; istk = xrealloc(istk, icap*sizeof(IfCtx)); }
                bool parent = active;
                bool curr = parent && cond;
                istk[ilen++] = (IfCtx){ parent, curr, curr, false };
            } else if (strcmp(kw, "if") == 0) {
                bool parent = active;
                bool cond = false;
                if (parent) {
                    long base_line = g_cur_line;
                    Str exln; sb_init(&exln);
                    int if_bl_start = g_block_comment_depth;
                    g_ifexpr_builtin_error = false;
                    g_cur_line = base_line;
                    g_expand_pass = 0;
                    g_block_comment_depth = if_bl_start;
                    expand_into_str_ifexpr(tbl, p, &exln);
                    int if_bl_depth = g_block_comment_depth;
                    for (int pass = 0; pass < PP_RESCAN_MAX; ++pass) {
                        Str next; sb_init(&next);
                        g_cur_line = base_line;
                        g_expand_pass = pass + 1;
                        g_block_comment_depth = if_bl_start;
                        expand_into_str_ifexpr(tbl, exln.buf ? exln.buf : "", &next);
                        g_block_comment_depth = if_bl_depth;
                        bool same = ((exln.buf == NULL && next.buf == NULL) || (exln.buf && next.buf && strcmp(exln.buf, next.buf) == 0));
                        sb_free(&exln);
                        exln = next;
                        if (same) break;
                    }
                    Expr ex = { .s = exln.buf ? exln.buf : "", .err = false };
                    ExValue v = ex_parse_expr(tbl, &ex);
                    ex.s = ex_skip(ex.s);
                    bool invalid_if_expr = g_ifexpr_builtin_error || ex.err || (ex.s && *ex.s != '\0');
                    if (invalid_if_expr) {
                        const char *dbg = getenv("CCPP_DEBUG_IFEXPR");
                        if (dbg && *dbg) {
                            fprintf(stderr, "ccpp debug: #if expr: %s\n", exln.buf ? exln.buf : "");
                        }
                    }
                    sb_free(&exln);
                    if (invalid_if_expr) {
                        pp_directive_error(this_path, line_no, "invalid #if expression");
                    }
                    cond = ex_truthy(v);
                    // Keep block-comment depth in sync from the raw directive line.
                    // This avoids both double-counting (via expansion side effects)
                    // and missing cross-line comments that start on #if/#elif lines.
                    g_block_comment_depth = if_bl_start;
                    scan_comment_depth(raw, &g_block_comment_depth);
                    comment_depth_handled = true;
                }
                if (ilen == icap) { icap = icap? icap*2:8; istk = xrealloc(istk, icap*sizeof(IfCtx)); }
                bool curr = parent && cond;
                istk[ilen++] = (IfCtx){ parent, curr, curr, false };
            } else if (strcmp(kw, "elif") == 0) {
                if (ilen == 0) {
                    pp_directive_error(this_path, line_no, "#elif without matching #if");
                }
                IfCtx *c = &istk[ilen-1];
                if (c->saw_else) {
                    pp_directive_error(this_path, line_no, "#elif after #else");
                }
                if (c->taken) {
                    c->current_active = false;
                } else if (!c->parent_active) {
                    c->current_active = false;
                } else {
                    long base_line = g_cur_line;
                    Str exln; sb_init(&exln);
                    int if_bl_start = g_block_comment_depth;
                    g_ifexpr_builtin_error = false;
                    g_cur_line = base_line;
                    g_expand_pass = 0;
                    g_block_comment_depth = if_bl_start;
                    expand_into_str_ifexpr(tbl, p, &exln);
                    int if_bl_depth = g_block_comment_depth;
                    for (int pass = 0; pass < PP_RESCAN_MAX; ++pass) {
                        Str next; sb_init(&next);
                        g_cur_line = base_line;
                        g_expand_pass = pass + 1;
                        g_block_comment_depth = if_bl_start;
                        expand_into_str_ifexpr(tbl, exln.buf ? exln.buf : "", &next);
                        g_block_comment_depth = if_bl_depth;
                        bool same = ((exln.buf == NULL && next.buf == NULL) || (exln.buf && next.buf && strcmp(exln.buf, next.buf) == 0));
                        sb_free(&exln);
                        exln = next;
                        if (same) break;
                    }
                    Expr ex = { .s = exln.buf ? exln.buf : "", .err = false };
                    ExValue v = ex_parse_expr(tbl, &ex);
                    ex.s = ex_skip(ex.s);
                    bool invalid_if_expr = g_ifexpr_builtin_error || ex.err || (ex.s && *ex.s != '\0');
                    if (invalid_if_expr) {
                        const char *dbg = getenv("CCPP_DEBUG_IFEXPR");
                        if (dbg && *dbg) {
                            fprintf(stderr, "ccpp debug: #elif expr: %s\n", exln.buf ? exln.buf : "");
                        }
                    }
                    sb_free(&exln);
                    if (invalid_if_expr) {
                        pp_directive_error(this_path, line_no, "invalid #if expression");
                    }
                    bool cond = ex_truthy(v);
                    c->current_active = c->parent_active && cond;
                    if (c->current_active) c->taken = true;
                    // Keep block-comment depth in sync from the raw directive line.
                    g_block_comment_depth = if_bl_start;
                    scan_comment_depth(raw, &g_block_comment_depth);
                    comment_depth_handled = true;
                }
            } else if (strcmp(kw, "else") == 0) {
                if (ilen == 0) {
                    pp_directive_error(this_path, line_no, "#else without matching #if");
                }
                IfCtx *c = &istk[ilen-1];
                if (c->saw_else) {
                    pp_directive_error(this_path, line_no, "duplicate #else");
                }
                c->current_active = c->parent_active && !c->taken;
                if (c->current_active) c->taken = true;
                c->saw_else = true;
            } else if (strcmp(kw, "endif") == 0) {
                if (ilen == 0) {
                    pp_directive_error(this_path, line_no, "#endif without matching #if");
                }
                ilen--;
            } else if (strcmp(kw, "pragma") == 0) {
                // Handle #pragma
                char w2[32]; size_t a2=0; parse_ident(p, &a2, w2, sizeof w2);
                if (strcmp(w2, "once") == 0 && this_path && *this_path) {
                    // Track #pragma once to avoid re-including this file
                    PPOpts *mopts = (PPOpts*)opts; // cast away const; acceptable within our program
                    mopts->once_paths = (const char**)xrealloc((void*)mopts->once_paths, sizeof(char*)*(mopts->nonce_paths+1));
                    mopts->once_paths[mopts->nonce_paths++] = xstrdup(this_path);
                } else if (active) {
                    // Preserve non-"once" pragmas by default for compatibility
                    // with downstream tools (including comelang). To explicitly
                    // suppress them, set CCPP_PRESERVE_PRAGMAS=0 in the env.
                    const char *preserve = getenv("CCPP_PRESERVE_PRAGMAS");
                    bool do_preserve = (!preserve || strcmp(preserve, "0") != 0);
                    if (do_preserve) {
                        if (has_prev) {
                            if (prev.buf) fputs(prev.buf, out);
                            fputc('\n', out);
                            sb_free(&prev); sb_init(&prev); has_prev = false;
                        }
                        fputs(raw, out); fputc('\n', out);
                        emit_directive_line = true;
                    }
                }
            } else {
                // Unknown directive: ignore
            }
            if (!emit_directive_line) {
                // Track block-comment depth across directive lines without
                // discarding subsequent source lines.
                if (!comment_depth_handled) {
                    scan_comment_depth(raw, &g_block_comment_depth);
                }
                if (g_keep_comments) {
                    update_hidden_comment_depth(directive_bl_start, g_block_comment_depth);
                }
            }
            if (emitted_line_marker) {
                need_line_directive = false;
            } else {
                need_line_directive = true;
            }
            sb_free(&line);
            continue; // do not emit directive lines
        }

        if (!active) {
            // Even in inactive branches, keep comment depth in sync so we don't
            // misinterpret commented-out directive-looking lines.
            int inactive_bl_start = g_block_comment_depth;
            scan_comment_depth(raw, &g_block_comment_depth);
            if (g_keep_comments) {
                update_hidden_comment_depth(inactive_bl_start, g_block_comment_depth);
            }
            need_line_directive = true;
            sb_free(&line);
            continue;
        }

        if (starts_c_include_block(raw)) {
            if (has_prev) {
                if (prev.buf) fputs(prev.buf, out);
                fputc('\n', out);
                sb_free(&prev); sb_init(&prev); has_prev = false;
            }
            fputs(raw, out);
            fputc('\n', out);
            c_include_brace_depth = 0;
            scan_c_include_brace_depth(raw, &c_include_brace_depth);
            in_c_include_block = c_include_brace_depth > 0;
            sb_free(&line);
            continue;
        }

        // Merge lines when parentheses stay open (macro calls can span newlines).
        int paren_depth = 0;
        int scan_bl_depth = g_block_comment_depth;
        scan_paren_state(raw, &paren_depth, &scan_bl_depth);
        while (paren_depth > 0) {
            Str nextline;
            if (!read_line(in, &nextline)) break;
            line_no++;
            sb_putc(&line, '\n');
            if (nextline.buf) sb_puts(&line, nextline.buf);
            scan_paren_state(nextline.buf ? nextline.buf : "", &paren_depth, &scan_bl_depth);
            sb_free(&nextline);
            raw = line.buf ? line.buf : "";
        }
        long raw_lines = count_lines_in_str(raw);

        // Reinforce key Darwin macros before expanding code lines, in case headers undef them
        if (mtable_get(tbl, "__APPLE__")) {
            mtable_set_obj(tbl, "_DARWIN_C_SOURCE", "1");
            mtable_set_obj(tbl, "_POSIX_C_SOURCE", "200809L");
            mtable_set_obj(tbl, "__DARWIN_NBBY", "8");
            mtable_set_obj(tbl, "__DARWIN_NFDBITS", "32");
            mtable_set_obj(tbl, "__DARWIN_NULL", "0");
        }

        int line_bl_start = g_block_comment_depth;
        long base_line = g_cur_line;
        Str outln; sb_init(&outln);
        g_cur_line = base_line;
        g_expand_pass = 0;
        expand_into_str(tbl, raw, &outln);
        int line_bl_depth = g_block_comment_depth;
        // Rescan to expand nested macros (with iteration cap to avoid oscillation)
        for (int pass = 0; pass < PP_RESCAN_MAX; ++pass) {
            Str next; sb_init(&next);
            g_cur_line = base_line;
            g_expand_pass = pass + 1;
            g_block_comment_depth = line_bl_start;
            expand_into_str(tbl, outln.buf ? outln.buf : "", &next);
            g_block_comment_depth = line_bl_depth;
            bool same = ((outln.buf == NULL && next.buf == NULL) || (outln.buf && next.buf && strcmp(outln.buf, next.buf) == 0));
            sb_free(&outln);
            outln = next;
            if (same) break;
        }
        // Macro expansion strips comments, so keep the global block-comment
        // depth synchronized from the original source line. Otherwise a
        // one-line block comment can leave following directives hidden.
        g_block_comment_depth = line_bl_start;
        scan_comment_depth(raw, &g_block_comment_depth);
        // Darwin-specific fallback: if some headers referenced _CACHED_RUNES as
        // an enum constant rather than macro, downstream non-C compilers may
        // fail. If we still see the token here, replace occurrences in code
        // with 256. We honor string/char/comment contexts.
        if (mtable_get(tbl, "__APPLE__") && outln.buf && strstr(outln.buf, "_CACHED_RUNES")) {
            enum { S_CODE, S_DQ, S_SQ, S_SL, S_BL } st = S_CODE;
            int bl_depth = 0;
            Str rep; sb_init(&rep);
            for (size_t i=0; outln.buf[i]; ) {
                char c = outln.buf[i];
                if (st == S_CODE) {
                    if (c=='"') { st=S_DQ; sb_putc(&rep,c); i++; continue; }
                    if (c=='\'') { st=S_SQ; sb_putc(&rep,c); i++; continue; }
                    if (c=='/' && outln.buf[i+1]=='/') { st=S_SL; sb_puts(&rep,"//"); i+=2; continue; }
                    if (c=='/' && outln.buf[i+1]=='*') { st=S_BL; bl_depth = 1; sb_puts(&rep,"/*"); i+=2; continue; }
                    if (isalpha((unsigned char)c) || c=='_') {
                        char id[256]; size_t j=0; size_t k=i;
                        while (outln.buf[k] && (isalnum((unsigned char)outln.buf[k]) || outln.buf[k]=='_')) { if (j+1<sizeof id) id[j++]=outln.buf[k]; k++; }
                        id[j]='\0';
                        if (strcmp(id, "_CACHED_RUNES") == 0) {
                            sb_puts(&rep, "256");
                            i = k; continue;
                        }
                        while (i<k) { sb_putc(&rep, outln.buf[i++]); }
                        continue;
                    }
                    sb_putc(&rep, c); i++;
                } else if (st == S_DQ) { sb_putc(&rep,c); if (c=='"' && outln.buf[i-1] != '\\') st=S_CODE; i++; }
                else if (st == S_SQ) { sb_putc(&rep,c); if (c=='\'' && outln.buf[i-1] != '\\') st=S_CODE; i++; }
                else if (st == S_SL) { sb_putc(&rep,c); i++; }
                else if (st == S_BL) {
                    if (c=='/' && outln.buf[i+1]=='*') { bl_depth++; sb_puts(&rep, "/*"); i+=2; continue; }
                    if (c=='*' && outln.buf[i+1]=='/') {
                        bl_depth--; sb_puts(&rep, "*/"); i+=2;
                        if (bl_depth <= 0) { bl_depth = 0; st = S_CODE; }
                        continue;
                    }
                    sb_putc(&rep, c); i++;
                }
            }
            sb_free(&outln);
            outln = rep;
        }
        if (g_cur_file && strstr(g_cur_file, "/netinet/ip6.h") &&
            outln.buf && strstr(outln.buf, "ip6_hdrctl")) {
            replace_ident_token(&outln, "ip6_hdrctl", "");
        }
        if (outln.buf && strstr(outln.buf, "_Complex")) {
            rewrite_complex_postfix(&outln);
        }
        if (g_cur_file && strstr(g_cur_file, "/linux/if_tun.h") &&
            outln.buf && strstr(outln.buf, "[][")) {
            rewrite_empty_array_dim(&outln);
        }
        if (g_cur_file && strstr(g_cur_file, "/netinet/ip_icmp.h") && outln.buf &&
            strstr(outln.buf, "struct")) {
            strip_struct_tag(&outln, "ih_idseq");
            strip_struct_tag(&outln, "ih_pmtu");
            strip_struct_tag(&outln, "ih_rtradv");
        }
        strip_extension_tokens(&outln);
        rewrite_anonymous_typedef_tag(&outln);
        if (outln.buf) {
            const char *q = clskip(outln.buf);
            if (skipping_sockaddr_union) {
                if (strstr(q, "__SOCKADDR_ARG") || strstr(q, "__CONST_SOCKADDR_ARG")) {
                    sb_free(&outln);
                    sb_init(&outln);
                    if (sockaddr_union_const) {
                        sb_puts(&outln, "typedef const struct sockaddr *__restrict __CONST_SOCKADDR_ARG;");
                    } else {
                        sb_puts(&outln, "typedef struct sockaddr *__restrict __SOCKADDR_ARG;");
                    }
                    skipping_sockaddr_union = false;
                    sockaddr_union_const = false;
                } else {
                    sb_free(&outln);
                    sb_free(&line);
                    need_line_directive = true;
                    continue;
                }
            } else if (starts_with(q, "typedef") && strstr(q, "union") &&
                       strstr(q, "__sockaddr__") && strstr(q, "struct sockaddr")) {
                skipping_sockaddr_union = true;
                sockaddr_union_const = (strstr(q, "const struct sockaddr") != NULL);
                sb_free(&outln);
                sb_free(&line);
                need_line_directive = true;
                continue;
            }
        }
        // Drop stray lines that are only _Pragma payloads that survived macro juggling:
        // pattern: optional space + "..." + optional space + ')' + optional space
        if (outln.buf) {
            const char *q = outln.buf; while (*q && isspace((unsigned char)*q)) q++;
            if (*q == '"') {
                // walk string literal
                q++;
                while (*q) {
                    if (*q == '"' && q[-1] != '\\') { q++; break; }
                    q++;
                }
                const char *r = q; while (*r && isspace((unsigned char)*r)) r++;
                if (*r == ')' ) {
                    r++; while (*r && isspace((unsigned char)*r)) r++;
                    if (*r == '\0') {
                        // skip emitting this line entirely
                        sb_free(&outln);
                        sb_free(&line);
                        need_line_directive = true;
                        continue;
                    }
                }
            }
        }
        // Track "using C" block start to optionally inject prototypes
        // Detect a line that is exactly "using C" (ignoring surrounding spaces)
        if (outln.buf) {
            const char *q1 = outln.buf; while (*q1 && isspace((unsigned char)*q1)) q1++;
            const char *qend = q1 + strlen(q1);
            while (qend > q1 && isspace((unsigned char)qend[-1])) qend--;
            size_t qlen = (size_t)(qend - q1);
            if (qlen == 7 && strncmp(q1, "using C", 7) == 0) {
                pending_using_c = true;
            } else if (pending_using_c && qlen == 1 && q1[0] == '{' &&
                       (!injected_libc_protos || !injected_libc_typedefs)) {
                // Optionally inject minimal libc prototypes frequently needed by comelang
                // in using C blocks. Controlled via CCPP_INJECT_LIBC_PROTOS (default on).
                if (!injected_libc_protos) {
                    const char *inj = getenv("CCPP_INJECT_LIBC_PROTOS");
                    bool do_inject = (!inj || strcmp(inj, "0") != 0);
                    if (do_inject) {
                        // Use unsigned long for size-like parameters to avoid depending on size_t
                        // typedefs at this point.
                        sb_puts(&outln,
                                "\nextern char *strdup(const char *);\n"
                                "extern int vsnprintf(char *, unsigned long, const char *, __builtin_va_list);\n"
                                "extern int snprintf(char *, unsigned long, const char *, ...);\n");
                        need_line_directive_after = true;
                    }
                    injected_libc_protos = true;
                }
                // Optionally inject libc typedefs that appear in glibc headers but are not
                // always recognized by the neo-c parser. Controlled via
                // CCPP_INJECT_LIBC_TYPEDEFS (default off).
                //
                // Keeping this off by default avoids overriding libc-provided typedefs
                // used by system structs (for example, sys/stat.h layouts).
                if (!injected_libc_typedefs) {
                    const char *inj_types = getenv("CCPP_INJECT_LIBC_TYPEDEFS");
                    bool do_inject_types = (inj_types && strcmp(inj_types, "0") != 0);
#ifdef CCPP_BARE
                    do_inject_types = (!inj_types || strcmp(inj_types, "0") != 0);
#endif
                    if (do_inject_types) {
                        sb_puts(&outln,
                                "\n/* ccpp: libc typedefs */\n"
                                "typedef float _Float32;\n"
                                "typedef double _Float64;\n"
                                "typedef double _Float32x;\n"
                                "typedef long double _Float64x;\n"
                                "typedef unsigned char __u_char;\n"
                                "typedef unsigned short int __u_short;\n"
                                "typedef unsigned int __u_int;\n"
                                "typedef unsigned long int __u_long;\n"
                                "typedef signed char __int8_t;\n"
                                "typedef unsigned char __uint8_t;\n"
                                "typedef signed short int __int16_t;\n"
                                "typedef unsigned short int __uint16_t;\n"
                                "typedef signed int __int32_t;\n"
                                "typedef unsigned int __uint32_t;\n"
                                "typedef signed long long int __int64_t;\n"
                                "typedef unsigned long long int __uint64_t;\n"
                                "typedef int __darwin_natural_t;\n"
                                "typedef __builtin_va_list __darwin_va_list;\n"
                                "typedef unsigned long __darwin_size_t;\n"
                                "typedef long __darwin_ssize_t;\n"
                                "typedef long __darwin_intptr_t;\n"
                                "typedef int __darwin_ct_rune_t;\n"
                                "typedef int __darwin_rune_t;\n"
                                "typedef int __darwin_wchar_t;\n"
                                "typedef int __darwin_mbstate_t;\n"
                                "typedef long __darwin_clock_t;\n"
                                "typedef int __darwin_wint_t;\n"
                                "typedef long long int __quad_t;\n"
                                "typedef unsigned long long int __u_quad_t;\n"
                                "typedef unsigned long int __ino_t;\n"
                                "typedef __uint64_t __ino64_t;\n"
                                "typedef __int64_t __off64_t;\n"
                                "typedef __off64_t __loff_t;\n"
                                "typedef struct __fsid_t { int __val[2]; } __fsid_t;\n");
                        need_line_directive_after = true;
                    }
                    injected_libc_typedefs = true;
                }
                pending_using_c = false;
            }
        }

        long out_lines = count_lines_in_str(outln.buf);
        if (out_lines != raw_lines) {
            need_line_directive_after = true;
        }

        if (has_prev) {
            if (prev.buf) fputs(prev.buf, out);
            fputc('\n', out);
            sb_free(&prev);
            sb_init(&prev);
            has_prev = false;
        }
        if (need_line_directive && this_path && *this_path) {
            emit_line_directive(out, base_line, this_path);
            need_line_directive = false;
        }
        prev = outln;
        has_prev = true;
        if (need_line_directive_after) {
            need_line_directive = true;
        }
        sb_free(&line);
    }
    if (has_prev) {
        if (prev.buf && prev.buf[0] != '\0') {
            fputs(prev.buf, out);
            fputc('\n', out);
        }
        sb_free(&prev);
    }
    if (ilen > 0) {
        pp_directive_error(this_path, line_no > 0 ? line_no : 1, "unterminated #if block");
    }
    free(istk);
    if (tbl == &tbl_local) mtable_free(tbl);

    // Pop current file from include stack
    if (pushed_this && opts && opts->nopen_stack > 0) {
        PPOpts *mopts = (PPOpts*)opts;
        size_t idx = mopts->nopen_stack - 1;
        free((void*)mopts->open_stack[idx]);
        mopts->nopen_stack--;
        // keep capacity as-is (realloc not necessary)
    }
    g_ifexpr_opts = prev_ifexpr_opts;
    g_ifexpr_curdir = prev_ifexpr_curdir;
    g_ifexpr_this_path = prev_ifexpr_this_path;
    free(line_file_override);
    g_cur_file = prev_file;
    g_cur_line = prev_line;
    g_keep_comments = prev_keep_comments;
    g_block_comment_depth = prev_block_comment_depth;
    g_hidden_block_comment_depth = prev_hidden_block_comment_depth;
    g_preprocess_depth--;
}

/*
int main(int argc, char **argv) {
    const char *path = NULL;
    PPOpts opts = {0};
    // Preserve comments by default (matches gcc -E behavior with -C implied
    // for this tool). The -C flag remains accepted (no-op) for compatibility.
    opts.keep_comments = true;
    // parse args: [-I dir]... [-DNAME[=VALUE]] [-UNAME] [-C] [input]
    for (int i=1; i<argc; ++i) {
        const char *a = argv[i];
        if (a[0]=='-' && a[1]=='I') {
            const char *dir = NULL;
            if (a[2] != '\0') dir = a+2; else if (i+1 < argc) { dir = argv[++i]; }
            if (dir) {
                opts.incdirs = (const char**)xrealloc((void*)opts.incdirs, sizeof(char*)*(opts.nincdirs+1));
                opts.incdirs[opts.nincdirs++] = dir;
            }
        } else if (a[0]=='-' && a[1]=='D') {
            const char *def = a+2;
            if (*def == '\0' && i+1 < argc) def = argv[++i];
            if (def && *def) {
                opts.cmd_defs = (const char**)xrealloc((void*)opts.cmd_defs, sizeof(char*)*(opts.ncmd_defs+1));
                opts.cmd_defs[opts.ncmd_defs++] = def;
            }
        } else if (a[0]=='-' && a[1]=='U') {
            const char *name = a+2;
            if (*name == '\0' && i+1 < argc) name = argv[++i];
            if (name && *name) {
                // Accept forms like "-U__GNUC__-v" by trimming the macro name
                // to the leading identifier portion.
                char idbuf[256]; size_t j=0; const char *p=name;
                if (!(isalpha((unsigned char)*p) || *p=='_')) {
                    strncpy(idbuf, name, sizeof idbuf - 1); idbuf[sizeof idbuf - 1] = '\0';
                } else {
                    while (*p && (isalnum((unsigned char)*p) || *p=='_')) { if (j+1<sizeof idbuf) idbuf[j++]=*p; p++; }
                    idbuf[j] = '\0';
                }
                opts.cmd_undefs = (const char**)xrealloc((void*)opts.cmd_undefs, sizeof(char*)*(opts.ncmd_undefs+1));
                opts.cmd_undefs[opts.ncmd_undefs++] = xstrdup(idbuf);
            }
        } else if (strcmp(a, "-C") == 0) {
            // Keep comments in output (accepted for compatibility). Comments are
            // already preserved by default in this tool, so this is a no-op.
            opts.keep_comments = true;
        } else if (strcmp(a, "-v") == 0) {
            // Verbose: accept and ignore for compatibility with toolchains
            ;
        } else if (a[0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", a);
            return 2;
        } else {
            path = a;
        }
    }
    FILE *in = stdin;
    const char *curdir = ".";
    if (path) {
        in = fopen(path, "r");
        if (!in) die("fopen");
        curdir = dirname_dup(path);
    }
    // populate system include paths from env and defaults
    const char *envs[] = { getenv("CPATH"), getenv("C_INCLUDE_PATH") };
    for (size_t ei=0; ei<sizeof(envs)/sizeof(envs[0]); ++ei) {
        const char *v = envs[ei];
        if (!v) continue;
        const char *p = v; const char *seg = p;
        while (*p) {
            if (*p == ':') {
                size_t L = (size_t)(p - seg);
                if (L > 0) {
                    char *dir = (char*)malloc(L+1); if (!dir) die("malloc");
                    memcpy(dir, seg, L); dir[L] = '\0';
                    opts.sysincdirs = (const char**)xrealloc((void*)opts.sysincdirs, sizeof(char*)*(opts.nsysincdirs+1));
                    opts.sysincdirs[opts.nsysincdirs++] = dir;
                }
                seg = p+1;
            }
            p++;
        }
        if (p != seg) {
            size_t L = (size_t)(p - seg);
            char *dir = (char*)malloc(L+1); if (!dir) die("malloc");
            memcpy(dir, seg, L); dir[L] = '\0';
            opts.sysincdirs = (const char**)xrealloc((void*)opts.sysincdirs, sizeof(char*)*(opts.nsysincdirs+1));
            opts.sysincdirs[opts.nsysincdirs++] = dir;
        }
    }
    // add SDKROOT if present
    const char *sdkroot = getenv("SDKROOT");
    if (sdkroot && *sdkroot) {
        char *p1 = path_join(sdkroot, "usr/include");
        opts.sysincdirs = (const char**)xrealloc((void*)opts.sysincdirs, sizeof(char*)*(opts.nsysincdirs+1));
        opts.sysincdirs[opts.nsysincdirs++] = p1;
        char *p2 = path_join(sdkroot, "usr/local/include");
        opts.sysincdirs = (const char**)xrealloc((void*)opts.sysincdirs, sizeof(char*)*(opts.nsysincdirs+1));
        opts.sysincdirs[opts.nsysincdirs++] = p2;
    }
    // add common defaults
    const char *defs[] = {
        "/usr/local/include",
        "/usr/include",
        "/opt/homebrew/include",
        "/usr/include/x86_64-linux-gnu",
        "/usr/include/aarch64-linux-gnu",
        "/usr/include/i386-linux-gnu",
        "/usr/include/arm-linux-gnueabihf",
        "/usr/include/arm-linux-gnueabi",
        "/usr/include/powerpc64le-linux-gnu",
        "/usr/include/riscv64-linux-gnu",
        "/usr/include/s390x-linux-gnu"
    };
    for (size_t di=0; di<sizeof(defs)/sizeof(defs[0]); ++di) {
        opts.sysincdirs = (const char**)xrealloc((void*)opts.sysincdirs, sizeof(char*)*(opts.nsysincdirs+1));
        opts.sysincdirs[opts.nsysincdirs++] = defs[di];
    }
    bool found_clang = false;
#ifdef __APPLE__
    // Add common macOS SDK locations (Xcode/CLT). They may not exist; harmless to try.
    {
        const char *mac_defs[] = {
            "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include",
            "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include"
        };
        for (size_t di=0; di<sizeof(mac_defs)/sizeof(mac_defs[0]); ++di) {
            opts.sysincdirs = (const char**)xrealloc((void*)opts.sysincdirs, sizeof(char*)*(opts.nsysincdirs+1));
            opts.sysincdirs[opts.nsysincdirs++] = mac_defs[di];
        }
    }
#endif
    preprocess(in, stdout, &opts, curdir, path, NULL);
    if (path && in) { fclose(in); free((void*)curdir); }
    return 0;
}
*/

static void preprocess_file(const char *path, FILE *in, const char *curdir, const PPOpts *opts, FILE *out, MacroTable *ptbl)
{
    preprocess(in, out, opts, curdir, path, ptbl);
}

MacroTable tbl_global;
PPOpts opts_global = {0};

void incldue_file_neo_c(char* path, int quoted, FILE* out);

void init_ccpp(int argc, char** argv)
{
    bool found_clang = false;
    mtable_init(&tbl_global);
    // Preserve comments by default (matches gcc -E behavior with -C implied
    // for this tool). The -C flag remains accepted (no-op) for compatibility.
    opts_global.keep_comments = true;
    // parse args: [-I dir]... [-DNAME[=VALUE]] [-UNAME] [-C] [input]
    for (int i=1; i<argc; ++i) {
        const char *a = argv[i];
        if (a[0]=='-' && a[1]=='I') {
            const char *dir = NULL;
            if (a[2] != '\0') dir = a+2; else if (i+1 < argc) { dir = argv[++i]; }
            if (dir) {
                opts_global.incdirs = (const char**)xrealloc((void*)opts_global.incdirs, sizeof(char*)*(opts_global.nincdirs+1));
                opts_global.incdirs[opts_global.nincdirs++] = dir;
            }
        } else if (strcmp(a, "-include") == 0 || strncmp(a, "-include", 8) == 0) {
            const char *path = NULL;
            if (strcmp(a, "-include") == 0) {
                if (i+1 < argc) path = argv[++i];
            } else {
                path = a + 8;
            }
            if (path && *path) {
                opts_global.cmd_includes = (const char**)xrealloc((void*)opts_global.cmd_includes, sizeof(char*)*(opts_global.ncmd_includes+1));
                opts_global.cmd_includes[opts_global.ncmd_includes++] = path;
            }
        } else if (a[0]=='-' && a[1]=='D') {
            const char *def = a+2;
            if (*def == '\0' && i+1 < argc) def = argv[++i];
            if (def && *def) {
                opts_global.cmd_defs = (const char**)xrealloc((void*)opts_global.cmd_defs, sizeof(char*)*(opts_global.ncmd_defs+1));
                opts_global.cmd_defs[opts_global.ncmd_defs++] = def;
            }
        } else if (a[0]=='-' && a[1]=='U') {
            const char *name = a+2;
            if (*name == '\0' && i+1 < argc) name = argv[++i];
            if (name && *name) {
                // Accept forms like "-U__GNUC__-v" by trimming the macro name
                // to the leading identifier portion.
                char idbuf[256]; size_t j=0; const char *p=name;
                if (!(isalpha((unsigned char)*p) || *p=='_')) {
                    strncpy(idbuf, name, sizeof idbuf - 1); idbuf[sizeof idbuf - 1] = '\0';
                } else {
                    while (*p && (isalnum((unsigned char)*p) || *p=='_')) { if (j+1<sizeof idbuf) idbuf[j++]=*p; p++; }
                    idbuf[j] = '\0';
                }
                opts_global.cmd_undefs = (const char**)xrealloc((void*)opts_global.cmd_undefs, sizeof(char*)*(opts_global.ncmd_undefs+1));
                opts_global.cmd_undefs[opts_global.ncmd_undefs++] = xstrdup(idbuf);
            }
        } else if (strcmp(a, "-C") == 0) {
            // Keep comments in output (accepted for compatibility). Comments are
            // already preserved by default in this tool, so this is a no-op.
            opts_global.keep_comments = true;
        } else if (strcmp(a, "-v") == 0) {
            // Verbose: accept and ignore for compatibility with toolchains
            ;
        } else if (a[0] == '-') {
        } else {
        }
    }
    // Initialize the shared macro table for neo-c preprocessing once.
    apply_predefined_macros(&tbl_global, &opts_global);
    // populate system include paths from env and defaults
    const char *envs[] = { getenv("CPATH"), getenv("C_INCLUDE_PATH") };
    for (size_t ei=0; ei<sizeof(envs)/sizeof(envs[0]); ++ei) {
        const char *v = envs[ei];
        if (!v) continue;
        const char *p = v; const char *seg = p;
        while (*p) {
            if (*p == ':') {
                size_t L = (size_t)(p - seg);
                if (L > 0) {
                    char *dir = (char*)malloc(L+1); if (!dir) die("malloc");
                    memcpy(dir, seg, L); dir[L] = '\0';
                    opts_global.sysincdirs = (const char**)xrealloc((void*)opts_global.sysincdirs, sizeof(char*)*(opts_global.nsysincdirs+1));
                    opts_global.sysincdirs[opts_global.nsysincdirs++] = dir;
                }
                seg = p+1;
            }
            p++;
        }
        if (p != seg) {
            size_t L = (size_t)(p - seg);
            char *dir = (char*)malloc(L+1); if (!dir) die("malloc");
            memcpy(dir, seg, L); dir[L] = '\0';
            opts_global.sysincdirs = (const char**)xrealloc((void*)opts_global.sysincdirs, sizeof(char*)*(opts_global.nsysincdirs+1));
            opts_global.sysincdirs[opts_global.nsysincdirs++] = dir;
        }
    }
    // add SDKROOT if present
    const char *sdkroot = getenv("SDKROOT");
    if (sdkroot && *sdkroot) {
        char *p1 = path_join(sdkroot, "usr/include");
        opts_global.sysincdirs = (const char**)xrealloc((void*)opts_global.sysincdirs, sizeof(char*)*(opts_global.nsysincdirs+1));
        opts_global.sysincdirs[opts_global.nsysincdirs++] = p1;
        char *p2 = path_join(sdkroot, "usr/local/include");
        opts_global.sysincdirs = (const char**)xrealloc((void*)opts_global.sysincdirs, sizeof(char*)*(opts_global.nsysincdirs+1));
        opts_global.sysincdirs[opts_global.nsysincdirs++] = p2;
        char *p3 = path_join(sdkroot, "System/Library/Frameworks/Kernel.framework/Headers");
        opts_global.sysincdirs = (const char**)xrealloc((void*)opts_global.sysincdirs, sizeof(char*)*(opts_global.nsysincdirs+1));
        opts_global.sysincdirs[opts_global.nsysincdirs++] = p3;
    }
    // add common defaults
    const char *defs[] = {
        "/usr/local/include",
        "/usr/include",
        "/opt/homebrew/include",
        "/usr/include/x86_64-linux-gnu",
        "/usr/include/aarch64-linux-gnu",
        "/usr/include/i386-linux-gnu",
        "/usr/include/arm-linux-gnueabihf",
        "/usr/include/arm-linux-gnueabi",
        "/usr/include/powerpc64le-linux-gnu",
        "/usr/include/riscv64-linux-gnu",
        "/usr/include/s390x-linux-gnu"
    };
    for (size_t di=0; di<sizeof(defs)/sizeof(defs[0]); ++di) {
        opts_global.sysincdirs = (const char**)xrealloc((void*)opts_global.sysincdirs, sizeof(char*)*(opts_global.nsysincdirs+1));
        opts_global.sysincdirs[opts_global.nsysincdirs++] = defs[di];
    }
#ifdef __APPLE__
    // Add common macOS SDK locations (Xcode/CLT). They may not exist; harmless to try.
    {
        const char *mac_defs[] = {
            "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include",
            "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include",
            "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks/Kernel.framework/Headers",
            "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/Kernel.framework/Headers"
        };
        for (size_t di=0; di<sizeof(mac_defs)/sizeof(mac_defs[0]); ++di) {
            opts_global.sysincdirs = (const char**)xrealloc((void*)opts_global.sysincdirs, sizeof(char*)*(opts_global.nsysincdirs+1));
            opts_global.sysincdirs[opts_global.nsysincdirs++] = mac_defs[di];
        }
    }
    if (add_best_clang_resource_include(&opts_global, "/Library/Developer/CommandLineTools/usr/lib/clang")) {
        found_clang = true;
    } else if (add_best_clang_resource_include(&opts_global, "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang")) {
        found_clang = true;
    }
#else
    if (add_best_clang_resource_include(&opts_global, "/usr/lib/clang")) {
        found_clang = true;
    }
    if (add_best_clang_resource_include(&opts_global, "/usr/local/lib/clang")) {
        found_clang = true;
    }
#endif
    // Add GCC internal include dirs only if clang resource headers aren't available.
    if (!found_clang) {
        add_gcc_internal_includes(&opts_global, "/usr/lib/gcc");
        add_gcc_internal_includes(&opts_global, "/usr/lib64/gcc");
    }
}

void preprocess_file_neo_c(const char *path, FILE *out)
{
    const char *curdir;
    FILE* in = fopen(path, "r");
    size_t i;
    if (!in) die("fopen");
    curdir = dirname_dup(path);
#ifdef CCPP_BARE
    fputs("/* ccpp: bare libc typedefs */\n"
          "typedef float _Float32;\n"
          "typedef double _Float64;\n"
          "typedef double _Float32x;\n"
          "typedef long double _Float64x;\n"
          "typedef signed char __int8_t;\n"
          "typedef unsigned char __uint8_t;\n"
          "typedef signed short int __int16_t;\n"
          "typedef unsigned short int __uint16_t;\n"
          "typedef signed int __int32_t;\n"
          "typedef unsigned int __uint32_t;\n"
          "typedef signed long long int __int64_t;\n"
          "typedef unsigned long long int __uint64_t;\n"
          "typedef int __darwin_natural_t;\n"
          "typedef __builtin_va_list __darwin_va_list;\n"
          "typedef unsigned long __darwin_size_t;\n"
          "typedef long __darwin_ssize_t;\n"
          "typedef long __darwin_intptr_t;\n"
          "typedef int __darwin_ct_rune_t;\n"
          "typedef int __darwin_rune_t;\n"
          "typedef int __darwin_wchar_t;\n"
          "typedef int __darwin_mbstate_t;\n"
          "typedef long __darwin_clock_t;\n"
          "typedef int __darwin_wint_t;\n",
          out);
#endif
    for (i = 0; i < opts_global.ncmd_includes; i++) {
        incldue_file_neo_c((char*)opts_global.cmd_includes[i], 0, out);
    }
    preprocess_file(path, in, curdir, &opts_global, out, &tbl_global);
}

const char *get_macro(const char *macro_name)
{
    if (!macro_name || !*macro_name) return NULL;
    Macro *m = mtable_find(&tbl_global, macro_name);
    if (!m) return NULL;
    return m->value ? m->value : "";
}


void macro_define(const char *def)
{
    if (!def || !*def) return;

    const char *p = def;
    while (*p && isspace((unsigned char)*p)) p++;
    bool has_define = false;
    if (*p == '#') {
        has_define = true;
    } else if (starts_with(p, "define")) {
        has_define = true;
    }

    char *line = NULL;
    if (has_define) {
        line = xstrdup(def);
    } else {
        size_t len = strlen(def);
        const char *prefix = "#define ";
        size_t plen = strlen(prefix);
        line = (char *)malloc(plen + len + 1);
        if (!line) die("malloc");
        memcpy(line, prefix, plen);
        memcpy(line + plen, def, len);
        line[plen + len] = '\0';
    }

    process_define(&tbl_global, line);
    free(line);
}


void macro_undef(const char *name)
{
    if (!name || !*name) return;

    const char *p = name;
    while (*p && isspace((unsigned char)*p)) p++;
    if (*p == '#') {
        p++;
        while (*p && isspace((unsigned char)*p)) p++;
    }
    if (starts_with(p, "undef")) {
        p += 5;
        while (*p && isspace((unsigned char)*p)) p++;
    }

    char idbuf[256];
    size_t i = 0;
    if (!(isalpha((unsigned char)*p) || *p == '_')) return;
    while (*p && (isalnum((unsigned char)*p) || *p == '_')) {
        if (i + 1 < sizeof idbuf) idbuf[i++] = *p;
        p++;
    }
    idbuf[i] = '\0';
    if (*idbuf) {
        mtable_unset(&tbl_global, idbuf);
    }
}


const char *call_func_macro(const char *macro_name, const char *args, const char *file, long line)
{
    if (!macro_name || !*macro_name) return NULL;
    Macro *m = mtable_find(&tbl_global, macro_name);
    if (!m || !m->is_func) return NULL;
    MacroTable *tbl = &tbl_global;

    size_t name_len = strlen(macro_name);
    size_t args_len = args ? strlen(args) : 0;
    size_t buf_len = name_len + 2 + args_len + 1;
    char *linebuf = (char *)malloc(buf_len);
    if (!linebuf) die("malloc");
    memcpy(linebuf, macro_name, name_len);
    linebuf[name_len] = '(';
    if (args_len > 0) {
        memcpy(linebuf + name_len + 1, args, args_len);
    }
    linebuf[name_len + 1 + args_len] = ')';
    linebuf[name_len + 2 + args_len] = '\0';

    const char *prev_file = g_cur_file;
    long prev_line = g_cur_line;
    int prev_pass = g_expand_pass;
    int prev_bl = g_block_comment_depth;

    if (file && *file) g_cur_file = file;
    if (line > 0) g_cur_line = line;
    g_expand_pass = 0;
    g_block_comment_depth = 0;

    Str out;
    sb_init(&out);
    expand_into_str(tbl, linebuf, &out);

    g_cur_file = prev_file;
    g_cur_line = prev_line;
    g_expand_pass = prev_pass;
    g_block_comment_depth = prev_bl;

    free(linebuf);

    if (g_macro_call_out.buf) free(g_macro_call_out.buf);
    g_macro_call_out = out;

    return g_macro_call_out.buf ? g_macro_call_out.buf : "";
}

static const char *snapshot_find_sig(const MacroSnapshot *snap, const char *name)
{
    if (!snap || !name) return NULL;
    for (size_t i = 0; i < snap->len; ++i) {
        if (strcmp(snap->items[i].name, name) == 0) return snap->items[i].sig;
    }
    return NULL;
}

static char *macro_signature(const Macro *m)
{
    Str s;
    sb_init(&s);
    if (m->is_func) {
        sb_puts(&s, "func(");
        for (size_t i = 0; i < m->nparams; ++i) {
            if (i > 0) sb_putc(&s, ',');
            sb_puts(&s, m->params[i]);
        }
        if (m->is_variadic) {
            if (m->nparams > 0) sb_putc(&s, ',');
            if (m->var_param) {
                sb_puts(&s, m->var_param);
                sb_puts(&s, "...");
            } else {
                sb_puts(&s, "...");
            }
        }
        sb_puts(&s, ")=");
    } else {
        sb_puts(&s, "obj=");
    }
    if (m->value) sb_puts(&s, m->value);
    if (!s.buf) return xstrdup("");
    return s.buf;
}

static char *macro_define_line(const Macro *m)
{
    Str s;
    sb_init(&s);
    sb_puts(&s, "#define ");
    sb_puts(&s, m->name);
    if (m->is_func) {
        sb_putc(&s, '(');
        for (size_t i = 0; i < m->nparams; ++i) {
            if (i > 0) sb_putc(&s, ',');
            sb_puts(&s, m->params[i]);
        }
        if (m->is_variadic) {
            if (m->nparams > 0) sb_putc(&s, ',');
            if (m->var_param) {
                sb_puts(&s, m->var_param);
                sb_puts(&s, "...");
            } else {
                sb_puts(&s, "...");
            }
        }
        sb_putc(&s, ')');
    }
    if (m->value && *m->value) {
        sb_putc(&s, ' ');
        sb_puts(&s, m->value);
    }
    if (!s.buf) return xstrdup("");
    return s.buf;
}

MacroSnapshot *macro_snapshot_create(void)
{
    MacroSnapshot *snap = (MacroSnapshot *)malloc(sizeof(MacroSnapshot));
    if (!snap) die("malloc");
    snap->items = NULL;
    snap->len = 0;
    snap->cap = 0;
    for (size_t i = 0; i < tbl_global.len; ++i) {
        if (snap->len == snap->cap) {
            size_t ncap = snap->cap ? snap->cap * 2 : 16;
            snap->items = (MacroSnapItem *)xrealloc(snap->items, ncap * sizeof(MacroSnapItem));
            snap->cap = ncap;
        }
        snap->items[snap->len].name = xstrdup(tbl_global.items[i].name);
        snap->items[snap->len].sig = macro_signature(&tbl_global.items[i]);
        snap->len++;
    }
    return snap;
}

char *macro_snapshot_diff_defines(MacroSnapshot *snap)
{
    if (!snap) return NULL;
    Str out;
    sb_init(&out);
    for (size_t i = 0; i < tbl_global.len; ++i) {
        Macro *m = &tbl_global.items[i];
        const char *oldsig = snapshot_find_sig(snap, m->name);
        char *newsig = macro_signature(m);
        bool changed = (!oldsig || strcmp(oldsig, newsig) != 0);
        if (changed) {
            char *line = macro_define_line(m);
            sb_puts(&out, line);
            sb_putc(&out, '\n');
            free(line);
        }
        free(newsig);
    }
    for (size_t i = 0; i < snap->len; ++i) {
        if (!mtable_find(&tbl_global, snap->items[i].name)) {
            sb_puts(&out, "#undef ");
            sb_puts(&out, snap->items[i].name);
            sb_putc(&out, '\n');
        }
    }
    if (!out.buf || out.len == 0) {
        sb_free(&out);
        return NULL;
    }
    return out.buf;
}

void macro_snapshot_free(MacroSnapshot *snap)
{
    if (!snap) return;
    for (size_t i = 0; i < snap->len; ++i) {
        free(snap->items[i].name);
        free(snap->items[i].sig);
    }
    free(snap->items);
    free(snap);
}


void set_macro(const char *name, const char *value)
{
    mtable_set_obj(&tbl_global, name, value);
}

void incldue_file_neo_c(char* path, int quoted, FILE* out)
{
    char* header = path;
    char *opened_path=NULL;
    char* curdir = getenv("PWD");
    FILE *f = NULL;
    f = open_in_search(header, quoted, curdir, &opts_global, &opened_path);
    if (f) {
        // check pragma once cache
        bool skip = false;
        for (size_t oi=0; oi<opts_global.nonce_paths; ++oi) {
            if (strcmp(opts_global.once_paths[oi], opened_path)==0) { skip = true; break; }
        }
        // also skip if this path is already in the current include stack (cycle)
        if (!skip && opts_global.nopen_stack > 0) {
            for (size_t si=0; si<opts_global.nopen_stack; ++si) {
                if (strcmp(opts_global.open_stack[si], opened_path) == 0) { skip = true; break; }
            }
        }
        if (!skip) {
            char *ndir = dirname_dup(opened_path);
            preprocess_file(opened_path, f, ndir, &opts_global, out, &tbl_global);
            fclose(f);
            free(ndir);
        }
        free(opened_path);
    }
}
