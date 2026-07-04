#ifndef NEO_C_STR_H
#define NEO_C_STR_H

#define UNIX 1

using neo-c;

typedef char*% string;


#ifndef NEO_C_H
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

#ifndef NULL
#define NULL ((void*)0)
#endif

using neo-c;
using unsafe;

extern string __builtin_string(const char* str, char* sname=__caller_sname__, int sline=__caller_line__);

extern bool xisalpha(char c);
extern bool xisblank(char c);
extern bool xisdigit(char c);
extern bool xisspace(char c);
extern bool xisalnum(char c);
extern bool xisascii(char c);
extern bool xispunct(char c);
extern string char*::reverse(const char* str)
extern int string::length(const char* str);
extern int char*::length(const char* str);
extern string char*::reverse(const char* str) ;
extern int string::length(const char* str);
extern int char*::length(const char* str);
extern int char[]::length(const char* str);
extern string string::operator_load_range_element(char* str, int head, int tail);
extern string char*::operator_load_range_element(char* str, int head, int tail);
extern string char*::substring(const char* str, int head, int tail);
extern string xsprintf(const char* msg, ...);
extern string char*::delete(char* str, int head, int tail) ;
extern string char*::xsprintf(char* self, const char* msg, ...);
extern string int::xsprintf(int self, const char* msg, ...);
extern string char*::printable(char* str);
extern string char[]::printable(char* str);
extern string char*::sub_plain(char* self, char* str, char* replace);

struct buffer 
{
    char*% buf;
    int len;
    int size;
};

extern buffer*% buffer*::initialize(buffer*% self);
extern void buffer*::finalize(buffer* self);
extern buffer*% buffer*::clone(buffer* self);
extern bool buffer*::equals(buffer* left, buffer* right);
extern buffer* buffer*::append_str(buffer* self, const char* mem);
extern buffer* buffer*::append(buffer* self, const char* mem, size_t size);
extern string xsprintf(const char* msg, ...);
extern string char*::to_string(const char* self);
extern string int::to_string(int self);
extern unsigned int bool::get_hash_key(bool value);
extern unsigned int _Bool::get_hash_key(bool value);
extern unsigned int char::get_hash_key(char value);
extern unsigned int short::get_hash_key(short int value);
extern unsigned int int::get_hash_key(int value);
extern unsigned int long::get_hash_key(long value);
extern unsigned int size_t::get_hash_key(size_t value);
extern unsigned int float::get_hash_key(float value);
extern unsigned int double::get_hash_key(double value);
extern unsigned int char*::get_hash_key(const char* value);
extern unsigned int string::get_hash_key(char* value);
extern unsigned int void*::get_hash_key(void* value);
extern string char*::substring(const char* str, int head, int tail);
extern buffer* buffer*::append_format(buffer* self, const char* msg, ...);
extern string buffer*::to_string(buffer* self);
extern string char*::to_string(const char* self);
extern string double::to_string(double self);
extern string float::to_string(float self);
extern string size_t::to_string(size_t self);
extern string long::to_string(long self);
extern string int::to_string(int self);
extern string short::to_string(short self);
extern string char::to_string(char self);
extern string bool::to_string(bool self);
extern string _Bool::to_string(bool self);
extern bool string::equals(char* self, const char* right);
extern void buffer*::reset(buffer* self);
extern buffer* buffer*::append_char(buffer* self, char c);
extern int buffer*::length(buffer* self) ;

struct neo_frame {
    neo_frame *prev;
    char* fun_name;
    unsigned long frame_id;
};
    
extern __thread neo_frame* neo_current_frame;
extern __thread unsigned long neo_frame_id;

extern bool come_is_alive(void* mem);
extern void stackframe();
extern void stackframe2(void* mem);
extern bool die(const char* msg, char* sname=__caller_sname__, int sline=__caller_line__);
extern void come_heap_final();
extern char* come_dynamic_typeof(void* mem);
extern size_t dynamic_sizeof(void* mem);
extern void* come_calloc(size_t count, size_t size, const char* sname=null, int sline=0, int id=0, const char* class_name="");
extern bool come_is_alive(void* mem);
extern void come_free(void* mem);
extern void* come_memdup(void* block, char* sname=null, int sline=0, int id=0, const char* class_name=null);
extern void* come_increment_ref_count(void* mem, char* sname, int sline, int id);
extern void* come_print_ref_count(void* mem);
extern int come_get_ref_count(void* mem);
extern void* come_decrement_ref_count(void* mem, void* protocol_fun, void* protocol_obj, bool no_decrement, bool no_free, void* result_obj, char* sname, int sline, int id);
extern void come_call_finalizer(void* fun, void* mem, void* protocol_fun, void* protocol_obj, int call_finalizer_only, int no_decrement, int no_free, void* result_obj, char* sname, int sline, int id);
extern void xassert(const char* msg, bool test);
extern void* come_null_checker(void* mem, const char* sname, int sline, int id);
extern void* come_heap_checker(void* mem, const char* sname, int sline, int id);
extern string __builtin_string(const char* str, char* sname=__caller_sname__, int sline=__caller_line__);

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

extern list<string>*% char*::split_char(char* self, char c) ;
extern list<string>*% char*::split_char(char* self, char c) ;
#endif

/*
 *
 * Mini regex-module inspired by Rob Pike's regex code described in:
 *
 * http://www.cs.princeton.edu/courses/archive/spr09/cos333/beautiful.html
 *
 *
 *
 * Supports:
 * ---------
 *   '.'        Dot, matches any character
 *   '^'        Start anchor, matches beginning of string
 *   '$'        End anchor, matches end of string
 *   '*'        Asterisk, match zero or more (greedy)
 *   '+'        Plus, match one or more (greedy)
 *   '?'        Question, match zero or one (non-greedy)
 *   '[abc]'    Character class, match if one of {'a', 'b', 'c'}
 *   '[^abc]'   Inverted class, match if NOT one of {'a', 'b', 'c'} -- NOTE: feature is currently broken!
 *   '[a-zA-Z]' Character ranges, the character set of the ranges { a-z | A-Z }
 *   '\s'       Whitespace, \t \f \r \n \v and spaces
 *   '\S'       Non-whitespace
 *   '\w'       Alphanumeric, [a-zA-Z0-9_]
 *   '\W'       Non-alphanumeric
 *   '\d'       Digits, [0-9]
 *   '\D'       Non-digits
 *   '()'       Grouping, allowing quantifiers on sub-expressions and capturing
 *
 *
 */


#ifndef RE_DOT_MATCHES_NEWLINE
// Define to 0 if you DON'T want '.' to match '\r' + '\n' 
#define RE_DOT_MATCHES_NEWLINE 0
#endif

// Typedef'd pointer to get abstract datatype. 
struct re_program;
typedef struct re_program* re_t;


typedef struct re_capture
{
  int start;
  int length;
} re_capture;


// Definitions:

#define MAX_REGEXP_OBJECTS   64   // Max number of regex symbols in expression, incl. groups. 
#define MAX_CHAR_CLASS_LEN   40   // Max length of character-class buffer.                   
#define MAX_CAPTURE_SLOTS    MAX_REGEXP_OBJECTS

enum
{
  RE_UNUSED,
  RE_DOT,
  RE_BEGIN,
  RE_END,
  RE_QUESTIONMARK,
  RE_STAR,
  RE_PLUS,
  RE_CHAR,
  RE_CHAR_CLASS,
  RE_INV_CHAR_CLASS,
  RE_DIGIT,
  RE_NOT_DIGIT,
  RE_ALPHA,
  RE_NOT_ALPHA,
  RE_WHITESPACE,
  RE_NOT_WHITESPACE,
  RE_GROUP,
  RE_GROUP_END
};


typedef struct regex_t regex_t;

struct regex_t
{
  unsigned char type;   // RE_CHAR, RE_STAR, RE_GROUP, etc. 
  union
  {
    unsigned char  ch;      // Literal character                
    unsigned char* ccl;     // Pointer to characters in a class  
    struct
    {
      regex_t* first;       // First token inside the group      
      regex_t* last;        // Sentinel token terminating group  
      int      id;          // Capture index (1-based)          
    } group;
  } u;
  regex_t* next;            // Linked list pointer for sequence 
};


