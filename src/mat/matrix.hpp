/*
 * matrix.hpp: Matrix implementation
 * (c) Mohammad Hasanzadeh Mofrad, 2019
 * (e) m.hasanzadeh.mofrad@gmail.com 
 */

#ifndef MATRIX_HPP
#define MATRIX_HPP
 
#include <cmath>
#include <algorithm>
#include <vector>
#include "mpi/types.hpp" 
#include "mat/tiling.hpp" 
#include "ds/indexed_sort.hpp"

enum Filtering_type
{
  _ROWS_,
  _COLS_,
  _NONE_,
  _SOME_
}; 

template<typename Weight, typename Integer_Type, typename Fractional_Type>
struct Tile2D { 
    template <typename Weight_, typename Integer_Type_, typename Fractional_Type_>
    friend class Matrix;
    std::vector<struct Triple<Weight, Integer_Type>>* triples;
    struct Compressed_column<Weight, Integer_Type>* compressor = nullptr;
    uint32_t rg, cg; // Row group, Column group
    uint32_t ith, jth, nth; // ith row, jth column, nth local row order tile,
    uint32_t mth, kth; // mth local column order tile, and kth global tile
    int32_t rank;
    int32_t leader_rank_rg, leader_rank_cg;
    int32_t rank_rg, rank_cg;
    int32_t leader_rank_rg_rg, leader_rank_cg_cg;
    uint64_t nedges;
    void allocate_triples();
    void free_triples();
};

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Tile2D<Weight, Integer_Type, Fractional_Type>::allocate_triples() {
    if (!triples)
        triples = new std::vector<struct Triple<Weight, Integer_Type>>;
}
template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Tile2D<Weight, Integer_Type, Fractional_Type>::free_triples() {
    if(triples) {
        triples->clear();
        triples->shrink_to_fit();
        delete triples;
        triples = nullptr;
    }
}
 
template<typename Weight, typename Integer_Type, typename Fractional_Type>
class Matrix {
    template<typename Weight_, typename Integer_Type_, typename Fractional_Type_>
    friend class Graph;
    template<typename Weight__, typename Integer_Type__, typename Fractional_Type__, typename Vertex_state>
    friend class Vertex_Program;
    
    public:    
        Matrix(Integer_Type nrows_, Integer_Type ncols_, uint32_t ntiles_, bool directed_, bool transpose_, bool parallel_edges_,
               Tiling_type tiling_type_, Compression_type compression_type_);
        ~Matrix();
        
    private:
        Integer_Type nrows, ncols;
        uint32_t ntiles, nrowgrps, ncolgrps;
        Integer_Type tile_height, tile_width;    
        uint64_t nedges = 0;
        Tiling* tiling;
        Compression_type compression_type;
        Filtering_type filtering_type;
        bool directed;
        bool transpose;
        bool parallel_edges;

        std::vector<std::vector<char>> I;           // Nonzero rows bitvectors (from tile width)
        std::vector<std::vector<Integer_Type>> IV;  // Nonzero rows indices    (from tile width)
        std::vector<std::vector<char>> J;           // Nonzero cols bitvectors (from tile height)
        std::vector<std::vector<Integer_Type>> JV;  // Nonzero cols indices    (from tile height)
        std::vector<Integer_Type> rowgrp_nnz_rows;  // Row group row indices         
        std::vector<Integer_Type> rowgrp_regular_rows; // Row group regular indices
        std::vector<Integer_Type> rowgrp_source_rows;  // Row group source column indices
        std::vector<Integer_Type> colgrp_nnz_columns;  // Column group column indices
        std::vector<Integer_Type> colgrp_sink_columns;    // Column group sink column indices
        std::vector<std::vector<Integer_Type>> regular_rows;// Row group regular indices
        std::vector<std::vector<char>> regular_rows_bitvector;// Row group regular bitvector
        std::vector<std::vector<Integer_Type>> source_rows;// Row group source indices
        std::vector<std::vector<char>> source_rows_bitvector;// Row group source bitvector
        std::vector<std::vector<Integer_Type>> regular_columns;// Col group regular indices
        std::vector<std::vector<char>> regular_columns_bitvector;// Col group regular bitvector
        std::vector<std::vector<Integer_Type>> sink_columns;// Col group sink indices
        std::vector<std::vector<char>> sink_columns_bitvector;// Col group sink bitvector
        
        std::vector<std::vector<struct Tile2D<Weight, Integer_Type, Fractional_Type>>> tiles;
        std::vector<std::vector<struct Tile2D<Weight, Integer_Type, Fractional_Type>>> tiles_rg;
        std::vector<std::vector<struct Tile2D<Weight, Integer_Type, Fractional_Type>>> tiles_cg;
        
        std::vector<uint32_t> local_tiles;
        std::vector<uint32_t> local_tiles_row_order;
        std::vector<uint32_t> local_tiles_col_order;
        std::vector<int32_t> local_row_segments;
        std::vector<int32_t> local_col_segments;
        
        std::vector<int32_t> leader_ranks;
        
        std::vector<int32_t> leader_ranks_rg;
        std::vector<int32_t> leader_ranks_cg;
        
        std::vector<int32_t> all_rowgrp_ranks;
        std::vector<int32_t> all_rowgrp_ranks_accu_seg;
        std::vector<int32_t> all_colgrp_ranks; 
        std::vector<int32_t> all_colgrp_ranks_accu_seg;
        
        std::vector<int32_t> follower_rowgrp_ranks;
        std::vector<int32_t> follower_rowgrp_ranks_accu_seg;
        std::vector<int32_t> follower_colgrp_ranks; 
        std::vector<int32_t> follower_colgrp_ranks_accu_seg;
        
        
        std::vector<int32_t> all_rowgrp_ranks_rg;
        std::vector<int32_t> all_rowgrp_ranks_accu_seg_rg;
        std::vector<int32_t> all_colgrp_ranks_cg;
        std::vector<int32_t> all_colgrp_ranks_accu_seg_cg;
        
        std::vector<int32_t> follower_rowgrp_ranks_rg;
        std::vector<int32_t> follower_rowgrp_ranks_accu_seg_rg;
        std::vector<int32_t> follower_colgrp_ranks_cg;
        std::vector<int32_t> follower_colgrp_ranks_accu_seg_cg;
        int32_t owned_segment, accu_segment_rg, accu_segment_cg, accu_segment_row, accu_segment_col;
        std::vector<Integer_Type> nnz_row_sizes_all;
        std::vector<Integer_Type> nnz_col_sizes_all;
        std::vector<Integer_Type> nnz_row_sizes_loc;
        std::vector<Integer_Type> nnz_col_sizes_loc;
        
        void free_tiling();
        void init_matrix();
        void del_triples();
        void init_tiles();
        void init_compression();
        //void init_csr();
        void init_csc();
        //void init_tcsr();
        void init_tcsc();
        void init_tcsc_cf();
        //void init_dcsr();
        void init_dcsc();
        //void init_bv();
        //void del_csr();
        //void del_csc();
        //void del_dcsr();
        //void construct_filter();
        //void destruct_filter();
        void del_compression();
        void del_filter();
        void del_classifier();
        //void del_filtering_indices();
        void print(std::string element);
        void distribute();
        void balance();
        void init_filtering();
        void filter_vertices(Filtering_type filtering_type_);
        void classify_vertices();
        
        
        struct Triple<Weight, Integer_Type> tile_of_local_tile(const uint32_t local_tile);
        struct Triple<Weight, Integer_Type> tile_of_triple(const struct Triple<Weight, Integer_Type>& triple);
        uint32_t local_tile_of_triple(const struct Triple<Weight, Integer_Type>& triple);
        uint32_t segment_of_tile(const struct Triple<Weight, Integer_Type>& pair);
		uint32_t owner_of_tile(const struct Triple<Weight, Integer_Type>& pair);
        uint32_t row_leader_of_tile(const struct Triple<Weight, Integer_Type>& pair);
		
        struct Triple<Weight, Integer_Type> base(const struct Triple<Weight, Integer_Type>& pair, Integer_Type rowgrp, Integer_Type colgrp);
        struct Triple<Weight, Integer_Type> rebase(const struct Triple<Weight, Integer_Type>& pair);
        void insert(const struct Triple<Weight, Integer_Type>& triple);
        void test(const struct Triple<Weight, Integer_Type>& triple);      
};

