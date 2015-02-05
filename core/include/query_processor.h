/**
 * @file   query_processor.h
 * @author Stavros Papadopoulos <stavrosp@csail.mit.edu>
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2014 Stavros Papadopoulos <stavrosp@csail.mit.edu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * @section DESCRIPTION
 *
 * This file defines class QueryProcessor. It also defines 
 * QueryProcessorException, which is thrown by QueryProcessor.
 */

#ifndef QUERY_PROCESSOR_H
#define QUERY_PROCESSOR_H

#include "array_schema.h"
#include "csv_file.h"
#include "storage_manager.h"
#include "expression_tree.h"
#include <queue>

/** 
 * This class implements the query processor module, which is responsible
 * for processing the various queries. 
 */
class QueryProcessor {
 public:
  // TYPE DEFINITIONS
  /** The bounding coordinates of a tile (see Tile::bounding_coordinates). */
  typedef std::pair<std::vector<double>, std::vector<double> > 
      BoundingCoordinatesPair;
  /** Mnemonic: (dist, rank). */
  typedef std::pair<double, uint64_t> DistRank;
  /** A vector of attribute ids. */
  typedef std::vector<unsigned int> AttributeIds;
  /** 
   * A hyper-rectangle in the logical space, including all the coordinates
   * of a tile. It is a list of low/high values across each dimension, i.e.,
   * (dim#1_low, dim#1_high, dim#2_low, dim#2_high, ...).
   */
  typedef std::vector<double> MBR;
  /** Mnemonic: (pos, coord). */ 
  typedef std::pair<uint64_t, std::vector<double> > PosCoord;
  /** Mnemonic: (rank, (pos, coord)). */ 
  typedef std::pair<uint64_t, PosCoord> RankPosCoord;
  /** Mnemonic: (dist,(rank, (pos, coord))). */ 
  typedef std::pair<double, RankPosCoord> DistRankPosCoord;

  // CONSTRUCTORS AND DESTRUCTORS
  /** 
   * Simple constructor. The workspace is where the query processor will create 
   * its data. The storage manager is the module the query processor interefaces
   * with.
   */
  QueryProcessor(const std::string& workspace, StorageManager& storage_manager);

  // QUERY FUNCTIONS
  /** 
   * Exports an array to a CSV file. Each line in the CSV file represents
   * a logical cell comprised of coordinates and attribute values. The 
   * coordinates are written first, and then the attribute values,
   * following the order as defined in the schema of the array.
   */
  void export_to_CSV(const StorageManager::ArrayDescriptor* ad,
                     const std::string& filename) const;
  /** 
   * Exports an array to a CSV file. Each line in the CSV file represents
   * a logical cell comprised of coordinates and attribute values. The 
   * coordinates are written first, and then the attribute values,
   * following the order as defined in the schema of the array. This function
   * operates on multiple array fragments.
   */
  void export_to_CSV(
      const std::vector<const StorageManager::ArrayDescriptor*>& ad,
      const std::string& filename) const;
  /** 
   * A filter query creates a new array from the input array descriptor, 
   * containing only the cells whose attribute values satisfy the input 
   * expression (given in the form of an expression tree). 
   * The new array will have the input result name.
   */
  void filter(const StorageManager::ArrayDescriptor* ad,
      const ExpressionTree* expression,
      const std::string& result_array_name) const;
  /** 
   * A filter query creates a new array from the input array descriptor, 
   * containing only the cells whose attribute values satisfy the input 
   * expression (given in the form of an expression tree). 
   * The new array will have the input result name. This function
   * operates on multiple array fragments.
   */
  void filter(
      const std::vector<const StorageManager::ArrayDescriptor*>& ad,
      const ExpressionTree* expression,
      const std::string& result_array_name) const;
  /** 
   * Joins the two input arrays (say, A and B). The result contains a cell only
   * if both the corresponding cells in A and B are non-empty. The input arrays
   * must be join-compatible (see ArraySchema::join_compatible). Moreover,
   * see ArraySchema::create_join_result_schema to see the schema of the
   * output array.
   */
  void join(const StorageManager::ArrayDescriptor* ad_A,
            const StorageManager::ArrayDescriptor* ad_B,
            const std::string& result_array_name) const;
  /** 
   * Returns the k nearest neighbors from query point q. The results (along with
   * all their attribute values) are stored in a new array. The distance metric
   * used to calculate proximity is the Euclidean distance.
   */
  void nearest_neighbors(const StorageManager::ArrayDescriptor* ad,
                         const std::vector<double>& q,
                         uint64_t k,
                         const std::string& result_array_name) const;
  /** 
   * A subarray query creates a new array from the input array descriptor, 
   * containing only the cells whose coordinates fall into the input range. 
   * The new array will have the input result name.
   */
  void subarray(const StorageManager::ArrayDescriptor* array_descriptor,
                const Tile::Range& range,
                const std::string& result_array_name) const;