typedef struct re_program
{
  regex_t*       start;
  int            group_count;
} regex_program_t;


typedef struct
{
  regex_t*        pool;
  int             pool_capacity;
  int             pool_size;
  unsigned char*  ccl_buf;
  int             ccl_capacity;
  int             ccl_idx;
  int             group_count;
} compiler_state;


typedef struct
{
  const char* base;
  re_capture* captures;
  int         capture_capacity;   // Slots provided by caller 
  int         total_groups;       // Groups present in pattern
  bool        ignore_case;
} match_context;



// Public functions: 
uniq int re_matchp_ex(re_t pattern, const char* text, int* matchlength, re_capture* captures, int max_captures, bool ignore_case)
{
  *matchlength = 0;
  if (pattern == 0)
  {
    return -1;
  }

  regex_program_t* program = (regex_program_t*) pattern;
  regex_t* start = program->start;
  if (start == 0)
  {
    return -1;
  }

  match_context ctx;
  ctx.base = text;
  ctx.captures = (captures != 0 && max_captures > 0) ? captures : 0;
  ctx.capture_capacity = (captures != 0 && max_captures > 0) ? max_captures : 0;
  if (ctx.capture_capacity > MAX_CAPTURE_SLOTS)
  {
    ctx.capture_capacity = MAX_CAPTURE_SLOTS;
  }
  ctx.total_groups = program->group_count;
  ctx.ignore_case = ignore_case;

  if (ctx.captures != 0)
  {
    clear_captures(&ctx);
  }

  if (start->type == RE_BEGIN)
  {
    const char* end = matchpattern(start->next, text, &ctx);
    if (end != 0)
    {
      *matchlength = (int)(end - text);
      if (ctx.captures != 0)
      {
        // Groups already recorded relative to ctx.base 
      }
      return 0;
    }
    return -1;
  }
  else
  {
    const char* cursor = text;
    while (1)
    {
      if (ctx.captures != 0)
      {
        clear_captures(&ctx);
      }
      const char* end = matchpattern(start, cursor, &ctx);
      if (end != 0)
      {
        if (*cursor == '\0' && cursor != text)
        {
          return -1; // Preserve legacy behaviour
        }
        *matchlength = (int)(end - cursor);
        return (int)(cursor - text);
      }

      if (*cursor == '\0')
      {
        break;
      }
      cursor += 1;
    }
  }

  return -1;
}

uniq int re_matchp(re_t pattern, const char* text, int* matchlength, re_capture* captures, int max_captures)
{
  return re_matchp_ex(pattern, text, matchlength, captures, max_captures, false);
}

uniq int neo_c_re_match(const char* pattern, const char* text, int* matchlength)
{
  return re_matchp(re_compile(pattern), text, matchlength, (re_capture*)0, 0);
}

uniq re_t re_compile(const char* pattern)
{
  static regex_t        re_compiled[MAX_REGEXP_OBJECTS];
  static unsigned char  ccl_buf[MAX_CHAR_CLASS_LEN];
  static regex_program_t program;

  compiler_state state;
  state.pool = re_compiled;
  state.pool_capacity = MAX_REGEXP_OBJECTS;
  state.pool_size = 0;
  state.ccl_buf = ccl_buf;
  state.ccl_capacity = MAX_CHAR_CLASS_LEN;
  state.ccl_idx = 1; // leave first slot unused to mimic original behaviour 
  state.group_count = 0;

  if (state.ccl_capacity > 0)
  {
    state.ccl_buf[0] = 0;
  }

  int pos = 0;
  regex_t* head = compile_sequence(&state, pattern, &pos, 0);
  if ((head == 0) || (pattern[pos] != '\0'))
  {
    return NULL;
  }

  program.start = head;
  program.group_count = state.group_count;
  return (re_t) &program;
}


uniq void re_print(re_t pattern)
{
  if (pattern == 0)
  {
    return;
  }

  regex_program_t* program = (regex_program_t*) pattern;
  if (program->start == 0)
  {
    return;
  }

  re_print_internal(program->start, 0);
}


// Private helper implementations 
uniq void clear_captures(match_context* ctx)
{
  if ((ctx->captures == 0) || (ctx->capture_capacity <= 0))
  {
    return;
  }

  for (int i = 0; i < ctx->capture_capacity; ++i)
  {
    ctx->captures[i].start = -1;
    ctx->captures[i].length = 0;
  }
}

uniq void snapshot_captures(const match_context* ctx, re_capture* buffer_)
{
  if ((ctx->captures == 0) || (ctx->capture_capacity <= 0))
  {
    return;
  }

  memcpy(buffer_, ctx->captures, sizeof(re_capture) * ctx->capture_capacity);
}

uniq void restore_captures(match_context* ctx, const re_capture* buffer_)
{
  if ((ctx->captures == 0) || (ctx->capture_capacity <= 0))
  {
    return;
  }

  memcpy(ctx->captures, buffer_, sizeof(re_capture) * ctx->capture_capacity);
}

uniq int re_unescape_literal_char(char escaped, unsigned char* out_char)
{
  switch (escaped)
  {
    case 't':
      *out_char = (unsigned char)'\t';
      return 1;
    case 'n':
      *out_char = (unsigned char)'\n';
      return 1;
    case 'r':
      *out_char = (unsigned char)'\r';
      return 1;
    case 'v':
      *out_char = (unsigned char)'\v';
      return 1;
    case 'f':
      *out_char = (unsigned char)'\f';
      return 1;
    default:
      return 0;
  }
}


uniq regex_t* new_token(compiler_state* st)
{
  if (st->pool_size >= st->pool_capacity)
  {
    return NULL;
  }

  regex_t* token = &st->pool[st->pool_size++];
  token->type = RE_UNUSED;
  token->u.ccl = NULL;
  token->next = NULL;
  token->u.group.first = NULL;
  token->u.group.last = NULL;
  token->u.group.id = 0;
  return token;
}

uniq int append_token(regex_t** head, regex_t** tail, regex_t* token)
{
  if (token == 0)
  {
    return 0;
  }

  if (*head == 0)
  {
    *head = token;
  }
  else
  {
    (*tail)->next = token;
  }
  *tail = token;
  return 1;
}

