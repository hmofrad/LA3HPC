/*
 * cc.h: Connected component benchmark helper
 * (c) Mohammad Mofrad, 2018
 * (e) m.hasanzadeh.mofrad@gmail.com 
 */

#ifndef CC_H
#define CC_H

#include "vp/vertex_program.hpp"

#define INF 2147483647

/* HAS_WEIGHT macro will be defined by compiler. 
   make MACROS=-DHAS_WEIGHT */ 
using em = Empty; // Weight (default is Empty)
#ifdef HAS_WEIGHT
using wp = uint32_t;
#else
using wp = em;
#endif

/*  Integer precision controls the number of vertices
    the engine can possibly process. */
using ip = uint32_t;

/* Fractional precision controls the precision of values.
   E.g. vertex rank in PageRank. */
using fp = uint32_t;

struct CC_State
{
    ip label = 0;
    ip get_state(){return(label);};
    ip get_inf(){return(INF);};    
    std::string print_state(){return("Label=" + std::to_string(label));};
};


template<typename Weight, typename Integer_Type, typename Fractional_Type>
class CC_Program : public Vertex_Program<Weight, Integer_Type, Fractional_Type, CC_State>
{
    public:  
        using Vertex_Program<Weight, Integer_Type, Fractional_Type, CC_State>::Vertex_Program;
        virtual bool initializer(Integer_Type vid, CC_State &state)
        {
            state.label = vid;
            return(true);
        }
        
        virtual Fractional_Type messenger(CC_State &state) 
        {
            return(state.label);
        }

        virtual void combiner(Fractional_Type &y1, const Fractional_Type &y2) 
        {
            y1 = (y1 < y2) ? y1 : y2;
        }

        virtual bool applicator(CC_State &state, const Fractional_Type &y) 
        {
            Fractional_Type tmp = state.label;
            state.label = (y < state.label) ? y : state.label;
            return(tmp != state.label);
        }   
        
        virtual Fractional_Type infinity()
        {
            return(INF);
        }
};
#endif