 private:
  // PRIVATE ATTRIBUTES
  /** The StorageManager object the QueryProcessor will be interfacing with. */
  StorageManager& storage_manager_;
  /** A folder in the disk where the query processor creates all its data. */
  std::string workspace_;

  // PRIVATE METHODS
  /** Advances all the cell iterators by 1. */
  void advance_cell_its(unsigned int attribute_num,
                        Tile::const_iterator* cell_its) const;
 /** 
   * Advances by one only the cell iterators of the attributes whose id is in 
   * attribute_ids. 
   */
  void advance_cell_its(Tile::const_iterator* cell_its,
                        const std::vector<unsigned int>& attribute_ids) const;
  /** Advances only the attribute cell iterators by step. */
  void advance_cell_its(unsigned int attribute_num,
                        Tile::const_iterator* cell_its,
                        int64_t step) const; 
  /** 
   * Advances by step only the cell iterators of the attributes whose id is in 
   * attribute_ids. 
   */
  void advance_cell_its(Tile::const_iterator* cell_its,
                        const std::vector<unsigned int>& attribute_ids,
                        int64_t step) const; 
  /** 
   * Advances all the cell iterators by 1. If the cell iterators are equal to
   * the end iterator, the tile iterators are advanced. If the tile iterators
   * are not equal to the end tile iterator, then new cell iterators are
   * initialized.
   */
  void advance_cell_tile_its(unsigned int attribute_num,
                             Tile::const_iterator* cell_its, 
                             Tile::const_iterator& cell_it_end,
                             StorageManager::const_iterator* tile_its,
                             StorageManager::const_iterator& tile_it_end) const;
  /** 
   * Advances the cell iterators by 1, focusing only on those described by
   * attribute_ids, plus the extra coordinates attribute. If the cell iterators
   * are equal to the end iterator, the corresponding tile iterators are
   * advanced. If the tile iterators are not equal to the end tile iterator,
   * then new cell iterators are initialized. The last three arguments are
   * updated based on what happens to the iterators:
   *
   * If tile iterators are advanced, skipped_tiles is incremented, skipped_cells
   * is set to 0, and non_cell_iterators_initialized is set to false. This is
   * because we changed tile in a subset of attributes. The above three 
   * changes are essential so that we keep track how to advance the cell and
   * tile iterators of the skipped attributes when the time comes to synchronize
   * them with the tile and cell iterators advanced in this function.
   *
   * If the tile iterators are not advanced, we simply increment skipped_cells.
   */
  void advance_cell_tile_its(
      unsigned int attribute_num,
      Tile::const_iterator* cell_its, 
      Tile::const_iterator& cell_it_end,
      StorageManager::const_iterator* tile_its,
      StorageManager::const_iterator& tile_it_end,
      const std::vector<unsigned int>& attribute_ids, 
      int64_t& skipped_tiles,
      uint64_t& skipped_cells,
      bool& non_cell_its_initialized) const;
  /** Advances all the tile iterators by 1. */
  void advance_tile_its(unsigned int attribute_num,
                        StorageManager::const_iterator* tile_its) const; 
  /** Advances only the attribute tile iterators by step. */
  void advance_tile_its(unsigned int attribute_num,
                        StorageManager::const_iterator* tile_its,
                        int64_t step) const; 
  /** 
   * Advances by one only the attribute tile iterators whose ids are in the 
   * last argument. 
   */
  void advance_tile_its(StorageManager::const_iterator* tile_its,
                        const std::vector<unsigned int>& attribute_ids) const;
  /** 
   * Advances by step only the attribute tile iterators whose ids are in the
   * last argument. 
   */
  void advance_tile_its(StorageManager::const_iterator* tile_its,
                        const std::vector<unsigned int>& attribute_ids,
                        int64_t step) const; 
  /** 
   * Appends a logical cell of an array (comprised of attribute values and 
   * coordinates held in the input cell iterators) into
   * another array (in the corresponding tiles held in input variable 'tiles').
   */
  void append_cell(const Tile::const_iterator* cell_its,
                   Tile** tiles,
                   unsigned int attribute_num) const;
  /** 
   * Appends a logical cell to an array C that is the result of joining
   * cells from arrays A and B (see QueryProcessor::join).
   */
  void append_cell(const Tile::const_iterator* cell_its_A,
                   const Tile::const_iterator* cell_its_B,
                   Tile** tiles_C,
                   unsigned int attribute_num_A,
                   unsigned int attribute_num_B) const;
  /** 
   * Returns true if the result of the expression is true on the values of the
   * attributes whose id is in the attribute_ids.
   */
  bool cell_satisfies_expression(const ArraySchema& array_schema,
                                 const Tile::const_iterator* cell_its,
                                 const std::vector<unsigned int>& attribute_ids,
                                 const ExpressionTree* expression) const;
  /** 
   * Converts a logical cell of an array into a CSV line. The cell is 
   * comprised of all coordinates and attribute values, which are contained 
   * in the input array of cell iterators.
   */
  CSVLine cell_to_csv_line(const Tile::const_iterator* cell_its,
                           unsigned int attribute_num) const;
  /** 
   * Returns a vector of pairs (dist, rank), sorted on dist, where rank is the
   * rank of a tile (indicating if it was appended first, second, etc., in the 
   * array, and dist is the (Euclidean) distance of its MBR from q. The rank
   * is useful for retrieving later each tile from the storage manager.
   */
  std::vector<DistRank> compute_sorted_dist_ranks(
      const StorageManager::ArrayDescriptor* ad,
      const std::vector<double>& q) const;
  /** 
   * Returns a vector with k tuples of the form (rank, (pos, coord)).
   * Each tuple corresponds to the coordinates of one of the k nearest
   * cells in a nearest neighbor query (see QueryProcessor::nearest_neighbors).
   *
   * coord: the coordinates of the cell.
   *
   * rank: the rank of the tile this cell belongs to.
   *
   * pos: the position of the cell in the tile.
   *
   * q is the query point for the nearest neighbor search.
   *
   * k is the number of nearest neighbors to be found.
   * 
   * sorted_dist_ranks contains pairs of the form (dist, rank), where rank
   * is a tile rank and dist is its distance to the query q.
   */
  std::vector<RankPosCoord> compute_sorted_kNN_coords(
      const StorageManager::ArrayDescriptor* ad,
      const std::vector<double>& q,
      uint64_t k,
      const std::vector<DistRank>& sorted_dist_ranks) const;
  /** Creates the workspace folder. */
  void create_workspace() const;
  /** 
   * Implementation of QueryProcessor::filter for the case of irregular tiles.
   */
  void filter_irregular(const StorageManager::ArrayDescriptor* array_descriptor,
                        const ExpressionTree* expression,
                        const std::string& result_array_name) const;
  /** 
   * Implementation of QueryProcessor::filter for the case of regular tiles. 
   * This function operators on multiple array fragments.
   */
  void filter_regular(
      const std::vector<const StorageManager::ArrayDescriptor*>& ad,
      const ExpressionTree* expression,
      const std::string& result_array_name) const;
  /** 
   * Implementation of QueryProcessor::filter for the case of irregular tiles.
   * This function operators on multiple array fragments.
   */
  void filter_irregular(
      const std::vector<const StorageManager::ArrayDescriptor*>& ad,
      const ExpressionTree* expression,
      const std::string& result_array_name) const;
  /** 
   * Implementation of QueryProcessor::filter for the case of regular tiles. 
   */
  void filter_regular(const StorageManager::ArrayDescriptor* array_descriptor,
                      const ExpressionTree* expression,
                      const std::string& result_array_name) const;