uniq regex_t* compile_sequence(compiler_state* st, const char* pattern, int* pos, int in_group)
{
  regex_t* head = (regex_t*)0;
  regex_t* tail = (regex_t*)0;

  while (pattern[*pos] != '\0')
  {
    char c = pattern[*pos];

    if (in_group && (c == ')'))
    {
      break;
    }

    regex_t* token = (regex_t*)0;

    switch (c)
    {
      case '^':
      {
        token = new_token(st);
        if (token == 0) return NULL;
        token->type = RE_BEGIN;
        (*pos)++;
      } break;

      case '$':
      {
        token = new_token(st);
        if (token == 0) return NULL;
        token->type = RE_END;
        (*pos)++;
      } break;

      case '.':
      {
        token = new_token(st);
        if (token == 0) return NULL;
        token->type = RE_DOT;
        (*pos)++;
      } break;

      case '*':
      {
        token = new_token(st);
        if (token == 0) return NULL;
        token->type = RE_STAR;
        (*pos)++;
      } break;

      case '+':
      {
        token = new_token(st);
        if (token == 0) return NULL;
        token->type = RE_PLUS;
        (*pos)++;
      } break;

      case '?':
      {
        token = new_token(st);
        if (token == 0) return NULL;
        token->type = RE_QUESTIONMARK;
        (*pos)++;
      } break;

      case '\\':
      {
        (*pos)++;
        if (pattern[*pos] == '\0')
        {
          return NULL;
        }

        token = new_token(st);
        if (token == 0) return NULL;

        switch (pattern[*pos])
        {
          case 'd': token->type = RE_DIGIT;          break;
          case 'D': token->type = RE_NOT_DIGIT;      break;
          case 'w': token->type = RE_ALPHA;          break;
          case 'W': token->type = RE_NOT_ALPHA;      break;
          case 's': token->type = RE_WHITESPACE;     break;
          case 'S': token->type = RE_NOT_WHITESPACE; break;
          default:
          {
            token->type = RE_CHAR;
            unsigned char escaped = 0;
            if (re_unescape_literal_char(pattern[*pos], &escaped))
            {
              token->u.ch = escaped;
            }
            else
            {
              token->u.ch = (unsigned char)pattern[*pos];
            }
          } break;
        }
        (*pos)++;
      } break;

      case '[':
      {
        int buf_begin = st->ccl_idx;
        int negated = 0;
        (*pos)++;

        if (pattern[*pos] == '^')
        {
          negated = 1;
          (*pos)++;
          if (pattern[*pos] == '\0')
          {
            return NULL;
          }
        }

        if (pattern[*pos] == '\0')
        {
          return NULL;
        }

        while ((pattern[*pos] != '\0') && (pattern[*pos] != ']'))
        {
          if (pattern[*pos] == '\\')
          {
            if (st->ccl_idx >= (st->ccl_capacity - 1))
            {
              return NULL;
            }
            st->ccl_buf[st->ccl_idx++] = '\\';
            (*pos)++;
            if (pattern[*pos] == '\0')
            {
              return NULL;
            }
          }

          if (st->ccl_idx >= st->ccl_capacity)
          {
            return NULL;
          }
          st->ccl_buf[st->ccl_idx++] = (unsigned char)pattern[*pos];
          (*pos)++;
        }

        if (pattern[*pos] != ']')
        {
          return NULL;
        }

        if (st->ccl_idx >= st->ccl_capacity)
        {
          return NULL;
        }
        st->ccl_buf[st->ccl_idx++] = 0;

        token = new_token(st);
        if (token == 0) return NULL;
        token->type = negated ? RE_INV_CHAR_CLASS : RE_CHAR_CLASS;
        token->u.ccl = &st->ccl_buf[buf_begin];

        (*pos)++;
      } break;

      case '(':
      {
        (*pos)++;
        regex_t* inner = compile_sequence(st, pattern, pos, 1);
        if (inner == 0)
        {
          return NULL;
        }
        if (pattern[*pos] != ')')
        {
          return NULL;
        }

        regex_t* tail = inner;
        while ((tail != 0) && (tail->type != RE_UNUSED))
        {
          tail = tail->next;
        }
        if (tail == 0)
        {
          return NULL;
        }

        token = new_token(st);
        if (token == 0) return NULL;
        token->type = RE_GROUP;
        token->u.group.first = inner;
        token->u.group.last = tail;
        token->u.group.id = ++st->group_count;

        tail->type = RE_GROUP_END;
        tail->u.group.first = token;

        (*pos)++;
      } break;

      case ')':
      {
        token = new_token(st);
        if (token == 0) return NULL;
        token->type = RE_CHAR;
        token->u.ch = (unsigned char)c;
        (*pos)++;
      } break;

      default:
      {
        token = new_token(st);
        if (token == 0) return NULL;
        token->type = RE_CHAR;
        token->u.ch = (unsigned char)c;
        (*pos)++;
      } break;
    }

    if (!append_token(&head, &tail, token))
    {
      return NULL;
    }
  }

  regex_t* sentinel = new_token(st);
  if (sentinel == 0)
  {
    return NULL;
  }
  sentinel->type = RE_UNUSED;
  sentinel->next = NULL;

  if (head == 0)
  {
    head = sentinel;
  }
  else
  {
    tail->next = sentinel;
  }

  return head;
}

uniq const char* matchpattern(regex_t* pattern, const char* text, match_context* ctx)
{
  if (pattern == 0)
  {
    return text;
  }

  if (pattern->type == RE_UNUSED)
  {
    return text;
  }

  re_capture snapshot[MAX_CAPTURE_SLOTS];
  snapshot_captures(ctx, snapshot);

  regex_t* current = pattern;
  const char* cursor = text;

  while (current != 0 && current->type != RE_UNUSED)
  {
    regex_t* next = current->next;

    if (current->type == RE_GROUP_END)
    {
      regex_t* owner = current->u.group.first;
      if ((owner != 0) && (owner->u.group.id > 0) && (ctx->captures != 0))
      {
        int idx = owner->u.group.id - 1;
        if (idx < ctx->capture_capacity)
        {
          int start = ctx->captures[idx].start;
          if (start >= 0)
          {
            ctx->captures[idx].length = (int)(cursor - ctx->base) - start;
            if (ctx->captures[idx].length < 0)
            {
              ctx->captures[idx].length = 0;
            }
          }
        }
      }
      current = current->next;
      continue;
    }

    if ((next != 0) && (next->type == RE_QUESTIONMARK))
    {
      const char* result = matchquestion(current, next->next, cursor, ctx);
      if (result != 0)
      {
        return result;
      }
      restore_captures(ctx, snapshot);
      return NULL;
    }
    else if ((next != 0) && (next->type == RE_STAR))
    {
      const char* result = matchstar(current, next->next, cursor, ctx);
      if (result != 0)
      {
        return result;
      }
      restore_captures(ctx, snapshot);
      return NULL;
    }
    else if ((next != 0) && (next->type == RE_PLUS))
    {
      const char* result = matchplus(current, next->next, cursor, ctx);
      if (result != 0)
      {
        return result;
      }
      restore_captures(ctx, snapshot);
      return NULL;
    }
    else if (current->type == RE_GROUP)
    {
      const char* result = matchgroup(current, next, cursor, ctx);
      if (result != 0)
      {
        return result;
      }
      restore_captures(ctx, snapshot);
      return NULL;
    }
    else if (current->type == RE_END)
    {
      if (*cursor != '\0')
      {
        restore_captures(ctx, snapshot);
        return NULL;
      }
      current = current->next;
    }
    else
    {
      const char* after = matchtoken(current, cursor, ctx);
      if (after == 0)
      {
        restore_captures(ctx, snapshot);
        return NULL;
      }
      cursor = after;
      current = current->next;
    }
  }

  return cursor;
}

uniq const char* matchgroup(regex_t* token, regex_t* rest, const char* text, match_context* ctx)
{
  re_capture snapshot_entry[MAX_CAPTURE_SLOTS];
  snapshot_captures(ctx, snapshot_entry);

  regex_t* end_token = token->u.group.last;
  regex_t* saved_next = end_token != 0 ? end_token->next : 0;
  if (end_token != 0)
  {
    end_token->next = rest;
  }

  if (token->u.group.id > 0 && ctx->captures != 0)
  {
    int idx = token->u.group.id - 1;
    if (idx < ctx->capture_capacity)
    {
      ctx->captures[idx].start = (int)(text - ctx->base);
      ctx->captures[idx].length = 0;
    }
  }

  const char* result = matchpattern(token->u.group.first, text, ctx);

  if (end_token != 0)
  {
    end_token->next = saved_next;
  }

  if (result == 0)
  {
    restore_captures(ctx, snapshot_entry);
  }

  return result;
}

uniq const char* matchstar(regex_t* token, regex_t* rest, const char* text, match_context* ctx)
{
  re_capture snapshot_entry[MAX_CAPTURE_SLOTS];
  snapshot_captures(ctx, snapshot_entry);

  const char* consume = matchtoken(token, text, ctx);
  while ((consume != 0) && (consume != text))
  {
    re_capture snapshot_after_token[MAX_CAPTURE_SLOTS];
    snapshot_captures(ctx, snapshot_after_token);

    const char* recursive = matchstar(token, rest, consume, ctx);
    if (recursive != 0)
    {
      return recursive;
    }

    restore_captures(ctx, snapshot_after_token);
    consume = matchtoken(token, consume, ctx);
  }

  restore_captures(ctx, snapshot_entry);
  return matchpattern(rest, text, ctx);
}

uniq const char* matchplus(regex_t* token, regex_t* rest, const char* text, match_context* ctx)
{
  re_capture snapshot_entry[MAX_CAPTURE_SLOTS];
  snapshot_captures(ctx, snapshot_entry);

  const char* first = matchtoken(token, text, ctx);
  if ((first == 0) || (first == text))
  {
    restore_captures(ctx, snapshot_entry);
    return NULL;
  }

  const char* result = matchstar(token, rest, first, ctx);
  if (result != 0)
  {
    return result;
  }

  restore_captures(ctx, snapshot_entry);
  return NULL;
}

