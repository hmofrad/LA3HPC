/*
 * ds.hpp
 * (c) Mohammad Mofrad, 2018
 * (e) m.hasanzadeh.mofrad@gmail.com 
 */

#include <iostream>
#include <stdint.h>
#include <sys/mman.h>
#include <cstring>


template <typename Weight>
struct Triple
{
    uint32_t row;
    uint32_t col;
    Weight weight;
    Triple(uint32_t row = 0, uint32_t col = 0, Weight weight = 0);
    ~Triple();
    void set_weight(Weight &w);
    Weight get_weight();
};

template <typename Weight>
Triple<Weight>::Triple(uint32_t row, uint32_t col, Weight weight)
      : row(row), col(col), weight(weight) {};

template <typename Weight>
Triple<Weight>::~Triple() {};
      
template <typename Weight>
void Triple<Weight>::set_weight(Weight &w)
{
    Triple<Weight>::weight = w;
}

template <typename Weight>
Weight Triple<Weight>::get_weight()
{
    return(Triple<Weight>::weight);
}

struct Empty {};

template <>
struct Triple <Empty>
{
    uint32_t row;
    union {
        uint32_t col;
        Empty weight;
    };
    void set_weight(Empty& w) {};
    bool get_weight() {return(1);};
};

template <typename Weight>
struct Functor
{
    bool operator()(const struct Triple<Weight>& a, const struct Triple<Weight>& b)
    {
        return((a.row == b.row) ? (a.col < b.col) : (a.row < b.row));
    }
};

template <typename Type = void>
struct basic_storage
{
    Type *data;
    uint64_t n;
    uint64_t nbytes;
    void allocate(uint64_t n);
    void free();
};

template <typename Type>
void basic_storage<Type>::allocate(uint64_t n)
{
    basic_storage<Type>::nbytes = n;
    
    if((this->data = (Type *) mmap(nullptr, basic_storage<Type>::nbytes, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == (void*) -1)
    {    
        fprintf(stderr, "Error mapping memory\n");
        std::exit(1);
    }
    memset(basic_storage<Type>::data, 0, basic_storage<Type>::nbytes);
}

template <typename Type>
void basic_storage<Type>::free()
{
    if(munmap(basic_storage<Type>::data, basic_storage<Type>::nbytes) == -1)
    {
        fprintf(stderr, "Error unmapping memory\n");
        std::exit(1);
    }
}

template <typename Type>
struct BV : basic_storage<Type> {};