  /** 
   * Returns the index of the fragment from which we will get the next
   * cell, based on the global order.
   */
  int get_next_fragment_index(StorageManager::const_iterator** tile_its,
                              StorageManager::const_iterator* tile_it_end,
                              unsigned int fragment_num,
                              Tile::const_iterator** cell_its,
                              Tile::const_iterator* cell_it_end,
                              const ArraySchema& array_schema) const;
  /** 
   * Returns the index of the fragment from which we will get the next cell, 
   * based on the global order. During retrieving the next cell, if it needs to
   * advance cell and tile iterators, it only focuses on those described in
   * attribute_ids, plus the extra coordinates attribute. 
   *
   * Since this function may advance cell tile iterators, we may need to update
   * some information (namely, skipped_tiles, skipped_cells, and
   * non_cell_its_initialized), so that we know how to advance the cell and 
   * tile iterators not being advanced by this function later in another 
   * function (so that the cells across all attributes are synchronized).
   */
  int get_next_fragment_index(
      StorageManager::const_iterator** tile_its,
      StorageManager::const_iterator* tile_it_end,
      unsigned int fragment_num,
      Tile::const_iterator** cell_its,
      Tile::const_iterator* cell_it_end,
      const std::vector<unsigned int>& attribute_ids,
      int64_t* skipped_tiles,
      uint64_t* skipped_cells,
      bool* non_cell_its_initialized,
      const ArraySchema& array_schema) const;
  /** 
   * Gets from the storage manager all the (attribute and coordinate) tiles
   * of the input array having the input id. 
   */
  void get_tiles(const StorageManager::ArrayDescriptor* array_descriptor,
                 uint64_t tile_id, const Tile** tiles) const;
  /** Initializes cell iterators. */
  void initialize_cell_its(const Tile** tiles,
                           unsigned int attribute_num,
                           Tile::const_iterator* cell_its, 
                           Tile::const_iterator& cell_it_end) const; 
  /** Initializes only the attribute cell iterators. */
  void initialize_cell_its(const Tile** tiles,
                           unsigned int attribute_num,
                           Tile::const_iterator* cell_its) const; 
  /** Initializes cell iterators. */
  void initialize_cell_its(const StorageManager::const_iterator* tile_its,
                           unsigned int attribute_num,
                           Tile::const_iterator* cell_its, 
                           Tile::const_iterator& cell_it_end) const; 
  /** Initializes the cell iterators described in attribute_ids. */
  void initialize_cell_its(
      const StorageManager::const_iterator* tile_its,
      Tile::const_iterator* cell_its, 
      Tile::const_iterator& cell_it_end,
      const std::vector<unsigned int>& attribute_ids) const;
  /** Initializes the cell iterators described in attribute_ids. */
  void initialize_cell_its(
      const StorageManager::const_iterator* tile_its,
      Tile::const_iterator* cell_its, 
      const std::vector<unsigned int>& attribute_ids) const;
  /** Initializes only the attribute cell iterators. */
  void initialize_cell_its(const StorageManager::const_iterator* tile_its,
                           unsigned int attribute_num,
                           Tile::const_iterator* cell_its) const; 
  /** Initializes tile iterators. */
  void initialize_tile_its(const StorageManager::ArrayDescriptor* ad,
                           StorageManager::const_iterator* tile_its,
                           StorageManager::const_iterator& tile_it_end) const;
  /** 
   * Initializes tile iterators. The last argument determines which attribute
   * the end tile iterator will correspond to.
   */
  void initialize_tile_its(const StorageManager::ArrayDescriptor* ad,
                           StorageManager::const_iterator* tile_its,
                           StorageManager::const_iterator& tile_it_end,
                           unsigned int end_attribute_id) const;
  /**
   * Returns true if the cell represents a deletion, i.e., when all its
   * attribute values are NULL.
   */
  bool is_null(const Tile::const_iterator* cell_its,
               unsigned int attribute_num) const;
  /** Implements QueryProcessor::join for arrays with irregular tiles. */
  void join_irregular(const StorageManager::ArrayDescriptor* ad_A,
                      const StorageManager::ArrayDescriptor* ad_B,
                      const ArraySchema& array_schema_C) const;
  /** Implements QueryProcessor::join for arrays with regular tiles. */
  void join_regular(const StorageManager::ArrayDescriptor* ad_A,
                    const StorageManager::ArrayDescriptor* ad_B,
                    const ArraySchema& array_schema_C) const;
  /** 
   * Joins two irregular tiles (from A and B respectively) and stores 
   * the result in the tiles of C. 
   */
  void join_tiles_irregular(
      unsigned int attribute_num_A, 
      const StorageManager::const_iterator* tile_its_A,
      Tile::const_iterator* cell_its_A,
      Tile::const_iterator& cell_it_end_A, 
      unsigned int attribute_num_B, 
      const StorageManager::const_iterator* tile_its_B,
      Tile::const_iterator* cell_its_B,
      Tile::const_iterator& cell_it_end_B,
      const StorageManager::ArrayDescriptor* ad_C, Tile** tiles_C,
      bool& attribute_cell_its_initialized_A,
      bool& attribute_cell_its_initialized_B) const;
  /** 
   * Joins two regular tiles (from A and B respectively) and stores 
   * the result in the tiles of C. 
   */
  void join_tiles_regular(
      unsigned int attribute_num_A, 
      const StorageManager::const_iterator* tile_its_A,
      Tile::const_iterator* cell_its_A,
      Tile::const_iterator& cell_it_end_A, 
      unsigned int attribute_num_B, 
      const StorageManager::const_iterator* tile_its_B,
      Tile::const_iterator* cell_its_B,
      Tile::const_iterator& cell_it_end_B,
      const StorageManager::ArrayDescriptor* ad_C, Tile** tiles_C) const;