uniq const char* matchquestion(regex_t* token, regex_t* rest, const char* text, match_context* ctx)
{
  re_capture snapshot_entry[MAX_CAPTURE_SLOTS];
  snapshot_captures(ctx, snapshot_entry);

  const char* skipped = matchpattern(rest, text, ctx);
  if (skipped != 0)
  {
    return skipped;
  }

  restore_captures(ctx, snapshot_entry);

  const char* consumed = matchtoken(token, text, ctx);
  if ((consumed == 0) || (consumed == text))
  {
    restore_captures(ctx, snapshot_entry);
    return NULL;
  }

  const char* with = matchpattern(rest, consumed, ctx);
  if (with != 0)
  {
    return with;
  }

  restore_captures(ctx, snapshot_entry);
  return NULL;
}

uniq unsigned char re_fold_char(unsigned char c, bool ignore_case)
{
  if (ignore_case && c >= 'A' && c <= 'Z')
  {
    return (unsigned char)(c - 'A' + 'a');
  }
  return c;
}

uniq const char* matchtoken(regex_t* token, const char* text, match_context* ctx)
{
  switch (token->type)
  {
    case RE_DOT:
      return (*text != '\0' && matchdot(*text)) ? text + 1 : 0;

    case RE_CHAR:
      return (*text != '\0'
        && re_fold_char(token->u.ch, ctx->ignore_case) == re_fold_char((unsigned char)*text, ctx->ignore_case))
        ? text + 1 : 0;

    case RE_CHAR_CLASS:
      return (*text != '\0' && matchcharclass(*text, (const char*)token->u.ccl, ctx->ignore_case)) ? text + 1 : 0;

    case RE_INV_CHAR_CLASS:
      return (*text != '\0' && !matchcharclass(*text, (const char*)token->u.ccl, ctx->ignore_case)) ? text + 1 : 0;

    case RE_DIGIT:
      return (*text != '\0' && matchdigit(*text)) ? text + 1 : 0;

    case RE_NOT_DIGIT:
      return (*text != '\0' && !matchdigit(*text)) ? text + 1 : 0;

    case RE_ALPHA:
      return (*text != '\0' && matchalphanum(*text)) ? text + 1 : 0;

    case RE_NOT_ALPHA:
      return (*text != '\0' && !matchalphanum(*text)) ? text + 1 : 0;

    case RE_WHITESPACE:
      return (*text != '\0' && matchwhitespace(*text)) ? text + 1 : 0;

    case RE_NOT_WHITESPACE:
      return (*text != '\0' && !matchwhitespace(*text)) ? text + 1 : 0;

    case RE_GROUP:
      return matchgroup(token, (regex_t*)0, text, ctx);

    case RE_BEGIN:
      return (text == ctx->base) ? text : 0;

    case RE_END:
      return (*text == '\0') ? text : 0;

    default:
      break;
  }

  return (const char*)0;
}


uniq int matchdigit(char c)
{
  return xisdigit((unsigned char)c);
}
uniq int matchalpha(char c)
{
  return xisalpha((unsigned char)c);
}
uniq int matchwhitespace(char c)
{
  return xisspace((unsigned char)c);
}
uniq int matchalphanum(char c)
{
  return ((c == '_') || matchalpha(c) || matchdigit(c));
}
uniq int matchrange(char c, const char* str, bool ignore_case)
{
  unsigned char needle = (unsigned char)c;
  unsigned char start = (unsigned char)str[0];
  unsigned char end = (unsigned char)str[2];
  if (ignore_case)
  {
    needle = re_fold_char(needle, true);
    start = re_fold_char(start, true);
    end = re_fold_char(end, true);
  }
  return (    (needle != '-')
           && (str[0] != '\0')
           && (str[0] != '-')
           && (str[1] == '-')
           && (str[2] != '\0')
           && (    (needle >= start)
                && (needle <= end)));
}
uniq int matchdot(char c)
{
#if defined(RE_DOT_MATCHES_NEWLINE) && (RE_DOT_MATCHES_NEWLINE == 1)
  (void)c;
  return 1;
#else
  return c != '\n' && c != '\r';
#endif
}
uniq int ismetachar(char c)
{
  return ((c == 's') || (c == 'S') || (c == 'w') || (c == 'W') || (c == 'd') || (c == 'D')
    || (c == 't') || (c == 'n') || (c == 'r') || (c == 'v') || (c == 'f'));
}

uniq int matchmetachar(char c, const char* str)
{
  switch (str[0])
  {
    case 'd': return  matchdigit(c);
    case 'D': return !matchdigit(c);
    case 'w': return  matchalphanum(c);
    case 'W': return !matchalphanum(c);
    case 's': return  matchwhitespace(c);
    case 'S': return !matchwhitespace(c);
    case 't': return (c == '\t');
    case 'n': return (c == '\n');
    case 'r': return (c == '\r');
    case 'v': return (c == '\v');
    case 'f': return (c == '\f');
    default:  return (c == str[0]);
  }
}

uniq int matchcharclass(char c, const char* str, bool ignore_case)
{
  unsigned char needle = re_fold_char((unsigned char)c, ignore_case);
  do
  {
    if (matchrange((char)needle, str, ignore_case))
    {
      return 1;
    }
    else if (str[0] == '\\')
    {
      str += 1;
      if (matchmetachar((char)needle, str))
      {
        return 1;
      }
      else if ((needle == re_fold_char((unsigned char)str[0], ignore_case)) && !ismetachar((char)str[0]))
      {
        return 1;
      }
    }
    else if (needle == re_fold_char((unsigned char)str[0], ignore_case))
    {
      if (needle == '-')
      {
        return ((str[-1] == '\0') || (str[1] == '\0'));
      }
      else
      {
        return 1;
      }
    }
  }
  while (*str++ != '\0');

  return 0;
}

uniq void re_print_internal(regex_t* pattern, int depth)
{
  const char* types[] =
  {
    "RE_UNUSED", "RE_DOT", "RE_BEGIN", "RE_END", "RE_QUESTIONMARK", "RE_STAR", "RE_PLUS", "RE_CHAR",
    "RE_CHAR_CLASS", "RE_INV_CHAR_CLASS", "RE_DIGIT", "RE_NOT_DIGIT", "RE_ALPHA", "RE_NOT_ALPHA",
    "RE_WHITESPACE", "RE_NOT_WHITESPACE", "RE_GROUP", "RE_GROUP_END"
  };

  while (pattern != 0 && pattern->type != RE_UNUSED)
  {
    for (int i = 0; i < depth; ++i)
    {
      putchar(' ');
    }
    printf("type: %s", types[pattern->type]);

    if ((pattern->type == RE_CHAR_CLASS) || (pattern->type == RE_INV_CHAR_CLASS))
    {
      printf(" [");
      const unsigned char* ccl = pattern->u.ccl;
      while (*ccl != '\0' && *ccl != ']')
      {
        printf("%c", *ccl);
        ++ccl;
      }
      printf("]");
    }
    else if (pattern->type == RE_CHAR)
    {
      printf(" '%c'", pattern->u.ch);
    }
    else if (pattern->type == RE_GROUP)
    {
      printf(" id=%d\n", pattern->u.group.id);
      re_print_internal(pattern->u.group.first, depth + 2);
      pattern = pattern->next;
      continue;
    }

    printf("\n");
    pattern = pattern->next;
  }
}

uniq int re_get_group_count(re_t pattern)
{
  if (pattern == 0)
  {
    return 0;
  }

  regex_program_t* program = (regex_program_t*) pattern;
  return program->group_count;
}

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

uniq string string::lower_case(const char* str)
{
    if(str == null) {
        return string("");
    }
    
    string result = string(str);
    int len = strlen(str);
    for(int i=0; i<len; i++) {
        if(str[i] >= 'A' && str[i] <= 'Z') {
            result[i] = str[i] - 'A' + 'a';
        }
    }
    
    return result;
}