template<typename Weight, typename Integer_Type, typename Fractional_Type>
Matrix<Weight, Integer_Type, Fractional_Type>::Matrix(Integer_Type nrows_, 
    Integer_Type ncols_, uint32_t ntiles_, bool directed_, bool transpose_, bool parallel_edges_, Tiling_type tiling_type_, 
    Compression_type compression_type_) {
    nrows = nrows_;
    ncols = ncols_;
    ntiles = ntiles_;
    nrowgrps = sqrt(ntiles_);
    ncolgrps = ntiles_ / nrowgrps;
    tile_height = (nrows_ / nrowgrps) + 1;
    tile_width  = (ncols_ / ncolgrps) + 1;
    directed = directed_;
    transpose = transpose_;
    parallel_edges = parallel_edges_;
    // Initialize tiling 
    tiling = new Tiling(Env::nranks, ntiles, nrowgrps, ncolgrps, tiling_type_);
    compression_type = compression_type_;
    init_matrix();
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
Matrix<Weight, Integer_Type, Fractional_Type>::~Matrix(){};

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::free_tiling() {
    delete tiling;
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
struct Triple<Weight, Integer_Type> Matrix<Weight, Integer_Type, Fractional_Type>::tile_of_local_tile(const uint32_t local_tile) {
    return{(local_tile - (local_tile % ncolgrps)) / ncolgrps, local_tile % ncolgrps};
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
struct Triple<Weight, Integer_Type> Matrix<Weight, Integer_Type, Fractional_Type>::tile_of_triple(const struct Triple<Weight, Integer_Type>& triple) {
    return{(triple.row / tile_height), (triple.col / tile_width)};
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
uint32_t Matrix<Weight, Integer_Type, Fractional_Type>::segment_of_tile(const struct Triple<Weight, Integer_Type>& pair) {
    return(pair.col);
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
uint32_t Matrix<Weight, Integer_Type, Fractional_Type>::owner_of_tile(const struct Triple<Weight, Integer_Type>& pair) {
    return(tiles[pair.row][pair.col].rank);
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
uint32_t Matrix<Weight, Integer_Type, Fractional_Type>::row_leader_of_tile(const struct Triple<Weight, Integer_Type>& pair) {
    return(tiles[pair.row][pair.row].rank);
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
struct Triple<Weight, Integer_Type> Matrix<Weight, Integer_Type, Fractional_Type>::base(const struct Triple<Weight, Integer_Type>& pair, 
                      Integer_Type rowgrp, Integer_Type colgrp) {
   return{(pair.row + (rowgrp * tile_height)), (pair.col + (colgrp * tile_width))};
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
struct Triple<Weight, Integer_Type> Matrix<Weight, Integer_Type, Fractional_Type>::rebase(const struct Triple<Weight, Integer_Type>& pair) {
    return{(pair.row % tile_height), (pair.col % tile_width)};
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::test(const struct Triple<Weight, Integer_Type>& triple) {        
    struct Triple<Weight, Integer_Type> pair = tile_of_triple(triple);
    uint32_t t = (pair.row * tiling->ncolgrps) + pair.col;
    if(not (std::find(local_tiles.begin(), local_tiles.end(), t) != local_tiles.end())) {
        printf("Invalid[r=%d,t=%d]: Tile[%d][%d] [%d %d]\n", Env::rank, t, pair.row, pair.col, triple.row, triple.col);
        fprintf(stderr, "Invalid[r=%d,t=%d]: Tile[%d][%d] [%d %d]\n", Env::rank, t, pair.row, pair.col, triple.row, triple.col);
        Env::exit(1);
    }
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
uint32_t Matrix<Weight, Integer_Type, Fractional_Type>::local_tile_of_triple(const struct Triple<Weight, Integer_Type>& triple) {
    struct Triple<Weight, Integer_Type> pair = tile_of_triple(triple);
    uint32_t t = (pair.row * tiling->ncolgrps) + pair.col;
    return(t);
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::insert(const struct Triple<Weight, Integer_Type>& triple) {
    struct Triple<Weight, Integer_Type> pair = tile_of_triple(triple);
    tiles[pair.row][pair.col].triples->push_back(triple);
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::init_matrix() {
    // Reserve the 2D vector of tiles. 
    tiles.resize(nrowgrps);
    for (uint32_t i = 0; i < nrowgrps; i++)
        tiles[i].resize(ncolgrps);
    // Initialize tiles 
    for (uint32_t i = 0; i < nrowgrps; i++) {
        for (uint32_t j = 0; j < ncolgrps; j++) {
            auto& tile = tiles[i][j];
            tile.rg = i;
            tile.cg = j;
            if(tiling->tiling_type == Tiling_type::_2D_) {
                tile.rank = ((j % tiling->rowgrp_nranks) * tiling->colgrp_nranks) +
                                   (i % tiling->colgrp_nranks);
                tile.ith = tile.rg   / tiling->colgrp_nranks;
                tile.jth = tile.cg   / tiling->rowgrp_nranks;
                
                tile.rank_rg = j % tiling->rowgrp_nranks;
                tile.rank_cg = i % tiling->colgrp_nranks;
                
                tile.leader_rank_rg = i;
                tile.leader_rank_cg = j;
                
                tile.leader_rank_rg_rg = i;
                tile.leader_rank_cg_cg = j;
            }
            else if(tiling->tiling_type == Tiling_type::_2DT_) {
                
                tile.rank = (i % tiling->colgrp_nranks) * tiling->rowgrp_nranks
                                                        + (j % tiling->rowgrp_nranks);
                
                tile.ith = tile.rg / tiling->colgrp_nranks; 
                tile.jth = tile.cg / tiling->rowgrp_nranks;
                
                tile.rank_rg = j % tiling->rowgrp_nranks;
                tile.rank_cg = i % tiling->colgrp_nranks;
                
                tile.leader_rank_rg = i;
                tile.leader_rank_cg = j;
                
                tile.leader_rank_rg_rg = i;
                tile.leader_rank_cg_cg = j;
            }
            tile.nth   = (tile.ith * tiling->rank_ncolgrps) + tile.jth;
            tile.mth   = (tile.jth * tiling->rank_nrowgrps) + tile.ith;
            tile.allocate_triples();
            //tile.allocated = false;
        }
    }
    /*
    * Reorganize the tiles so that each rank is placed in
    * at least one diagonal tile then calculate 
    * the leader ranks per row group.
    */
    leader_ranks.resize(nrowgrps, -1);
    leader_ranks_rg.resize(nrowgrps);
    leader_ranks_cg.resize(ncolgrps);
    for (uint32_t i = 0; i < nrowgrps; i++) {
        for (uint32_t j = i; j < ncolgrps; j++) {
            if(not (std::find(leader_ranks.begin(), leader_ranks.end(), tiles[j][i].rank)
                 != leader_ranks.end())) {
                std::swap(tiles[j], tiles[i]);
                break;
            }
        }
        leader_ranks[i] = tiles[i][i].rank;
        leader_ranks_rg[i] = tiles[i][i].rank_rg;
        leader_ranks_cg[i] = tiles[i][i].rank_cg;
    }
    
    //Calculate local tiles and local column segments
    struct Triple<Weight, Integer_Type> pair;
    for (uint32_t i = 0; i < nrowgrps; i++) {
        for (uint32_t j = 0; j < ncolgrps; j++) {
            auto& tile = tiles[i][j];
            tile.rg = i;
            tile.cg = j;
            tile.kth   = (tile.rg * tiling->ncolgrps) + tile.cg;
            if(tile.rank == Env::rank) {
                pair.row = i;
                pair.col = j;    
                local_tiles.push_back(tile.kth);
                local_tiles_row_order.push_back(tile.kth);
                if (std::find(local_col_segments.begin(), local_col_segments.end(), pair.col) == local_col_segments.end())
                    local_col_segments.push_back(pair.col);
                
                if (std::find(local_row_segments.begin(), local_row_segments.end(), pair.row) == local_row_segments.end())
                    local_row_segments.push_back(pair.row);
            }
            tile.leader_rank_rg = tiles[i][i].rank;
            tile.leader_rank_cg = tiles[j][j].rank;
                
            tile.leader_rank_rg_rg = tiles[i][i].rank_rg;
            tile.leader_rank_cg_cg = tiles[j][j].rank_cg;
            if((tile.rank == Env::rank) and (i == j)) {
                owned_segment = i;
                //owned_segment_vec.push_back(owned_segment);
            }
        }
    }
    
    for (uint32_t j = 0; j < ncolgrps; j++) {
        for (uint32_t i = 0; i < nrowgrps; i++) {
            auto& tile = tiles[i][j];
            if(tile.rank == Env::rank)
                local_tiles_col_order.push_back(tile.kth);
        }
    }
    
    for(uint32_t t: local_tiles_row_order) {
        pair = tile_of_local_tile(t);
        if(pair.row == pair.col) {            
            for(uint32_t j = 0; j < ncolgrps; j++) {
                if(tiles[pair.row][j].rank == Env::rank) {
                    if(std::find(all_rowgrp_ranks.begin(), all_rowgrp_ranks.end(), tiles[pair.row][j].rank) 
                              == all_rowgrp_ranks.end()) {
                        all_rowgrp_ranks.push_back(tiles[pair.row][j].rank);
                        all_rowgrp_ranks_accu_seg.push_back(tiles[pair.row][j].cg);
                
                        all_rowgrp_ranks_rg.push_back(tiles[pair.row][j].rank_rg);
                        all_rowgrp_ranks_accu_seg_rg.push_back(tiles[pair.row][j].cg);
                    }
                }
                else {
                    if(std::find(follower_rowgrp_ranks.begin(), follower_rowgrp_ranks.end(), tiles[pair.row][j].rank) 
                            == follower_rowgrp_ranks.end()) {
                        all_rowgrp_ranks.push_back(tiles[pair.row][j].rank);
                        all_rowgrp_ranks_accu_seg.push_back(tiles[pair.row][j].cg);
                        
                        all_rowgrp_ranks_rg.push_back(tiles[pair.row][j].rank_rg);
                        all_rowgrp_ranks_accu_seg_rg.push_back(tiles[pair.row][j].cg);
                        
                        follower_rowgrp_ranks.push_back(tiles[pair.row][j].rank);
                        follower_rowgrp_ranks_accu_seg.push_back(tiles[pair.row][j].cg);
                        
                        follower_rowgrp_ranks_rg.push_back(tiles[pair.row][j].rank_rg);
                        follower_rowgrp_ranks_accu_seg_rg.push_back(tiles[pair.row][j].cg);
                    }
                }
            }
            for(uint32_t i = 0; i < nrowgrps; i++) {
                if(tiles[i][pair.col].rank == Env::rank) {
                    if(std::find(all_colgrp_ranks.begin(), all_colgrp_ranks.end(), tiles[i][pair.col].rank) 
                              == all_colgrp_ranks.end()) {
                        all_colgrp_ranks.push_back(tiles[i][pair.col].rank);
                        all_colgrp_ranks_accu_seg.push_back(tiles[i][pair.col].rg);
            
                        all_colgrp_ranks_cg.push_back(tiles[i][pair.col].rank_cg);
                        all_colgrp_ranks_accu_seg_cg.push_back(tiles[i][pair.col].rg);
                    }
                }
                else {
                    if(std::find(follower_colgrp_ranks.begin(), follower_colgrp_ranks.end(), tiles[i][pair.col].rank) 
                              == follower_colgrp_ranks.end()) {
                        all_colgrp_ranks.push_back(tiles[i][pair.col].rank);
                        all_colgrp_ranks_accu_seg.push_back(tiles[i][pair.col].rg);
                        
                        all_colgrp_ranks_cg.push_back(tiles[i][pair.col].rank_cg);
                        all_colgrp_ranks_accu_seg_cg.push_back(tiles[i][pair.col].rg);
                        
                        follower_colgrp_ranks.push_back(tiles[i][pair.col].rank);
                        follower_colgrp_ranks_accu_seg.push_back(tiles[i][pair.col].rg);
                        
                        follower_colgrp_ranks_cg.push_back(tiles[i][pair.col].rank_cg);
                        follower_colgrp_ranks_accu_seg_cg.push_back(tiles[i][pair.col].rg);
                    }
                }
            }
            break;
            /* We do not keep iterating as the ranks in row/col groups are the same */
        }
    }
    
    // Spilitting communicator among row/col groups       
    indexed_sort<int32_t, int32_t>(all_rowgrp_ranks, all_rowgrp_ranks_accu_seg);
    indexed_sort<int32_t, int32_t>(all_rowgrp_ranks_rg, all_rowgrp_ranks_accu_seg_rg);
    // Make sure there is at least one follower
    if(follower_rowgrp_ranks.size() > 1) {
        indexed_sort<int32_t, int32_t>(follower_rowgrp_ranks, follower_rowgrp_ranks_accu_seg);
        indexed_sort<int32_t, int32_t>(follower_rowgrp_ranks_rg, follower_rowgrp_ranks_accu_seg_rg);
    }
    indexed_sort<int32_t, int32_t>(all_colgrp_ranks, all_colgrp_ranks_accu_seg);
    indexed_sort<int32_t, int32_t>(all_colgrp_ranks_cg, all_colgrp_ranks_accu_seg_cg);
    // Make sure there is at least one follower
    if(follower_colgrp_ranks.size() > 1) {
        indexed_sort<int32_t, int32_t>(follower_colgrp_ranks, follower_colgrp_ranks_accu_seg);
        indexed_sort<int32_t, int32_t>(follower_colgrp_ranks_cg, follower_colgrp_ranks_accu_seg_cg);
    }
    if(Env::comm_split and not Env::get_comm_split()) {
        Env::rowgrps_init(all_rowgrp_ranks, tiling->rowgrp_nranks);
        Env::colgrps_init(all_colgrp_ranks, tiling->colgrp_nranks);
        Env::set_comm_split();
    }
    // Which column index in my rowgrps is mine when I'm the accumulator
    for(uint32_t j = 0; j < tiling->rank_nrowgrps; j++) {
        if(all_rowgrp_ranks[j] == Env::rank)
            accu_segment_rg = j;
    }
    // Which row index in my colgrps is mine when I'm the accumulator
    for(uint32_t j = 0; j < tiling->rank_ncolgrps; j++) {
        if(all_colgrp_ranks[j] == Env::rank)
            accu_segment_cg = j;
    } 
    // Which rowgrp is mine
    for(uint32_t j = 0; j < tiling->rank_nrowgrps; j++) {
        if(leader_ranks[local_row_segments[j]] == Env::rank)
            accu_segment_row = j;
    }
    // Which colgrp is mine
    for(uint32_t j = 0; j < tiling->rank_ncolgrps; j++) {
        if(leader_ranks[local_col_segments[j]] == Env::rank)
            accu_segment_col = j;
    } 
    // Print tiling assignment
    if(Env::is_master) {
        printf("Tiling Info: %d x %d [nrows x ncols]\n", nrows, ncols);
        printf("Tiling Info: %d x %d [rowgrps x colgrps] with height of %d\n", nrowgrps, ncolgrps, tile_height);
        printf("Tiling Info: %d x %d [rowgrp_nranks x colgrp_nranks]\n", tiling->rowgrp_nranks, tiling->colgrp_nranks);
        printf("Tiling Info: %d x %d [rank_nrowgrps x rank_ncolgrps]\n", tiling->rank_nrowgrps, tiling->rank_ncolgrps);
    }
    print("rank");
    Env::barrier();
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::print(std::string element)
{
    if(Env::is_master) {    
        uint32_t skip = 15;
        for (uint32_t i = 0; i < nrowgrps; i++) {
            for (uint32_t j = 0; j < ncolgrps; j++) {
                auto& tile = tiles[i][j];
                if(element.compare("rank") == 0) 
                    printf("%02d ", tile.rank);
                else if(element.compare("kth") == 0) 
                    printf("%3d ", tile.kth);
                else if(element.compare("ith") == 0) 
                    printf("%2d ", tile.ith);
                else if(element.compare("jth") == 0) 
                    printf("%2d ", tile.jth);
                else if(element.compare("rank_rg") == 0) 
                    printf("%2d ", tile.rank_rg);
                else if(element.compare("rank_cg") == 0) 
                    printf("%2d ", tile.rank_cg);
                else if(element.compare("leader_rank_rg") == 0) 
                    printf("%2d ", tile.leader_rank_rg);
                else if(element.compare("leader_rank_cg") == 0) 
                    printf("%2d ", tile.leader_rank_cg);
                else if(element.compare("nedges") == 0) 
                    printf("%lu ", tile.nedges);
                if(j > skip) {
                    printf("...");
                    break;
                }
            }
            printf("\n");
            if(i > skip) {
                printf(".\n.\n.\n");
                break;
            }
        }
    }
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::init_tiles() {
    if(Env::is_master)
        printf("Edge distribution: Distributing edges among %d ranks\n", Env::nranks);     
    
    distribute();
    Triple<Weight, Integer_Type> pair;
    ColSort<Weight, Integer_Type> f_col;
    auto f_comp = [] (const Triple<Weight, Integer_Type> &a, const Triple<Weight, Integer_Type> &b) {return (a.row == b.row and a.col == b.col);};    
    for(uint32_t t: local_tiles_row_order) {
        pair = tile_of_local_tile(t);
        auto& tile = tiles[pair.row][pair.col];
        if(tile.triples->size()) {
            std::sort(tile.triples->begin(), tile.triples->end(), f_col);
            /* remove parallel edges (duplicates), necessary for triangle couting */
            if(not parallel_edges) {
                auto last = std::unique(tile.triples->begin(), tile.triples->end(), f_comp);
                tile.triples->erase(last, tile.triples->end());
            }
        }
		tile.nedges = tile.triples->size();
    }
    balance();
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::balance()
{
    std::vector<std::vector<uint64_t>> nedges_grid(Env::nranks);
    std::vector<uint64_t> rank_nedges(Env::nranks);
    std::vector<uint64_t> rowgrp_nedges(nrowgrps);
    std::vector<uint64_t> colgrp_nedges(ncolgrps);
    
    for(int32_t i = 0; i < Env::nranks; i++)
        nedges_grid[i].resize(tiling->rank_ntiles);
    
    uint32_t k = 0;
    for(uint32_t i = 0; i < nrowgrps; i++)
    {
        for(uint32_t j = 0; j < ncolgrps; j++)  
        {
            auto &tile = tiles[i][j];
            if(Env::rank == (int32_t) tile.rank)
            {
                nedges_grid[Env::rank][k] = tile.nedges;
                k++;
            }
        }
    }

    Env::barrier();
    for(int32_t r = 0; r < Env::nranks; r++)
    {
        if(r != Env::rank)
        {
            auto &out_edges = nedges_grid[Env::rank];
            auto &in_edges = nedges_grid[r];
            MPI_Sendrecv(out_edges.data(), out_edges.size(), MPI_UNSIGNED_LONG, r, Env::rank, 
                         in_edges.data(), in_edges.size(), MPI_UNSIGNED_LONG, r, r, Env::MPI_WORLD, MPI_STATUS_IGNORE);
        }
    }
    Env::barrier();
    
    for(uint32_t i = 0; i < nrowgrps; i++)
    {
        for(uint32_t j = 0; j < ncolgrps; j++)  
        {
            auto &tile = tiles[i][j];
            if(Env::rank != tile.rank)
                tile.nedges = nedges_grid[tile.rank][tile.nth];
            
            rank_nedges[tile.rank] += tile.nedges;
            rowgrp_nedges[i] += tile.nedges;
            colgrp_nedges[j] += tile.nedges;
            nedges += tile.nedges;
        }
    }
    
    print("nedges");
     
    if(!Env::rank)
    {   
        int32_t skip = 15;
        double imbalance_threshold = .2;
        double ratio = 0;
        uint32_t count = 0;
        printf("\nEdge balancing info (Not functional):\n");  
        printf("Edge balancing: Total number of edges = %lu\n", nedges);
        printf("Edge balancing: Balanced number of edges per ranks = %lu \n", nedges/Env::nranks);
        printf("Edge balancing: imbalance ratio per ranks [0-%d]\n", Env::nranks-1);
        printf("Edge balancing: ");
        for(int32_t r = 0; r < Env::nranks; r++)
        {
            ratio = (double) (rank_nedges[r] / (double) (nedges/Env::nranks));
            if(r < skip)
                printf("%2.2f ", ratio);
            if(fabs(ratio - 1) > imbalance_threshold)
                count++;
        }
        if(Env::nranks > skip)
            printf("...\n");
        else
            printf("\n");
        if(count)
        {
            printf("Edge balancing: Edge distribution among %d ranks are not balanced.\n", count);
        }
        count = 0;
        
        printf("Edge balancing: Imbalance ratio per rowgroups [0-%d]\n", nrowgrps-1);
        printf("Edge balancing: ");
        for(uint32_t i = 0; i < nrowgrps; i++)
        {
            ratio = (double) (rowgrp_nedges[i] / (double) (nedges/nrowgrps));
            if(i < (uint32_t) skip)
                printf("%2.2f ", ratio);
            if(fabs(ratio - 1) > imbalance_threshold)
                count++;
        }
        if(nrowgrps > (uint32_t) skip)
            printf("...\n");
        else
            printf("\n");
        if(count)
        {
            printf("Edge balancing: Edge distribution among %d rowgroup(s) are not balanced.\n", count);
        }
        count = 0;
        
        printf("Edge balancing: Imbalance ratio per colgroups [0-%d]\n", ncolgrps-1);
        printf("Edge balancing: ");
        for(uint32_t j = 0; j < ncolgrps; j++)
        {
            ratio = (double) (colgrp_nedges[j] / (double) (nedges/ncolgrps));
            if(j < (uint32_t) skip)
                printf("%2.2f ", ratio);
            if(fabs(ratio - 1) > imbalance_threshold)
                count++;
        }
        if(ncolgrps > (uint32_t) skip)
            printf("...\n");
        else
            printf("\n");
        if(count)
        {
            printf("Edge balancing: Edge distribution among %d colgroups are not balanced.", count);
        }
        printf("\n");
    }
    Env::barrier();
}

/* Inspired from LA3 code @
   https://github.com/cmuq-ccl/LA3/blob/master/src/matrix/dist_matrix2d.hpp
*/
template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::distribute()
{
   /* Sanity check on # of edges */
    uint64_t nedges_start_local = 0, nedges_end_local = 0,
             nedges_start_global = 0, nedges_end_global = 0;
             
    std::vector<MPI_Request> out_requests;
    std::vector<MPI_Request> in_requests;
    MPI_Request request;                 
             
    for (uint32_t i = 0; i < nrowgrps; i++) {
        for (uint32_t j = 0; j < ncolgrps; j++) {
            auto &tile = tiles[i][j];
            if(tile.triples->size() > 0)
                nedges_start_local += tile.triples->size();
        }
    }

    MPI_Datatype MANY_TRIPLES;
    const uint32_t many_triples_size = 1;
    MPI_Type_contiguous(many_triples_size * sizeof(Triple<Weight, Integer_Type>), MPI_BYTE, &MANY_TRIPLES);
    MPI_Type_commit(&MANY_TRIPLES);
    std::vector<std::vector<Triple<Weight, Integer_Type>>> outboxes(Env::nranks);
    std::vector<std::vector<Triple<Weight, Integer_Type>>> inboxes(Env::nranks);
    std::vector<uint32_t> inbox_sizes(Env::nranks);
    for (uint32_t i = 0; i < nrowgrps; i++) {
        for (uint32_t j = 0; j < ncolgrps; j++)   {
            auto &tile = tiles[i][j];
            if(tile.rank != Env::rank) {
                auto &outbox = outboxes[tile.rank];
                outbox.insert(outbox.end(), tile.triples->begin(), tile.triples->end());
                tile.free_triples();
            }
        }
    }
    
   // MPI_Barrier(MPI_COMM_WORLD);    
    for (int32_t r = 0; r < Env::nranks; r++) {
        if (r != Env::rank) {
            auto &outbox = outboxes[r];
            uint32_t outbox_size = outbox.size();
            MPI_Sendrecv(&outbox_size, 1, MPI_UNSIGNED, r, Env::rank, &inbox_sizes[r], 1, MPI_UNSIGNED, 
                                                        r, r, Env::MPI_WORLD, MPI_STATUS_IGNORE);
            auto &inbox = inboxes[r];
            inbox.resize(inbox_sizes[r]);
            MPI_Sendrecv(outbox.data(), outbox.size(), MANY_TRIPLES, r, Env::rank, inbox.data(), inbox.size(), MANY_TRIPLES, 
                                                        r, r, Env::MPI_WORLD, MPI_STATUS_IGNORE);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);    
    
    /*
    for (int32_t r = 0; r < Env::nranks; r++) {
        if (r != Env::rank) {
            auto &inbox = inboxes[r];
            inbox.resize(inbox_sizes[r]);
            auto &outbox = outboxes[r];
            MPI_Sendrecv(outbox.data(), outbox.size(), MANY_TRIPLES, r, Env::rank, inbox.data(), inbox.size(), MANY_TRIPLES, 
                                                        r, r, Env::MPI_WORLD, MPI_STATUS_IGNORE);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);    
    */
    
    /*
    for (int32_t i = 0; i < Env::nranks; i++) {
        int32_t r = (Env::rank + i) % Env::nranks;
        if(r != Env::rank) {
            auto &inbox = inboxes[r];
            inbox.resize(inbox_sizes[r]);
            MPI_Irecv(inbox.data(), inbox.size(), MANY_TRIPLES, r, 1, Env::MPI_WORLD, &request);    
            in_requests.push_back(request);
        }
    }
    for (int32_t i = 0; i < Env::nranks; i++) {
        int32_t r = (Env::rank + i) % Env::nranks;
        if(r != Env::rank) {
            auto &outbox = outboxes[r];
            MPI_Isend(outbox.data(), outbox.size(), MANY_TRIPLES, r, 1, Env::MPI_WORLD, &request);
            out_requests.push_back(request);
        }
    }     
    MPI_Waitall(in_requests.size(), in_requests.data(), MPI_STATUSES_IGNORE);
    in_requests.clear();
    MPI_Waitall(out_requests.size(), out_requests.data(), MPI_STATUSES_IGNORE);   
    out_requests.clear();
    
    */
    
    for (int32_t r = 0; r < Env::nranks; r++) {
        if (r != Env::rank) {
            auto &inbox = inboxes[r];
            for (uint32_t i = 0; i < inbox_sizes[r]; i++) {
                test(inbox[i]);
                insert(inbox[i]);
            }
            inbox.clear();
            inbox.shrink_to_fit();
        }
    }
    
    
    Triple<Weight, Integer_Type> pair;
    for(uint32_t t: local_tiles_row_order) {
        pair = tile_of_local_tile(t);
        auto& tile = tiles[pair.row][pair.col];
        nedges_end_local += tile.triples->size();
    }
    
    MPI_Allreduce(&nedges_start_local, &nedges_start_global, 1, MPI_UNSIGNED_LONG, MPI_SUM, Env::MPI_WORLD);
    MPI_Allreduce(&nedges_end_local, &nedges_end_global, 1, MPI_UNSIGNED_LONG, MPI_SUM, Env::MPI_WORLD);
    assert(nedges_start_global == nedges_end_global);
    if(Env::is_master)
        printf("Edge distribution: Sanity check for exchanging %lu edges is done\n", nedges_end_global);
    auto retval = MPI_Type_free(&MANY_TRIPLES);
    assert(retval == MPI_SUCCESS);   
    Env::barrier();
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::init_filtering() {

    I.resize(tiling->rank_nrowgrps);
    for (uint32_t i = 0; i < tiling->rank_nrowgrps; i++)
        I[i].resize(tile_height);
    IV.resize(tiling->rank_nrowgrps);
    for (uint32_t i = 0; i < tiling->rank_nrowgrps; i++)
        IV[i].resize(tile_height);
    filter_vertices(_ROWS_);
    uint32_t io = accu_segment_row;
    auto& i_data = I[io];
    rowgrp_nnz_rows.resize(nnz_row_sizes_loc[io]);
    Integer_Type k = 0;
    for(Integer_Type i = 0; i < tile_height; i++) {
        if(i_data[i]) {
            rowgrp_nnz_rows[k] = i;
            k++;
        }
    }    

    J.resize(tiling->rank_ncolgrps);
    for (uint32_t i = 0; i < tiling->rank_ncolgrps; i++)
        J[i].resize(tile_width);
    JV.resize(tiling->rank_ncolgrps);
    for (uint32_t i = 0; i < tiling->rank_ncolgrps; i++)
        JV[i].resize(tile_width);    
    filter_vertices(_COLS_);
    uint32_t jo = accu_segment_col;
    auto& j_data = J[jo];
    colgrp_nnz_columns.resize(nnz_col_sizes_loc[jo]);
    k = 0;
    for(Integer_Type j = 0; j < tile_width; j++) {
        if(j_data[j]) {
            colgrp_nnz_columns[k] = j;
            k++;
        }
    }
        
    classify_vertices();

    rowgrp_regular_rows = regular_rows[io];
    rowgrp_source_rows = source_rows[io];
    colgrp_sink_columns = sink_columns[jo];
    Env::barrier();
    
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::filter_vertices(Filtering_type filtering_type_) {
    std::vector<std::vector<char>> *K;
    std::vector<std::vector<Integer_Type>> *KV;
    uint32_t rank_nrowgrps_, rank_ncolgrps_;
    uint32_t rowgrp_nranks_;
    Integer_Type tile_length;
    std::vector<int32_t> local_row_segments_;
    std::vector<int32_t> all_rowgrp_ranks_accu_seg_;
    std::vector<uint32_t> local_tiles_row_order_;
    int32_t accu_segment_rg_, accu_segment_row_;
    std::vector<int32_t> follower_rowgrp_ranks_; 
    std::vector<int32_t> follower_rowgrp_ranks_accu_seg_;
    std::vector<Integer_Type> nnz_sizes_all, nnz_sizes_loc;
    if(filtering_type_ == _ROWS_) {
        K = &I;
        KV = &IV;
        rank_nrowgrps_ = tiling->rank_nrowgrps;
        rank_ncolgrps_ = tiling->rank_ncolgrps;
        rowgrp_nranks_ = tiling->rowgrp_nranks;
        tile_length = tile_height;
        local_row_segments_ = local_row_segments;
        all_rowgrp_ranks_accu_seg_ = all_rowgrp_ranks_accu_seg;
        follower_rowgrp_ranks_ = follower_rowgrp_ranks;
        follower_rowgrp_ranks_accu_seg_ = follower_rowgrp_ranks_accu_seg;
        local_tiles_row_order_ = local_tiles_row_order;  
        accu_segment_rg_ = accu_segment_rg;
        accu_segment_row_ = accu_segment_row;
    }
    else if(filtering_type_ == _COLS_) {
        K = &J;
        KV = &JV;
        rank_nrowgrps_ = tiling->rank_ncolgrps;
        rank_ncolgrps_ = tiling->rank_nrowgrps;
        rowgrp_nranks_ = tiling->colgrp_nranks;
        tile_length = tile_width;
        local_row_segments_ = local_col_segments;
        all_rowgrp_ranks_accu_seg_ = all_colgrp_ranks_accu_seg;
        follower_rowgrp_ranks_ = follower_colgrp_ranks;
        follower_rowgrp_ranks_accu_seg_ = follower_colgrp_ranks_accu_seg;
        local_tiles_row_order_ = local_tiles_col_order;  
        accu_segment_rg_ = accu_segment_cg;
        accu_segment_row_ = accu_segment_col;
    }
    
    MPI_Datatype TYPE_INT = Types<Weight, Integer_Type, Integer_Type>::get_data_type();
    MPI_Datatype TYPE_CHAR = Types<Weight, Integer_Type, char>::get_data_type();
    std::vector<MPI_Request> out_requests;
    std::vector<MPI_Request> in_requests;
    MPI_Request request;
    int32_t leader, follower, my_rank, accu, this_segment;
    uint32_t tile_th, pair_idx;
    bool vec_owner, communication;
    uint32_t fi = 0, fo = 0;
    
    /* F is a temprorary 3D array designated to the filtering step. We'd 
       rather use char for this because of narrowing down the heap usage.*/
    std::vector<std::vector<std::vector<char>>> F;
    F.resize(rank_nrowgrps_);
    for(uint32_t j = 0; j < rank_nrowgrps_; j++) {
        if(local_row_segments_[j] == owned_segment) {
            F[j].resize(rowgrp_nranks_);
            for(uint32_t i = 0; i < rowgrp_nranks_; i++)
                F[j][i].resize(tile_length, 0);
        }
        else {
            F[j].resize(1);
            F[j][0].resize(tile_length, 0);
        }
    }

    for(uint32_t t: local_tiles_row_order_) {
        auto pair = tile_of_local_tile(t);
        auto &tile = tiles[pair.row][pair.col];
        if(filtering_type_ == _ROWS_) {
            tile_th = tile.nth;
            pair_idx = pair.row;
        }
        else if(filtering_type_ == _COLS_) {
            tile_th = tile.mth;
            pair_idx = pair.col;
        }
        vec_owner = (leader_ranks[pair_idx] == Env::rank);
        if(vec_owner)
            fo = accu_segment_rg_;
        else
            fo = 0;
        auto &f_data = F[fi][fo];
        Integer_Type f_nitems = tile_length;
        if(filtering_type_ == _ROWS_) {
            for (auto& triple : *(tile.triples)) {
                test(triple);
                auto pair1 = rebase(triple);
                if(!f_data[pair1.row])
                    f_data[pair1.row] = 1;
            }
        }
        else if(filtering_type_ == _COLS_) {
            for (auto& triple : *(tile.triples)) {
                test(triple);
                auto pair1 = rebase(triple);
                if(!f_data[pair1.col]) {
                    f_data[pair1.col] = 1;
                }
            }
        }        
        communication = (((tile_th + 1) % rank_ncolgrps_) == 0);
        if(communication) {
            if(filtering_type_ == _ROWS_)
                leader = tile.leader_rank_rg;
            if(filtering_type_ == _COLS_)
                leader = tile.leader_rank_cg;
            my_rank = Env::rank;
            if(leader == my_rank) {
                for(uint32_t j = 0; j < rowgrp_nranks_ - 1; j++) {
                    follower = follower_rowgrp_ranks_[j];
                    accu = follower_rowgrp_ranks_accu_seg_[j];
                    auto &fj_data = F[fi][accu];
                    Integer_Type fj_nitems = tile_length;
                    MPI_Irecv(fj_data.data(), fj_nitems, TYPE_CHAR, follower, pair_idx, Env::MPI_WORLD, &request);
                    in_requests.push_back(request);
                }
            }   
            else {
                MPI_Isend(f_data.data(), f_nitems, TYPE_CHAR, leader, pair_idx, Env::MPI_WORLD, &request);
                out_requests.push_back(request);
            }
            fi++;
        }
    }
    MPI_Waitall(out_requests.size(), out_requests.data(), MPI_STATUSES_IGNORE);
    MPI_Waitall(in_requests.size(), in_requests.data(), MPI_STATUSES_IGNORE);
    in_requests.clear();
    out_requests.clear();
    fi = accu_segment_row_;
    fo = accu_segment_rg_;
    auto &f_data = F[fi][fo];
    Integer_Type f_nitems = tile_length;
    for(uint32_t j = 0; j < rowgrp_nranks_ - 1; j++) {
        accu = follower_rowgrp_ranks_accu_seg_[j];
        auto &fj_data = F[fi][accu];
        Integer_Type fj_nitems = tile_length;                  
        for(uint32_t i = 0; i < fj_nitems; i++) {
            if(fj_data[i] and !f_data[i])
                f_data[i] = 1;
        }
    }    
    Integer_Type nnz_local = 0;
    for(uint32_t i = 0; i < f_nitems; i++) {
        if(f_data[i])
            nnz_local++;
    }
    nnz_sizes_all.resize(Env::nranks);
    nnz_sizes_all[owned_segment] = nnz_local;
    Env::barrier();     
    for (int32_t j = 0; j < Env::nranks; j++) {
        int32_t r = leader_ranks[j];
        if (j != owned_segment)
            MPI_Sendrecv(&nnz_sizes_all[owned_segment], 1, TYPE_INT, r, Env::rank, &nnz_sizes_all[j], 1, TYPE_INT, 
                                                                         r, r, Env::MPI_WORLD, MPI_STATUS_IGNORE);
    }
    Env::barrier();     
    for(uint32_t j = 0; j < rank_nrowgrps_; j++) {
        this_segment = local_row_segments_[j];
        nnz_sizes_loc.push_back(nnz_sizes_all[this_segment]);
    }
    if(nnz_sizes_all[owned_segment]) {
        uint32_t ko = accu_segment_row_;
        auto &kj_data =  (*K)[ko];
        auto &kvj_data = (*KV)[ko];
        Integer_Type j = 0;
        for(uint32_t i = 0; i < f_nitems; i++) {
            if(f_data[i]) {
                kj_data[i] = 1;
                kvj_data[i] = j; 
                j++;
            }
            else {
                kj_data[i] = 0;
                kvj_data[i] = 0;
            }
        }
        assert(j == nnz_sizes_all[owned_segment]);
    }
    for(uint32_t j = 0; j < rank_nrowgrps_; j++) {
        this_segment = local_row_segments_[j];
        leader = leader_ranks[this_segment];
        auto &kj_data = (*K)[j];
        if(this_segment == owned_segment) {
            for(uint32_t i = 0; i < rowgrp_nranks_ - 1; i++) {
                follower = follower_rowgrp_ranks_[i];
                MPI_Isend(kj_data.data(), tile_length, TYPE_CHAR, follower, owned_segment, Env::MPI_WORLD, &request);
                out_requests.push_back(request);
            }
        }
        else {
            MPI_Irecv(kj_data.data(), tile_length, TYPE_CHAR, leader, this_segment, Env::MPI_WORLD, &request);
            in_requests.push_back(request);
        }
    } 
    MPI_Waitall(out_requests.size(), out_requests.data(), MPI_STATUSES_IGNORE);
    MPI_Waitall(in_requests.size(), in_requests.data(), MPI_STATUSES_IGNORE);
    in_requests.clear();
    out_requests.clear();
    for(uint32_t j = 0; j < rank_nrowgrps_; j++) {
        this_segment = local_row_segments_[j];
        leader = leader_ranks[this_segment];
        auto &kvj_data = (*KV)[j];
        if(this_segment == owned_segment) {
            for(uint32_t i = 0; i < rowgrp_nranks_ - 1; i++) {
                follower = follower_rowgrp_ranks_[i];
                MPI_Isend(kvj_data.data(), tile_length, TYPE_INT, follower, owned_segment, Env::MPI_WORLD, &request);
                out_requests.push_back(request);
            }
        }
        else {
            MPI_Irecv(kvj_data.data(), tile_length, TYPE_INT, leader, this_segment, Env::MPI_WORLD, &request);
            in_requests.push_back(request);
        }
    } 
    MPI_Waitall(out_requests.size(), out_requests.data(), MPI_STATUSES_IGNORE);
    MPI_Waitall(in_requests.size(), in_requests.data(), MPI_STATUSES_IGNORE);
    in_requests.clear();
    out_requests.clear();
    for (uint32_t j = 0; j < rank_nrowgrps_; j++)
    {
        auto &kj_data = (*K)[j];
        auto &kvj_data = (*KV)[j];
        Integer_Type k = 0;
        for(uint32_t i = 0; i < tile_length; i++) {
            if(kj_data[i]) {
                assert(kvj_data[i] == k);
                k++;
            }
            else
                assert(kvj_data[i] == 0);
        }
    }
    if(filtering_type_ == _ROWS_) {
        nnz_row_sizes_all = nnz_sizes_all;
        nnz_row_sizes_loc = nnz_sizes_loc;
    }
    else if(filtering_type_ == _COLS_) {
        nnz_col_sizes_all = nnz_sizes_all;
        nnz_col_sizes_loc = nnz_sizes_loc;
    }
    
    for(uint32_t j = 0; j < rank_nrowgrps_; j++) {
        if(local_row_segments_[j] == owned_segment) {
            for(uint32_t i = 0; i < rowgrp_nranks_; i++) {
                F[j][i].clear();
                F[j][i].shrink_to_fit();
            }
        }
        else {
            F[j][0].clear();
            F[j][0].shrink_to_fit();
        }
    }
    F.clear();
    F.shrink_to_fit();
    Env::barrier();
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::classify_vertices() {
    uint32_t io = accu_segment_row;
    auto& i_data = I[io];
    uint32_t jo = accu_segment_col;
    auto& j_data = J[jo];
    
    regular_rows.resize(tiling->rank_nrowgrps);
    source_rows.resize(tiling->rank_nrowgrps);
    regular_columns.resize(tiling->rank_ncolgrps);
    sink_columns.resize(tiling->rank_ncolgrps);
    for(Integer_Type i = 0; i < tile_height; i++) {
        if(i_data[i] and j_data[i]) {
            regular_rows[io].push_back(i);
            regular_columns[jo].push_back(i);
        }
        if(i_data[i] and !j_data[i])
            source_rows[io].push_back(i);
        if(!i_data[i] and j_data[i])
            sink_columns[jo].push_back(i);
    }
    // regular rows/cols
    MPI_Datatype TYPE_INT = Types<Weight, Integer_Type, Integer_Type>::get_data_type();
    std::vector<MPI_Request> out_requests;
    std::vector<MPI_Request> in_requests;
    MPI_Request request;
    MPI_Status status;
    int32_t leader, my_rank, row_group, col_group;
    int32_t follower;
    Integer_Type nitems = 0;
    for (uint32_t i = 0; i < tiling->rank_nrowgrps; i++) {
        row_group = local_row_segments[i];
        leader = leader_ranks[row_group];
        my_rank = Env::rank;
        if(leader == my_rank) {            
            for(uint32_t j = 0; j < tiling->rowgrp_nranks - 1; j++) {
                follower = follower_rowgrp_ranks[j];
                nitems = regular_rows[io].size();
                MPI_Send(&nitems, 1, TYPE_INT, follower, row_group, Env::MPI_WORLD);
                MPI_Isend(regular_rows[io].data(), regular_rows[io].size(), TYPE_INT, follower, row_group, Env::MPI_WORLD, &request);
                out_requests.push_back(request);
            }
        }
        else {
            MPI_Recv(&nitems, 1, TYPE_INT, leader, row_group, Env::MPI_WORLD, &status);
            regular_rows[i].resize(nitems);
            MPI_Irecv(regular_rows[i].data(), regular_rows[i].size(), TYPE_INT, leader, row_group, Env::MPI_WORLD, &request);
            in_requests.push_back(request);
        }
        
    }
    MPI_Waitall(in_requests.size(), in_requests.data(), MPI_STATUSES_IGNORE);
    in_requests.clear();
    MPI_Waitall(out_requests.size(), out_requests.data(), MPI_STATUSES_IGNORE);
    out_requests.clear();
    regular_rows_bitvector.resize(tiling->rank_nrowgrps);
    for(uint32_t i = 0; i < tiling->rank_nrowgrps; i++)
        regular_rows_bitvector[i].resize(tile_height);
    for(uint32_t i = 0; i < tiling->rank_nrowgrps; i++) {
        for(uint32_t j: regular_rows[i])
            regular_rows_bitvector[i][j] = 1;
    }
    // Source rows
    for (uint32_t i = 0; i < tiling->rank_nrowgrps; i++) {
        row_group = local_row_segments[i];
        leader = leader_ranks[row_group];
        my_rank = Env::rank;
        if(leader == my_rank) {            
            for(uint32_t j = 0; j < tiling->rowgrp_nranks - 1; j++) {
                follower = follower_rowgrp_ranks[j];
                nitems = source_rows[io].size();
                MPI_Send(&nitems, 1, TYPE_INT, follower, row_group, Env::MPI_WORLD);
                MPI_Isend(source_rows[io].data(), source_rows[io].size(), TYPE_INT, follower, row_group, Env::MPI_WORLD, &request);
                out_requests.push_back(request);
            }
        }
        else {
            MPI_Recv(&nitems, 1, TYPE_INT, leader, row_group, Env::MPI_WORLD, &status);
            source_rows[i].resize(nitems);
            MPI_Irecv(source_rows[i].data(), source_rows[i].size(), TYPE_INT, leader, row_group, Env::MPI_WORLD, &request);
            in_requests.push_back(request);
        }
    }
    MPI_Waitall(in_requests.size(), in_requests.data(), MPI_STATUSES_IGNORE);
    in_requests.clear();
    MPI_Waitall(out_requests.size(), out_requests.data(), MPI_STATUSES_IGNORE);
    out_requests.clear();
    source_rows_bitvector.resize(tiling->rank_nrowgrps);
    for(uint32_t i = 0; i < tiling->rank_nrowgrps; i++)
        source_rows_bitvector[i].resize(tile_height);
    for(uint32_t i = 0; i < tiling->rank_nrowgrps; i++) {
        for(Integer_Type j: source_rows[i])
                source_rows_bitvector[i][j] = 1;
    }
    // Regular columns
    for(uint32_t i = 0; i < tiling->colgrp_nranks - 1; i++) {
        col_group = local_col_segments[jo];
        follower = follower_colgrp_ranks[i];
        nitems = regular_columns[jo].size();
        MPI_Send(&nitems, 1, TYPE_INT, follower, col_group, Env::MPI_WORLD);
        MPI_Isend(regular_columns[jo].data(), regular_columns[jo].size(), TYPE_INT, follower, col_group, Env::MPI_WORLD, &request);
        out_requests.push_back(request);
    }
    
    for(uint32_t i = 0; i < tiling->rank_ncolgrps; i++) {
        col_group = local_col_segments[i];
        leader = leader_ranks[col_group];
        my_rank = Env::rank;
        if(leader != my_rank) {
            MPI_Recv(&nitems, 1, TYPE_INT, leader, col_group, Env::MPI_WORLD, &status);
            regular_columns[i].resize(nitems);
            MPI_Irecv(regular_columns[i].data(), regular_columns[i].size(), TYPE_INT, leader, col_group, Env::MPI_WORLD, &request);
            in_requests.push_back(request);
        }
    }
    MPI_Waitall(in_requests.size(), in_requests.data(), MPI_STATUSES_IGNORE);
    in_requests.clear();
    MPI_Waitall(out_requests.size(), out_requests.data(), MPI_STATUSES_IGNORE);
    out_requests.clear();
    regular_columns_bitvector.resize(tiling->rank_ncolgrps);
    for(uint32_t i = 0; i < tiling->rank_ncolgrps; i++)
        regular_columns_bitvector[i].resize(tile_height);
    for(uint32_t i = 0; i < tiling->rank_ncolgrps; i++) {
        for(Integer_Type j: regular_columns[i])
            regular_columns_bitvector[i][j] = 1;
    }     
    // Sink columns
    for(uint32_t i = 0; i < tiling->colgrp_nranks - 1; i++) {
        col_group = local_col_segments[jo];
        follower = follower_colgrp_ranks[i];
        nitems = sink_columns[jo].size();
        MPI_Send(&nitems, 1, TYPE_INT, follower, col_group, Env::MPI_WORLD);
        MPI_Isend(sink_columns[jo].data(), nitems, TYPE_INT, follower, col_group, Env::MPI_WORLD, &request);
        out_requests.push_back(request);
    }
    
    for(uint32_t i = 0; i < tiling->rank_ncolgrps; i++) {
        col_group = local_col_segments[i];
        leader = leader_ranks[col_group];
        my_rank = Env::rank;
        if(leader != my_rank) {
            MPI_Recv(&nitems, 1, TYPE_INT, leader, col_group, Env::MPI_WORLD, &status);
            sink_columns[i].resize(nitems);
            MPI_Irecv(sink_columns[i].data(), sink_columns[i].size(), TYPE_INT, leader, col_group, Env::MPI_WORLD, &request);
            in_requests.push_back(request);
        }
    }
    MPI_Waitall(in_requests.size(), in_requests.data(), MPI_STATUSES_IGNORE);
    in_requests.clear();
    MPI_Waitall(out_requests.size(), out_requests.data(), MPI_STATUSES_IGNORE);
    out_requests.clear();
    sink_columns_bitvector.resize(tiling->rank_ncolgrps);
    for(uint32_t i = 0; i < tiling->rank_ncolgrps; i++)
        sink_columns_bitvector[i].resize(tile_height);
    for(uint32_t i = 0; i < tiling->rank_ncolgrps; i++) {
        for(Integer_Type j: sink_columns[i])
            sink_columns_bitvector[i][j] = 1;
    }    
}


template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::init_compression() {
    if(compression_type == _CSC_) {
        if(Env::is_master)
            printf("Edge compression: CSC\n");     
        init_csc();
    } 
    else if(compression_type == _DCSC_) {
        if(Env::is_master)
            printf("Edge compression: DCSC\n");
        init_dcsc();
    }   
    else if(compression_type == _TCSC_){
        if(Env::is_master)
            printf("Edge compression: TCSC\n");
        init_tcsc();
    }
    else {
        if(Env::is_master)
            printf("Edge compression: TCSC_CF\n");
        init_tcsc_cf();
    }
    Env::barrier();
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::init_csc() {
    for(uint32_t t: local_tiles_row_order) {
        auto pair = tile_of_local_tile(t);
        auto& tile = tiles[pair.row][pair.col];
        tile.compressor = new CSC_BASE<Weight, Integer_Type>(tile.nedges, tile_width);
        if(tile.nedges)
            tile.compressor->populate(tile.triples, tile_height, tile_width);
    }
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::init_dcsc() {
    uint32_t yi = 0, xi = 0, next_row = 0;
    for(uint32_t t: local_tiles_row_order)
    {
        auto pair = tile_of_local_tile(t);
        auto& tile = tiles[pair.row][pair.col];
        Integer_Type c_nitems = nnz_col_sizes_loc[xi];
        auto& j_data = J[xi];
        auto& jv_data = JV[xi];
        tile.compressor = new DCSC_BASE<Weight, Integer_Type>(tile.nedges, c_nitems);
        if(tile.nedges)
            tile.compressor->populate(tile.triples, tile_height, tile_width, j_data, jv_data);
        xi++;
        next_row = (((tile.nth + 1) % tiling->rank_ncolgrps) == 0);
        if(next_row)
            xi = 0;
    }   
}



template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::init_tcsc()
{
    uint32_t yi = 0, xi = 0, next_row = 0;
    for(uint32_t t: local_tiles_row_order)
    {
        auto pair = tile_of_local_tile(t);
        auto& tile = tiles[pair.row][pair.col];
        Integer_Type c_nitems = nnz_col_sizes_loc[xi];
        Integer_Type r_nitems = nnz_row_sizes_loc[yi];
        auto& i_data = I[yi];
        auto& iv_data = IV[yi];
        auto& j_data = J[xi];
        auto& jv_data = JV[xi];
        tile.compressor = new TCSC_BASE<Weight, Integer_Type>(tile.nedges, c_nitems, r_nitems);
        if(tile.nedges)
            tile.compressor->populate(tile.triples, tile_height, tile_width, i_data, iv_data, j_data, jv_data);
        xi++;
        next_row = (((tile.nth + 1) % tiling->rank_ncolgrps) == 0);
        if(next_row) {
            xi = 0;
            yi++;
        }
    }  
    del_classifier();
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::init_tcsc_cf()
{
    uint32_t yi = 0, xi = 0, next_row = 0;
    for(uint32_t t: local_tiles_row_order)
    {
        auto pair = tile_of_local_tile(t);
        auto& tile = tiles[pair.row][pair.col];
        Integer_Type c_nitems = nnz_col_sizes_loc[xi];
        Integer_Type r_nitems = nnz_row_sizes_loc[yi];
        auto& i_data = I[yi];
        auto& iv_data = IV[yi];
        auto& j_data = J[xi];
        auto& jv_data = JV[xi];
        auto& regular_rows_data = regular_rows[yi];
        auto& regular_rows_bv_data = regular_rows_bitvector[yi];
        auto& source_rows_data = source_rows[yi];
        auto& source_rows_bv_data = source_rows_bitvector[yi];
        auto& regular_columns_data = regular_columns[xi];
        auto& regular_columns_bv_data = regular_columns_bitvector[xi];
        auto& sink_columns_data = sink_columns[xi];
        auto& sink_columns_bv_data = sink_columns_bitvector[xi];
        tile.compressor = new TCSC_CF_BASE<Weight, Integer_Type>(tile.nedges, c_nitems, r_nitems);
        if(tile.nedges)
            tile.compressor->populate(tile.triples, tile_height, tile_width, i_data, iv_data, j_data, jv_data, regular_rows_data, regular_rows_bv_data, source_rows_data, source_rows_bv_data, regular_columns_data, regular_columns_bv_data, sink_columns_data, sink_columns_bv_data);
        xi++;
        next_row = (((tile.nth + 1) % tiling->rank_ncolgrps) == 0);
        if(next_row) {
            xi = 0;
            yi++;
        }
    }  
    del_classifier();
}


template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::del_classifier() {
    for(uint32_t i = 0; i < tiling->rank_nrowgrps; i++) {
        regular_rows[i].clear();
        regular_rows[i].shrink_to_fit();
        regular_rows_bitvector[i].clear();
        regular_rows_bitvector[i].shrink_to_fit();
    }
    regular_rows.clear();
    regular_rows.shrink_to_fit();    
    regular_rows_bitvector.clear();
    regular_rows_bitvector.shrink_to_fit();    
    for(uint32_t i = 0; i < tiling->rank_nrowgrps; i++) {
        source_rows[i].clear();
        source_rows[i].shrink_to_fit();
        source_rows_bitvector[i].clear();
        source_rows_bitvector[i].shrink_to_fit();
    }
    source_rows.clear();
    source_rows.shrink_to_fit();
    source_rows_bitvector.clear();
    source_rows_bitvector.shrink_to_fit();
    for(uint32_t i = 0; i < tiling->rank_ncolgrps; i++) {
        regular_columns[i].clear();
        regular_columns[i].shrink_to_fit();
        regular_columns_bitvector[i].clear();
        regular_columns_bitvector[i].shrink_to_fit();
    }
    regular_columns.clear();
    regular_columns.shrink_to_fit();
    regular_columns_bitvector.clear();
    regular_columns_bitvector.shrink_to_fit();
    for(uint32_t i = 0; i < tiling->rank_ncolgrps; i++) {
        sink_columns[i].clear();
        sink_columns[i].shrink_to_fit();
        sink_columns_bitvector[i].clear();
        sink_columns_bitvector[i].shrink_to_fit();
    }
    sink_columns.clear();
    sink_columns.shrink_to_fit();
    sink_columns_bitvector.clear();
    sink_columns_bitvector.shrink_to_fit();
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::del_filter() {
    for(uint32_t i = 0; i < tiling->rank_nrowgrps; i++) {
        I[i].clear();
        I[i].shrink_to_fit();
    }
    I.clear();
    I.shrink_to_fit();
    for(uint32_t i = 0; i < tiling->rank_ncolgrps; i++) {
        J[i].clear();
        J[i].shrink_to_fit();
    }
    J.clear();
    J.shrink_to_fit();
    for(uint32_t i = 0; i < tiling->rank_nrowgrps; i++) {   
        IV[i].clear();
        IV[i].shrink_to_fit();
    }
    IV.clear();
    IV.shrink_to_fit();
    for(uint32_t i = 0; i < tiling->rank_ncolgrps; i++) {   
        JV[i].clear();
        JV[i].shrink_to_fit();
    }
    JV.clear();
    JV.shrink_to_fit();
    
    rowgrp_nnz_rows.clear();
    rowgrp_nnz_rows.shrink_to_fit();
    rowgrp_regular_rows.clear();
    rowgrp_regular_rows.shrink_to_fit();
    rowgrp_source_rows.clear();
    rowgrp_source_rows.shrink_to_fit();
    colgrp_nnz_columns.clear();
    colgrp_nnz_columns.shrink_to_fit();
    colgrp_sink_columns.clear();
    colgrp_sink_columns.shrink_to_fit();
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::del_compression() {
    Triple<Weight, Integer_Type> pair;
    for(uint32_t t: local_tiles_row_order) {
        pair = tile_of_local_tile(t);
        auto& tile = tiles[pair.row][pair.col];
        delete tile.compressor;
        tile.compressor = nullptr;
    }
}

template<typename Weight, typename Integer_Type, typename Fractional_Type>
void Matrix<Weight, Integer_Type, Fractional_Type>::del_triples() {
    Triple<Weight, Integer_Type> pair;
    for(uint32_t t: local_tiles_row_order) {
        pair = tile_of_local_tile(t);
        auto& tile = tiles[pair.row][pair.col];
        tile.free_triples();
    }
}
#endif