  /** Returns true if the input tiles may produce join results. */
  bool may_join(const StorageManager::const_iterator& it_A, 
                const StorageManager::const_iterator& it_B) const; 
  /** 
   * Implementation of QueryProcessor::nearest_neighbors for the case of 
   * irregular tiles.
   */
  void nearest_neighbors_irregular(
      const StorageManager::ArrayDescriptor* ad,
      const std::vector<double>& q,
      uint64_t k,
      const std::string& result_array_name) const;
  /** 
   * Implementation of QueryProcessor::nearest_neighbors for the case of 
   * regular tiles.
   */
  void nearest_neighbors_regular(
      const StorageManager::ArrayDescriptor* ad,
      const std::vector<double>& q,
      uint64_t k,
      const std::string& result_array_name) const;
  /** 
   * Creates an array of Tile objects with the input tile id based on the input 
   * array schema. 
   */
  void new_tiles(const ArraySchema& array_schema,
                 uint64_t tile_id, Tile** tiles) const;
  /** Returns true if the input MBRs overlap. */
  bool overlap(const MBR& mbr_A, const MBR& mbr_B) const;
  /** 
   * Returns true if the cell id ranges along the global order (derived from 
   * the input bounding coordinates and array schema) intersect. 
   */
  bool overlap(const BoundingCoordinatesPair& bounding_coordinates_A, 
               const BoundingCoordinatesPair& bounding_coordinates_B,
               const ArraySchema& array_schema) const;
  /** Returns true if the input path is an existing directory. */
  bool path_exists(const std::string& path) const;
  /** Returns the squared Euclidean distance between a point q and an MBR. */
  double point_to_mbr_distance(const std::vector<double>& q,
                          const std::vector<double>& mbr) const;
  /** Returns the squared Euclidean distance between points q and p. */
  double point_to_point_distance(const std::vector<double>& q,
                                 const std::vector<double>& p) const;
  /** Simply sets the workspace. */
  void set_workspace(const std::string& path);
  /** Sends the input tiles to the storage manager. */
  void store_tiles(const StorageManager::ArrayDescriptor* ad, 
                   Tile** tiles) const;
  /** Implements QueryProcessor::subarray for arrays with irregular tiles. */
  void subarray_irregular(
      const StorageManager::ArrayDescriptor* array_descriptor,
      const Tile::Range& range, const std::string& result_array_name) const;
  /** Implements QueryProcessor::subarray for arrays with regular tiles. */
  void subarray_regular(
      const StorageManager::ArrayDescriptor* array_descriptor,
      const Tile::Range& range, const std::string& result_array_name) const;
};

/** This exception is thrown by QueryProcessor. */
class QueryProcessorException {
 public:
  // CONSTRUCTORS & DESTRUCTORS
  /** Takes as input the exception message. */
  QueryProcessorException(const std::string& msg) 
      : msg_(msg) {}
  /** Empty destructor. */
  ~QueryProcessorException() {}

  // ACCESSORS
  /** Returns the exception message. */
  const std::string& what() const { return msg_; }

 private:
  /** The exception message. */
  std::string msg_;
};

#endif