uniq string string::upper_case(const char* str)
{
    if(str == null) {
        return string("");
    }
    
    string result = string(str);
    int len = strlen(str);
    for(int i=0; i<len; i++) {
        if(str[i] >= 'a' && str[i] <= 'z') {
            result[i] = str[i] - 'a' + 'A';
        }
    }
    
    return result;
}

uniq int char*::index_regex(const char* self, const char* reg, int default_value, bool ignore_case=false)
{
    if(self == null || reg == null) {
        return default_value;
    }
    
    re_t re = re_compile(reg);
    
    if(re == NULL) {
        return default_value;
    }
    
    int result = default_value;
    
    int offset = 0;

    int n = 0;
    
    int result = default_value;

    while(true) {
        int matchlength = 0;
        int max_captures = 8;
        re_capture captures[max_captures];
        int regex_result = re_matchp_ex(re, self, &matchlength, captures, max_captures, ignore_case);

        /// match and no group strings ///
        if(regex_result >= 0) 
        {
            result = regex_result;
            break;
        }
        /// no match ///
        {
            break;
        }
    }

    return result;
}

uniq int char*::rindex(const char* str, const char* search_str, int default_value)
{
    if(str == null || search_str == null) {
        return default_value;
    }
    
    int len = strlen(search_str);
    int str_len = strlen(str);
    if(len == 0) {
        return str_len;
    }
    if(len > str_len) {
        return default_value;
    }
    char* p = str + str_len - len;

    while(p >= str) {
        if(strncmp(p, search_str, len) == 0) {
            return p - str;
        }

        p--;
    }

    return default_value;
}

uniq int char*::rindex_regex(const char* self, const char* reg, int default_value, bool ignore_case=false)
{
    if(self == null || reg == null) {
        return default_value;
    }
    
    re_t re = re_compile(reg);
    
    if(re == NULL) {
        return default_value;
    }
    
    int result = default_value;
    
    int offset = 0;

    int n = 0;
    
    string self2 = self.reverse();

    int result = default_value;

    while(true) {
        int matchlength = 0;
        int max_captures = 8;
        re_capture captures[max_captures];
        int regex_result = re_matchp_ex(re, self2, &matchlength, captures, max_captures, ignore_case);

        /// match and no group strings ///
        if(regex_result >= 0) 
        {
            result = strlen(self) -matchlength;
            break;
        }
        /// no match ///
        {
            break;
        }
    }

    return result;
}

uniq string char*::strip(const char* self)
{
    if(self == null) {
        return string("");
    }
    string result = string(self);
    
    int len = strlen(self);
    
    if(self[len-1] == '\n') {
        result[len-1] = '\0';
    }
    else if(self[len-1] == '\r') {
        result[len-1] = '\0';
    }
    else if(len > 2 && self[len-2] == '\r' && self[len-1] == '\n') {
        result[len-2] = '\0';
    }
    
    return result;
}


uniq string string::chomp(const char* str)
{
    if(str == null) {
        return string("");
    }
    string result = string(str);
    
    if(result[result.length()-1] == '\n') {
        return result.substring(0, -2);
    }
    
    return result;
}

#ifdef UNIX
    uniq string xrealpath(const char* path)
    {
        if(path == null) {
            return string("");
        }
        char* result = realpath(path, null);
    
        string result2 = string(result);
    
        free(result);
    
        return result2;
    }
#endif

uniq string char*::replace(char* self, int index, char c)
{
    if(self == null) {
        return string("");
    }
    
    int len = strlen(self);

    if(strcmp(self, "") == 0) {
        return string(self);
    }
    
    if(index < 0) {
       index += len;
    }

    if(index >= len) {
        index = len-1;
    }

    if(index < 0) {
        index = 0;
    }
    
    self[index] = c;
    
    return string(self);
}

uniq string char*::multiply(char* str, int n)
{
    if(str == null) {
        return string("");
    }
    
    int len = strlen(str) * n + 1;

    char*% result = new char[len];

    result[0] = '\0';

    for(int i=0; i<n; i++) {
        strncat(result, str, len);
    }

    return result;
}

uniq list<string>*% char*::split_str(const char* self, const char* str) 
{
    if(self == null || str == null) {
        return new list<string>();
    }
    
    var result = new list<string>.initialize();

    var buf = new buffer.initialize();

    int self_len = self.length();
    int needle_len = strlen(str);
    for(int i=0; i<self_len; i++) {
        if(strstr(self + i, str) == self + i) {
            result.push_back(string(buf.buf));
            buf.reset();
            i += needle_len-1;
        }
        else {
            buf.append_char(self[i]);
        }
    }
    if(buf.length() != 0) {
        result.push_back(string(buf.buf));
    }

    return result;
}

uniq int string::rindex(char* str, const char* search_str, int default_value=-1) 
{
    return char*::rindex(str, search_str, default_value);
}

uniq int string::rindex_regex(char* self, const char* reg, int default_value=-1, bool ignore_case=false)
{
    return char*::rindex_regex(self, reg, default_value, ignore_case);
}

uniq string string::strip(const char* self)
{
    return char*::strip(self);
}


uniq int string::index_regex(char* self, const char* reg, int default_value=-1, bool ignore_case=false)
{
    return char*::index_regex(self, reg, default_value, ignore_case);
}

uniq string string::replace(char* self, int index, char c)
{
    return char*::replace(self, index, c);
}

uniq string string::multiply(char* str, int n)
{
    return char*::multiply(str, n);
}

uniq bool char*::match(char* self, const char* reg, bool ignore_case=false)
{
    if(self == null || reg == null) {
        return false;
    }
    
    re_t re = re_compile(reg);
    
    if(re == NULL) {
        return false;
    }
    
    int offset = 0;

    int n = 0;
    
    int matchlength = 0;
    int max_captures = 8;
    re_capture captures[max_captures];
    int regex_result = re_matchp_ex(re, self, &matchlength, captures, max_captures, ignore_case);

    /// match and no group strings ///
    if(regex_result >= 0)
    {
        return true;
    }
    /// no match ///
    else
    {
        return false;
    }
}

uniq list<string>*% char*::scan(const char* self, const char* reg, bool ignore_case=false)
{
    if(self == null || reg == null) {
        return new list<string>();
    }
    var result = new list<string>();
    
    re_t re = re_compile(reg);
    
    if(re == NULL) {
        return new list<string>();
    }
    
    int offset = 0;

    int n = 0;
    
    int group_count = re_get_group_count(re);

    while(true) {
        int matchlength = 0;
        int max_captures = 8;
        re_capture captures[max_captures];
        int regex_result = re_matchp_ex(re, self + offset, &matchlength, captures, max_captures, ignore_case);

        /// match and no group strings ///
        if(regex_result >= 0 && group_count == 0)
        {
            string str = self.substring(offset + regex_result, offset + regex_result + matchlength);

            result.add(str);
            
            if(matchlength == 0) {
                offset++;
            }
            else {
                offset = offset + regex_result + matchlength;
            }
        }
        /// group strings ///
        else if(regex_result >= 0 && group_count > 0) {
            int capture_count = group_count;
            if(capture_count > max_captures) {
                capture_count = max_captures;
            }
            for(int i=0; i<capture_count; i++) {
                re_capture* cp = &captures[i];
                if(cp->start < 0 || cp->length < 0) {
                    result.push_back(string(""));
                    continue;
                }
                var match_string = (self + offset).substring(cp->start, cp->start + cp->length);
                result.push_back(match_string);
            }
            
            if(matchlength == 0) {
                offset++;
            }
            else {
                offset = offset + regex_result + matchlength;
            }
        }
        /// no match ///
        else {
            break;
        }
    }

    return result;
}

uniq list<string>*% char*::split(const char* self, const char* reg, bool ignore_case=false)
{
    if(self == null || reg == null) {
        return new list<string>();
    }
    
    var result = new list<string>();
    
    re_t re = re_compile(reg);
    
    if(re == NULL) {
        return new list<string>();
    }
    
    int offset = 0;

    int n = 0;
    
    int group_count = re_get_group_count(re);

    while(true) {
        int matchlength = 0;
        int max_captures = 8;
        re_capture captures[max_captures];
        int regex_result = re_matchp_ex(re, self + offset, &matchlength, captures, max_captures, ignore_case);

        /// match and no group strings ///
        if(regex_result >= 0 && group_count == 0)
        {
            string str = self.substring(offset, offset + regex_result);

            result.add(str);
            
            if(matchlength == 0) {
                offset++;
            }
            else {
                offset = offset + regex_result + matchlength;
            }
        }
        /// no match ///
        else
        {
            break;
        }
    }

    if(offset < self.length()) {
        string str = self.substring(offset, -1);
        result.push_back(str);
    }

    return result;
}

