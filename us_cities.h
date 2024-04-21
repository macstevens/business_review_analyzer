
// us_cities.h :

#ifndef US_CITIES_H
#define US_CITIES_H

#include  <cassert>
#include  <algorithm>
#include  <iostream>
#include  <string>
#include  <fstream>
#include  <ctime>
#include  <map>
#include  <memory>
#include  <set>
#include  <vector>


const std::string& get_state_abbr(const std::string& state);


/*
* organize cities by size so that each city lies in one bin
* or partially in two bins
*/
struct bin_index_pair{
    size_t m_idx_a=0;
    double m_ratio_a=0.0;
    size_t m_idx_b=0;
    double m_ratio_b=0.0;
    };

struct city_and_state{
    std::string m_city;
    std::string m_state;
    bool operator<(const city_and_state& other) const{
        return( (m_city < other.m_city) ||
        ((m_city == other.m_city) && (m_state < other.m_state)));
        }
    };
typedef std::vector<city_and_state> city_and_state_vec;


struct city_bin{
public:
    double m_tally=0.0;
    city_and_state m_small_city;
    size_t m_small_city_population=0;
    city_and_state m_large_city;
    size_t m_large_city_population=0;
};
typedef std::vector<city_bin> city_bin_vec;
typedef city_bin_vec::const_iterator city_bin_vec_citr;
typedef city_bin_vec::iterator city_bin_vec_itr;


/*

* 
* 
* 
* */
class city_bins{
private:
    typedef std::pair<size_t, size_t> szt_szt;
    typedef std::vector<szt_szt> szt_szt_vec;
    typedef szt_szt_vec::const_iterator szt_szt_vec_citr;
    typedef szt_szt_vec::iterator szt_szt_vec_itr;

    typedef std::map<city_and_state, bin_index_pair> cs_bip_map;
    typedef cs_bip_map::const_iterator cs_bip_map_citr;
    typedef cs_bip_map::iterator cs_bip_map_itr;

private:
    size_t m_bin_count;
    size_t m_total_us_city_population;
    szt_szt_vec m_pop_cityidx_sort_vec;
    double m_population_per_bin;

    city_bin_vec m_city_bins;
    city_and_state_vec m_unrecognized_cs_vec;
    cs_bip_map m_cs_bip_map;
public:
    city_bins();
   ~city_bins();
    void init(const size_t& bin_count);
    void add_to_bin_tally(const city_and_state& cs);
    double get_total_tally() const;
    const city_bin& get_bin(const size_t& bin_idx) const;
    size_t get_bin_count() const;
    double get_avg_tally_per_bin() const;
    size_t get_unrecognized_cs_count() const;
    const city_and_state& get_unrecognized_city_state(const size_t idx) const;
private:
    void insert_in_cs_bip_map(const city_and_state& cs,
        const bin_index_pair& bip);
};


#endif /* US_CITIES_H */