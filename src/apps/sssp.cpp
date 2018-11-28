/*
 * sssp.cpp: Single Source Shortest Path (SSSP) benchmark
 * (c) Mohammad Mofrad, 2018
 * (e) m.hasanzadeh.mofrad@gmail.com 
 */

#include <iostream>
#include <unistd.h>
 
#include "mpi/env.hpp"
#include "mat/graph.hpp"

#include "sssp.h"

int main(int argc, char **argv)
{ 
    bool comm_split = true;
    Env::init(comm_split);    
    double time1 = Env::clock();   
    
    if(argc > 4)
    {
        if(Env::is_master)
            std::cout << "\"Usage: " << argv[0] << " <file_path> <num_vertices> <root>\"" << std::endl; 
        Env::exit(1);
    }
      
    std::string file_path = argv[1]; 
    ip num_vertices = std::atoi(argv[2]);
    ip root = (argc > 3) ? std::atoi(argv[3]) : 0;
    bool directed = true;
    bool transpose = false;
    bool self_loops = false;
    bool acyclic = false;
    bool parallel_edges = false;
    Tiling_type TT = _2D_;
    Compression_type CT = _CSC_; // Only CSC is supported
    Filtering_type FT = _SOME_;
    Hashing_type HT = NONE;
    bool parread = true;
    
    /* Single Source Shortest Path (SSSP) execution*/
    bool stationary = false;
    // Requirement for nonstationary algorithms on directed graphs
    if(not stationary and directed)
        transpose = not transpose; 
    Graph<wp, ip, fp> G;    
    G.load(file_path, num_vertices, num_vertices, directed, transpose, self_loops, acyclic, parallel_edges, TT, CT, FT, HT, parread);
    bool activity_filtering = true;
    bool tc_family  = false;
    bool gather_depends_on_apply = true;
    bool apply_depends_on_iter  = false;
    Ordering_type OT = _ROW_;
    SSSP_Program<wp, ip, fp> V(G, stationary, activity_filtering, gather_depends_on_apply, apply_depends_on_iter, tc_family, OT);   
    V.root = root;
    V.execute();
    V.checksum();
    V.display();
    V.free();
    G.free();    

    double time2 = Env::clock();
    Env::print_time("Single Source Shortest Path (SSSP) end-to-end", time2 - time1);
    Env::finalize();
    return(0);
}