uniq string string::sub(char* self, const char* reg, const char* replace, bool ignore_case=false)
{
    return char*::sub(self, reg, replace, true, ignore_case);
}

uniq list<string>*% string::split_str(char* self, const char* str)
{
    return char*::split_str(self, str);
}

uniq list<string>*% string::scan(char* self, const char* reg, bool ignore_case=false)
{
    return char*::scan(self, reg, ignore_case);
}

uniq list<string>*% string::split(char* self, const char* reg, bool ignore_case=false)
{
    return char*::split(self, reg, ignore_case);
}

uniq bool string::match(char* self, const char* reg, bool ignore_case=false)
{
    return char*::match(self, reg, ignore_case);
}

uniq string char*::sub(char* self, const char* reg, const char* replace, bool global=true, bool ignore_case=false)
{
    if(self == null || reg == null) {
        return string("");
    }
    
    re_t re = re_compile(reg);
    
    if(re == NULL) {
        return string("");
    }
    
    int offset = 0;

    int n = 0;
    
    var result = new buffer.initialize();
    
    int group_count = re_get_group_count(re);

    while(true) {
        int matchlength = 0;
        int max_captures = 8;
        re_capture captures[max_captures];
        int regex_result = re_matchp_ex(re, self + offset, &matchlength, captures, max_captures, ignore_case);

        /// match and no group strings ///
        if(regex_result >= 0 && group_count == 0)
        {
            string str = self.substring(offset, offset + regex_result);

            result.append_str(str);
            result.append_str(replace);
            
            if(matchlength == 0) {
                offset++;
            }
            else {
                offset = offset + regex_result + matchlength;
            }
            
            if(!global) {
                string str = self.substring(offset, -1);
                result.append_str(str);
                break;
            }
        }
        /// no match ///
        else {
            string str = self.substring(offset, -1);
            result.append_str(str);
            break;
        }
    }

    return result.to_string();
}

uniq string char*::sub_block(char* self, const char* reg, bool global=true, bool ignore_case=false, void* parent, string (*block)(void* parent, char* match_string, list<string>* group_strings))
{
    if(self == null || reg == null) {
        return string("");
    }
    
    var result = new buffer();
    
    re_t re = re_compile(reg);
    
    if(re == NULL) {
        return string("");
    }
    
    int offset = 0;

    int n = 0;
    
    int group_count = re_get_group_count(re);

    while(true) {
        int matchlength = 0;
        int max_captures = 8;
        re_capture captures[max_captures];
        int regex_result = re_matchp_ex(re, self + offset, &matchlength, captures, max_captures, ignore_case);

        /// match and no group strings ///
        if(regex_result >= 0 && group_count == 0)
        {
            string str = self.substring(offset, offset + regex_result);

            result.append_str(str);
            
            list<string>*% group_strings = new list<string>.initialize();
            
            string match_string = self.substring(offset + regex_result, offset + regex_result + matchlength);
            
            string block_result = block(parent, match_string, group_strings);
            
            result.append_str(block_result);
            
            if(matchlength == 0) {
                offset++;
            }
            else {
                offset = offset + regex_result + matchlength;
            }
            
            if(!global) {
                string str = self.substring(offset, -1);
                result.append_str(str);
                break;
            }
        }

        /// group strings ///
        else if(regex_result >= 0 && group_count > 0) {
            string str = self.substring(offset, offset + regex_result);

            result.append_str(str);

            list<string>*% group_strings = new list<string>.initialize();

            int capture_count = group_count;
            if(capture_count > max_captures) {
                capture_count = max_captures;
            }
            for(int i=0; i<capture_count; i++) {
                re_capture* cp = &captures[i];
                if(cp->start < 0 || cp->length < 0) {
                    group_strings.push_back(string(""));
                    continue;
                }
                var match_string = (self + offset).substring(cp->start, cp->start + cp->length);
                group_strings.push_back(match_string);
            }
            
            string match_string = self.substring(offset + regex_result, offset + regex_result + matchlength);
            
            string block_result = block(parent, match_string, group_strings);
            
            result.append_str(block_result);
            
            if(matchlength == 0) {
                offset++;
            }
            else {
                offset = offset + regex_result + matchlength;
            }
        }
        /// no match ///
        else {
            string str = self.substring(offset, -1);
            result.append_str(str);
            break;
        }
    }
    return result.to_string();
}

uniq list<string>*% char*::scan_block(const char* self, const char* reg, bool ignore_case=false, void* parent, string (*block)(void* parent, char* match_string, list<string>* group_strings))
{
    if(self == null || reg == null) {
        return new list<string>();
    }
    var result = new list<string>();
    
    re_t re = re_compile(reg);
    
    if(re == NULL) {
        return new list<string>();
    }
    
    int offset = 0;

    int n = 0;
    
    int group_count = re_get_group_count(re);

    while(true) {
        int matchlength = 0;
        int max_captures = 8;
        re_capture captures[max_captures];
        int regex_result = re_matchp_ex(re, self + offset, &matchlength, captures, max_captures, ignore_case);

        /// match and no group strings ///
        if(regex_result >= 0 && group_count == 0)
        {
            list<string>*% group_strings = new list<string>.initialize();
            
            string match_string = self.substring(offset + regex_result, offset + regex_result + matchlength);
            
            string block_result = block(parent, match_string, group_strings);
            
            result.add(block_result);
            
            if(matchlength == 0) {
                offset++;
            }
            else {
                offset = offset + regex_result + matchlength;
            }
        }
        /// group strings ///
        else if(regex_result >= 0 && group_count > 0) {
            list<string>*% group_strings = new list<string>.initialize();

            int capture_count = group_count;
            if(capture_count > max_captures) {
                capture_count = max_captures;
            }
            for(int i=0; i<capture_count; i++) {
                re_capture* cp = &captures[i];
                if(cp->start < 0 || cp->length < 0) {
                    group_strings.push_back(string(""));
                    continue;
                }
                var match_string = (self + offset).substring(cp->start, cp->start + cp->length);
                group_strings.push_back(match_string);
            }
            
            string match_string = self.substring(offset + regex_result, offset + regex_result + matchlength);
            
            string block_result = block(parent, match_string, group_strings);
            
            result.add(block_result);
            
            if(matchlength == 0) {
                offset++;
            }
            else {
                offset = offset + regex_result + matchlength;
            }
        }
        /// no match ///
        else {
            break;
        }
    }

    return result;
}

uniq string string::sub_block(char* self, const char* reg, bool global=true, bool ignore_case=false, void* parent, string (*block)(void* parent, char* match_string, list<string>* group_strings))
{
    return char*::sub_block(self, reg, global, ignore_case, parent, block);
}

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

