#ifndef NEO_C_H
#define NEO_C_H

#define __BEGIN_DECLS
#define __END_DECLS

#undef __cplusplus

#ifdef __STDC__
#define __P(protos) protos
#else
#define __P(protos) ()
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE _GNU_SOURCE
#endif

#define ALLOCATED_MAGIC_NUM 177783

#define nullptr ((void*)0)
typedef char*% string;

#if defined(__MINUX__)
#define UNIX 1
#elif defined(__BAREMETAL__) && defined(__NEO_BARE_HOST_SYSCALLS__) && defined(__linux__) && defined(__x86_64__)
#define UNIX 1
#elif defined(__BAREMETAL__) && defined(__NEO_BARE_HOST_SYSCALLS__) && defined(__APPLE__)
#define UNIX 1
#elif defined(__BAREMETAL__)
#elif defined(__PICO__)
#elif defined(__M5STACK__)
#else
#define UNIX 1
#endif


///////////////////////////////////////////////////////////////////////////
// BARE METAL
///////////////////////////////////////////////////////////////////////////
#if defined(__BAREMETAL__)
    #include "neo-c-libc.h"

    using neo-c;
    using unsafe;

///////////////////////////////////////////////////////////////////////////
// PICO
///////////////////////////////////////////////////////////////////////////
#elif defined(__PICO__)
    c_include {#define _GNU_SOURCE}
    c_include {#include "stdarg.h"}
    c_include {#include "stdlib.h"}
    c_include {#include "stdint.h"}
    c_include {#include "string.h"}
    c_include {#include "stdio.h"}
    c_include {#include "ctype.h"}
    c_include {#include "wchar.h"}
    c_include {#include "pico/stdlib.h"}
    c_include {#include "pico/stdio.h"}
    c_include {#include "pico/time.h"}
    c_include {#include "hardware/irq.h"}
    c_include {#include "hardware/timer.h"}
    c_include {#include "hardware/uart.h"}
    
    c_include {#include "pico/mutex.h"}
    c_include {#include "pico/multicore.h"}

    #define MUTEX_INITIALIZER (mutex_t){ .locked = false, .core = NULL }
    #ifndef NULL
    #define NULL ((void*)0)
    #endif

    typedef __builtin_va_list va_list;

    using neo-c;
    using unsafe;

///////////////////////////////////////////////////////////////////////////
// M5STACK
///////////////////////////////////////////////////////////////////////////
#elif defined(__M5STACK__)
    c_include {#include <M5Stack.h>}

    typedef __builtin_va_list va_list;
    
    #ifndef NULL
    #define NULL ((void*)0)
    #endif
    
    using neo-c;
    using unsafe;
///////////////////////////////////////////////////////////////////////////
// UNIX
///////////////////////////////////////////////////////////////////////////
#else
    using C;
    
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdarg.h>
    #undef va_start
    #define va_start(ap, last)  __builtin_va_start(ap, last)
    #include <limits.h>
    #include <locale.h>
    #include <errno.h>
    #include <assert.h>
    #include <stdbool.h>
    #include <wchar.h>
    #include <stdint.h>
    #if defined(__NEO_NATIVE_BACKTRACE__) && (defined(__linux__) || defined(__APPLE__))
    #include <execinfo.h>
    #include <dlfcn.h>
    #endif

    #ifndef NULL
    #define NULL ((void*)0)
    #endif

    using neo-c;
    using unsafe;
#endif

///////////////////////////////////////////////////////////////////////////
// PREVIOUS DEFINITIONS
///////////////////////////////////////////////////////////////////////////
struct buffer 
{
    char*% buf;
    int len;
    int size;
};

uniq buffer*% buffer*::initialize(buffer*% self);
uniq void buffer*::finalize(buffer* self);
uniq buffer*% buffer*::clone(buffer* self);
uniq bool buffer*::equals(buffer* left, buffer* right);
uniq buffer* buffer*::append_str(buffer* self, const char* mem);
uniq buffer* buffer*::append(buffer* self, const char* mem, size_t size);
uniq string xsprintf(const char* msg, ...);
uniq string char*::to_string(const char* self);
uniq string int::to_string(int self);
uniq unsigned int bool::get_hash_key(bool value);
uniq unsigned int _Bool::get_hash_key(bool value);
uniq unsigned int char::get_hash_key(char value);
uniq unsigned int short::get_hash_key(short int value);
uniq unsigned int int::get_hash_key(int value);
uniq unsigned int long::get_hash_key(long value);
uniq unsigned int size_t::get_hash_key(size_t value);
uniq unsigned int float::get_hash_key(float value);
uniq unsigned int double::get_hash_key(double value);
uniq unsigned int char*::get_hash_key(const char* value);
uniq unsigned int string::get_hash_key(char* value);
uniq unsigned int void*::get_hash_key(void* value);
uniq string char*::substring(const char* str, int head, int tail);
uniq buffer* buffer*::append_format(buffer* self, const char* msg, ...);
uniq string buffer*::to_string(buffer* self);
uniq string char*::to_string(const char* self);
uniq string double::to_string(double self);
uniq string float::to_string(float self);
uniq string size_t::to_string(size_t self);
uniq string long::to_string(long self);
uniq string int::to_string(int self);
uniq string short::to_string(short self);
uniq string char::to_string(char self);
uniq string bool::to_string(bool self);
uniq string _Bool::to_string(bool self);
uniq bool string::equals(char* self, const char* right);

#ifdef __NEO_MICRO_RAM_8K__
#define COME_STACKFRAME_MAX 2
#else
#define COME_STACKFRAME_MAX 8
#endif
#define COME_STACKFRAME_SNAME_MAX 8

struct neo_frame {
    neo_frame *prev;
    char* fun_name;
    unsigned long frame_id;
};
    
using unsafe {
uniq __thread neo_frame* neo_current_frame = (void*)0;
uniq __thread unsigned long neo_frame_id = 0;
}

uniq _norecord bool neo_frame_is_alive(unsigned long frame_id)
{
    neo_frame* f = neo_current_frame;
    
    while(f) {
        if(f->frame_id == frame_id) {
            return true;
        }
        
        f = f->prev;
    }
    
    return false;
}

uniq void neo_print_native_backtrace_symbol(void* addr)
{
#if defined(__NEO_NATIVE_BACKTRACE__) && (defined(__linux__) || defined(__APPLE__)) && !defined(__BAREMETAL__) && !defined(__NEO_MICRO__)
    Dl_info info;
    if(dladdr(addr, &info) && info.dli_fname) {
        char command[2048];
        char line[2048];
        FILE* fp;
#if defined(__APPLE__)
        snprintf(command, sizeof(command), "atos -o '%s' -l 0x%lx 0x%lx 2>/dev/null",
            info.dli_fname, (unsigned long)(uintptr_t)info.dli_fbase,
            (unsigned long)(uintptr_t)addr);
#else
        snprintf(command, sizeof(command), "addr2line -f -p -e '%s' 0x%lx 2>/dev/null",
            info.dli_fname,
            (unsigned long)((uintptr_t)addr - (uintptr_t)info.dli_fbase));
#endif
        fp = popen(command, "r");
        if(fp) {
            if(fgets(line, sizeof(line), fp) && strncmp(line, "??", 2) != 0) {
                printf("%s", line);
                pclose(fp);
                return;
            }
            pclose(fp);
        }
#if defined(__linux__)
        snprintf(command, sizeof(command), "addr2line -f -p -e '%s' 0x%lx 2>/dev/null",
            info.dli_fname, (unsigned long)(uintptr_t)addr);
        fp = popen(command, "r");
        if(fp) {
            if(fgets(line, sizeof(line), fp) && strncmp(line, "??", 2) != 0) {
                printf("%s", line);
                pclose(fp);
                return;
            }
            pclose(fp);
        }
#endif
        if(info.dli_sname) {
            printf("%s (%p)\n", info.dli_sname, addr);
        }
        else {
            printf("%s (%p)\n", info.dli_fname, addr);
        }
    }
    else {
        printf("%p\n", addr);
    }
#else
    printf("%p\n", addr);
#endif
}

uniq bool neo_print_native_backtrace()
{
#if defined(__NEO_NATIVE_BACKTRACE__) && (defined(__linux__) || defined(__APPLE__)) && !defined(__BAREMETAL__) && !defined(__NEO_MICRO__)
    void* frames[64];
    int n = backtrace(frames, 64);
    int i;

    if(n <= 0) {
        return false;
    }

    puts("native backtrace:");
    for(i=1; i<n; i++) {
        printf("  #%d ", i-1);
        neo_print_native_backtrace_symbol(frames[i]);
    }

    return true;
#else
    return false;
#endif
}

uniq void stackframe()
{
    if(neo_print_native_backtrace()) {
        return;
    }

    neo_frame *f = neo_current_frame;
    while(f) {
        char* fun_name = f->fun_name;
        
        printf("%s\n", fun_name);
        f = f->prev;
    }
}

bool come_is_alive(void* mem);

uniq void stackframe2(void* mem)
{
    if(come_is_alive(mem) && mem) {
#ifndef __NEO_MICRO_RAM_8K__
        sMemHeader* it = (sMemHeader*)((char*)mem - sizeof(size_t) - sizeof(size_t) - sizeof(sMemHeader));
        printf("allocated at %s %d #%d. type is %s.\n", it->sname, it->sline, it->id, it->class_name);
#endif
    }

    if(neo_print_native_backtrace()) {
        return;
    }

    neo_frame *f = neo_current_frame;
    while(f) {
        char* fun_name = f->fun_name;
        
        printf("%s\n", fun_name);
        f = f->prev;
    }
}

uniq bool die(const char* msg, char* sname=__caller_sname__, int sline=__caller_line__)
{
    puts(s"\{sname} \{sline} : " + msg);
    stackframe();
    exit(4);
    
    return false;
}

//////////////////////////////
/// HEAP
//////////////////////////////
struct sMemHeader
{
#ifdef __NEO_MICRO_RAM_8K__
    size_t size;
    size_t compiletime_size;
    size_t alloc_size;
#else
    long size;
    long compiletime_size;
    long alloc_size;
#endif
    int32_t allocated;        /// ALLOCATED_MAGIC_NUM
    int alive;
    struct sMemHeader* next;
    struct sMemHeader* prev;
    struct sMemHeader* free_next;

#if defined(__NEO_MEMLEAK_STACKTRACE__) && !defined(__NEO_MICRO_RAM_8K__)
    char* fun_name[COME_STACKFRAME_MAX];
#endif

#ifndef __NEO_MICRO_RAM_8K__
    const char* class_name;
    const char* sname;
    int sline;
    int id;
#endif
};

using unsafe {
uniq sMemHeader* gAllocMem = (void*)0;
uniq sMemHeader* gFreeMem = (void*)0;
}

uniq void come_memleak_checker()
{
    using unsafe;
    
    sMemHeader* it = gAllocMem;
    int n = 0;
    while(it) {
        n++;

#ifdef __NEO_MICRO_RAM_8K__
        printf("#%d allocation\n", n);
#else
        printf("#%d ", n);
        if(it->class_name) {
            printf("%p (%s) %s %d: ", (char*)it + sizeof(sMemHeader) + sizeof(size_t) + sizeof(size_t), it->class_name, it->sname, it->sline);
        }
#ifdef __NEO_MEMLEAK_STACKTRACE__
        for(int i=0; i<COME_STACKFRAME_MAX; i++) {
            if(it->fun_name[i]) {
                printf("%s, ", it->fun_name[i]);
            }
        }
#endif
        puts("");
#endif
        it = it->next;
    }
    if(n > 0) printf("%d memory leaks. %d alloc, %d free.\n", n, gNumAlloc, gNumFree);
}
    
uniq void* alloc_from_pages(size_t size)
{
    using unsafe; 
    
    sMemHeader* it = gFreeMem;
    sMemHeader* it_prev = null;
    while(it) {
        if(size <= it->alloc_size) {
            if(it_prev == null) {
                gFreeMem = null;
            }
            else {
                it_prev->free_next = it->free_next;
            }
            memset(it, 0, size);
            return it;
        }
        it_prev = it;
        it = it->free_next;
    }
    
    sMemHeader* it = (sMemHeader*)calloc(1, size);
    if(it == null) {
        return null;
    }
    it->alloc_size = size;
    return it;
}
    
uniq void come_free_mem_of_heap_pool(void* mem)
{
    using unsafe; 
    
    if(mem) {
        sMemHeader* it = (sMemHeader*)((char*)mem - sizeof(sMemHeader));
        
        if(it->allocated != ALLOCATED_MAGIC_NUM) {
            return;
        }
        
        it->allocated = 0;
        
        sMemHeader* prev_it = it->prev;
        sMemHeader* next_it = it->next;
        
        if(gAllocMem == it) {
            gAllocMem = next_it;
            
            if(gAllocMem) {
                gAllocMem->prev = null;
            }
        }
        else {
            if(prev_it) {
                prev_it->next = next_it;
            }
            if(next_it) {
                next_it->prev = prev_it;
            }
        }
        
        size_t size = it->size;
        
        it->free_next = gFreeMem;
        gFreeMem = it;
        
        it->alive = 0;
/*
        free(it);
*/
        
        gNumFree++;
    }
}

uniq void* come_alloc_mem_from_heap_pool(size_t compiletime_size, size_t size, const char* sname=null, int sline=0, int id=0, const char* class_name="")
{
    using unsafe; 
    
    size_t size2 = size + sizeof(sMemHeader);
#ifdef __32BIT_CPU__
    size2 = (size2 + 3 & ~0x3);
#elif defined(__16BIT_CPU__)
    size2 = (size2 + 1 & ~0x1);
#else
    size2 = (size2 + 7 & ~0x7);
#endif
    void* result = alloc_from_pages(size2);
    if(result == null) {
        puts("out of memory");
        stackframe();
        exit(5);
        return null;
    }
    
    sMemHeader* it = result;
    
    it->allocated = ALLOCATED_MAGIC_NUM;
    
    it->compiletime_size = compiletime_size;
    it->size = size2;
    it->free_next = NULL;
    it->alive = 1;
    
#if defined(__NEO_MEMLEAK_STACKTRACE__) && !defined(__NEO_MICRO_RAM_8K__)
    int n = 0;
    neo_frame *f = neo_current_frame;
    while(f && n < COME_STACKFRAME_MAX) {
        char* fun_name = f->fun_name;
        
        it.fun_name[n] = fun_name;
        
        n++;
        f = f->prev;
    }
#endif
    
#ifndef __NEO_MICRO_RAM_8K__
    it->next = gAllocMem;
    it->prev = null;
    
    it->class_name = class_name; 
    it->sname = sname;
    it->sline = sline;
    it->id = id;
#endif
    
    if(gAllocMem) {
        gAllocMem->prev = it;
    }
    
    gAllocMem = it;
    
    gNumAlloc++;
    
    return (char*)result + sizeof(sMemHeader);
}

uniq char* come_dynamic_typeof(void* mem)
{
    using unsafe; 
    
    if(!come_is_alive(mem)) {
        puts("invalid heap object");
        stackframe2(mem);
        exit(3);
    }
    
    sMemHeader* it = (sMemHeader*)((char*)mem - sizeof(size_t) - sizeof(size_t) - sizeof(sMemHeader));
    
    if(it->allocated != ALLOCATED_MAGIC_NUM) {
        printf("invalid heap object(%p)(1)\n", it);
        stackframe2(mem);
        exit(2);
    }
    
#ifdef __NEO_MICRO_RAM_8K__
    return "";
#else
    return (char*)it->class_name;
#endif
}

uniq size_t dynamic_sizeof(void* mem)
{
    using unsafe; 
    
    if(!come_is_alive(mem)) {
        puts("invalid heap object");
        stackframe2(mem);
        exit(3);
    }
    
    sMemHeader* it = (sMemHeader*)((char*)mem - sizeof(size_t) - sizeof(size_t) - sizeof(sMemHeader));
    
    if(it->allocated != ALLOCATED_MAGIC_NUM) {
        printf("invalid heap object(%p)(1)\n", it);
        stackframe2(mem);
        exit(2);
    }
    size_t size = it->compiletime_size;
    
    return size;
}

uniq int gNumAlloc = 0;
uniq int gNumFree = 0;

uniq void* come_calloc(size_t count, size_t size, const char* sname=null, int sline=0, int id=0, const char* class_name="")
{
    using unsafe; 
    
    char* mem = come_alloc_mem_from_heap_pool(count*size, sizeof(size_t)+sizeof(size_t)+count*size, sname, sline, id, class_name);
    
    size_t* ref_count = (size_t*)mem;

    *ref_count = 0;
    
    size_t* size2 = (size_t*)(mem + sizeof(size_t));
    
    *size2 = size*count + sizeof(size_t) + sizeof(size_t);
    
    return mem + sizeof(size_t) + sizeof(size_t);
}

uniq bool come_is_alive(void* mem)
{
    if(mem == null) {
        return false;
    }
    sMemHeader* it = (sMemHeader*)((char*)mem - sizeof(size_t) - sizeof(size_t) - sizeof(sMemHeader));
    
    return it->alive;
}

uniq void come_free(void* mem)
{
    using unsafe; 
    
    if(mem == NULL) {
        return;
    }
    
    if(!come_is_alive(mem)) {
        puts("invalid heap object");
        stackframe2(mem);
        exit(3);
    }
    
    size_t* ref_count = (size_t*)((char*)mem - sizeof(size_t) - sizeof(size_t));
    
    come_free_mem_of_heap_pool((char*)ref_count);
}

uniq void* come_memdup(void* block, char* sname=null, int sline=0, int id=0, const char* class_name=null)
{
    using unsafe; 
    
    if(block == null) {
        return null;
    }
    
    if(!come_is_alive(block)) {
        puts(s"invalid heap object. \{sname} \{sline} #\{id}");
        stackframe2(block);
        exit(3);
    }

    char* mem = (char*)block - sizeof(size_t) - sizeof(size_t);
    
    size_t* size_p = (size_t*)(mem + sizeof(size_t));

    size_t size = *size_p - sizeof(size_t) - sizeof(size_t);
    
    void* result = come_calloc(1, size, sname, sline, id, class_name);

    memcpy(result, block, size);
    
    return result;
}

uniq void* come_increment_ref_count(void* mem, char* sname, int sline, int id)
{
    using unsafe; 
    
    if(mem == NULL) {
        return mem;
    }
    if(!come_is_alive(mem)) {
        puts(s"invalid heap object \{sname} \{sline} #\{id}");
        stackframe2(mem);
        exit(3);
    }
    
    size_t* ref_count = (size_t*)((char*)mem - sizeof(size_t) - sizeof(size_t));
    
    (*ref_count)++;
    
    return mem;
}

uniq void* come_print_ref_count(void* mem)
{
    using unsafe; 
    
    if(mem == NULL) {
        return mem;
    }
    if(!come_is_alive(mem)) {
        puts("invalid heap object");
        stackframe();
        exit(3);
    }
    
    size_t* ref_count = (size_t*)((char*)mem - sizeof(size_t) - sizeof(size_t));
    
    printf("ref_count %ld\n", *ref_count);
    
    return mem;
}

uniq int come_get_ref_count(void* mem)
{
    using unsafe; 
    
    if(mem == NULL) {
        return 0;
    }
    if(!come_is_alive(mem)) {
        puts("invalid heap object");
        stackframe();
        exit(3);
    }
    
    size_t* ref_count = (size_t*)((char*)mem - sizeof(size_t) - sizeof(size_t));
    
    return *ref_count;
}

uniq void* come_decrement_ref_count(void* mem, void* protocol_fun, void* protocol_obj, bool no_decrement, bool no_free, void* result_obj, char* sname, int sline, int id)
{
    using unsafe; 
    
    if(result_obj) {
        if(mem == result_obj) {
            return mem;
        }
    }
    if(mem == NULL) {
        return NULL;
    }
    if(!come_is_alive(mem)) {
        puts(s"invalid heap object \{sname} \{sline} #\{id}");
        stackframe2(mem);
        exit(3);
    }
    
    long* ref_count = (long*)((char*)mem - sizeof(size_t) - sizeof(size_t));
    
    if(!no_decrement) {
        (*ref_count)--;
    }
    
    long count = *ref_count;
    if(!no_free && count <= 0) {
        if(protocol_obj && protocol_fun) {
            void (*finalizer)(void*) = (void (*)(void*))protocol_fun;
            finalizer(protocol_obj);
            
            come_free(protocol_obj);
        }
        come_free(mem);
        return NULL;
    }
    
    return mem;
}

uniq void come_call_finalizer(void* fun, void* mem, void* protocol_fun, void* protocol_obj, int call_finalizer_only, int no_decrement, int no_free, void* result_obj, char* sname, int sline, int id)
{
    using unsafe;
    
    if(result_obj) {
        if(mem == result_obj) {
            return;
        }
    }
    if(mem == NULL) {
        return;
    }
    
    if(call_finalizer_only) {
        if(fun) {
            if(protocol_obj && protocol_fun) {
                void (*finalizer)(void*) = (void (*)(void*))protocol_fun;
                finalizer(protocol_obj);
            }
            void (*finalizer)(void*) = (void (*)(void*))fun;
            finalizer(mem);
        }
        else {
            if(protocol_obj && protocol_fun) {
                void (*finalizer)(void*) = (void (*)(void*))protocol_fun;
                finalizer(protocol_obj);
            }
        }
    }
    else {
        if(!come_is_alive(mem)) {
            puts(s"invalid heap object. \{sname} \{sline} #\{id}");
            stackframe2(mem);
            exit(3);
        }
        
        long* ref_count = (long*)((char*)mem - sizeof(size_t) - sizeof(size_t));
        
        if(!no_decrement) {
            (*ref_count)--;
        }
        
        long count = *ref_count;
        if(!no_free && count <= 0) {
            if(mem) {
                if(fun) {
                    if(protocol_obj && protocol_fun) {
                        void (*finalizer)(void*) = (void (*)(void*))protocol_fun;
                        finalizer(protocol_obj);
                        come_free(protocol_obj);
                    }
                    if(fun) {
                        void (*finalizer)(void*) = (void (*)(void*))fun;
                        finalizer(mem);
                    }
                }
                else {
                    if(protocol_obj && protocol_fun) {
                        void (*finalizer)(void*) = (void (*)(void*))protocol_fun;
                        finalizer(protocol_obj);
                        come_free(protocol_obj);
                    }
                }
                come_free(mem);
            }
        }
    }
}

uniq void xassert(const char* msg, bool test)
{
    printf("%s...", msg);
    if(!test) {
        puts("false");
        stackframe();
        exit(2);
    }
    puts("ok");
}
        
uniq void* come_null_checker(void* mem, const char* sname, int sline, int id)
{
    if(mem) {
        return mem;
    }
    
    puts(s"null pointer exception \{sname} \{sline} #\{id}");
    stackframe();
    exit(1);
}

uniq void* come_heap_checker(void* mem, const char* sname, int sline, int id)
{
    if(mem) {
        if(come_is_alive(mem)) {
            return mem;
        }
        else {
            puts(s"heap pointer exception \{sname} \{sline} #\{id}");
            stackframe();
            exit(1);
        }
    }
    
    puts(s"null pointer exception \{sname} \{sline} \{id}");
    stackframe();
    exit(1);
}

uniq string __builtin_string(const char* str, char* sname=__caller_sname__, int sline=__caller_line__)
{
    if(str == null) {
        return null;
    }
    int len = strlen(str) + 1;
    
    char* result = come_calloc(1, sizeof(char)*(len), sname, sline, 0, "string");
    //char*% result = new char[len];

    strncpy(result, str, len);

    return dummy_heap result;
}

#ifndef UNIX
    uniq void stackframe() version 2
    {
        inherit();
    }
    
    uniq void* come_calloc(size_t count, size_t size, const char* sname=null, int sline=0, int id=0, const char* class_name="") version 2
    {
        return inherit(count, size, sname, sline, id, class_name);
    }
    
    uniq void come_free(void* mem) version 2
    {
        inherit(mem);
    }
#endif

//////////////////////////////
// ref
//////////////////////////////
struct ref<T>
{
    T^ p;
    bool global;
    bool heap;
    bool local;
    
    unsigned long frame_id;
    void* heaptop;
};

impl ref<T>
{
    ref<T>*% initialize(ref<T>*% self, T^ p, bool global_, bool heap_, bool local_, unsigned long frame_id) {
        if(!ispointer(T) || p == null) {
            puts(s"ref is pointer and not null");
            stackframe2(self);
            exit(2);
        }
        self.p = p;
        self.global = global_;
        self.heap = heap_;
        self.local = local_;
        self.frame_id = frame_id;
        self.heaptop = p;
        return self;
    }
    
    _norecord T^ unwrap(ref<T>* self) {
        using unsafe;
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        if(self.local) {
            if(!neo_frame_is_alive(self.frame_id)) {
                puts("refferenced stack object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(self.heap) {
            if(!come_is_alive(self.heaptop)) {
                puts("refferenced heap object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        return self.p;
    }
    _norecord T] operator_derefference(ref<T>* self)
    {
        using unsafe;
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        
        if(self.local) {
            if(!neo_frame_is_alive(self.frame_id)) {
                puts("refferenced stack object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(self.heap) {
            if(!come_is_alive(self.heaptop)) {
                puts("refferenced heap object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        
        return *self.p;
    }
    _norecord void operator_store_derefference(ref<T>* self, T] item)
    {
        using unsafe;
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        
        if(self.local) {
            if(!neo_frame_is_alive(self.frame_id)) {
                puts("refferenced stack object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(self.heap) {
            if(!come_is_alive(self.heaptop)) {
                puts("refferenced heap object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        
        *self.p = item;
    }
}

//////////////////////////////
// optional
//////////////////////////////
enum Result<T> {
    Some(T),
    None,
};

impl Result<T>
{
    bool is_ok(Result<T>* self)
    {
        return self != null && self.is_Some();
    }
    bool is_err(Result<T>* self)
    {
        return !self.is_ok();
    }
    T unwrap_or(Result<T>* self, T^ default_value) {
        if(self.is_err()) {
            return dummy_heap default_value;
        }

        return dummy_heap self.get_Some();
    }
    T unwrap_or_default(Result<T>* self) {
        T default_value;
        memset(&default_value, 0, sizeof(T));

        return self.unwrap_or(default_value);
    }
    T expect(Result<T>* self, const char* message) {
        if(self.is_err()) {
            puts(message);
            stackframe2(self);
            exit(2);
        }

        return dummy_heap self.get_Some();
    }
    T unwrap(Result<T>* self) {
        if(self.is_err()) {
            puts("exception");
            stackframe2(self);
            exit(2);
        }

        return dummy_heap self.get_Some();
    }
};

struct optional<T>
{
    T p;   // owned
    bool global;
    bool heap;
    bool local;
    
    unsigned long frame_id;
};

impl optional<T>
{
    optional<T>*% initialize(optional<T>*% self, T p, bool global_, bool heap_, bool local_, unsigned long frame_id) {
        self.p = p;
        self.global = global_;
        self.heap = heap_;
        self.local = local_;
        self.frame_id = frame_id;
        return self;
    }
    
    _norecord T unwrap(optional<T>* self) {
        using unsafe;
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        if(self.local) {
            if(!neo_frame_is_alive(self.frame_id)) {
                puts("refferenced stack object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(self.heap) {
            if(!come_is_alive(self.p)) {
                puts("refferenced heap object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(ispointer(T) && self.p == (void*)0) {
            puts("null pointer exception");
            stackframe();
            exit(2);
        }
        return dummy_heap self.p;
    }
    _norecord T] operator_derefference(optional<T>* self)
    {
        using unsafe;
        
        if(self == null) {
            puts("null pointer exception");
            stackframe();
            exit(2);
        }
        
        if(self.local) {
            if(!neo_frame_is_alive(self.frame_id)) {
                puts("refferenced stack object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(self.heap) {
            if(!come_is_alive(self.p)) {
                puts("refferenced heap object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(ispointer(T) && self.p == (void*)0) {
            puts("null pointer exception");
            stackframe();
            exit(2);
        }
        
        T p = self.p;
        
        return *p;
    }
    _norecord void operator_store_derefference(optional<T>* self, T] item)
    {
        using unsafe;
        
        if(self == null) {
            puts("null pointer exception");
            stackframe();
            exit(2);
        }
        
        if(self.local) {
            if(!neo_frame_is_alive(self.frame_id)) {
                puts("refferenced stack object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(self.heap) {
            if(!come_is_alive(self.p)) {
                puts("refferenced heap object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(ispointer(T) && self.p == (void*)0) {
            puts("null pointer exception");
            stackframe();
            exit(2);
        }
        
        T p = self.p;
        
        *p = item;
    }
}

//////////////////////////////
// span
//////////////////////////////
struct span<T> {
    char* memory;
    T^ p;
    size_t len;
    bool local;
    bool heap;
    bool global;
    unsigned long frame_id;
};

impl span<T>
{
    span<T>*% initialize(span<T>*% self, void* head, size_t len, bool local, bool heap, bool global, unsigned long frame_id) {
        using unsafe; 
        
        if(!ispointer(T)) {
            puts("invalid span");
            stackframe2(self);
            exit(2);
        }
        self.memory = (char*)head;
        
        self.p = (T^)head;
        self.len = len;
        self.local = local;
        self.heap = heap;
        self.global = global;
        self.frame_id = frame_id;
        
        return self;
    }
    size_t position(span<T>* self) {
        return self.p - (T^)self.memory;
    }
    _norecord T^ unwrap(span<T>* self) {
        using unsafe; 
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        if(self->local) {
            if(!neo_frame_is_alive(self->frame_id)) {
                puts("refferenced stack object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(self.heap) {
            if(!come_is_alive(self.memory)) {
                puts("refferenced heap object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(sizeof(T]) > self.len) {
            puts("invalid span. len is few");
            stackframe2(self);
            exit(2);
        }
        if(self.p >= (char*)self.memory + self.len) {
            puts("out of range of span(1)");
            stackframe2(self);
            exit(1);
        }
        if(self.p < (char*)self.memory) {
            puts("out of range of span(2)");
            stackframe2(self);
            exit(1);
        }
        
        return (T^)self.p;
    }
    
    _norecord span<T>* operator_plus_plus(span<T>* self) {
        using unsafe;
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        
        self.p++;
        
        return self;
    }
    _norecord span<T>* operator_plus_equal(span<T>* self, size_t value) {
        using unsafe;
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        
        self.p += value;
        
        return self;
    }
    
    _norecord span<T>* operator_minus_minus(span<T>* self) {
        using unsafe;
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        
        self.p--;
        
        return self;
    }
    
    _norecord span<T>* operator_minus_equal(span<T>* self, size_t value) {
        using unsafe;
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        
        self.p -= value;
        
        return self;
    }
    
    _norecord T^ operator_add(span<T>* self, size_t rvalue) {
        using unsafe;
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        
        T^ result = self.p + rvalue;
        
        return result;
    }
    
    _norecord T^ operator_sub(span<T>* self, size_t rvalue) {
        using unsafe;
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        
        T^ result = self.p - rvalue;
        
        return result;
    }
    
    _norecord T]^ operator_derefference(span<T>* self) {
        using unsafe;
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        
        if(self->local) {
            if(!neo_frame_is_alive(self->frame_id)) {
                puts("refferenced stack object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(self.heap) {
            if(!come_is_alive(self.memory)) {
                puts("refferenced heap object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        
        T^ p = self.p;
        
        if(sizeof(T]) > self.len) {
            puts("invalid span. len is few");
            stackframe2(self);
            exit(2);
        }
        if(self.p >= (char*)self.memory + self.len) {
            puts("out of range of span(3)");
            stackframe2(self);
            exit(1);
        }
        if(self.p < (char*)self.memory) {
            puts("out of range of span(4)");
            stackframe2(self);
            exit(1);
        }
        
        return *p;
    }
    _norecord void operator_store_derefference(span<T>* self, T^] item) {
        self.operator_store_element(0, item);
    }
    _norecord void operator_store_element(span<T>* self, int position, T^] item) {
        using unsafe; 
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        
        if(self->local) {
            if(!neo_frame_is_alive(self->frame_id)) {
                puts("refferenced stack object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(self.heap) {
            if(!come_is_alive(self.memory)) {
                puts("refferenced heap object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(sizeof(T]) > self.len) {
            puts("invalid span. len is few");
            stackframe2(self);
            exit(2);
        }
        if(self.p + position >= (char*)self.memory + self.len) {
            puts("out of range of span(5)");
            stackframe2(self);
            exit(1);
        }
        if(self.p + position < (char*)self.memory) {
            puts("out of range of span(6)");
            stackframe2(self);
            exit(1);
        }
        
        T^ p = self.p;
        
        p\[position] = item;
    }
    _norecord T^] operator_load_element(span<T>* self, int position) {
        using unsafe; 
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        
        if(self->local) {
            if(!neo_frame_is_alive(self->frame_id)) {
                puts("refferenced stack object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(self.heap) {
            if(!come_is_alive(self.memory)) {
                puts("refferenced heap object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(sizeof(T]) > self.len) {
            puts("invalid span. len is few");
            stackframe2(self);
            exit(2);
        }
        if(self.p + position >= (char*)self.memory + self.len) {
            puts("out of range of span(7)");
            stackframe2(self);
            exit(1);
        }
        if(self.p + position < (char*)self.memory) {
            puts("out of range of span(8)");
            stackframe2(self);
            exit(1);
        }
        
        T^ p = self.p;
        
        return p\[position];
    }
    
    _norecord string to_string(span<T>* self) {
        using unsafe; 
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        
        return s"head \{self.memory} p \{self.p} len \{self.len}";
    }
    _norecord int len(span<T>* self) {
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        return self.len;
    }
    _norecord int memcmp(span<T>* self, void* mem, size_t len) {
        using unsafe; 
        
        if(self == null) {
            puts("null pointer exception. self is null");
            stackframe();
            exit(2);
        }
        
        if(self->local) {
            if(!neo_frame_is_alive(self->frame_id)) {
                puts("refferenced stack object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(self.heap) {
            if(!come_is_alive(self.memory)) {
                puts("refferenced heap object is vanished");
                stackframe2(self);
                exit(127);
            }
        }
        if(sizeof(T]) > self.len) {
            puts("invalid span. len is few");
            stackframe2(self);
            exit(2);
        }
        if((char*)self.p >= (char*)self.memory + self.len) {
            puts("out of range of span(9)");
            stackframe2(self);
            exit(1);
        }
        if((char*)self.p + len >= (char*)self.memory + self.len) {
            puts("out of range of span(10)");
            stackframe2(self);
            exit(1);
        }
        if(self.p < (char*)self.memory) {
            puts("out of range of span(11)");
            stackframe2(self);
            exit(1);
        }
        
        return memcmp((void*)self.p, mem, len);
    }
}

//////////////////////////////
// rawptr
//////////////////////////////
struct rawptr<T> {
    T* p;
};

impl rawptr<T>
{
    rawptr<T>*% initialize(rawptr<T>*% self, void* memory)
    {
        self.p = memory;
        
        return self;
    }
    
    T* unwrap(rawptr<T>* self) {
        return self.p;
    }
    
    rawptr<T>* operator_plus_plus(rawptr<T>* self) {
        using unsafe;
        
        self.p++;
        
        return self;
    }
    rawptr<T>* operator_plus_equal(rawptr<T>* self, size_t value) {
        using unsafe;
        
        self.p += value;
        
        return self;
    }
    
    rawptr<T>* operator_minus_minus(rawptr<T>* self) {
        using unsafe;
        
        self.p--;
        
        return self;
    }
    
    rawptr<T>* operator_minus_equal(rawptr<T>* self, size_t value) {
        using unsafe;
        
        self.p -= value;
        
        return self;
    }
    
    T^ operator_add(rawptr<T>* self, size_t rvalue) {
        using unsafe;
        
        T^ result = self.p + rvalue;
        
        return result;
    }
    
    T^ operator_sub(rawptr<T>* self, size_t rvalue) {
        using unsafe;
        
        T^ result = self.p - rvalue;
        
        return result;
    }
    
    T^ operator_derefference(rawptr<T>* self) {
        using unsafe;
        
        T*^ p = self.p;
        
        return *p;
    }
    void operator_store_derefference(rawptr<T>* self, T] item) {
        using unsafe;
        
        T*^ p = self.p;
        
        *p = item;
    }
    void operator_store_element(rawptr<T>* self, int position, T] item) {
        using unsafe; 
        
        T*^ p = self.p;
        
        p\[position] = item;
    }
    T^ operator_load_element(rawptr<T>* self, int position) {
        using unsafe; 
        
        T*^ p = self.p;
        
        return p\[position];
    }
    
    string to_string(rawptr<T>* self) {
        using unsafe; 
        
        return s"head \{self.memory} p";
    }
}

//////////////////////////////
// list
//////////////////////////////
struct list_item<T>
{
    T item;
    list_item<T>* prev;
    list_item<T>* next;
};

struct list<T>
{
    list_item<T>* head;
    list_item<T>* tail;
    int len;

    list_item<T>* it;
};

impl list <T>
{
    list<T>*% initialize(list<T>*% self) {
        self.head = null;
        self.tail = null;
        self.len = 0;

        return self;
    }
    list<T>*% initialize_with_values(list<T>*% self, int num_value, T^* values) 
    {
        self.head = null;
        self.tail = null;
        self.len = 0;
        
        for(int i=0; i<num_value; i++) {
            self.push_back(dummy_heap values[i]);
        }

        return self;
    }
    void finalize(list<T>* self) {
        if(self == null) return;
        
        list_item<T>* it = self.head;
        while(it != null) {
            var prev_it = it;
            it = it.next;
            delete prev_it;
        }
    }
    list<T>*% clone(list<T>* self) {
        if(self == null) {
            return null;
        }
        var result = new list<T>();

        list_item<T>* it = self.head;
        while(it != null) {
            if(isheap(T)) {
                result.add(clone it.item);
            }
            else {
                result.add(dummy_heap dupe it.item);
            }

            it = it.next;
        }

        return result;
    }
    list<T>* add(list<T>* self, T item)
    {
        if(self == null) return self;
        
        if(self.len == 0) {
            list_item<T>* litem = borrow gc_inc(new list_item<T>);
            
            litem.prev = null;
            litem.next = null;
            litem.item = item;
            
            self.tail = litem;
            self.head = litem;
        }
        else if(self.len == 1) {
            list_item<T>* litem = borrow gc_inc(new list_item<T>);

            litem.prev = self.head;
            litem.next = null;
            litem.item = item;
            
            self.tail = litem;
            self.head.next = litem;
        }
        else {
            list_item<T>* litem = borrow gc_inc(new list_item<T>);

            litem.prev = self.tail;
            litem.next = null;
            litem.item = item;
            
            self.tail.next = litem;
            self.tail = litem;
        }

        self.len++;
        
        return self;
    }
    T pop_front(list<T>* self) {
        T result;

        if(self == null) { return result; }
        
        if(self.len == 1) {
            result = self.head.item;
            
            list_item<T>* litem = self.head;
            self.head = null;
            self.tail = null;
            
            delete borrow litem;
            
            self.len--;
        }
        else if(self.len == 2) {
            list_item<T>* litem = self.head;
            
            result = self.head.item;
            
            self.head = self.head.next;
            self.head.prev = null;
            self.head.next = null;
            self.tail = self.head;
            
            delete borrow litem;
            
            self.len--;
        }
        else if(self.len >= 3) {
            list_item<T>* litem = self.head;
            
            result = self.head.item;
            
            self.head = litem.next;
            self.head.prev = null;
            
            delete borrow litem;
            
            self.len--;
        }
        return result;
    }
    list<T>* push_back(list<T>* self, T item)
    {
        if(self == null) {
            return self;
        }
        
        if(self.len == 0) {
            list_item<T>* litem = borrow gc_inc(new list_item<T>);
            
            litem.prev = null;
            litem.next = null;
            litem.item = item;
            
            self.tail = litem;
            self.head = litem;
        }
        else if(self.len == 1) {
            list_item<T>* litem = borrow gc_inc(new list_item<T>);

            litem.prev = self.head;
            litem.next = null;
            litem.item = item;
            
            self.tail = litem;
            self.head.next = litem;
        }
        else {
            list_item<T>* litem = borrow gc_inc(new list_item<T>);

            litem.prev = self.tail;
            litem.next = null;
            litem.item = item;
            
            self.tail.next = litem;
            self.tail = litem;
        }

        self.len++;
        
        return self;
    }
    
    string to_string(list<T>* self)
    {
        if(self == null) {
            return string("");
        }
        
        buffer*% result = new buffer();
        
        result.append_str("[");
        list_item<T>* it = self.head;
        int i = 0;
        while(it != null) {
            result.append_str(it.item.to_string());
            it = it.next;
            
            i++;
            
            if(i != self.length()) {
                result.append_str(",");
            }
        }
        
        result.append_str("]");
        
        return result.to_string();
    }
    
    T^ begin(list<T>* self) {
        using unsafe;
        
        if(self == null) {
            T^/ result;
            memset(&result, 0, sizeof(T));
            return result;
        }
        self.it = self.head;

        if(self.it) {
            return self.it.item;
        }
        
        T^/ result;
        memset(&result, 0, sizeof(T));
        return result;
    }

    T^ next(list<T>* self) {
        using unsafe;
        
        if(self == null || self.it == null) {
            T^/ result;
            memset(&result, 0, sizeof(T));
            return result;
        }
        
        self.it = self.it.next;

        if(self.it) {
            return self.it.item;
        }
        
        T^/ result;
        memset(&result, 0, sizeof(T));
        return result;
    }

    bool end(list<T>* self) {
        return self == null || self.it == null;
    }
    list<T>* each(list<T>* self, void* parent, void (*block)(void*, T,int,bool*)) 
    {
        if(self == null) {
            return null;
        }
        
        list_item<T>* it = self.head;
        int i = 0;
        while(it != null) {
            bool end_flag = false;
            block(parent, it.item, i, &end_flag);

            if(end_flag == true) {
                break;
            }
            it = it.next;
            i++;
        }
        
        return self;
    }
    T item(list<T>* self, int position, T^ default_value) 
    {
        if(self == null) {
            return dummy_heap default_value;
        }
        
        if(position < 0) {
            position += self.len;
        }

        list_item<T>* it = self.head;
        var i = 0;
        while(it != null) {
            if(position == i) {
                return it.item;
            }
            it = it.next;
            i++;
        };

        return dummy_heap default_value;
    }

    int length(list<T>* self)
    {
        if(self == null) {
            return 0;
        }
        return self.len;
    }
    int len(list<T>* self)
    {
        return self.length();
    }
    bool is_empty(list<T>* self)
    {
        return self.len() == 0;
    }
    
    list<T>* insert(list<T>* self, int position, T item)
    {
        using unsafe;
        
        if(self == null) {
            return null;
        }
        if(position < 0) {
            position += self.len + 1;
        }
        if(position < 0) {
            position = 0;
        }
        if(self.len == 0 || position >= self.len) {
            int len = self.len;
            for(int i=0; i<position-len; i++) {
                T/ default_value;
                memset(&default_value, 0, sizeof(T));
                self.push_back(default_value);
            }
            self.push_back(item);
            return self;
        }

        if(position == 0) {
            list_item<T>* litem = borrow gc_inc(new list_item<T>);

            litem.prev = null;
            litem.next = self.head;
            litem.item = item;
            
            self.head.prev = litem;
            self.head = litem;

            self.len++;
        }
        else if(self.len == 1) {
            var litem = borrow gc_inc(new list_item<T>);

            litem.prev = self.head;
            litem.next = self.tail;
            litem.item = item;
            
            self.tail.prev = litem;
            self.head.next = litem;

            self.len++;
        }
        else {
            list_item<T>* it = self.head;
            int i = 0;
            while(it != null) {
                if(position == i) {
                    list_item<T>* litem = borrow gc_inc(new list_item<T>);

                    litem.prev = it.prev;
                    litem.next = it;
                    litem.item = item;

                    it.prev.next = litem;
                    it.prev = litem;

                    self.len++;
                }

                it = it.next;
                i++;
            }
        }
        
        return self;
    }
    list<T>* reset(list<T>* self) {
        if(self == null) {
            return self;
        }
        
        list_item<T>* it = self.head;
        while(it != null) {
            var prev_it = it;
            it = it.next;
            delete prev_it;
        }

        self.head = null;
        self.tail = null;

        self.len = 0;
        
        return self;
    }
    list<T>* remove(list<T>* self, T^ item, bool by_pointer=false) {
        if(self == null) {
            return self;
        }
        
        int it2 = 0;
        list_item<T>* it = self.head;
        while(it != null) {
            if((!by_pointer && it.item.equals(item)) || (by_pointer && it.item == item)) {
                self.delete(it2, it2+1);
                break;
            }
            it2++;
            
            it = it.next;
        }
        
        return self;
    }
    list<T>* delete(list<T>* self, int head, int tail)
    {
        if(self == null) {
            return self;
        }
        
        if(head < 0) {
            head += self.len;
        }
        if(tail < 0) {
            tail += self.len + 1;
        }

        if(head > tail) {
            int tmp = tail;
            tail = head;
            head = tmp;
        }

        if(head < 0) {
            head = 0;
        }

        if(tail > self.len) {
            tail = self.len;
        }
        
        if(head >= self.len) {
            return self;
        }

        if(head == tail) {
            return self;
        }
        
        if(head == 0 && tail == self.len) 
        {
            self.reset();
        }
        else if(head == 0) {
            list_item<T>* it = self.head;
            int i = 0;
            while(it != null) {
                if(i < tail) {
                    list_item<T>* prev_it = it;

                    it = it.next;
                    i++;

                    delete prev_it;

                    self.len--;
                }
                else if(i == tail) {
                    self.head = it;
                    self.head.prev = null;
                    break;
                }
                else {
                    it = it.next;
                    i++;
                }
            }
        }
        else if(tail == self.len) {
            list_item<T>* it = self.head;
            int i = 0;
            while(it != null) {
                if(i == head) {
                    self.tail = it.prev;
                    self.tail.next = null;
                }

                if(i >= head) {
                    list_item<T>* prev_it = it;

                    it = it.next;
                    i++;

                    delete prev_it;

                    self.len--;
                }
                else {
                    it = it.next;
                    i++;
                }
            }
        }
        else {
            list_item<T>* it = self.head;

            list_item<T>* head_prev_it = null;
            list_item<T>* tail_it = null;


            int i = 0;
            while(it != null) {
                if(i == head) {
                    head_prev_it = it.prev;
                }
                if(i == tail) {
                    tail_it = it;
                }

                if(i >= head && i < tail) 
                {
                    list_item<T>* prev_it = it;

                    it = it.next;
                    i++;

                    delete prev_it;

                    self.len--;
                }
                else {
                    it = it.next;
                    i++;
                }
            }

            if(head_prev_it != null) {
                head_prev_it.next = tail_it;
            }
            if(tail_it != null) {
                tail_it.prev = head_prev_it;
            }
        }
        
        return self;
    }
    list<T>* replace(list<T>* self, int position, T item)
    {
        using unsafe; 
        
        if(self == null) {
            return self;
        }
        
        if(position < 0) {
            position += self.len;
        }
        if(position < 0) {
            position = 0;
        }
        
        if(self.len == 0 || position >= self.len) {
            int len = self.len;
            for(int i=0; i<position-len; i++) {
                T/ default_value;
                memset(&default_value, 0, sizeof(T));
                self.push_back(default_value);
            }
            self.push_back(item);
            return self;
        }

        list_item<T>* it = self.head;
        int i = 0;
        while(it != null) {
            if(position == i) {
                it.item = item;
                break;
            }
            it = it.next;
            i++;
        }
        
        return self;
    }

    int find(list<T>* self, T^ item, int default_value, bool by_pointer=false) {
        if(self == null) {
            return default_value;
        }
        
        int it2 = 0;
        list_item<T>* it = self.head;
        while(it != null) {
            if((!by_pointer && it.item.equals(item)) || (by_pointer && it.item == item)) {
                return it2;
            }
            it2++;
            
            it = it.next;
        }

        return default_value;
    }
    bool equals(list<T>* left, list<T>* right)
    {
        if(left == null && right == null) {
            return true;
        }
        else if(left == null || right == null) {
            return false;
        }
        
        if(left.len != right.len) {
            return false;
        }

        list_item<T>* it = left.head;
        list_item<T>* it2 = right.head;

        while(it != null) {
            if(!it.item.equals(it2.item)) {
                return false;
            }

            it = it.next;
            it2 = it2.next;
        }

        return true;
    }
    list<T>*% sublist(list<T>* self, int begin, int tail) {
        if(self == null) {
            return dummy_heap self;
        }
        
        list<T>*% result = new list<T>.initialize();

        if(begin < 0) {
            begin += self.len;
        }

        if(tail < 0) {
            tail += self.len + 1;
        }

        if(begin < 0) {
            begin = 0;
        }
        
        if(begin >= self.len) {
            return new list<T>();
        }

        if(tail >= self.len) {
            tail = self.len;
        }

        list_item<T>* it = self.head;
        int i = 0;
        while(it != null) {
            if(i >= begin && i < tail) {
                result.push_back(it.item);
            }
            it = it.next;
            i++;
        };

        return result;
    }
    void operator_store_element(list<T>* self, int position, T item) {
        self.replace(position, item);
    }
    T operator_load_element(list<T>* self, int position) {
        using unsafe;
        
        if(self == null) {
            T/ default_value;
            memset(&default_value, 0, sizeof(T));
            return default_value;
        }
        
        if(position < 0) {
            position += self.len;
        }
        
        list_item<T>* it = self.head;
        var i = 0;
        while(it != null) {
            if(position == i) {
                return it.item;
            }
            it = it.next;
            i++;
        };

        T/ default_value;
        memset(&default_value, 0, sizeof(T));
        return default_value;
    }
    Result<T>*% operator_load_element_optional(list<T>* self, int position) {
        using unsafe;

        if(self == null) {
            return new Result<T>.None();
        }

        if(position < 0) {
            position += self.len;
        }

        list_item<T>* it = self.head;
        var i = 0;
        while(it != null) {
            if(position == i) {
                if(isheap(T)) {
                    return new Result<T>.Some(gc_inc(it.item));
                }
                return new Result<T>.Some(it.item);
            }
            it = it.next;
            i++;
        };

        return new Result<T>.None();
    }
    list<T>*% operator_load_range_element(list<T>* self, int begin, int tail) {
        list<T>*% result = new list<T>.initialize();
        
        if(self == null) {
            return result;
        }

        if(begin < 0) {
            begin += self.len;
        }

        if(tail < 0) {
            tail += self.len + 1;
        }

        if(begin < 0) {
            begin = 0;
        }

        if(tail >= self.len) {
            tail = self.len;
        }
        
        if(begin >= self.len) {
            return result;
        }

        list_item<T>* it = self.head;
        int i = 0;
        while(it != null) {
            if(i >= begin && i < tail) {
                result.push_back(it.item);
            }
            it = it.next;
            i++;
        };

        return result;
    }
    bool operator_equals(list<T>* left, list<T>* right) 
    {
        if(left == null && right == null) {
            return true;
        }
        else if(left == null || right == null) {
            return false;
        }
        
        if(left.len != right.len) {
            return false;
        }

        list_item<T>* it = left.head;
        list_item<T>* it2 = right.head;

        while(it != null) {
            if(!(it.item === it2.item)) {
                return false;
            }

            it = it.next;
            it2 = it2.next;
        }

        return true;
    }
    bool operator_not_equals(list<T>* left, list<T>* right) {
        return !left.operator_equals(right);
    }
    bool contained(list<T>* self, T^ item, bool by_pointer=false) {
        if(self == null) {
            return false;
        }
        
        for(var it = self.begin(); !self.end(); it = self.next()) {
            if((!by_pointer && it.equals(item)) || (by_pointer && it == item)) {
                return true;
            }
        }
        
        return false;
    }
    list<T>*% merge_list_with_lambda(list<T>* left, list<T>* right, int (*compare)(T^,T^)) {
        var result = new list<T>.initialize();

        list_item<T>* it = left.head;
        list_item<T>* it2= right.head;

        while(true) {
            if(it && it2) {
                if(it.item == null) {
                    it = it.next;
                }
                else if(it2.item == null) {
                    it2 = it2.next;
                }
                else if(compare(it.item, it2.item) <= 0) 
                {
                    if(isheap(T)) {
                        result.push_back(clone it.item);
                    }
                    else {
                        result.push_back(dummy_heap dupe it.item);
                    }

                    it = it.next;
                }
                else {
                    if(isheap(T)) {
                        result.push_back(clone it2.item);
                    }
                    else {
                        result.push_back(dummy_heap dupe it2.item);
                    }


                    it2 = it2.next;
                }
            }

            if(it == null) {
                if(it2 != null) {
                    while(it2 != null) {
                        if(isheap(T)) {
                            result.push_back(clone it2.item);
                        }
                        else {
                            result.push_back(dummy_heap dupe it2.item);
                        }

                        it2 = it2.next;
                    }
                }
                break;
            }
            else if(it2 == null) {
                if(it != null) {
                    while(it != null) {
                        if(isheap(T)) {
                            result.push_back(clone it.item);
                        }
                        else {
                            result.push_back(dummy_heap dupe it.item);
                        }

                        it = it.next;
                    }
                }
                break;
            }
        }

        return result;
    }
    list<T>*% merge_sort_with_lambda(list<T>* self, int (*compare)(T^,T^)) {
        if(self.head == null) {
            return clone self;
        }
        if(self.head.next == null) {
            return clone self;
        }

        var list1 = new list<T>.initialize();
        var list2 = new list<T>.initialize();

        list_item<T>* it = self.head;

        while(true) {
            list1.push_back(dupe it.item);
            list2.push_back(dupe it.next.item);

            if(it.next.next == null) {
                break;
            }

            it = it.next.next;

            if(it.next == null) {
                if(isheap(T)) {
                    list1.push_back(clone it.item);
                }
                else {
                    list1.push_back(dummy_heap dupe it.item);
                }
                break;
            }
        }
        
        var left_list = list1.merge_sort_with_lambda(compare);
        var right_list = list2.merge_sort_with_lambda(compare);
        
        return left_list.merge_list_with_lambda(right_list, compare);
    }
    list<T>*% sort_with_lambda(list<T>* self, int (*compare)(T^,T^)) {
        if(self == null) {
            return new list<T>();
        }
        return self.merge_sort_with_lambda(compare);
    }
    list<T>*% sort(list<T>* self) {
        if(self == null) {
            return new list<T>();
        }
        return self.merge_sort_with_lambda(int lambda(T^ left, T^ right) { return left.compare(right); });
    }
    
    template<R> list<R>*% map(list<T>* self, void* parent, R (*block)(void*, T))
    {
        if(self == null) {
            return new list<R>();
        }
        var result = new list<R>.initialize();

        list_item<T>* it = self.head;
        while(it != null) {
            R item = block(parent, it.item);
            result.push_back(item);

            it = it.next;
        }

        return result;
    }
    list<T>*% reverse(list<T>* self) {
        list<T>%* result = new list<T>();
        
        if(self == null) { 
            return result;
        }

        list_item<T>* it = self.tail;
        while(it != null) {
            result.push_back(dupe it.item);
            it = it.prev;
        };

        return result;
    }
    list<T>*% uniq(list<T>* self, bool by_pointer=false) {
        list<T>*% result = new list<T>.initialize();
        
        if(self == null) {
            return result;
        }

        if(self.length() > 0) {
            T item_before = self.head.item;

            result.push_back(dupe item_before);

            list_item<T>* it = self.head;
            it = it.next;
            while(it != null) {
                if(!((!by_pointer && it.item.equals(item_before)) || (by_pointer && it.item == item_before))) {
                    result.push_back(dupe it.item);
                }

                item_before = it.item;
                
                it = it.next;
            }
        }

        return result;
    }
    list<T>*% filter(list<T>* self, void* parent, bool (*block)(void*, T))
    {
        list<T>*% result = new list<T>();
        
        if(self == null) {
            return result;
        }

        list_item<T>* it = self.head;
        while(it != null) {
            if(block(parent, it.item)) {
                result.push_back(dupe it.item);
            }

            it = it.next;
        }

        return result;
    } 
    bool any(list<T>* self, void* parent, bool (*block)(void*, T))
    {
        if(self == null) {
            return false;
        }
        
        list_item<T>* it = self.head;
        while(it != null) {
            if(block(parent, it.item)) {
                return true;
            }
            it = it.next;
        }
        
        return false;
    }
    bool all(list<T>* self, void* parent, bool (*block)(void*, T))
    {
        if(self == null) {
            return true;
        }
        
        list_item<T>* it = self.head;
        while(it != null) {
            if(!block(parent, it.item)) {
                return false;
            }
            it = it.next;
        }
        
        return true;
    }
    int count(list<T>* self)
    {
        return self.length();
    }
    int position(list<T>* self, int default_value, void* parent, bool (*block)(void*, T))
    {
        if(self == null) {
            return default_value;
        }
        
        list_item<T>* it = self.head;
        int i = 0;
        while(it != null) {
            if(block(parent, it.item)) {
                return i;
            }
            it = it.next;
            i++;
        }
        
        return default_value;
    }
    T find_value(list<T>* self, T^ default_value, void* parent, bool (*block)(void*, T))
    {
        if(self == null) {
            return dummy_heap default_value;
        }
        
        list_item<T>* it = self.head;
        while(it != null) {
            if(block(parent, it.item)) {
                return dummy_heap it.item;
            }
            it = it.next;
        }
        
        return dummy_heap default_value;
    }
    T nth(list<T>* self, int index, T^ default_value)
    {
        if(self == null) {
            return dummy_heap default_value;
        }
        
        if(index < 0) {
            index += self.len;
        }
        
        list_item<T>* it = self.head;
        int i = 0;
        while(it != null) {
            if(i == index) {
                return dummy_heap it.item;
            }
            it = it.next;
            i++;
        }
        
        return dummy_heap default_value;
    }
    T last(list<T>* self, T^ default_value)
    {
        if(self == null || self.tail == null) {
            return dummy_heap default_value;
        }
        
        return dummy_heap self.tail.item;
    }
    T sum(list<T>* self)
    {
        T result;
        memset(&result, 0, sizeof(T));
        
        if(self == null) {
            return result;
        }
        
        list_item<T>* it = self.head;
        while(it != null) {
            result += it.item;
            it = it.next;
        }
        
        return result;
    }
    T product(list<T>* self)
    {
        T result = 1;
        
        if(self == null) {
            return result;
        }
        
        list_item<T>* it = self.head;
        while(it != null) {
            result *= it.item;
            it = it.next;
        }
        
        return result;
    }
    T min(list<T>* self, T^ default_value)
    {
        if(self == null || self.head == null) {
            return dummy_heap default_value;
        }
        
        bool first = true;
        T result = dummy_heap default_value;
        list_item<T>* it = self.head;
        while(it != null) {
            if(first || it.item.compare(result) < 0) {
                result = dummy_heap it.item;
                first = false;
            }
            it = it.next;
        }
        
        return dummy_heap result;
    }
    T max(list<T>* self, T^ default_value)
    {
        if(self == null || self.head == null) {
            return dummy_heap default_value;
        }
        
        bool first = true;
        T result = dummy_heap default_value;
        list_item<T>* it = self.head;
        while(it != null) {
            if(first || it.item.compare(result) > 0) {
                result = dummy_heap it.item;
                first = false;
            }
            it = it.next;
        }
        
        return dummy_heap result;
    }
    
    list<T>*% operator_add(list<T>*% left, list<T>*% right) {
        list<T>*% result = new list<T>();
        
        if(left == null || right == null) {
            return result;
        }

        list_item<T>* it = left.head;
        while(it != null) {
            result.push_back(dupe it.item);

            it = it.next;
        }

        it = right.head;
        while(it != null) {
            result.push_back(dupe it.item);

            it = it.next;
        }

        return result;
    }
    list<T>*% operator_mult(list<T>* left, int right) {
        list<T>*% result = new list<T>();
        
        if(left == null) {
            return result;
        }

        for(int i=0; i<right; i++) {
            list_item<T>* it = left.head;
            while(it != null) {
                result.push_back(dupe it.item);
    
                it = it.next;
            }
        }

        return result;
    }
    string join(list<T>* self, const char* sep=" ") {
        if(self == null) {
            return string("");
        }
        
        buffer*% buf = new buffer();
        
        int n = 0;
        for(var it = self.begin(); !self.end(); it = self.next()) {
            buf.append_str(it);
            
            if(n < self.length()-1) {
                buf.append_str(sep);
            }
            
            n++;
        }
        
        return buf.to_string();
    }
}


//////////////////////////////
// list
//////////////////////////////
struct vector<T> 
{
    T^* items;
    int len;
    int size;

    int it;
};

impl vector<T> 
{
    vector<T>*% initialize(vector<T>*% self) 
    {
#ifdef __NEO_MICRO_RAM_8K__
        self.size = 4;
#else
        self.size = 16;
#endif
        self.len = 0;
        self.items = borrow new T[self.size];
        
        return self;
    }
    vector<T>*% initialize_with_values(vector<T>*% self, int num_value, T^* values) 
    {
        using unsafe;
        
        self.size = num_value;
        self.len = num_value;
        self.items = borrow new T[self.size];
        
        memcpy(self.items, values, sizeof(T)*self.size);
        
        return self;
    }

    vector<T>*% clone(vector<T>* self)
    {
        using unsafe;
        
        vector<T>*% result = new vector<T>;

        result.len = self.len;
        result.size = self.size;
        result.it = 0;
        result.items = borrow new T[result.size];
        
        if(isheap(T)) {
            for(int i=0; i<self.len; i++) 
            {
                result.items[i] = gc_inc(clone self.items[i]);
            }
        }
        else {
            for(int i=0; i<self.len; i++) 
            {
                result.items[i] = self.items[i];
            }
        }

        return result;
    }

    void finalize(vector<T>* self)
    {
        using unsafe;
        
        if(isheap(T)) {
            for(int i=0; i<self.len; i++) 
            {
                delete borrow self.items[i];
            }
        }
        if(self && self.items) {
            come_free((char*)self.items);
        }
    }
    
    vector<T>*% operator_add(vector<T>* left, vector<T>* right) {
        using unsafe;
        
        vector<T>*% result = new vector<T>.initialize();
        
        foreach(it, left) {
            if(isheap(T)) {
                result.add(clone it);
            }
            else {
                result.add(it);
            }
        }
        
        foreach(it, right) {
            if(isheap(T)) {
                result.add(clone it);
            }
            else {
                result.add(dummy_heap it);
            }
        }
        
        return result;
    }
    vector<T>*% operator_mult(vector<T>* left, int n) {
        using unsafe;
        
        vector<T>*% result = new vector<T>.initialize();
        
        for(int i=0; i<n; i++) {
            foreach(it, left) {
                if(isheap(T)) {
                    result.add(clone it);
                }
                else {
                    result.add(dummy_heap it);
                }
            }
        }
        
        return result;
    }
    bool operator_equals(vector<T>* left, vector<T>* right) {
        return left.equals(right);
    }
    bool operator_not_equals(vector<T>* left, vector<T>* right) {
        return !left.equals(right);
    }
    void operator_store_element(vector<T>* self, int index, T item) {
        self.replace(index, item);
    }
    
    vector<T>* add(vector<T>* self, T item) {
        using unsafe;
        
        if(self.len == self.size) {
            auto new_size = self.size + self.size / 2;
            if(new_size <= self.size) {
                new_size = self.size + 1;
            }
            auto items = self.items;

            self.items = come_calloc(1, sizeof(T)*new_size);

            int i;
            for(i=0; i<self.size; i++) {
                self.items[i] = items[i];
            }

            self.size = new_size;

            come_free((char*)items);
        }

        if(isheap(T)) {
            self.items[self.len] = gc_inc(item);
        }
        else {
            self.items[self.len] = item;
        }
        self.len++;
        
        return self;
    }

    T^ item(vector<T>* self, int index, T^ default_value) 
    {
        using unsafe;
        
        if(index < 0) {
            index += self.len;
        }

        if(index >= 0 && index < self.len)
        {
            return self.items[index];
        }

        return default_value;
    }


    bool equals(vector<T>* left, vector<T>* right)
    {
        using unsafe;
        
        if(left.len != right.len) {
            return false;
        }

        for(int i=0; i<left.len; i++) {
            if(!(left.items[i].equals(right.items[i])))
            {
                return false;
            }
        }

        return true;
    }
    
    void replace(vector<T>* self, int index, T value)
    {
        using unsafe;
        
        if(index < 0) {
            index += self.len;
        }

        if(index >= 0 && index < self.len)
        {
            if(isheap(T)) {
                delete borrow self.items[index];
            }

            if(isheap(T)) {
                self.items[index] = gc_inc(value);
            }
            else {
                self.items[index] = value;
            }
        }
    }
    
    int find(vector<T>* self, T^ item, int default_value) {
        using unsafe;
        
        int it2 = 0;
        foreach(it, self) {
            if(it.equals(item)) {
                return it2;
            }
            it2++;
        }

        return default_value;
    }
    bool any(vector<T>* self, void* parent, bool (*block)(void*, T))
    {
        using unsafe;
        
        if(self == null) {
            return false;
        }
        
        for(int i=0; i<self.len; i++) {
            if(block(parent, self.items[i])) {
                return true;
            }
        }
        
        return false;
    }
    bool all(vector<T>* self, void* parent, bool (*block)(void*, T))
    {
        using unsafe;
        
        if(self == null) {
            return true;
        }
        
        for(int i=0; i<self.len; i++) {
            if(!block(parent, self.items[i])) {
                return false;
            }
        }
        
        return true;
    }
    int count(vector<T>* self)
    {
        return self.length();
    }
    int position(vector<T>* self, int default_value, void* parent, bool (*block)(void*, T))
    {
        using unsafe;
        
        if(self == null) {
            return default_value;
        }
        
        for(int i=0; i<self.len; i++) {
            if(block(parent, self.items[i])) {
                return i;
            }
        }
        
        return default_value;
    }
    T^ find_value(vector<T>* self, T^ default_value, void* parent, bool (*block)(void*, T))
    {
        using unsafe;
        if(self == null) {
            return default_value;
        }
        
        for(int i=0; i<self.len; i++) {
            if(block(parent, self.items[i])) {
                return self.items[i];
            }
        }
        
        return default_value;
    }
    T^ nth(vector<T>* self, int index, T^ default_value)
    {
        using unsafe;
        if(self == null) {
            return default_value;
        }
        
        if(index < 0) {
            index += self.len;
        }
        
        if(index < 0 || index >= self.len) {
            return default_value;
        }
        
        return self.items[index];
    }
    T^ last(vector<T>* self, T^ default_value)
    {
        return self.nth(-1, default_value);
    }
    T sum(vector<T>* self)
    {
        using unsafe;
        
        T result;
        memset(&result, 0, sizeof(T));
        
        if(self == null) {
            return result;
        }
        
        for(int i=0; i<self.len; i++) {
            result += self.items[i];
        }
        
        return result;
    }
    T product(vector<T>* self)
    {
        using unsafe;
        
        T result = 1;
        
        if(self == null) {
            return result;
        }
        
        for(int i=0; i<self.len; i++) {
            result *= self.items[i];
        }
        
        return result;
    }
    T^ min(vector<T>* self, T^ default_value)
    {
        using unsafe;
        if(self == null || self.len == 0) {
            return default_value;
        }
        
        T result = default_value;
        for(int i=0; i<self.len; i++) {
            if(i == 0 || self.items[i].compare(result) < 0) {
                result = self.items[i];
            }
        }
        
        return result;
    }
    T^ max(vector<T>* self, T^ default_value)
    {
        using unsafe;
        if(self == null || self.len == 0) {
            return default_value;
        }
        
        T result = default_value;
        for(int i=0; i<self.len; i++) {
            if(i == 0 || self.items[i].compare(result) > 0) {
                result = self.items[i];
            }
        }
        
        return result;
    }

    int length(vector<T>* self)
    {
        using unsafe;
        
        return self.len;
    }
    int len(vector<T>* self)
    {
        return self.length();
    }
    bool is_empty(vector<T>* self)
    {
        return self.len() == 0;
    }
    
    int alloc_size(vector<T>* self)
    {
        using unsafe;
        
        return sizeof(T) * self.len;
    }

    void reset(vector<T>* self) {
        using unsafe;
        
        if(isheap(T)) {
            for(int i=0; i<self.len; i++) 
            {
                delete borrow self.items[i];
            }
        }
        come_free((char*)self.items);
        
        self.size = 16;
        self.len = 0;
        self.items = borrow new T[self.size];
        
        self.len = 0;
    }

    T^ begin(vector<T>* self) {
        using unsafe;
        
        self.it = 0;

        T^/ default_value;
        return self.item(0, default_value);
    }

    T^ next(vector<T>* self) {
        using unsafe;
        
        self.it++;

        T^/ default_value;
        return self.item(self.it, default_value);
    }

    bool end(vector<T>* self) {
        using unsafe;
        
        return self.it >= self.len;
    }
    vector<T>* iter(vector<T>* self)
    {
        // Zero-cost adapter: no wrapper object, just return self.
        return self;
    }
    
    T^ operator_load_element(vector<T>* self, int position) {
        using unsafe;
        
        T^/ default_value;
        memset(&default_value, 0, sizeof(T));
        
        return self.item(position, default_value);
    }
    Result<T>*% operator_load_element_optional(vector<T>* self, int position) {
        using unsafe;

        if(self == null) {
            return new Result<T>.None();
        }

        if(position < 0) {
            position += self.len;
        }

        if(position >= 0 && position < self.len) {
            if(isheap(T)) {
                return new Result<T>.Some(gc_inc(self.items[position]));
            }
            return new Result<T>.Some(self.items[position]);
        }

        return new Result<T>.None();
    }
    vector<T>*% operator_load_range_element(vector<T>* self, int begin, int tail) {
        vector<T>*% result = new vector<T>.initialize();
        
        if(self == null) {
            return result;
        }

        if(begin < 0) {
            begin += self.len;
        }

        if(tail < 0) {
            tail += self.len + 1;
        }

        if(begin < 0) {
            begin = 0;
        }

        if(begin >= self.len) {
            return result;
        }

        if(tail >= self.len) {
            tail = self.len;
        }

        for(int i=begin; i<tail; i++) {
            if(isheap(T)) {
                result.add(clone self.items[i]);
            }
            else {
                result.add(self.items[i]);
            }
        }

        return result;
    }
    vector<T>*% operator_load_range(vector<T>* self, int begin, int tail) {
        return self.operator_load_range_element(begin, tail);
    }
    void operator_store_element(list<T>* self, int position, T item) {
        self.replace(position, item);
    }
    
    vector<T>* quick_sort(vector<T>* self, int left, int right, int (*compare)(T,T)) {
        int l_hold = left;
        int r_hold = right;

        T^ pivot = self.items[left];
        int pivot_num = left;

        while(left < right) {
            while((compare(self.items[right], pivot) >= 0) && (left < right)) {
                right--;
            }

            if(left != right) {
                self.items[left] = self.items[right];
                left++;
            }

            while((compare(self.items[left], pivot) <= 0) && (left < right)) {
                left++;
            }

            if(left != right) {
                self.items[right] = self.items[left];
                right--;
            }
        }

        self.items[left] = pivot;

        pivot_num = left;

        int left2 = l_hold;
        int right2 = r_hold;

        if(left2 < pivot_num) {
            self.quick_sort(left2, pivot_num-1, compare);
        }

        if(right2 > pivot_num) {
            self.quick_sort(pivot_num+1, right2, compare);
        }

        return self;
    }
    vector<T>* sort_with_lambda(vector<T>* self, int (*compare)(T^, T^)) {
        return self.quick_sort(0, self.length()-1, fun);
    }
    vector<T>* sort(vector<T>* self) {
        return self.quick_sort(0, self.length()-1, int lambda(T^ left, T^ right) { return left.compare(right); });
    }
    string to_string(vector<T>* self)
    {
        if(self == null) {
            return string("");
        }
        
        buffer*% result = new buffer();
        
        result.append_str("v[");
        int i;
        foreach(it, self) {
            result.append_str(it.to_string());
            
            i++;
            
            if(i != self.length()) {
                result.append_str(",");
            }
        }
        
        result.append_str("]");
        
        return result.to_string();
    }
    
    span<T*>*% to_span(vector<T>* self)
    {
        using unsafe;
        
        if(self == null || self.items == null) {
            return null;
        }
        
        return new span<T*> {
            memory: (char*)self.items,
            p: self.items,
            len: sizeof(T) * self.len,
            local: false,
            heap: true,
            global: false,
            frame_id:0
        };
    }
}

impl list<T>
{
    vector<T>*% to_vector(list<T>* self)
    {
        vector<T>*% result = new vector<T>.initialize();
        
        if(self == null) {
            return result;
        }
        
        list_item<T>* it = self.head;
        while(it != null) {
            if(isheap(T)) {
                result.add(clone it.item);
            }
            else {
                result.add(dummy_heap dupe it.item);
            }
            it = it.next;
        }
        
        return result;
    }
}

//////////////////////////////
// map
//////////////////////////////
struct map<T, T2>
{
    T*^ keys;
    unsigned int* hashes;
    bool* item_existance;
    T2*^ items;
    int size;
    int len;
    
    list<T>*% key_list;

    int it;
};

#ifdef __NEO_MICRO_RAM_8K__
#define MAP_TABLE_DEFAULT_SIZE 16
#else
#define MAP_TABLE_DEFAULT_SIZE 128
#endif
#define MAP_TABLE_GROWTH_FACTOR 2

impl map <T, T2>
{
    map<T,T2>*% initialize(map<T,T2>*% self) {
        using unsafe;
        
        self.keys = borrow gc_inc(new T[MAP_TABLE_DEFAULT_SIZE]);
        self.items = borrow gc_inc(new T2[MAP_TABLE_DEFAULT_SIZE]);
        self.hashes = borrow gc_inc(new unsigned int[MAP_TABLE_DEFAULT_SIZE]);
        self.item_existance = borrow gc_inc(new bool[MAP_TABLE_DEFAULT_SIZE]);

        for(int i=0; i<MAP_TABLE_DEFAULT_SIZE; i++)
        {
            self.hashes[i] = 0;
            self.item_existance[i] = false;
        }

        self.size = MAP_TABLE_DEFAULT_SIZE;
        self.len = 0;
        
        self.key_list = new list<T>();

        self.it = 0;

        return self;
    }
    map<T,T2>*% initialize_with_values(map<T,T2>*% self, int num_keys, T^* keys, T2^* values) 
    {
        using unsafe;
        
        self.keys = borrow gc_inc(new T[MAP_TABLE_DEFAULT_SIZE]);
        self.items = borrow gc_inc(new T2[MAP_TABLE_DEFAULT_SIZE]);
        self.hashes = borrow gc_inc(new unsigned int[MAP_TABLE_DEFAULT_SIZE]);
        self.item_existance = borrow gc_inc(new bool[MAP_TABLE_DEFAULT_SIZE]);

        for(int i=0; i<MAP_TABLE_DEFAULT_SIZE; i++)
        {
            self.hashes[i] = 0;
            self.item_existance[i] = false;
        }

        self.size = MAP_TABLE_DEFAULT_SIZE;
        self.len = 0;

        self.it = 0;
        
        self.key_list = new list<T>();
        
        for(int i=0; i<num_keys; i++) {
            self.insert(dummy_heap keys\[i], dummy_heap values[i]);
        }

        return self;
    }
    void finalize(map<T,T2>* self) {
        using unsafe;
        
        for(int i=0; i<self.size; i++) {
            if(self.item_existance[i]) {
                if(isheap(T2)) {
                    delete self.items\[i];
                }
            }
        }
        come_free((char*)self.items);

        for(int i=0; i<self.size; i++) {
            if(self.item_existance[i]) {
                if(isheap(T)) {
                    delete self.keys\[i];
                }
            }
        }
        come_free((char*)self.keys);
        
        delete borrow self.key_list;

        delete borrow self.hashes;
        delete borrow self.item_existance;
    }
    int key_position(map<T,T2>* self, T^ key, bool by_pointer=false)
    {
        if(self == null) {
            return -1;
        }
        
        int pos = 0;
        list_item<T>* it = self.key_list.head;
        while(it != null) {
            if((!by_pointer && it.item.equals(key)) || (by_pointer && it.item == key)) {
                return pos;
            }
            it = it.next;
            pos++;
        }
        
        return -1;
    }
    void remove_ordered_entry(map<T,T2>* self, T^ key, bool by_pointer=false)
    {
        int pos = self.key_position(key, by_pointer);
        if(pos >= 0) {
            self.key_list.delete(pos, pos+1);
        }
    }
    map<T, T2>*% clone(map<T, T2>* self)
    {
        using unsafe;
        
        if(self == null) {
            return null;
        }
        
        var result = new map<T,T2>();
        
        result.key_list = new list<T>();

        for(var it = self.begin(); !self.end(); it = self.next()) {
            T2/ default_value;
            memset(&default_value, 0, sizeof(T2));
            
            var it2 = self.at(it, default_value, true);

            if(isheap(T) && isheap(T2)) {
                result.put(clone it, clone it2);
            }
            else if(isheap(T)) {
                result.put(clone it, dummy_heap dupe it2);
            }
            else if(isheap(T2)) {
                result.put(dummy_heap dupe it, clone it2);
            }
            else {
                result.put(dummy_heap dupe it, dummy_heap dupe it2);
            }
        }

        return result;
    }
    
    string to_string(map<T,T2>* self)
    {
        using unsafe;
        
        if(self == null) {
            return string("");
        }
        
        buffer*% result = new buffer();
        
        result.append_str("[");
        
        list_item<T>* it = self.key_list.head;
        while(it) {
            T2/ default_value;
            memset(&default_value, 0, sizeof(T2));
            T2 it2 = self.at(it.item, default_value, true);
            
            result.append_str(it.item.to_string());
            result.append_str("^");
            result.append_str(it2.to_string());
            
            it = it.next;
            
            if(it != null) {
                result.append_str(",");
            }
        }
        
        result.append_str("]");
        
        return result.to_string();
    }
    
    T2 at(map<T, T2>* self, T^ key, T2 default_value, bool by_pointer=false) {
        using unsafe;
        
        if(self == null) {
            return default_value;
        }
        
        unsigned int key_hash = ((T)key).get_hash_key();
        unsigned int hash = key_hash % self.size;
        unsigned int it = hash;
        
        while(true) {
            if(self.item_existance[it])
            {
                if(self.hashes[it] == key_hash
                    && ((!by_pointer && self.keys\[it].equals(key)) || (by_pointer && self.keys\[it] == key)))
                {
                    return dummy_heap self.items\[it];
                }

                if(++it >= self.size) {
                    it = 0;
                }
                if(it == hash) {
                    return default_value;
                }
            }
            else {
                return default_value;
            }
        }

        return default_value;
    }
    map<T,T2>* remove(map<T, T2>* self, T key, bool by_pointer=false) {
        using unsafe;
        
        if(self == null) {
            return self;
        }
        
        unsigned int key_hash = ((T)key).get_hash_key();
        unsigned int hash = key_hash % self.size;
        unsigned int it = hash;
        
        while(true) {
            if(self.item_existance[it])
            {
                if(self.hashes[it] == key_hash
                    && ((!by_pointer && self.keys\[it].equals(key)) || (by_pointer && self.keys\[it] == key)))
                {
                    self.remove_ordered_entry(self.keys\[it], by_pointer);
                    
                    self.item_existance[it] = false;
                    self.hashes[it] = 0;
                    if(isheap(T)) {
                        delete borrow self.keys\[it];
                    }
                    self.keys\[it] = null;
   
                    if(isheap(T2)) {
                        delete borrow self.items\[it];
                    }
                    memset(self.items + it, 0, sizeof(T2));
                    
                    self.len--;
                    break;
                }

                if(++it >= self.size) {
                    it = 0;
                }
                if(it == hash) {
                    break;
                }
            }
            else {
                break;
            }
        }
        
        return self;
    }
    int length(map<T, T2>* self) {
        if(self == null) {
            return 0;
        }
        return self.len;
    }
    int len(map<T, T2>* self)
    {
        return self.length();
    }
    bool is_empty(map<T, T2>* self)
    {
        return self.len() == 0;
    }
    bool any(map<T, T2>* self, void* parent, bool (*block)(void*, T))
    {
        if(self == null) {
            return false;
        }
        
        list_item<T>* it = self.key_list.head;
        while(it != null) {
            if(block(parent, it.item)) {
                return true;
            }
            it = it.next;
        }
        
        return false;
    }
    bool all(map<T, T2>* self, void* parent, bool (*block)(void*, T))
    {
        if(self == null) {
            return true;
        }
        
        list_item<T>* it = self.key_list.head;
        while(it != null) {
            if(!block(parent, it.item)) {
                return false;
            }
            it = it.next;
        }
        
        return true;
    }
    int count(map<T, T2>* self)
    {
        return self.length();
    }
    int position(map<T, T2>* self, int default_value, void* parent, bool (*block)(void*, T))
    {
        if(self == null) {
            return default_value;
        }
        
        list_item<T>* it = self.key_list.head;
        int i = 0;
        while(it != null) {
            if(block(parent, it.item)) {
                return i;
            }
            it = it.next;
            i++;
        }
        
        return default_value;
    }
    T find_value(map<T, T2>* self, T^ default_value, void* parent, bool (*block)(void*, T))
    {
        if(self == null) {
            return dummy_heap default_value;
        }
        
        list_item<T>* it = self.key_list.head;
        while(it != null) {
            if(block(parent, it.item)) {
                return dummy_heap it.item;
            }
            it = it.next;
        }
        
        return dummy_heap default_value;
    }
    T nth(map<T, T2>* self, int index, T^ default_value)
    {
        if(self == null) {
            return dummy_heap default_value;
        }
        
        return self.key_list.nth(index, default_value);
    }
    T last(map<T, T2>* self, T^ default_value)
    {
        if(self == null) {
            return dummy_heap default_value;
        }
        
        return self.key_list.last(default_value);
    }
    T sum(map<T, T2>* self)
    {
        T result;
        memset(&result, 0, sizeof(T));
        
        if(self == null) {
            return result;
        }
        
        list_item<T>* it = self.key_list.head;
        while(it != null) {
            result += it.item;
            it = it.next;
        }
        
        return result;
    }
    T product(map<T, T2>* self)
    {
        T result = 1;
        
        if(self == null) {
            return result;
        }
        
        list_item<T>* it = self.key_list.head;
        while(it != null) {
            result *= it.item;
            it = it.next;
        }
        
        return result;
    }
    T min(map<T, T2>* self, T^ default_value)
    {
        if(self == null || self.key_list.head == null) {
            return dummy_heap default_value;
        }
        
        list_item<T>* it = self.key_list.head;
        bool first = true;
        T result = dummy_heap default_value;
        while(it != null) {
            if(first || it.item.compare(result) < 0) {
                result = dummy_heap it.item;
                first = false;
            }
            it = it.next;
        }
        
        return dummy_heap result;
    }
    T max(map<T, T2>* self, T^ default_value)
    {
        if(self == null || self.key_list.head == null) {
            return dummy_heap default_value;
        }
        
        list_item<T>* it = self.key_list.head;
        bool first = true;
        T result = dummy_heap default_value;
        while(it != null) {
            if(first || it.item.compare(result) > 0) {
                result = dummy_heap it.item;
                first = false;
            }
            it = it.next;
        }
        
        return dummy_heap result;
    }
    
    T^ begin(map<T, T2>* self) {
        using unsafe;
        
        if(self == null) {
            T^/ result;
            memset(&result, 0, sizeof(T));
            return result;
        }
        self.key_list.it = self.key_list.head;

        if(self.key_list.it) {
            return self.key_list.it.item;
        }
        
        T/^ result;
        memset(&result, 0, sizeof(T));
        return result;
    }

    T^ next(map<T, T2>* self) {
        using unsafe;
        
        if(self == null || self.key_list.it == null) {
            T/^ result;
            memset(&result, 0, sizeof(T));
            return result;
        }
        self.key_list.it = self.key_list.it.next;

        if(self.key_list.it) {
            return self.key_list.it.item;
        }
        
        T/^ result;
        memset(&result, 0, sizeof(T));
        return result;
    }

    bool end(map<T, T2>* self) {
        return self == null || self.key_list == null || self.key_list.it == null;
    }
    
    void rehash(map<T,T2>* self) {
        using unsafe;
        
        int old_size = self.size;
        int size = self.size * MAP_TABLE_GROWTH_FACTOR;
        T^* keys = borrow gc_inc(new T[size]);
        T2^* items = borrow gc_inc(new T2[size]);
        unsigned int* hashes = borrow gc_inc(new unsigned int[size]);
        bool* item_existance = borrow gc_inc(new bool[size]);
        for(int i=0; i<size; i++) {
            hashes[i] = 0;
            item_existance[i] = false;
        }

        int len = 0;

        for(int i=0; i<old_size; i++) {
            if(!self.item_existance[i]) {
                continue;
            }

            unsigned int key_hash = self.hashes[i];
            unsigned int hash = key_hash % size;
            int n = hash;

            while(true) {
                if(item_existance[n])
                {
                    if(++n >= size) {
                        n = 0;
                    }
                    if(n == hash) {
                        printf("unexpected error in map.rehash(1)\n");
                        stackframe2(self);
                        exit(2);
                    }
                }
                else {
                    item_existance[n] = true;
                    hashes[n] = key_hash;
                    keys\[n] = self.keys\[i];
                    items\[n] = self.items\[i];

                    len++;
                    break;
                }
            }
        }

        come_free((char*)self.items);
        delete borrow self.hashes;
        delete borrow self.item_existance;
        come_free((char*)self.keys);

        self.keys = keys;
        self.items = items;
        self.hashes = hashes;
        self.item_existance = item_existance;

        self.size = size;
        self.len = len;
    }

    void show_map(map<T, T2>* self) {
        using unsafe;
        
        if(self == null) {
            return;
        }
        
        for(var it = self.begin(); !self.end(); it = self.next()) {
            puts("key " + it.to_string());
            
            T2/ default_value;
            memset(&default_value, 0, sizeof(T2));
            
            var item = self.at(it, default_value, true);
            if(item) {
                puts("item " + item.to_string());
            }
        }
    }
    
    map<T,T2>* insert(map<T,T2>* self, T key, T2 item, bool by_pointer=false) {
        using unsafe; 
        
        if(self == null) {
            return self;
        }
        
        if(self.len*2 >= self.size) {
            self.rehash();
        }
        bool add_to_key_list = false;
        unsigned int key_hash = ((T)key).get_hash_key();
        unsigned int hash = key_hash % self.size;
        unsigned int it = hash;
        
        while(true) {
            if(self.item_existance[it])
            {
                if(self.hashes[it] == key_hash
                    && ((!by_pointer && self.keys\[it].equals(key)) || (by_pointer && self.keys\[it] == key))) 
                {
                    self.remove_ordered_entry(self.keys\[it], by_pointer);
                    if(isheap(T)) {
                        delete self.keys\[it];
                        self.keys\[it] = borrow gc_inc(key);
                    }
                    else {
                        self.keys\[it] = borrow key;
                    }
                    if(isheap(T2)) {
                        delete self.items\[it];
                        self.items\[it] = borrow gc_inc(item);
                    }
                    else {
                        self.items\[it] = borrow item;
                    }
                    self.hashes[it] = key_hash;
                    add_to_key_list = true;
                    break;
                }

                if(++it >= self.size) {
                    it = 0;
                }
                if(it == hash) {
                    printf("unexpected error in map.insert\n");
                    stackframe2(self);
                    exit(2);
                }
            }
            else {
                self.item_existance[it] = true;
                self.hashes[it] = key_hash;
                if(isheap(T)) {
                    self.keys\[it] = borrow gc_inc(key);
                }
                else {
                    self.keys\[it] = borrow key;
                }
                if(isheap(T2)) {
                    self.items\[it] = borrow gc_inc(item);
                }
                else {
                    self.items\[it] = borrow item;
                }

                self.len++;
                add_to_key_list = true;

                break;
            }
        }
        
        if(add_to_key_list) {
            self.key_list.push_back(key);
        }
        
        return self;
    }
    map<T,T2>* put(map<T,T2>* self, T key, T2 item, bool by_pointer=false) {
        using unsafe;
        
        if(self == null) {
            return self;
        }
        
        if(self.len*2 >= self.size) {
            self.rehash();
        }
        bool add_to_key_list = false;
        unsigned int key_hash = ((T)key).get_hash_key();
        unsigned int hash = key_hash % self.size;
        int it = hash;

        while(true) {
            if(self.item_existance[it])
            {
                if(self.hashes[it] == key_hash
                    && ((!by_pointer && self.keys\[it].equals(key)) || (by_pointer && self.keys\[it] == key)))
                {
                    self.remove_ordered_entry(self.keys\[it], by_pointer);
                    if(isheap(T)) {
                        delete self.keys\[it];
                        self.keys\[it] = borrow gc_inc(key);
                    }
                    else {
                        self.keys\[it] = borrow key;
                    }
                    if(isheap(T2)) {
                        delete self.items\[it];
                        self.items\[it] = borrow gc_inc(item);
                    }
                    else {
                        self.items\[it] = borrow item;
                    }
                    self.hashes[it] = key_hash;
                    add_to_key_list = true;
                    break;
                }

                if(++it >= self.size) {
                    it = 0;
                }
                if(it == hash) {
                    printf("unexpected error in map.insert\n");
                    stackframe2(self);
                    exit(2);
                }
            }
            else {
                self.item_existance[it] = true;
                self.hashes[it] = key_hash;
                if(isheap(T)) {
                    self.keys\[it] = borrow gc_inc(key);
                }
                else {
                    self.keys\[it] = borrow key;
                }
                if(isheap(T2)) {
                    self.items\[it] = borrow gc_inc(item);
                }
                else {
                    self.items\[it] = borrow item;
                }

                self.len++;
                add_to_key_list = true;

                break;
            }
        }
        
        if(add_to_key_list) {
            self.key_list.push_back(key);
        }
        
        return self;
    }
    T2 operator_load_element(map<T, T2>* self, T^ key) {
        using unsafe;
        
        T2/ default_value;
        memset(&default_value, 0, sizeof(T2));
        
        if(self == null) {
            return default_value;
        }
        
        unsigned int key_hash = ((T)key).get_hash_key();
        unsigned int hash = key_hash % self.size;
        unsigned int it = hash;
        
        while(true) {
            if(self.item_existance[it])
            {
                if(self.hashes[it] == key_hash && self.keys\[it].equals(key))
                {
                    return dummy_heap self.items\[it];
                }

                if(++it >= self.size) {
                    it = 0;
                }
                if(it == hash) {
                    return default_value;
                }
            }
            else {
                return default_value;
            }
        }

        return default_value;
    }
    Result<T2>*% operator_load_element_optional(map<T, T2>* self, T^ key) {
        using unsafe;

        if(self == null) {
            return new Result<T2>.None();
        }

        unsigned int key_hash = ((T)key).get_hash_key();
        unsigned int hash = key_hash % self.size;
        unsigned int it = hash;

        while(true) {
            if(self.item_existance[it])
            {
                if(self.hashes[it] == key_hash && self.keys\[it].equals(key))
                {
                    if(isheap(T2)) {
                        return new Result<T2>.Some(gc_inc(self.items\[it]));
                    }
                    return new Result<T2>.Some(self.items\[it]);
                }

                if(++it >= self.size) {
                    it = 0;
                }
                if(it == hash) {
                    return new Result<T2>.None();
                }
            }
            else {
                return new Result<T2>.None();
            }
        }

        return new Result<T2>.None();
    }
    
    void operator_store_element(map<T, T2>* self, T key, T2 item) {
        if(self == null) {
            return ;
        }
        self.insert(key, item);
    }
    
    bool equals(map<T, T2>* left, map<T, T2>* right)
    {
        using unsafe;
        
        if(left == null && right == null) {
            return true;
        }
        else if(left == null || right == null) {
            return false;
        }
        
        if(left.len != right.len) {
            return false;
        }

        int n = 0;
        bool result = true;
        for(var it = left.key_list.begin(); !left.key_list.end(); it = left.key_list.next()) {
            T^/ default_value;
            memset(&default_value, 0, sizeof(T));
            T it2 = right.key_list.item(n, default_value);
            
            if(it.equals(it2)) {
                T2/ default_value2;
                memset(&default_value2, 0, sizeof(T2));
                T2 item = left.at(it, default_value2, true);
                T2 item2 = right.at(it2, default_value2, true);
                
                if(!item.equals(item2)) {
                    result = false;
                }
            }
            else {
                result = false;
            }
            
            n++;
        }

        return result;
    }
    
    bool operator_equals(map<T, T2>* left, map<T,T2>* right) {
        using unsafe;
        
        if(left == null && right == null) {
            return true;
        }
        else if(left == null || right == null) {
            return false;
        }
        if(left.len != right.len) {
            return false;
        }

        int n = 0;
        bool result = true;
        for(var it = left.key_list.begin(); !left.key_list.end(); it = left.key_list.next()) {
            T/ default_value;
            memset(&default_value, 0, sizeof(T));
            T it2 = right.key_list.item(n, default_value);
            
            if(it === it2) {
                T2/ default_value2;
                memset(&default_value2, 0, sizeof(T2));
                T2 item = left.at(it, default_value2, true);
                T2 item2 = right.at(it2, default_value2, true);
                
                if(!(item === item2)) {
                    result = false;
                }
            }
            else {
                result = false;
            }
            
            n++;
        }

        return result;
    }
    
    bool operator_not_equals(map<T, T2>* left, map<T,T2>* right) {
        if(left == null && right == null) {
            return false;
        }
        else if(left == null || right == null) {
            return true;
        }
        
        return !(left.operator_equals(right));
    }
    
    bool find(map<T, T2>* self, T^ key, bool by_pointer=false) {
        using unsafe;
        
        if(self == null) {
            return false;
        }
        
        unsigned int key_hash = ((T)key).get_hash_key();
        unsigned int hash = key_hash % self.size;
        int it = hash;

        while(true) {
            if(self.item_existance[it])
            {
                if(self.hashes[it] == key_hash
                    && ((!by_pointer && self.keys\[it].equals(key)) || (by_pointer && self.keys\[it] == key)))
                {
                    return true;
                }

                if(++it >= self.size) {
                    it = 0;
                }
                if(it == hash) {
                    return false;
                }
            }
            else {
                return false;
            }
        }

        return false;
    }
    map<T,T2>*% operator_add(map<T,T2>* left, map<T,T2>* right) {
        using unsafe;
        
        map<T,T2>*% result = new map<T,T2>();
        
        if(left == null || right == null) {
            return result;
        }

        for(var it = left.key_list.begin(); !left.key_list.end(); it = left.key_list.next()) {
            T2/ default_value;
            memset(&default_value, 0, sizeof(T2));
            T2 it2 = left.at(it, default_value, true);
            
            if(isheap(T) && isheap(T2)) {
                result.insert(clone it, clone it2);
            }
            else if(isheap(T)) {
                result.insert(clone it, dummy_heap dupe it2);
            }
            else if(isheap(T2)) {
                result.insert(dummy_heap dupe it, clone it2);
            }
            else {
                result.insert(dummy_heap dupe it, dummy_heap dupe it2);
            }
        }

        for(var it = right.key_list.begin(); !right.key_list.end(); it = right.key_list.next()) {
            T2/ default_value;
            memset(&default_value, 0, sizeof(T2));
            T2 it2 = right.at(it, default_value, true);
            
            if(isheap(T) && isheap(T2)) {
                result.insert(clone it, clone it2);
            }
            else if(isheap(T)) {
                result.insert(clone it, dummy_heap dupe it2);
            }
            else if(isheap(T2)) {
                result.insert(dummy_heap dupe it, clone it2);
            }
            else {
                result.insert(dummy_heap dupe it, dummy_heap dupe it2);
            }
        }

        return result;
    }
    map<T,T2>*% operator_mult(map<T,T2>* left, int right) {
        using unsafe;
        
        map<T,T2>*% result = new map<T,T2>();
        
        if(left == null || right == null) {
            return result;
        }

        for(int i=0; i<right; i++ ) {
            for(var it = left.key_list.begin(); !left.key_list.end(); it = left.key_list.next()) {
                T2/ default_value;
                memset(&default_value, 0, sizeof(T2));
                
                T2 it2 = left.at(it, default_value, true);
                
                if(isheap(T) && isheap(T2)) {
                    result.insert(clone it, clone it2);
                }
                else if(isheap(T)) {
                    result.insert(clone it, dummy_heap dupe it2);
                }
                else if(isheap(T2)) {
                    result.insert(dummy_heap dupe it, clone it2);
                }
                else {
                    result.insert(dummy_heap dupe it, dummy_heap dupe it2);
                }
            }
        }

        return result;
    }
    list<T>*% keys(map<T, T2>* self) {
        var result = new list<T>();
        
        if(self == null) {
            return result;
        }
        
        for(var it = self.key_list.begin(); !self.key_list.end(); it = self.key_list.next()) {
            if(isheap(T)) {
                result.push_back(clone it);
            }
            else {
                result.push_back(dummy_heap dupe it);
            }
        }
        
        return result;
    }
    
    list<T2>*% values(map<T, T2>* self) {
        using unsafe;
        
        var result = new list<T2>();
        
        if(self == null) {
            return result;
        }
        
        for(var it = self.key_list.begin(); !self.key_list.end(); it = self.key_list.next()) { 
            T2/ default_value;
            memset(&default_value, 0, sizeof(T2));
        
            var it2 = self.at(it, default_value, true);
            
            if(isheap(T2)) {
                result.push_back(clone it2);
            }
            else {
                result.push_back(dummy_heap dupe it2);
            }
        }
        
        return result;
    }
    
}

impl map<T, T>
{
    vector<T>*% to_vector(map<T, T>* self) {
        using unsafe;
        
        var result = new vector<T>.initialize();
        
        if(self == null) {
            return result;
        }
        
        for(var key = self.key_list.begin(); !self.key_list.end(); key = self.key_list.next()) {
            T/ default_value;
            memset(&default_value, 0, sizeof(T));
            var item = self.at(key, default_value, true);
            
            if(isheap(T)) {
                result.add(clone key);
                result.add(clone item);
            }
            else {
                result.add(dummy_heap dupe key);
                result.add(dummy_heap dupe item);
            }
        }
        
        return result;
    }
}

//////////////////////////////
// tuple
//////////////////////////////
struct tuple1<T>
{
    T v1;
};

impl tuple1 <T>
{
    tuple1<T>*% initialize(tuple1<T>*% self, T v1)
    {
        self.v1 = v1;
        
        return self;
    }
    
    bool equals(tuple1<T>* self, tuple1<T>* right)
    {
        if(self == null && right == null) {
            return true;
        }
        else if(self == null || right == null) {
            return false;
        }
        if(!self.v1.equals(right.v1)) {
            return false;
        }
        
        return true;
    }
    bool operator_equals(tuple1<T>* self, tuple1<T>* right) 
    {
        if(self == null && right == null) {
            return true;
        }
        else if(self == null || right == null) {
            return false;
        }
        if(!(self.v1 === right.v1)) {
            return false;
        }
        
        return true;
    }
    bool operator_not_equals(tuple1<T>* left, tuple1<T>* right) {
        return !left.operator_equals(right);
    }
    
    string to_string(tuple1<T>* self)
    {
        if(self == null) {
            return string("");
        }
        return "(" + self.v1.to_string() + ")";
    }
    int len(tuple1<T>* self)
    {
        if(self == null) {
            return 0;
        }
        return 1;
    }
    int count(tuple1<T>* self)
    {
        return self.len();
    }
}

struct tuple2<T, T2>
{
    T v1;
    T2 v2;
};

impl tuple2 <T, T2>
{
    tuple2<T, T2>*% initialize(tuple2<T, T2>*% self, T v1, T2 v2)
    {
        self.v1 = v1;
        self.v2 = v2;
        
        return self;
    }
    
    string to_string(tuple2<T, T2>* self)
    {
        if(self == null) {
            return string("");
        }
        return "(" + self.v1.to_string() + "," + self.v2.to_string() + ")";
    }
    bool equals(tuple2<T,T2>* self, tuple2<T,T2>* right)
    {
        if(self == null && right == null) {
            return true;
        }
        else if(self == null || right == null) {
            return false;
        }
        if(!self.v1.equals(right.v1)) {
            return false;
        }
        if(!self.v2.equals(right.v2)) {
            return false;
        }
        
        return true;
    }
    bool operator_equals(tuple2<T,T2>* self, tuple2<T,T2>* right) 
    {
        if(self == null && right == null) {
            return true;
        }
        else if(self == null || right == null) {
            return false;
        }
        if(!(self.v1 === right.v1)) {
            return false;
        }
        if(!(self.v2 === right.v2)) {
            return false;
        }
        
        return true;
    }
    bool operator_not_equals(tuple2<T,T2>* left, tuple2<T,T2>* right) {
        return !left.operator_equals(right);
    }
    int len(tuple2<T, T2>* self)
    {
        if(self == null) {
            return 0;
        }
        return 2;
    }
    int count(tuple2<T, T2>* self)
    {
        return self.len();
    }
}


struct tuple3<T, T2, T3>
{
    T v1;
    T2 v2;
    T3 v3;
};

impl tuple3 <T, T2, T3>
{
    tuple3<T, T2, T3>*% initialize(tuple3<T, T2, T3>*% self, T v1, T2 v2, T3 v3)
    {
        self.v1 = v1;
        self.v2 = v2;
        self.v3 = v3;
        
        return self;
    }
    
    string to_string(tuple3<T, T2, T3>* self)
    {
        if(self == null) {
            return string("");
        }
        return "(" + self.v1.to_string() + "," + self.v2.to_string() + "," + self.v3.to_string() + ")";
    }
    bool equals(tuple3<T,T2,T3>* self, tuple3<T,T2,T3>* right)
    {
        if(self == null && right == null) {
            return true;
        }
        else if(self == null || right == null) {
            return false;
        }
        if(!self.v1.equals(right.v1)) {
            return false;
        }
        if(!self.v2.equals(right.v2)) {
            return false;
        }
        if(!self.v3.equals(right.v3)) {
            return false;
        }
        
        return true;
    }
    bool operator_equals(tuple3<T,T2,T3>* self, tuple3<T,T2,T3>* right) 
    {
        if(self == null && right == null) {
            return true;
        }
        else if(self == null || right == null) {
            return false;
        }
        if(!(self.v1 === right.v1)) {
            return false;
        }
        if(!(self.v2 === right.v2)) {
            return false;
        }
        if(!(self.v3 === right.v3)) {
            return false;
        }
        
        return true;
    }
    bool operator_not_equals(tuple3<T,T2,T3>* left, tuple3<T,T2,T3>* right) {
        return !left.operator_equals(right);
    }
    int len(tuple3<T, T2, T3>* self)
    {
        if(self == null) {
            return 0;
        }
        return 3;
    }
    int count(tuple3<T, T2, T3>* self)
    {
        return self.len();
    }
}

struct tuple4<T, T2, T3, T4>
{
    T v1;
    T2 v2;
    T3 v3;
    T4 v4;
};

impl tuple4 <T, T2, T3, T4>
{
    tuple4<T, T2, T3, T4>*% initialize(tuple4<T, T2, T3, T4>*% self, T v1, T2 v2, T3 v3, T4 v4)
    {
        self.v1 = v1;
        self.v2 = v2;
        self.v3 = v3;
        self.v4 = v4;
        
        return self;
    }
    
    string to_string(tuple4<T, T2, T3, T4>* self)
    {
        if(self == null) {
            return string("");
        }
        return "(" + self.v1.to_string() + "," + self.v2.to_string() + "," + self.v3.to_string() + "," + self.v4.to_string() + ")";
    }
    bool equals(tuple4<T,T2,T3,T4>* self, tuple4<T,T2,T3,T4>* right)
    {
        if(self == null && right == null) {
            return true;
        }
        else if(self == null || right == null) {
            return false;
        }
        if(!self.v1.equals(right.v1)) {
            return false;
        }
        if(!self.v2.equals(right.v2)) {
            return false;
        }
        if(!self.v3.equals(right.v3)) {
            return false;
        }
        if(!self.v4.equals(right.v4)) {
            return false;
        }
        
        return true;
    }
    bool operator_equals(tuple4<T,T2,T3,T4>* self, tuple4<T,T2,T3,T4>* right) 
    {
        if(self == null && right == null) {
            return true;
        }
        else if(self == null || right == null) {
            return false;
        }
        if(!(self.v1 === right.v1)) {
            return false;
        }
        if(!(self.v2 === right.v2)) {
            return false;
        }
        if(!(self.v3 === right.v3)) {
            return false;
        }
        if(!(self.v4 === right.v4)) {
            return false;
        }
        
        return true;
    }
    bool operator_not_equals(tuple4<T,T2,T3,T4>* left, tuple4<T,T2,T3,T4>* right) {
        return !left.operator_equals(right);
    }
    int len(tuple4<T, T2, T3, T4>* self)
    {
        if(self == null) {
            return 0;
        }
        return 4;
    }
    int count(tuple4<T, T2, T3, T4>* self)
    {
        return self.len();
    }
}

struct tuple5<T, T2, T3, T4, T5>
{
    T v1;
    T2 v2;
    T3 v3;
    T4 v4;
    T5 v5;
};

impl tuple5 <T, T2, T3, T4, T5>
{
    tuple5<T, T2, T3, T4, T5>*% initialize(tuple5<T, T2, T3, T4, T5>*% self, T v1, T2 v2, T3 v3, T4 v4, T5 v5)
    {
        self.v1 = v1;
        self.v2 = v2;
        self.v3 = v3;
        self.v4 = v4;
        self.v5 = v5;
        
        return self;
    }
    
    string to_string(tuple5<T, T2, T3, T4, T5>* self)
    {
        if(self == null) {
            return string("");
        }
        return "(" + self.v1.to_string() + "," + self.v2.to_string() + "," + self.v3.to_string() + "," + self.v4.to_string() + "," + self.v5.to_string() + ")";
    }
    bool equals(tuple5<T,T2,T3,T4,T5>* self, tuple5<T,T2,T3,T4,T5>* right)
    {
        if(self == null && right == null) {
            return true;
        }
        else if(self == null || right == null) {
            return false;
        }
        if(!self.v1.equals(right.v1)) {
            return false;
        }
        if(!self.v2.equals(right.v2)) {
            return false;
        }
        if(!self.v3.equals(right.v3)) {
            return false;
        }
        if(!self.v4.equals(right.v4)) {
            return false;
        }
        if(!self.v5.equals(right.v5)) {
            return false;
        }
        
        return true;
    }
    bool operator_equals(tuple5<T,T2,T3,T4,T5>* self, tuple5<T,T2,T3,T4,T5>* right) 
    {
        if(self == null && right == null) {
            return true;
        }
        else if(self == null || right == null) {
            return false;
        }
        if(!(self.v1 === right.v1)) {
            return false;
        }
        if(!(self.v2 === right.v2)) {
            return false;
        }
        if(!(self.v3 === right.v3)) {
            return false;
        }
        if(!(self.v4 === right.v4)) {
            return false;
        }
        if(!(self.v5 === right.v5)) {
            return false;
        }
        
        return true;
    }
    bool operator_not_equals(tuple5<T,T2,T3,T4,T5>* left, tuple5<T,T2,T3,T4,T5>* right) {
        return !left.operator_equals(right);
    }
    int len(tuple5<T, T2, T3, T4, T5>* self)
    {
        if(self == null) {
            return 0;
        }
        return 5;
    }
    int count(tuple5<T, T2, T3, T4, T5>* self)
    {
        return self.len();
    }
}

//////////////////////////////
// buffer
//////////////////////////////
uniq buffer*% buffer*::initialize(buffer*% self) 
{
    self.size = 128;
    self.buf = new char[self.size];
    ((char*)self.buf)[0] = '\0';
    self.len = 0;

    return self;
}

uniq buffer*% buffer*::initialize_with_value(buffer*% self, const char* mem, size_t size) 
{
    self.size = 128;
    self.buf = new char[self.size];
    ((char*)self.buf)[0] = '\0';
    self.len = 0;
    
    self.append(mem, size);

    return self;
}

uniq void buffer*::finalize(buffer* self)
{
    if(self && self.buf) delete borrow self.buf;
}

uniq buffer*% buffer*::clone(buffer* self)
{
    if(self == null) {
        return null;
    }
    
    var result = new buffer;
    
    result.size = self.size;
    result.buf = new char[self.size];
    result.len = self.len;
    memcpy(result.buf, self.buf, self.len);
    
    return result;
}

uniq bool buffer*::equals(buffer* left, buffer* right)
{
    if(left == null && right == null) {
        return true;
    }
    else if(left == null || right == null) {
        return false;
    }
    
    return left.to_string().equals(right.to_string());
}

uniq int buffer*::length(buffer* self) 
{
    if(self == null) {
        return 0;
    }
    return self.len;
}

uniq void buffer*::reset(buffer* self)
{
    if(self == null) {
        return;
    }
    ((char*)self.buf)[0] = '\0';
    self.len = 0;
}

uniq void buffer*::trim(buffer* self, int len)
{
    if(self == null) {
        return;
    }
    self.len -= len;
    if(self.len >= 0) {
        ((char*)self.buf)[self.len] = '\0';
    }
    else {
        self.len = 0;
        ((char*)self.buf)[0] = '\0';
    }
}

uniq buffer* buffer*::append(buffer* self, const char* mem, size_t size)
{
    if(self == null || mem == null) {
        return self;
    }
    if(self.len + size + 1 + 1 >= self.size) {
        int old_len = self.len;
        char*% old_buf = new char[old_len+1];
        memcpy(old_buf, self.buf, old_len+1);
        
        int new_size = (self.size + size + 1) * 2;
        self.buf = new char[new_size];
        memcpy(self.buf, old_buf, old_len);
        ((char*)self.buf)[old_len] = '\0';
        self.size = new_size;
    }

    memcpy(self.buf + self.len, mem, size);
    self.len += size;
    ((char*)self.buf)[self.len] = '\0';
    
    return self;
}

uniq buffer* buffer*::append_char(buffer* self, char c)
{
    if(self == null) {
        return null;
    }
    if(self.len + 1 + 1 + 1 >= self.size) {
        int old_len = self.len;
        char*% old_buf = new char[old_len+1];
        memcpy(old_buf, self.buf, old_len+1);
        
        int new_size = (self.size + 10 + 1) * 2;
        self.buf = new char[new_size];
        memcpy(self.buf, old_buf, old_len);
        ((char*)self.buf)[old_len] = '\0';
        self.size = new_size;
    }

    ((char*)self.buf)[self.len] = c;
    self.len++;

    ((char*)self.buf)[self.len] = '\0';
    
    return self;
}

uniq buffer* buffer*::append_str(buffer* self, const char* mem)
{
    if(self == null || mem == null) {
        return self;
    }
    
    int size = strlen(mem);
    if(self.len + size + 1 + 1 >= self.size) {
        int old_len = self.len;
        char*% old_buf = new char[old_len+1];
        memcpy(old_buf, self.buf, old_len+1);
        int new_size = (self.size + size + 1) * 2;
        self.buf = new char[new_size];
        memcpy(self.buf, old_buf, old_len);
        ((char*)self.buf)[old_len] = '\0';
        self.size = new_size;
    }

    memcpy(self.buf + self.len, mem, size);
    self.len += size;
    ((char*)self.buf)[self.len] = '\0';
    
    return self;
}

#ifndef UNIX
    uniq buffer* buffer*::append_format(buffer* self, const char* msg, ...)
    {
        if(self == null || msg == null) {
            return self;
        }
        
        char result[128];
        
        va_list/ args;
        va_start(args, msg);
        vsnprintf(result, 128, msg, args);
        va_end(args);
        
        int len = strlen(result);
        
        string mem = string(result);
        
        int size = strlen(mem);
        if(self.len + size + 1 + 1 >= self.size) {
            int old_len = self.len;
            char*% old_buf = new char[old_len+1];
            memcpy(old_buf, self.buf, old_len+1);
            int new_size = (self.size + size + 1) * 2;
            self.buf = new char[new_size];
            memcpy(self.buf, old_buf, old_len);
            ((char*)self.buf)[old_len] = '\0';
            self.size = new_size;
        }
    
        memcpy(self.buf + self.len, mem, size);
        self.len += size;
        ((char*)self.buf)[self.len] = '\0';
        
        free(result);
        
        return self;
    }
#else
    uniq buffer* buffer*::append_format(buffer* self, const char* msg, ...)
    {
        if(self == null || msg == null) {
            return self;
        }
        
        va_list/ args;
        va_start(args, msg);
        char* result;
        int len = vasprintf(&result, msg, args);
        va_end(args);
        
        if(len < 0) {
            return self;
        }
        
        string mem = string(result);
        
        int size = strlen(mem);
        if(self.len + size + 1 + 1 >= self.size) {
            int old_len = self.len;
            char*% old_buf = new char[old_len+1];
            memcpy(old_buf, self.buf, old_len+1);
            int new_size = (self.size + size + 1) * 2;
            self.buf = new char[new_size];
            memcpy(self.buf, old_buf, old_len);
            ((char*)self.buf)[old_len] = '\0';
            self.size = new_size;
        }
    
        memcpy(self.buf + self.len, mem, size);
        self.len += size;
        ((char*)self.buf)[self.len] = '\0';
        
        free(result);
        
        return self;
    }
#endif

uniq buffer* buffer*::append_nullterminated_str(buffer* self, const char* mem)
{
    if(self == null || mem == null) {
        return self;
    }
    int size = strlen(mem) + 1;
    if(self.len + size + 1 + 1 + 1 >= self.size) {
        int old_len = self.len;
        char*% old_buf = new char[old_len+1];
        memcpy(old_buf, self.buf, old_len+1);
        int new_size = (self.size + size + 1) * 2;
        self.buf = new char[new_size];
        memcpy(self.buf, old_buf, old_len);
        ((char*)self.buf)[old_len] = '\0';
        self.size = new_size;
    }

    memcpy(self.buf + self.len, mem, size);
    self.len += size;
    ((char*)self.buf)[self.len] = '\0';
    self.len++;
    
    return self;
}

uniq buffer* buffer*::append_int(buffer* self, int value) 
{
    if(self == null) {
        return null;
    }
    int* mem = &value;
    int size = sizeof(int);
    
    if(self.len + size + 1 + 1 >= self.size) {
        int old_len = self.len;
        char*% old_buf = new char[old_len+1];
        memcpy(old_buf, self.buf, old_len+1);
        int new_size = (self.size + size + 1) * 2;
        self.buf = new char[new_size];
        memcpy(self.buf, old_buf, old_len);
        ((char*)self.buf)[old_len] = '\0';
        self.size = new_size;
    }

    memcpy(self.buf + self.len, mem, size);
    self.len += size;
    ((char*)self.buf)[self.len] = '\0';
    
    return self;
}

uniq buffer* buffer*::append_long(buffer* self, long value) 
{
    if(self == null) {
        return null;
    }
    long* mem = &value;
    int size = sizeof(long);
    
    if(self.len + size + 1 + 1 >= self.size) {
        int old_len = self.len;
        char*% old_buf = new char[old_len+1];
        memcpy(old_buf, self.buf, old_len+1);
        int new_size = (self.size + size + 1) * 2;
        self.buf = new char[new_size];
        memcpy(self.buf, old_buf, old_len);
        ((char*)self.buf)[old_len] = '\0';
        self.size = new_size;
    }

    memcpy(self.buf + self.len, mem, size);
    self.len += size;
    ((char*)self.buf)[self.len] = '\0';
    
    return self;
}

uniq buffer* buffer*::append_short(buffer* self, short value) 
{
    if(self == null) {
        return null;
    }
    
    short* mem = &value;
    int size = sizeof(short);
    
    if(self.len + size + 1 + 1 >= self.size) {
        int old_len = self.len;
        char*% old_buf = new char[old_len+1];
        memcpy(old_buf, self.buf, old_len+1);
        int new_size = (self.size + size + 1) * 2;
        self.buf = new char[new_size];
        memcpy(self.buf, old_buf, old_len);
        ((char*)self.buf)[old_len] = '\0';
        self.size = new_size;
    }

    memcpy(self.buf + self.len, mem, size);
    self.len += size;
    ((char*)self.buf)[self.len] = '\0';
    
    return self;
}

uniq buffer* buffer*::alignment(buffer* self) 
{
    if(self == null) {
        return null;
    }
    
    int len = self.len;
    len = (len + 3) & ~3;
    
    if(len >= self.size) {
        int old_len = self.len;
        char*% old_buf = new char[old_len+1];
        memcpy(old_buf, self.buf, old_len+1);
        int new_size = (self.size + 1 + 1) * 2;
        self.buf = new char[new_size];
        memcpy(self.buf, old_buf, old_len);
        ((char*)self.buf)[old_len] = '\0';
        self.size = new_size;
    }

    for(int i=self.len; i<len; i++) {
        ((char*)self.buf)[i] = '\0';
    }
    
    self.len = len;
    
    return self;
}

uniq int buffer*::compare(buffer* left, buffer* right) 
{
    if(left == null && right == null) {
        return 0;
    }
    else if(left == null) {
        return -1;
    }
    else if(right == null) {
        return 1;
    }
    
    return strcmp(left.buf, right.buf);
}

uniq buffer*% char*::to_buffer(const char* self) 
{
    var result = new buffer.initialize();
    
    if(self == null) {
        return result;
    }

    result.append_str(self);

    return result;
}

uniq string buffer*::to_string(buffer* self)
{
    if(self == null) {
        return string("");
    }
    
    return string(self.buf);
}

uniq unsigned char* buffer*::head_pointer(buffer* self)
{
    if(self == null) {
        return null;
    }
    return (unsigned char*)self.buf;
}

uniq buffer*% char[]::to_buffer(char* self, size_t len) 
{
    var result = new buffer();
    if(self == null) {
        return result;
    }
    result.append(self, sizeof(char)*len);
    return result;
}

uniq buffer*% char*[]::to_buffer(char** self, size_t len) 
{
    var result = new buffer();
    if(self == null) {
        return result;
    }
    for(int i=0; i<len; i++) {
        result.append(self[i], strlen(self[i]));
    }
    return result;
}

uniq buffer*% short[]::to_buffer(short* self, size_t len) 
{
    var result = new buffer();
    if(self == null) {
        return result;
    }
    result.append((char*)self, sizeof(short)*len);
    return result;
}

uniq buffer*% int[]::to_buffer(int* self, size_t len) 
{
    var result = new buffer();
    if(self == null) {
        return result;
    }
    result.append((char*)self, sizeof(int)*len);
    return result;
}

uniq buffer*% long[]::to_buffer(long* self, size_t len) 
{
    var result = new buffer();
    if(self == null) {
        return result;
    }
    result.append((char*)self, sizeof(long)*len);
    return result;
}

uniq buffer*% float[]::to_buffer(float* self, size_t len) 
{
    var result = new buffer();
    if(self == null) {
        return result;
    }
    result.append((char*)self, sizeof(float)*len);
    return result;
}

uniq buffer*% double[]::to_buffer(double* self, size_t len) 
{
    var result = new buffer();
    if(self == null) {
        return result;
    }
    result.append((char*)self, sizeof(double)*len);
    return result;
}

uniq string buffer*::printable(buffer* self)
{
    int len = self.len;
    string result = new char[len*2+1];
    
    if(self == null) {
        return result;
    }

    int n = 0;
    for(int i=0; i<len; i++) {
        unsigned char c = self.buf[i];

        if((c >= 0 && c < ' ') 
            || c == 127)
        {
            result[n++] = '^';
            result[n++] = c + 'A' - 1;
        }
        else if(c > 127) {
            result[n++] = '?';
        }
        else {
            result[n++] = c;
        }
    }

    result[n] = '\0'

    return result;
}

impl list <T>
{
    buffer*% to_buffer(list<T>* self) {
        var result = new buffer();
        if(self == null) {
            return result;
        }
        for(var it = self.begin(); !self.end(); it = self.next()) {
            result.append((char*)&it, sizeof(T));
        }
        return result;
    }
}
//////////////////////////////
/// smart pointer
//////////////////////////////

//////////////////////////////
/// base library(primitive array)
//////////////////////////////
uniq list<char>*% char[]::to_list(char* self, size_t len) 
{
    return new list<char>.initialize_with_values(len, self);
}

uniq list<char*>*% char*[]::to_list(char** self, size_t len) 
{
    return new list<char*>.initialize_with_values(len, self);
}

uniq list<short>*% short[]::to_list(short* self, size_t len) 
{
    return new list<short>.initialize_with_values(len, self);
}

uniq list<int>*% int[]::to_list(int* self, size_t len) 
{
    return new list<int>.initialize_with_values(len, self);
}

uniq list<long>*% long[]::to_list(long* self, size_t len) 
{
    return new list<long>.initialize_with_values(len, self);
}

uniq list<float>*% float[]::to_list(float* self, size_t len) 
{
    return new list<float>.initialize_with_values(len, self);
}

uniq list<double>*% double[]::to_list(double* self, size_t len) 
{
    return new list<double>.initialize_with_values(len, self);
}

//////////////////////////////
/// base library(equals)
//////////////////////////////
uniq bool bool::equals(bool self, bool right) 
{
    return self == right;
}

uniq bool _Bool::equals(_Bool self, _Bool right) 
{
    return self == right;
}

uniq bool char::equals(char self, char right) 
{
    return self == right;
}

uniq bool short::equals(short self, short right) 
{
    return self == right;
}

uniq bool int::equals(int self, int right) 
{
    return self == right;
}

uniq bool long::equals(long self, long right) 
{
    return self == right;
}

uniq bool size_t::equals(size_t self, size_t right) 
{
    return self == right;
}

uniq bool float::equals(float self, float right) 
{
    return self == right;
}

uniq bool double::equals(double self, double right) 
{
    return self == right;
}

uniq bool bool::operator_equals(bool self, bool right)
{
    return self == right;
}

uniq bool _Bool::operator_equals(bool self, bool right)
{
    return self == right;
}

uniq bool char::operator_equals(char self, char right)
{
    return self == right;
}

uniq bool short::operator_equals(short self, short right)
{
    return self == right;
}

uniq bool int::operator_equals(int self, int right)
{
    return self == right;
}

uniq bool long::operator_equals(long self, long right)
{
    return self == right;
}

uniq bool bool::operator_not_equals(bool self, bool right)
{
    return !(self == right);
}

uniq bool _Bool::operator_not_equals(bool self, bool right)
{
    return !(self == right);
}

uniq bool char::operator_not_equals(char self, char right)
{
    return !(self == right);
}

uniq bool short::operator_not_equals(short self, short right)
{
    return !(self == right);
}

uniq bool int::operator_not_equals(int self, int right)
{
    return !(self == right);
}

uniq bool long::operator_not_equals(long self, long right)
{
    return !(self == right);
}

uniq bool char*::equals(const char* self, const char* right) 
{
    if(self == null && right == null) {
        return true;
    }
    else if(self == null || right == null) {
        return false;
    }
    else if(self == right) {
        return true;
    }
    
    return strcmp(self, right) == 0;
}

uniq bool string::equals(char* self, const char* right) 
{
    if(self == null && right == null) {
        return true;
    }
    else if(self == null || right == null) {
        return false;
    }
    else if(self == right) {
        return true;
    }
    
    return strcmp(self, right) == 0;
}

uniq bool void*::equals(void* self, void* right) 
{
    return self == right;
}

uniq bool bool*::equals(bool* self, bool* right) 
{
    return *self == *right;
}

uniq bool string::operator_equals(char* self, const char* right) 
{
    if(self == null && right == null) {
        return true;
    }
    else if(self == null || right == null) {
        return false;
    }
    else if(self == right) {
        return true;
    }
    
    return strcmp(self, right) == 0;
}

uniq bool char*::operator_equals(const char* self, const char* right) 
{
    if(self == null && right == null) {
        return true;
    }
    else if(self == null || right == null) {
        return false;
    }
    else if(self == right) {
        return true;
    }
    
    return strcmp(self, right) == 0;
}

uniq bool char[]::operator_equals(char* self, const char* right) 
{
    if(self == null && right == null) {
        return true;
    }
    else if(self == null || right == null) {
        return false;
    }
    
    return strcmp(self, right) == 0;
}

uniq bool void*::operator_equals(const char* self, const char* right) 
{
    return self == right;
}

uniq bool void*::operator_not_equals(const char* self, const char* right) 
{
    return !self.operator_equals(right);
}

uniq bool string::operator_not_equals(char* self, const char* right) 
{
    if(self == null && right == null) {
        return false;
    }
    else if(self == null || right == null) {
        return true;
    }
    
    return strcmp(self, right) != 0;
}

uniq bool char*::operator_not_equals(const char* self, const char* right) 
{
    if(self == null && right == null) {
        return false;
    }
    else if(self == null || right == null) {
        return true;
    }
    
    return strcmp(self, right) != 0;
}

uniq bool char[]::operator_not_equals(char* self, const char* right) 
{
    if(self == null && right == null) {
        return false;
    }
    else if(self == null || right == null) {
        return true;
    }
    
    return strcmp(self, right) != 0;
}


uniq string char*::operator_add(const char* self, const char* right) 
{
    if(self == null || right == null) {
        return string("");
    }
    int len = strlen(self) + strlen(right);
   
    char*% result = new char[len+1];
    
    strncpy(result, self, len+1);
    strncat(result, right, len+1);
    
    return result;
}

uniq string string::operator_add(char* self, const char* right) 
{
    if(self == null || right == null) {
        return string("");
    }
    int len = strlen(self) + strlen(right);
   
    char*% result = new char[len+1];
    
    strncpy(result, self, len+1);
    strncat(result, right, len+1);
    
    return result;
}

uniq string char*::operator_mult(const char* self, int right) 
{
    if(self == null) {
        return string("");
    }
    var buf = new buffer();
    
    for(int i=0; i<right; i++) {
        buf.append_str(self);
    }
    
    return buf.to_string();
}

uniq string string::operator_mult(const char* self, int right) 
{
    if(self == null) {
        return string("");
    }
    var buf = new buffer();
    
    for(int i=0; i<right; i++) {
        buf.append_str(self);
    }
    
    return buf.to_string();
}

uniq size_t char[]::length(const char* self, size_t len) 
{
    return len;
}

uniq bool char*[]::contained(const char* self[], size_t len, const char* str) 
{
    bool result = false;
    if(self == null) {
        return result;
    }
    for(int i=0; i<len; i++) {
        if(strncmp(self[i], str, strlen(self[i])) == 0) {
            result = true;
            break;
        }
    }
    return result;
}

uniq size_t short[]::length(short* self, size_t len) 
{
    return len;
}

uniq size_t int[]::length(int* self, size_t len) 
{
    return len;
}

uniq size_t long[]::length(long* self, size_t len) 
{
    return len;
}

uniq size_t float[]::length(float* self, size_t len) 
{
    return len;
}

uniq size_t double[]::length(double* self, size_t len) 
{
    return len;
}

//////////////////////////////
/// base library(get_hash key)
//////////////////////////////
uniq unsigned int bool::get_hash_key(bool value)
{
    return (((int)value).get_hash_key());
}

uniq unsigned int _Bool::get_hash_key(bool value)
{
    return (((int)value).get_hash_key());
}

uniq unsigned int char::get_hash_key(char value)
{
    return value;
}

uniq unsigned int short::get_hash_key(short int value)
{
    return value;
}

uniq unsigned int int::get_hash_key(int value)
{
    return value;
}

uniq unsigned int long::get_hash_key(long value)
{
    return value;
}

uniq unsigned int size_t::get_hash_key(size_t value)
{
    return value;
}

uniq unsigned int float::get_hash_key(float value)
{
    return (unsigned int)value;
}

uniq unsigned int double::get_hash_key(double value)
{
    return (unsigned int)value;
}

uniq unsigned int char*::get_hash_key(const char* value)
{
    if(value == null) {
        return 0;
    }
    int result = 0;
    const char* p = value;
    while(*p) {
        result += (*p);
        p++;
    }
    return result;
}

uniq unsigned int string::get_hash_key(char* value)
{
    if(value == null) {
        return 0;
    }
    int result = 0;
    char* p = value;
    while(*p) {
        result += (*p);
        p++;
    }
    return result;
}

uniq unsigned int void*::get_hash_key(void* value)
{
    return (((int)value).get_hash_key());
}

//////////////////////////////
/// base library(clone)
//////////////////////////////
uniq bool bool::clone(bool self)
{
    return self;
}

uniq bool _Bool::clone(bool self)
{
    return self;
}

uniq char char::clone(char self)
{
    return self;
}

uniq short int short::clone(short self)
{
    return self;
}

uniq int int::clone(int self)
{
    return self;
}

uniq long int long::clone(long self)
{
    return self;
}

uniq size_t size_t::clone(size_t self)
{
    return self;
}

uniq double double::clone(double self)
{
    return self;
}

uniq float float::clone(float self)
{
    return self;
}

//////////////////////////////
/// base library(character code)
//////////////////////////////

uniq bool xisalpha(char c)
{
    bool result = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    return result;
}

uniq bool xisblank(char c)
{
    return c == ' ' || c == '\t';
}

uniq bool xisdigit(char c)
{
    return (c >= '0' && c <= '9');
}

uniq bool xisspace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == 28 || c == 11;
}

uniq bool xisalnum(char c)
{
    return xisalpha(c) || xisdigit(c);
}

uniq bool xisascii(char c)
{
    bool result = (c >= ' ' && c <= '~');
    return result;
}

uniq bool xispunct(char c) {
    return (c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '`') || (c >= '{' && c <= '~');
 }

//////////////////////////////
/// base library(simple string library)
//////////////////////////////
uniq int string::length(const char* str)
{
    if(str == null) {
        return 0;
    }
    return strlen(str);
}

uniq int char*::length(const char* str) {
    if(str == null) {
        return 0;
    }
    return strlen(str);
}

uniq int char[]::length(const char* str) {
    if(str == null) {
        return 0;
    }
    return strlen(str);
}

uniq int char[]::length(const char* str) {
    if(str == null) {
        return 0;
    }
    return strlen(str);
}

uniq string char*::reverse(const char* str) 
{
    if(str == null) {
        return string("");
    }
    int len = strlen(str);
    char*% result = new char[len + 1];

    for(int i=0; i<len; i++) {
        result[i] = str[len-i-1];
    }

    result[len] = '\0';

    return result;
}

uniq string string::operator_load_range_element(char* str, int head, int tail)
{
    if(str == null) {
        return string("");
    }

    int len = strlen(str);

    if(head < 0) {
        head += len;
    }
    if(tail < 0) {
        tail += len + 1;
    }

    if(head > tail) {
        return str.substring(tail, head).reverse();
    }

    if(head < 0) {
        head = 0;
    }

    if(tail >= len) {
        tail = len;
    }

    if(head == tail) {
        return string("");
    }

    if(tail-head+1 < 1) {
        return string("");
    }

    string result = new char[tail-head+1];

    memcpy(result, str + head, tail-head);
    result[tail-head] = '\0';

    return result;
}

uniq string char*::operator_load_range_element(char* str, int head, int tail)
{
    if(str == null) {
        return string("");
    }

    int len = strlen(str);

    if(head < 0) {
        head += len;
    }
    if(tail < 0) {
        tail += len + 1;
    }

    if(head > tail) {
        return str.substring(tail, head).reverse();
    }

    if(head < 0) {
        head = 0;
    }

    if(tail >= len) {
        tail = len;
    }

    if(head == tail) {
        return string("");
    }

    if(tail-head+1 < 1) {
        return string("");
    }

    string result = new char[tail-head+1];

    memcpy(result, str + head, tail-head);
    result[tail-head] = '\0';

    return result;
}

uniq string char*::substring(const char* str, int head, int tail)
{
    if(str == null) {
        return string("");
    }

    int len = strlen(str);

    if(head < 0) {
        head += len;
    }
    if(tail < 0) {
        tail += len + 1;
    }

    if(head > tail) {
        return str.substring(tail, head).reverse();
    }

    if(head < 0) {
        head = 0;
    }

    if(tail >= len) {
        tail = len;
    }

    if(head == tail) {
        return string("");
    }

    if(tail-head+1 < 1) {
        return string("");
    }

    string result = new char[tail-head+1];

    memcpy(result, str + head, tail-head);
    result[tail-head] = '\0';

    return result;
}

uniq string xsprintf(const char* msg, ...)
{
    if(msg == null) {
        return string("");
    }
    va_list/ args;
    va_start(args, msg);
    char* result;
    int len = vasprintf(&result, msg, args);
    va_end(args);
    
    if(len < 0) {
        return string("");
    }
    
    string result2 = string(result);
    
    free(result);
    
    return result2;
}

uniq string char*::delete(char* str, int head, int tail) 
{
    if(str == null) {
        return string("");
    }
    
    int len = strlen(str);
    
    if(head >= len) {
        return string(str);
    }

    if(strcmp(str, "") == 0) {
        return string(str);
    }
    
    if(head < 0) {
       head += len;
    }
    
    if(tail < 0) {
       tail += len + 1;
    }

    if(head < 0) {
        head = 0;
    }

    if(tail < 0) {
        return string(str);
    }

    if(tail >= len) {
        tail = len;
    }
    
    char*% result = new char[len-(tail-head)+1];
    
    memcpy(result, str, head);
    memcpy(result + head, str + tail, len-tail);
    
    result[len -(tail-head)] = '\0';

    return result;
}

uniq list<string>*% char*::split_char(char* self, char c) 
{
    if(self == null) {
        return new list<string>();
    }
    
    var result = new list<string>.initialize();

    var str = new buffer.initialize();

    int self_len = self.length();
    for(int i=0; i<self_len; i++) {
        if(self[i] == c) {
            result.push_back(string(str.buf));
            str.reset();
        }
        else {
            str.append_char(self[i]);
        }
    }
    if(str.length() != 0) {
        result.push_back(string(str.buf));
    }

    return result;
}

uniq string char*::xsprintf(char* self, const char* msg, ...)
{
    return xsprintf(msg, self);
}

uniq string int::xsprintf(int self, const char* msg, ...)
{
    return xsprintf(msg, self);
}


uniq string char*::printable(char* str)
{
    if(str == null) { return string(""); }
    
    int len = str.length();
    string result = new char[len*2+1];

    int n = 0;
    for(int i=0; i<len; i++) {
        char c = str[i];

        if((c >= 0 && c < ' ') 
            || c == 127)
        {
            result[n++] = '^';
            result[n++] = c + 'A' - 1;
        }
        else {
            result[n++] = c;
        }
    }

    result[n] = '\0'

    return result;
}

uniq string char[]::printable(char* str)
{
    return char*::printable(str);
}

uniq string char*::sub_plain(char* self, char* str, char* replace)
{
    if(self == null || str == null || replace == null) {
        return string(self);
    }

    var result = new buffer.initialize();
    
    char* p = self;
    
    while(true) {
        char* p2 = strstr(p, str);
        
        if(p2 == null) {
            p2 = p;
            while(*p2) {
                p2++;
            }
            result.append(p, p2 - p);
            break;
        }
        
        result.append(p, p2 - p);
        result.append_str(replace);
        
        p = p2 + strlen(str);
    }

    return result.to_string();
}

//////////////////////////////
/// base library(path library)
//////////////////////////////
uniq string xbasename(char* path)
{
    if(path == null) {
        return string("");
    }
    char* p = path + strlen(path);
    
    while(p >= path) {
        if(*p == '/') {
            break;
        }
        else {
            p--;
        }
    }
    
    if(p < path) {
        return string(path);
    }
    else {
        return string(p+1);  
    }
    
    return string("");
}

uniq string xnoextname(char* path)
{
    if(path == null) {
        return string("");
    }
    string path2 = xbasename(path);
    
    char* p = path2 + strlen(path2);
    
    while(p >= path2) {
        if(*p == '.') {
            break;
        }
        else {
            p--;
        }
    }
    
    if(p < path2) {
        return string(path2);
    }
    else {
        return path2.substring(0, p - path2);
    }
    
    return string("");
}

uniq string xextname(char* path)
{
    if(path == null) {
        return string("");
    }
    char* p = path + strlen(path);
    
    while(p >= path) {
        if(*p == '.') {
            break;
        }
        else {
            p--;
        }
    }
    
    if(p < path) {
        return string(path);
    }
    else {
        return string(p+1);  
    }
    
    return string("");
}

//////////////////////////////
/// base library(to_string)
//////////////////////////////
uniq string bool::to_string(bool self)
{
    if(self) {
        return string("true");
    }
    else {
        return string("false");
    }
}

uniq string _Bool::to_string(bool self)
{
    if(self) {
        return string("true");
    }
    else {
        return string("false");
    }
}

uniq string char::to_string(char self)
{
    return xsprintf("%c", self);
}

uniq string short::to_string(short self)
{
    return xsprintf("%d", self);
}

uniq string int::to_string(int self)
{
    return xsprintf("%d", self);
}

uniq string long::to_string(long self)
{
    return xsprintf("%ld", self);
}

uniq string size_t::to_string(size_t self)
{
    return xsprintf("%ld", self);
}

uniq string float::to_string(float self)
{
    return xsprintf("%f", self);
}

uniq string double::to_string(double self)
{
    return xsprintf("%lf", self);
}

uniq string string::to_string(char* self)
{
    if(self == null) {
        return string("");
    }
    return string(self);
}

uniq string char*::to_string(const char* self)
{
    if(self == null) {
        return string("");
    }
    return string(self);
}

//////////////////////////////
/// base library(compare)
//////////////////////////////
uniq int bool::compare(bool left, bool right)
{
    if(!left && right) {
        return -1;
    }
    else if(left && right) {
        return 0;
    }
    else if(!left && !right) {
        return 0;
    }
    else {
        return 1;
    }
    
    return 0;
}

uniq int _Bool::compare(bool left, bool right)
{
    if(!left && right) {
        return -1;
    }
    else if(left && right) {
        return 0;
    }
    else if(!left && !right) {
        return 0;
    }
    else {
        return 1;
    }
    
    return 0;
}

uniq int char::compare(char left, char right) 
{
    if(left < right) {
        return -1;
    }
    else if(left > right) {
        return 1;
    }
    else {
        return 0;
    }
    
    return 0;
}

uniq int short::compare(short left, short right) 
{
    if(left < right) {
        return -1;
    }
    else if(left > right) {
        return 1;
    }
    else {
        return 0;
    }
    
    return 0;
}

uniq int int::compare(int left, int right) 
{
    if(left < right) {
        return -1;
    }
    else if(left > right) {
        return 1;
    }
    else {
        return 0;
    }
    
    return 0;
}

uniq int long::compare(long left, long right) 
{
    if(left < right) {
        return -1;
    }
    else if(left > right) {
        return 1;
    }
    else {
        return 0;
    }
    
    return 0;
}

uniq int size_t::compare(size_t left, size_t right) 
{
    if(left < right) {
        return -1;
    }
    else if(left > right) {
        return 1;
    }
    else {
        return 0;
    }
    
    return 0;
}

uniq int float::compare(float left, float right) 
{
    if(left < right) {
        return -1;
    }
    else if(left > right) {
        return 1;
    }
    else {
        return 0;
    }
    
    return 0;
}

uniq int double::compare(double left, double right) 
{
    if(left < right) {
        return -1;
    }
    else if(left > right) {
        return 1;
    }
    else {
        return 0;
    }
    
    return 0;
}

uniq int string::compare(char* left, char* right) 
{
    if(left == null && right == null) {
        return 0;
    }
    else if(left == null) {
        return -1;
    }
    else if(right == null) {
        return 1;
    }
    
    return strcmp(left,right);
}

uniq int char*::compare(char* left, char* right) 
{
    if(left == null && right == null) {
        return 0;
    }
    else if(left == null) {
        return -1;
    }
    else if(right == null) {
        return 1;
    }
    
    return strcmp(left,right);
}

//////////////////////////////
/// base library(STDOUT, STDIN)
//////////////////////////////
uniq string char*::puts(char* self)
{
    if(self == null) {
        return string("");
    }
    puts(self);
    
    return string(self);
}

uniq string char*::print(char* self)
{
    if(self == null) {
        return string("");
    }
    printf("%s", self);
    
    return string(self);
}

#ifndef UNIX
    uniq string char*::printf(char* self, ...)
    {
        if(self == null) {
            return string("");
        }
        char* msg2;
    
        char msg2[128];
        
        va_list/ args;
        va_start(args, self);
        int len = snprintf(msg2, 128, self, args);
        va_end(args);
        
        printf("%s", msg2);
    
        free(msg2);
        
        return string(self);
    }
#else
    uniq string char*::printf(char* self, ...)
    {
        if(self == null) {
            return string("");
        }
        char* msg2;
    
        va_list/ args;
        va_start(args, self);
        vasprintf(&msg2,self,args);
        va_end(args);
        
        printf("%s", msg2);
    
        free(msg2);
        
        return string(self);
    }
#endif

uniq int int::printf(int self, char* msg)
{
    printf(msg, self);
    
    return self;
}

uniq long long::printf(long self, char* msg)
{
    printf(msg, self);
    
    return self;
}

uniq void int::times(int self, void* parent, void (*block)(void* parent, int it))
{
    for(int i = 0; i < self; i++) {
        block(parent, i);
    }
}


//////////////////////////////
/// base library(IO-FILE)
//////////////////////////////
#ifdef UNIX
    uniq Result<FILE*>*% xfopen(const char* file_name, const char* mode)
    {
        FILE* f = fopen(file_name, mode);

        if(f == NULL) {
            return new Result<FILE*>.None();
        }

        return new Result<FILE*>.Some(f);
    }
    uniq Result<buffer*%>*% FILE*::fread(FILE* f)
    {
        if(f == null) {
            return new Result<buffer*%>.None();
        }
        buffer*% buf = new buffer.initialize();
        
        while(1) {
            char buf2[BUFSIZ];
            
            int size = fread(buf2, 1, BUFSIZ, f);
            
            buf.append(buf2, size);

            if(size < BUFSIZ) {
                break;
            }
        }

        if(ferror(f)) {
            return new Result<buffer*%>.None();
        }

        return new Result<buffer*%>.Some(buf);
    }

    uniq Result<int>*% FILE*::fwrite(FILE* f, const char* str)
    {
        if(f == null || str == null) {
            return new Result<int>.None();
        }

        int result = fwrite(str, 1, strlen(str), f);

        if(result != strlen(str)) {
            return new Result<int>.None();
        }

        return new Result<int>.Some(result);
    }

    uniq Result<int>*% FILE*::fclose(FILE* f)
    {
        if(f == null) {
            return new Result<int>.None();
        }
        
        int result = fclose(f);
        
        if(result < 0) {
            return new Result<int>.None();
        }
        
        return new Result<int>.Some(result);
    }
    
    uniq Result<FILE*>*% FILE*::fprintf(FILE* f, const char* msg, ...)
    {
        if(f == null || msg == null) {
            return new Result<FILE*>.None();
        }
        char msg2[1024*2*2*2];
    
        va_list/ args;
        va_start(args, msg);
        vsnprintf(msg2, 1024*2*2*2, msg, args);
        va_end(args);
    
        int result = fprintf(f, "%s", msg2);
        
        if(result < 0) {
            return new Result<FILE*>.None();
        }
        
        return new Result<FILE*>.Some(f);
    }
    
    uniq Result<list<string>*%>*% FILE*::readlines(FILE* f)
    {
        list<string>*% result = new list<string>.initialize();
        
        if(f == null) {
            return new Result<list<string>*%>.None();
        }
        
        while(1) {
            char buf[BUFSIZ];
            
            if(fgets(buf, BUFSIZ, f) == NULL) {
                break;
            }
            
            result.push_back(string(buf));
        }

        if(ferror(f)) {
            return new Result<list<string>*%>.None();
        }

        return new Result<list<string>*%>.Some(result);
    }
    
    uniq bool xiswalpha(wchar_t c)
    {
        bool result = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        return result;
    }
    
    uniq bool xiswblank(wchar_t c)
    {
        return c == ' ' || c == '\t';
    }
    
    uniq bool xiswdigit(wchar_t c)
    {
        return (c >= '0' && c <= '9');
    }
    
    uniq bool xiswalnum(wchar_t c)
    {
        return xiswalpha(c) || xiswdigit(c);
    }
    
    uniq bool xiswascii(wchar_t c)
    {
        bool result = (c >= ' ' && c <= '~');
        return result;
    }
#endif

// DSL PIPE FILTERS WITH ZERO COST 
impl list<T>
{
    iter_begin iter() {
        ({
            var _li = new list<T>();
            int i = 0;
            bool _skip_while_done = false;
            foreach(it, `self) \{
                `next();
                i++;
            \};
            _li
        })
    }
    iter filter() {
        bool result = `block();
        
        if(result) \{
            `next();
        \}
    }
    iter take(int n) {
        if(i < n) \{
            `next();
        \}
    }
    iter skip(int skip_num) {
        if(i >= skip_num) \{
            `next();
        \}
    }
    iter take_while() {
        bool result = `block();
        
        if(result) \{
            `next();
        \}
        else \{
            break;
        \}
    }
    iter skip_while() {
        bool result = true;
        
        if(!_skip_while_done) \{
            result = `block();
            
            if(!result) \{
                _skip_while_done = true;
            \}
        \}
        
        if(_skip_while_done) \{
            `next();
        \}
    }
    iter step_by(int step_by_num) {
        if(i % step_by_num == 0) \{
            `next();
        \}
    }
    iter cloned() {
        if(isheap(T)) \{
            `it = clone it;
        \}
        else \{
            `it = dummy_heap dupe it;
        \}
        `next();
    }
    iter copied() {
        if(isheap(T)) \{
            `it = clone it;
        \}
        else \{
            `it = dummy_heap dupe it;
        \}
        `next();
    }
    iter map() {
        `it = `block();
    }
    iter enumerate() {
        `it = t(i, it);
    }
    iter inspect() {
        `block();
        `next();
    }
    iter find() {
        bool result = `block();
        
        if(result) \{
            `next();
            break;
        \}
    }
    iter_end each() {
        `block();
    }
    iter_end for_each() {
        `block();
    }
    iter_end collect() {
        _li.add(it);
    }
    iter_end end() {
    }
}

impl vector<T>
{
    iter_begin iter() {
        ({
            var _li = new vector<T>();
            int i = 0;
            bool _skip_while_done = false;
            foreach(it, `self) \{
                `next();
                i++;
            \};
            _li
        })
    }
    iter filter() {
        bool result = `block();
        
        if(result) \{
            `next();
        \}
    }
    iter take(int n) {
        if(i < n) \{
            `next();
        \}
    }
    iter skip(int skip_num) {
        if(i >= skip_num) \{
            `next();
        \}
    }
    iter take_while() {
        bool result = `block();
        
        if(result) \{
            `next();
        \}
        else \{
            break;
        \}
    }
    iter skip_while() {
        bool result = true;
        
        if(!_skip_while_done) \{
            result = `block();
            
            if(!result) \{
                _skip_while_done = true;
            \}
        \}
        
        if(_skip_while_done) \{
            `next();
        \}
    }
    iter step_by(int step_by_num) {
        if(i % step_by_num == 0) \{
            `next();
        \}
    }
    iter cloned() {
        if(isheap(T)) \{
            `it = clone it;
        \}
        else \{
            `it = dummy_heap dupe it;
        \}
        `next();
    }
    iter copied() {
        if(isheap(T)) \{
            `it = clone it;
        \}
        else \{
            `it = dummy_heap dupe it;
        \}
        `next();
    }
    iter map() {
        `it = `block();
    }
    iter enumerate() {
        `it = t(i, it);
    }
    iter inspect() {
        `block();
        `next();
    }
    iter find() {
        bool result = `block();
        
        if(result) \{
            `next();
            break;
        \}
    }
    iter_end each() {
        `block();
    }
    iter_end for_each() {
        `block();
    }
    iter_end collect() {
        _li.add(it);
    }
    iter_end end() {
    }
}

impl map<T, T2>
{
    iter_begin iter() {
        ({
            var _li = new list<T>();
            int i = 0;
            bool _skip_while_done = false;
            foreach(it, `self) \{
                `next();
                i++;
            \};
            _li
        })
    }
    iter filter() {
        bool result = `block();
        
        if(result) \{
            `next();
        \}
    }
    iter take(int n) {
        if(i < n) \{
            `next();
        \}
    }
    iter skip(int skip_num) {
        if(i >= skip_num) \{
            `next();
        \}
    }
    iter take_while() {
        bool result = `block();
        
        if(result) \{
            `next();
        \}
        else \{
            break;
        \}
    }
    iter skip_while() {
        bool result = true;
        
        if(!_skip_while_done) \{
            result = `block();
            
            if(!result) \{
                _skip_while_done = true;
            \}
        \}
        
        if(_skip_while_done) \{
            `next();
        \}
    }
    iter step_by(int step_by_num) {
        if(i % step_by_num == 0) \{
            `next();
        \}
    }
    iter cloned() {
        if(isheap(T)) \{
            `it = clone it;
        \}
        else \{
            `it = dummy_heap dupe it;
        \}
        `next();
    }
    iter copied() {
        if(isheap(T)) \{
            `it = clone it;
        \}
        else \{
            `it = dummy_heap dupe it;
        \}
        `next();
    }
    iter map() {
        `it = `block();
    }
    iter enumerate() {
        `it = t(i, it);
    }
    iter inspect() {
        `block();
        `next();
    }
    iter find() {
        bool result = `block();
        
        if(result) \{
            `next();
            break;
        \}
    }
    iter_end each() {
        `block();
    }
    iter_end for_each() {
        `block();
    }
    iter_end collect() {
        _li.add(it);
    }
    iter_end end() {
    }
}

uniq int string::index(char* str, const char* search_str, int default_value=-1)
{
    return char*::index(str, search_str, default_value);
}

uniq int char*::index(const char* str, const char* search_str, int default_value)
{
    if(str == null || search_str == null) {
        return default_value;
    }
    
    char* head = strstr(str, search_str);

    if(head == null) {
        return default_value;
    }

    return head - str;
}


typedef wchar_t*% wstring;

extern string string::lower_case(const char* str);
extern string string::upper_case(const char* str);
extern int char*::index_regex(const char* self, const char* reg, int default_value, bool ignore_case=false);
extern int char*::rindex(const char* str, const char* search_str, int default_value);
extern int char*::rindex_regex(const char* self, const char* reg, int default_value, bool ignore_case=false);
extern string char*::strip(const char* self);
extern string string::chomp(const char* str)
extern string xrealpath(const char* path);
extern string char*::replace(char* self, int index, char c);
extern string char*::multiply(char* str, int n);
extern list<string>*% char*::split_str(const char* self, const char* str) ;
extern int string::rindex(char* str, const char* search_str, int default_value=-1) ;
extern int string::rindex_regex(char* self, const char* reg, int default_value=-1, bool ignore_case=false);
extern string string::strip(const char* self);
extern int string::index_regex(char* self, const char* reg, int default_value=-1, bool ignore_case=false);
extern string string::replace(char* self, int index, char c);
extern string string::multiply(char* str, int n);
extern bool char*::match(char* self, const char* reg, bool ignore_case=false);
extern list<string>*% char*::scan(const char* self, const char* reg, bool ignore_case=false);
extern list<string>*% char*::split(const char* self, const char* reg, bool ignore_case=false);
extern string string::sub(char* self, const char* reg, const char* replace, bool ignore_case=false);
extern list<string>*% string::split_str(char* self, const char* str);
extern list<string>*% string::scan(char* self, const char* reg, bool ignore_case=false);
extern list<string>*% string::split(char* self, const char* reg, bool ignore_case=false);
extern bool string::match(char* self, const char* reg, bool ignore_case=false);
extern string char*::sub(char* self, const char* reg, const char* replace, bool global=true, bool ignore_case=false);
extern string char*::sub_block(char* self, const char* reg, bool global=true, bool ignore_case=false, void* parent, string (*block)(void* parent, char* match_string, list<string>* group_strings));
extern list<string>*% char*::scan_block(const char* self, const char* reg, bool ignore_case=false, void* parent, string (*block)(void* parent, char* match_string, list<string>* group_strings));
extern string string::sub_block(char* self, const char* reg, bool global=true, bool ignore_case=false, void* parent, string (*block)(void* parent, char* match_string, list<string>* group_strings));
extern wstring __builtin_wstring(const char* str, char* sname=__caller_sname__, int sline=__caller_line__);
extern int wchar_t*::length(const wchar_t* str);
extern int wchar_t[]::length(const wchar_t* str);
extern int wstring::length(const wchar_t* str);
extern string string::lower_case(char* str);
extern string string::upper_case(char* str);
extern wstring wchar_t*::substring(const wchar_t* str, int head, int tail);
extern int char*::index_count(const char* str, const char* search_str, int count, int default_value);
extern int char*::rindex(const char* str, const char* search_str, int default_value);
extern int char*::rindex_count(const char* str, const char* search_str, int count, int default_value);
extern string char*::strip(const char* self);
extern string wchar_t*::to_string(const wchar_t* wstr);
extern string wchar_t[]::to_string(const wchar_t* wstr);
extern wstring char*::to_wstring(const char* str);
extern wstring char[]::to_wstring(char* str);
extern wstring wchar_t*::delete(wchar_t* str, int head, int tail) ;
extern int wchar_t*::index(const wchar_t* str, const wchar_t* search_str, int default_value);
extern int wchar_t*::rindex(const wchar_t* str, const wchar_t* search_str, int default_value);
extern wstring wchar_t*::reverse(const wchar_t* str) ;
extern wstring wchar_t*::multiply(const wchar_t* str, int n);
extern wstring wchar_t*::printable(const wchar_t* str);
extern int wchar_t*::compare(const wchar_t* left, wchar_t* right);
extern int wstring::compare(const wchar_t* left, const wchar_t* right);
extern bool wchar_t*::equals(const wchar_t left, wchar_t right);
extern wstring wchar_t*::operator_mult(const wchar_t* str, int n);
extern wstring wstring::operator_mult(const wchar_t* str, int n);
extern bool wchar_t*::operator_equals(const wchar_t* left, const wchar_t* right);
extern bool wstring::operator_equals(const wchar_t* left, const wchar_t* right);
extern bool wchar_t*::operator_not_equals(const wchar_t* left, const wchar_t* right);
extern bool wstring::operator_not_equals(const wchar_t* left, const wchar_t* right);
extern wstring wchar_t*::operator_add(const wchar_t* left, const wchar_t* right);
extern wstring wstring::operator_add(const wchar_t* left, const wchar_t* right);
extern string char*::replace(char* self, int index, char c);
extern string char*::multiply(const char* str, int n);
extern list<string>*% char*::split_str(const char* self, const char* str) ;
extern unsigned int wchar_t*::get_hash_key(const wchar_t* value);
extern bool wchar_t*::equals(const wchar_t* left, const wchar_t* right);
extern bool wstring::equals(const wchar_t* left, const wchar_t* right);
extern bool wchar_t::operator_equals(wchar_t left, wchar_t right);
extern bool wchar_t::operator_not_equals(wchar_t left, wchar_t right);
extern unsigned int wchar_t::get_hash_key(wchar_t value);
extern bool wchar_t::equals(wchar_t left, wchar_t right);
extern string wchar_t::to_string(wchar_t wc);
extern string xrealpath(const char* path);
extern string xdirname(const char* path);
extern size_t xwcslen(const wchar_t* wstr);
extern wstring wstring::substring(const wchar_t* str, int head, int tail) ;
extern int string::index_count(const char* str, const char* search_str, int count=1, int default_value=-1);
extern int string::rindex(const char* str, const char* search_str, int default_value=-1) ;
extern int string::rindex_count(const char* str, const char* search_str, int count=1, int default_value=-1);
extern string string::strip(const char* self);
extern wstring string::to_wstring(const char* str);
extern string wstring::to_string(const wchar_t* wstr);
extern wstring int::to_wstring(int self);
extern wstring wstring::delete(wchar_t* str, int head, int tail);
extern int wstring::index(const wchar_t* str, const wchar_t* search_str, int default_value=1);
extern int wstring::rindex(const wchar_t* str, const wchar_t* search_str, int default_value=-1);
extern wstring wstring::reverse(const wchar_t* str);
extern wstring wstring::multiply(const wchar_t* str, int n);
extern wstring wstring::printable(const wchar_t* str);
extern unsigned int wstring::get_hash_key(const wchar_t* value);
extern string string::replace(char* self, int index, char c);
extern string string::multiply(const char* str, int n);
extern list<string>*% string::split_str(const char* self, const char* str);
extern wstring string::to_wstring(const char* str);
extern string char*::chomp(const char* str);
extern bool wchar_t*::equals(const wchar_t* left, const wchar_t* right);
extern bool wchar_t*::operator_equals(const wchar_t* left, const wchar_t* right);
extern bool wchar_t*::operator_not_equals(const wchar_t* left, const wchar_t* right);

#endif