#ifdef UNIX
    #include <wchar.h>
    #include <libgen.h>
    
    typedef wchar_t*% wstring;
    
    uniq wstring __builtin_wstring(const char* str, char* sname=__caller_sname__, int sline=__caller_line__)
    {
        if(str == null) {
            return null;
        }
        int len = strlen(str);
    
        wchar_t* wstr = come_calloc(1, sizeof(wchar_t)*(len+1), sname, sline, 0, "wstring");
        //new wchar_t[len+1];
    
        int ret = mbstowcs(wstr, str, len+1);
        wstr[ret] = '\0';
    
        if(ret < 0) {
            wstr[0] = 0;
        }
    
        return dummy_heap wstr;
    }
    
    uniq int wchar_t*::length(const wchar_t* str)
    {
        if(str == null) {
            return 0;
        }
        return wcslen(str);
    }
    
    uniq int wchar_t[]::length(const wchar_t* str)
    {
        if(str == null) {
            return 0;
        }
        return wcslen(str);
    }
    
    uniq int wstring::length(const wchar_t* str)
    {
        return wchar_t*::length(str);
    }
    
    uniq string string::lower_case(char* str)
    {
        if(str == null) {
            return string("");
        }
        string result = string(str);
        int len = strlen(str);
        for(int i=0; i<len; i++) {
            if(str[i] >= 'A' && str[i] <= 'Z') {
                result[i] = str[i] - 'A' + 'a';
            }
        }
        
        return result;
    }
    
    uniq string string::upper_case(char* str)
    {
        if(str == null) {
            return string("");
        }
        string result = string(str);
        int len = strlen(str);
        for(int i=0; i<len; i++) {
            if(str[i] >= 'a' && str[i] <= 'z') {
                result[i] = str[i] - 'a' + 'A';
            }
        }
        
        return result;
    }
    
    uniq wstring wchar_t*::substring(const wchar_t* str, int head, int tail)
    {
        if(str == null) {
            return wstring("");
        }
    
        int len = wcslen(str);
    
        if(head < 0) {
            head += len;
        }
        if(tail < 0) {
            tail += len + 1;
        }
    
        if(head > tail) {
            return wstring("");
        }
    
        if(head < 0) {
            head = 0;
        }
    
        if(tail >= len) {
            tail = len;
        }
        
        if(head >= len) {
            return wstring("");
        }
    
        if(head == tail) {
            return wstring("");
        }
    
        if(tail-head+1 < 1) {
            return wstring("");
        }
    
        wstring result = new wchar_t[tail-head+1];
    
        memcpy(result, str + head, sizeof(wchar_t)*(tail-head));
        result[tail-head] = '\0';
    
        return result;
    }
    
    uniq int char*::index_count(const char* str, const char* search_str, int count, int default_value)
    {
        if(str == null || search_str == null) {
            return default_value;
        }
        
        int n = 0;
        int len = strlen(str);
        int len2 = strlen(search_str);
        for(int i=0; i<len; i++) {
            int j;
            for(j=0; j<len2; j++) {
                if(str[i+j] != search_str[j]) {
                    break;
                }
            }
            
            if(j == len2) {
                n++;
                
                if(n == count) {
                    return i;
                }
            }
        }
        
        return default_value;
    }
    
    
    uniq int char*::rindex(const char* str, const char* search_str, int default_value)
    {
        if(str == null || search_str == null) {
            return default_value;
        }
        int len = strlen(search_str);
        int str_len = strlen(str);
        if(len == 0) {
            return str_len;
        }
        if(len > str_len) {
            return default_value;
        }
        char* p = (char*)(str + str_len - len);
    
        while(p >= str) {
            if(strncmp(p, search_str, len) == 0) {
                return p - str;
            }
    
            p--;
        }
    
        return default_value;
    }
    
    
    uniq int char*::rindex_count(const char* str, const char* search_str, int count, int default_value)
    {
        if(str == null || search_str == null) {
            return default_value;
        }
        int len = strlen(search_str);
        int str_len = strlen(str);
        if(len == 0) {
            return count <= 1 ? str_len : default_value;
        }
        if(len > str_len) {
            return default_value;
        }
        const char* p = (char*)str + str_len - len;
        
        int n = 0;
    
        while(p >= str) {
            if(strncmp(p, search_str, len) == 0) {
                n++;
                if(n == count) {
                    return p - str;
                }
            }
    
            p--;
        }
    
        return default_value;
    }
    
    uniq string char*::strip(const char* self)
    {
        if(self == null) {
            return string("");
        }
        
        string result = string(self);
        
        int len = strlen(self);
        
        if(self[len-1] == '\n') {
            result[len-1] = '\0';
        }
        else if(self[len-1] == '\r') {
            result[len-1] = '\0';
        }
        else if(len > 2 && self[len-2] == '\r' && self[len-1] == '\n') {
            result[len-2] = '\0';
        }
        
        return result;
    }
    
    uniq string wchar_t*::to_string(const wchar_t* wstr)
    {
        if(wstr == null) {
            return string("");
        }
        
        int len = MB_LEN_MAX*(wcslen(wstr)+1);
    
        string result = new char[len];
    
        if(wcstombs(result, wstr, len) < 0) 
        {
            strncpy(result, "", len);
        }
    
        return result;
    }
    
    uniq string wchar_t[]::to_string(const wchar_t* wstr)
    {
        if(wstr == null) {
            return string("");
        }
        return wchar_t*::to_string(wstr);
    }
    
    uniq wstring char*::to_wstring(const char* str)
    {
        if(str == null) {
            return wstring("");
        }
        return wstring(str);
    }
    
    uniq wstring char[]::to_wstring(char* str)
    {
        if(str == null) {
            return wstring("");
        }
        return wstring(str);
    }
    
uniq wstring wchar_t*::delete(wchar_t* str, int head, int tail) 
    {
        if(str == null) {
            return wstring("");
        }
        int len = wcslen(str);
    
        if(len == 0) {
            return str.to_string().to_wstring();
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
            return str.to_string().to_wstring();
        }
    
        if(tail >= len) {
            tail = len;
        }
        
        wstring sub_str = str.substring(tail, -1);
    
        memcpy(str + head, sub_str, sizeof(wchar_t)*(sub_str.length()+1));
    
        return str.to_string().to_wstring();
    }
    
    uniq int wchar_t*::index(const wchar_t* str, const wchar_t* search_str, int default_value)
    {
        if(str == null || search_str == null) {
            return default_value;
        }
        
        wchar_t* head = wcsstr(str, search_str);
    
        if(head == null) {
            return default_value;
        }
    
        return head - str;
    }
    
    uniq int wchar_t*::rindex(const wchar_t* str, const wchar_t* search_str, int default_value)
    {
        if(str == null || search_str == null) {
            return default_value;
        }
        
        int len = wcslen(search_str);
    
        wchar_t* p = (wchar_t*)str + wcslen(str) - len;
    
        while(p >= str) {
            int len2 = wcslen(p);
            bool result = true;
            int i;
            for(i=0; i<len && i < len2; i++) {
                if(p[i] != search_str[i]) {
                    result = false;
                }
            }
            if(result) {
                return (p - str);
            }
    
            p--;
        }
    
        return default_value;
    }
    
    uniq wstring wchar_t*::reverse(const wchar_t* str) 
    {
        if(str == null) {
            return wstring("");
        }
        
        int len = wcslen(str);
        wstring result = new wchar_t[len + 1];
    
        for(int i=0; i<len; i++) {
            result[i] = str[len-i-1];
        }
    
        result[len] = '\0';
    
        return result;
    }
    
    uniq wstring wchar_t*::multiply(const wchar_t* str, int n)
    {
        if(str == null) {
            return wstring("");
        }
        
        int len = wcslen(str) * n + 1;
    
        wstring result = new wchar_t[len];
    
        result[0] = '\0';
    
        for(int i=0; i<n; i++) {
            wcscat(result, str);
        }
    
        return result;
    }
    
    uniq wstring wchar_t*::printable(const wchar_t* str)
    {
        if(str == null) {
            return wstring("");
        }
        int len = str.length();
        wstring result = new wchar_t[len*2+1];
    
        int n = 0;
        for(int i=0; i<len; i++) {
            wchar_t c = str[i];
    
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
    
    
    uniq int wchar_t*::compare(const wchar_t* left, wchar_t* right)
    {
        if(left == null) {
            if(right == null) {
                return 0;
            }
            else {
                return 1;
            }
        }
        else if(right == null) {
            if(left == null) {
                return 0;
            }
            else {
                return -1;
            }
        }
        return wcscmp(left, right);
    }
    
    uniq int wstring::compare(const wchar_t* left, const wchar_t* right)
    {
        if(left == null) {
            if(right == null) {
                return 0;
            }
            else {
                return 1;
            }
        }
        else if(right == null) {
            if(left == null) {
                return 0;
            }
            else {
                return -1;
            }
        }
        return wcscmp(left, right);
    }
    
    
    uniq bool wchar_t*::equals(const wchar_t left, wchar_t right)
    {
        return left == right;
    }
    
    
    uniq wstring wchar_t*::operator_mult(const wchar_t* str, int n)
    {
        return wchar_t*::multiply(str, n);
    }
    
    uniq wstring wstring::operator_mult(const wchar_t* str, int n)
    {
        return wchar_t*::multiply(str, n);
    }
    
    uniq bool wchar_t*::operator_equals(const wchar_t* left, const wchar_t* right)
    {
        return wcscmp(left, right) == 0;
    }
    
    uniq bool wstring::operator_equals(const wchar_t* left, const wchar_t* right)
    {
        return wcscmp(left, right) == 0;
    }
    
    uniq bool wchar_t*::operator_not_equals(const wchar_t* left, const wchar_t* right)
    {
        return wcscmp(left, right) != 0;
    }
    
    uniq bool wstring::operator_not_equals(const wchar_t* left, const wchar_t* right)
    {
        return wcscmp(left, right) != 0;
    }
    
    
    uniq wstring wchar_t*::operator_add(const wchar_t* left, const wchar_t* right)
    {
        if(left == null || right == null) {
            return wstring("");
        }
        wchar_t*% result = new wchar_t[wcslen(left) + wcslen(right) + 1];
        
        wcscpy(result, left);
        wcscat(result, right);
        
        return result;
    }
    
    uniq wstring wstring::operator_add(const wchar_t* left, const wchar_t* right)
    {
        if(left == null || right == null) {
            return wstring("");
        }
        wchar_t*% result = new wchar_t[wcslen(left) + wcslen(right) + 1];
        
        wcscpy(result, left);
        wcscat(result, right);
        
        return result;
    }
    
    
    uniq string char*::replace(char* self, int index, char c)
    {
        if(self == null) {
            return string("");
        }
        int len = strlen(self);
    
        if(strcmp(self, "") == 0) {
            return string(self);
        }
        
        if(index < 0) {
           index += len;
        }
    
        if(index >= len) {
            index = len-1;
        }
    
        if(index < 0) {
            index = 0;
        }
        
        self[index] = c;
        
        return string(self);
    }
    
    uniq string char*::multiply(const char* str, int n)
    {
        if(str == null) {
            return string("");
        }
        int len = strlen(str) * n + 1;
    
        char*% result = new char[len];
    
        result[0] = '\0';
    
        for(int i=0; i<n; i++) {
            strcat(result, str);
        }
    
        return result;
    }
    
    uniq list<string>*% char*::split_str(const char* self, const char* str) 
    {
        if(self == null || str == null) {
            return new list<string>();
        }
        var result = new list<string>.initialize();
    
        var buf = new buffer.initialize();
    
        int self_len = self.length();
        int needle_len = strlen(str);
        for(int i=0; i<self_len; i++) {
            if(strstr(self + i, str) == self + i) {
                result.push_back(string(buf.buf));
                buf.reset();
                i += needle_len-1;
            }
            else {
                buf.append_char(self[i]);
            }
        }
        if(buf.length() != 0) {
            result.push_back(string(buf.buf));
        }
    
        return result;
    }
    
    uniq unsigned int wchar_t*::get_hash_key(const wchar_t* value)
    {
        if(value == null) {
            return 0;
        }
        int result = 0;
        wchar_t* p = (wchar_t*)value;
        while(*p) {
            result += (*p);
            p++;
        }
        return result;
    }
    
    uniq bool wchar_t*::equals(const wchar_t* left, const wchar_t* right)
    {
        if(left == null && right == null) {
            return true;
        }
        else if(left == null || right == null) {
            return false;
        }
        return wcscmp(left, right) == 0;
    }
    
    uniq bool wstring::equals(const wchar_t* left, const wchar_t* right)
    {
        if(left == null && right == null) {
            return true;
        }
        else if(left == null || right == null) {
            return false;
        }
        return wcscmp(left, right) == 0;
    }
    
    
    uniq bool wchar_t::operator_equals(wchar_t left, wchar_t right)
    {
        return left == right;
    }
    
    uniq bool wchar_t::operator_not_equals(wchar_t left, wchar_t right)
    {
        return left != right;
    }
    
    uniq unsigned int wchar_t::get_hash_key(wchar_t value)
    {
        return value;
    }
    
    uniq bool wchar_t::equals(wchar_t left, wchar_t right)
    {
        return left == right;
    }
    
    uniq string wchar_t::to_string(wchar_t wc)
    {
        return xsprintf("%ls", wc);
    }
    
    uniq string xrealpath(const char* path)
    {
        if(path == null) {
            return string("");
        }
        char* result = realpath(path, null);
    
        string result2 = string(result);
    
        free(result);
    
        return result2;
    }
    
    uniq string xdirname(const char* path)
    {
        if(path == null) {
            return string("");
        }
        return string(dirname(string(path)));
    }
    
    uniq size_t xwcslen(const wchar_t* wstr)
    {
        if(wstr == null) {
            return 0;
        }
        wchar_t* p = (wchar_t*)wstr;
        
        size_t len = 0;
        while(*p) {
            p++;
            len++;
        }
        
        return len;
    }
    
    uniq wstring wstring::substring(const wchar_t* str, int head, int tail) 
    {
        return wchar_t*::substring(str, head, tail);
    }
    
    uniq int string::index_count(const char* str, const char* search_str, int count=1, int default_value=-1)
    {
        return char*::index_count(str, search_str, count, default_value);
    }
    
    
    uniq int string::rindex(const char* str, const char* search_str, int default_value=-1) 
    {
        return char*::rindex(str, search_str, default_value);
    }
    
    uniq int string::rindex_count(const char* str, const char* search_str, int count=1, int default_value=-1)
    {
        return char*::rindex_count(str, search_str, count, default_value);
    }
    
    uniq string string::strip(const char* self)
    {
        return char*::strip(self);
    }
    
    uniq wstring string::to_wstring(const char* str)
    {
        return char*::to_wstring(str);
    }
    
    uniq string wstring::to_string(const wchar_t* wstr)
    {
        return wchar_t*::to_string(wstr);
    }
    
    uniq wstring int::to_wstring(int self)
    {
        return xsprintf("%d", self).to_wstring();
    }
    
uniq wstring wstring::delete(wchar_t* str, int head, int tail)
    {
        return wchar_t*::delete(str, head, tail);
    }
    
    uniq int wstring::index(const wchar_t* str, const wchar_t* search_str, int default_value=1)
    {
        return wchar_t*::index(str, search_str, default_value);
    }
    
    uniq int wstring::rindex(const wchar_t* str, const wchar_t* search_str, int default_value=-1)
    {
        return wchar_t*::rindex(str, search_str, default_value);
    }
    
    uniq wstring wstring::reverse(const wchar_t* str)
    {
        return wchar_t*::reverse(str);
    }
    
    uniq wstring wstring::multiply(const wchar_t* str, int n)
    {
        return wchar_t*::multiply(str, n);
    }
    
    uniq wstring wstring::printable(const wchar_t* str)
    {
        return wchar_t*::printable(str);
    }
    
    uniq unsigned int wstring::get_hash_key(const wchar_t* value)
    {
        return wchar_t*::get_hash_key(value);
    }
    
    
    
    uniq string string::replace(char* self, int index, char c)
    {
        return char*::replace(self, index, c);
    }
    
    uniq string string::multiply(const char* str, int n)
    {
        return char*::multiply(str, n);
    }
    
    uniq list<string>*% string::split_str(const char* self, const char* str)
    {
        return char*::split_str(self, str);
    }
    
    uniq wstring string::to_wstring(const char* str)
    {
        return char*::to_wstring(str);
    }
    
    uniq string char*::chomp(const char* str)
    {
        return string::chomp(str);
    }
    
    uniq bool wchar_t*::equals(const wchar_t* left, const wchar_t* right)
    {
        return wcscmp(left, right) == 0;
    }
    
    uniq bool wchar_t*::operator_equals(const wchar_t* left, const wchar_t* right)
    {
        return wcscmp(left, right) == 0;
    }
    
    uniq bool wchar_t*::operator_not_equals(const wchar_t* left, const wchar_t* right)
    {
        return wcscmp(left, right) != 0;
    }
#endif

#endif
