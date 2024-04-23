// business_review_analyzer.h


#if !defined(BUSINESS_REVIEW_ANALYZER_H)
#define BUSINESS_REVIEW_ANALYZER_H

#include <cassert>
#include <algorithm>
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "us_cities.h"

#pragma warning(disable:4996)

/*
* TODO: instructions
*       test case
*       city count file
*       word count file
*       add numeric category to word count file
* 
* BBB Instructions
 1. Run Firefox browser
 2. Open web page  https://www.bbb.org/us/ca/calabasas/profile/gold-buyers/goldco-direct-llc-1216-100109958/customer-reviews
 3. Scroll to bottom of web page.  Press [Load More] button.
 4. Repeat pressing [Load More] button until all reviews are loaded.
 5. Press ctrl-A to select all text on web page.
 6. Press ctrl-C to copy.
 7. Open text editor.
 8. Press ctrl-V in text editor to paste text.
 9. Save file from text editor. 
* 
* 
* Consumer Affairs Instructions
 1. Run Firefox browser
 2. Open web page https://www.consumeraffairs.com/finance/goldco-precious-metals.html
 3. Save file as html:  Right-click -> Save Page As...  consumer_affairs_goldco_01.htm
 4. Scroll to bottom of web page.  Press [>] button
 5. The following web page should now be open. https://www.consumeraffairs.com/finance/goldco-precious-metals.html?page=2#scroll_to_reviews=true
 6. Repeat steps 2-5 until all reviews are downloaded.
 7. In text editor, concatenate all resulting .htm files into a single file.
* 
* 
Google Instructions
 1. Run Firefox browser
 2. Open web page https://www.google.com/search?client=firefox-b-1-d&q=goldco+google+reviews
 3. On right side of page, click on link "2,416 Google reviews" ( https://www.google.com/search?client=firefox-b-1-d&q=goldco+google+reviews# )
 4. Smaller pop-up page appears.  This page contains the reviews.  
 5. Scroll to bottom of pop-up review page.  This causes more reviews to load.
 6. Repeat process of scrolling to bottom until all reviews load.
 7. Save file as html:  Right-click -> Save Page As...  google_goldco.htm
 8. Open resulting .htm file in text editor.
 9 Add line stating download time in GMT (zulu time) at top of file. For example:
  <reference_time>2024-04-03T20:00:00.000Z</reference_time>  <! download time = 2024/04/03 12:00 Pacific >
 10. Save file from text editor
* 
* 
TrustLink Instructions
 1. Run Firefox browser
 2. Open web page https://www.trustlink.org/Reviews/Goldco-206527051
 3. Save file as html:  Right-click -> Save Page As...  trustlink_goldco_01.htm
 4. At bottom of page, find page number links "1 2 3 4 5 ..." . Click on "2" to load next page of reviews.
 5. Repeat steps 3-4 until all reviews are downloaded.
 6. In text editor, concatenate all resulting .htm files into a single file.
* 
* 
TrustPilot Instructions
 1. Run Firefox browser
 2. Open web page https://www.trustpilot.com/review/goldco.com
 3. Save file as html:  Right-click -> Save Page As...  trustpilot_goldco_01.htm
 4. At bottom of page, press [Next Page] button to load https://www.trustpilot.com/review/goldco.com?page=2
 5. Repeat steps 3-4 until all reviews are downloaded.
 6. In text editor, concatenate all resulting .htm files into a single file.
*
* 
Yelp (Recommended) Instructions
 1. Open text editor.
 2. Run Firefox browser
 3. Open web page https://www.yelp.com/biz/goldco-calabasas
 4. Press ctrl-A to select all text on web page.
 5. Press ctrl-C to copy.
 6. Open text editor.
 7. Press ctrl-V in text editor to paste text. 
 8. At bottom of web page, press [>] button to load next page https://www.yelp.com/biz/goldco-calabasas?start=10
 9. Repeat steps 4-8 until all reviews are loaded into text editor.
 10. Manually add rating for each review by inserting a line before the date.  For example:
    Photo of Conni D.
    Conni D.
    Sun City, AZ
    0
    3
    5 Stars
    Nov 11, 2022

    I was pleased with the level of service and received answers to all my questions. I couldn't be happier with my advisor, Ben M, who made a complicated process as easy to complete as possible.
 11. Save file from text editor.
*
* 
Yelp (Not Recommended) Instructions
 1. Run Firefox browser
 2. Open web page https://www.yelp.com/biz/goldco-calabasas
 3. At bottom of page, click on link "191 other reviews that are not currently recommended" to here:
        https://www.yelp.com/not_recommended_reviews/goldco-calabasas?not_recommended_start=0
 4. Save file as html:  Right-click -> Save Page As...  yelp_not_recommended_goldco_01.htm
 5. At bottom of page, press [Next] button to load https://www.yelp.com/not_recommended_reviews/goldco-calabasas?not_recommended_start=10
 6. Repeat steps 3-4 until all reviews are downloaded.
 7. In text editor, concatenate all resulting .htm files into a single file.
* 
* 
Yellow Pages Instructions
 https://www.yellowpages.com/woodland-hills-ca/mip/goldco-precious-metals-505154425
* 
* */

enum business_review_type{
    BUSINESS_REVIEW_TYPE_BBB,
    BUSINESS_REVIEW_TYPE_CONSUMER_AFFAIRS,
    BUSINESS_REVIEW_TYPE_GOOGLE,
    BUSINESS_REVIEW_TYPE_TRUSTLINK,  /* https://www.trustlink.org/Reviews/Goldco-206527051 */
    BUSINESS_REVIEW_TYPE_TRUSTPILOT,
    BUSINESS_REVIEW_TYPE_YELP, /* https://www.yelp.com/biz/goldco-calabasas */
    BUSINESS_REVIEW_TYPE_YELP_NOT_RECOMMENDED, /* https://www.yelp.com/not_recommended_reviews/goldco-calabasas */

    BUSINESS_REVIEW_TYPE_COUNT
    };

const std::string& business_review_type_to_str(
    const business_review_type& t);


business_review_type parse_file_determine_review_type(
    const std::string& file_name );



struct business_review{
    business_review_type        m_review_type;
    std::string                 m_full_name;
    std::vector<std::string>    m_parsed_name;
    std::string                 m_full_location_name;
    std::string                 m_city;
    std::string                 m_state;
    int                         m_star_count=0;
    std::string                 m_date_str;
    time_t                      m_time_stamp=0;
    time_t                      m_time_stamp_min=0;
    time_t                      m_time_stamp_max=0;
    std::shared_ptr<tm>         m_time_stamp_tm;
    std::string                 m_review_str;
};

typedef std::vector<business_review> business_review_vec;
typedef business_review_vec::const_iterator business_review_vec_citr;
typedef business_review_vec::iterator business_review_vec_itr;

class business_review_analyzer{
public:
    struct input_params{
        std::string m_file_name;
        time_t m_start_time_stamp;
        time_t m_end_time_stamp;
        };
public:
    static int run_main( int argc, char *argv[] );
    static int do_analysis( const input_params& p );
    static void trim_str( std::string *s);
    static void make_lower_str( std::string *s);
    static void split_str( const std::string& s, std::vector<std::string> *v);
    static void split_name( const std::string& s, std::vector<std::string> *v);
    static std::shared_ptr<tm> parse_date_mon_day_year( const std::string& date_str );
    static std::shared_ptr<tm> parse_date_ymd_dash( const std::string& date_str );
    static std::shared_ptr<tm> parse_date_mdy_slash( const std::string& date_str );
    static std::shared_ptr<tm> parse_date_t_time( const std::string& date_str );
private:
    enum bbb_parse_state{
        BBB_PARSE_STATE_SEARCHING_FOR_REVIEW,
        BBB_PARSE_STATE_SEARCHING_FOR_STARS,
        BBB_PARSE_STATE_SEARCHING_FOR_DATE,
        BBB_PARSE_STATE_PARSING_REVIEW_STR,

        BBB_PARSE_STATE_DONE
        };

    enum consumer_affairs_parse_state{
        CONSUMER_AFFAIRS_PARSE_STATE_SEARCHING_FOR_REVIEW,
        CONSUMER_AFFAIRS_PARSE_STATE_SEARCHING_FOR_RATING,
        CONSUMER_AFFAIRS_PARSE_STATE_SEARCHING_FOR_DATE,
        CONSUMER_AFFAIRS_PARSE_STATE_SEARCHING_FOR_REVIEW_STR,
        CONSUMER_AFFAIRS_PARSE_STATE_PARSING_REVIEW_STR,

        CONSUMER_AFFAIRS_PARSE_STATE_DONE
        };

    enum trustlink_parse_state{
        TRUSTLINK_PARSE_STATE_SEARCHING_FOR_REVIEW,
        TRUSTLINK_PARSE_STATE_SEARCHING_FOR_RATING,
        TRUSTLINK_PARSE_STATE_SEARCHING_FOR_DATE,
        TRUSTLINK_PARSE_STATE_SEARCHING_FOR_AUTHOR,
        TRUSTLINK_PARSE_STATE_SEARCHING_FOR_REVIEW_STR,
        TRUSTLINK_PARSE_STATE_PARSING_REVIEW_STR,

        TRUSTLINK_PARSE_STATE_DONE
        };

    enum trustpilot_parse_state{
        TRUSTPILOT_PARSE_STATE_SEARCHING_FOR_REVIEW,
        TRUSTPILOT_PARSE_STATE_SEARCHING_FOR_RATING,
        TRUSTPILOT_PARSE_STATE_SEARCHING_FOR_DATE,
        TRUSTPILOT_PARSE_STATE_PARSING_REVIEW_STR,

        TRUSTPILOT_PARSE_STATE_DONE
        };

    enum yelp_recommended_parse_state{  /* Yelp, recommended reviews */
        YELP_RECOMENDED_PARSE_STATE_SEARCHING_FOR_REVIEW,
        YELP_RECOMENDED_PARSE_STATE_PARSING_LOCATION,
        YELP_RECOMENDED_PARSE_STATE_SEARCHING_FOR_RATING,
        YELP_RECOMENDED_PARSE_STATE_PARSING_DATE,
        YELP_RECOMENDED_PARSE_STATE_PARSING_REVIEW_STR,

        YELP_RECOMENDED_PARSE_STATE_DONE
        };

    enum yelp_nrr_parse_state{  /* Yelp, not-recommended reviews */
        YELP_NRR_PARSE_STATE_SEARCHING_FOR_REVIEW,
        YELP_NRR_PARSE_STATE_SEARCHING_FOR_USER_NAME,
        YELP_NRR_PARSE_STATE_SEARCHING_FOR_LOCATION,
        YELP_NRR_PARSE_STATE_PARSING_LOCATION,
        YELP_NRR_PARSE_STATE_SEARCHING_FOR_RATING,
        YELP_NRR_PARSE_STATE_SEARCHING_FOR_DATE_PRE_TOKEN,
        YELP_NRR_PARSE_STATE_SEARCHING_FOR_DATE,
        YELP_NRR_PARSE_STATE_SEARCHING_FOR_REVIEW_STR,
        YELP_NRR_PARSE_STATE_PARSING_REVIEW_STR,

        YELP_NRR_PARSE_STATE_DONE
        };

    typedef std::map<std::string, size_t> str_szt_map;
    typedef str_szt_map::const_iterator str_szt_map_citr;
    typedef str_szt_map::iterator str_szt_map_itr;
    typedef std::pair<size_t, std::string> szt_str_pair;
    typedef std::vector<szt_str_pair> szt_str_pair_map;

    typedef std::vector<time_t> time_t_vec;
    typedef time_t_vec::const_iterator time_t_vec_citr;
    typedef time_t_vec::iterator time_t_vec_itr;

    typedef std::map<int, time_t_vec> int_tv_map;
    typedef int_tv_map::const_iterator int_tv_map_citr;
    typedef int_tv_map::iterator int_tv_map_itr;

    typedef std::vector<size_t> szt_vec;
    typedef szt_vec::const_iterator szt_vec_citr;
    typedef szt_vec::iterator szt_vec_itr;
private:
    /* input */
    std::string m_file_name;
    time_t m_start_time_stamp;
    time_t m_end_time_stamp;

    /* working data */
    business_review_type m_business_review_type;
    business_review_vec m_reviews;

    /* for each word found in a review, 
    the number of reviews that word is found */
    str_szt_map m_review_str_count_map;
    szt_str_pair_map m_review_count_str_vec;

    /* for each star count (1-5), a sorted vector of review timestamps */
    int_tv_map m_star_tv_map;

    /* count first name letters and last name letters  */
    szt_vec m_first_name_letter_tallies;
    szt_vec m_last_name_letter_tallies;
    
public:
    business_review_analyzer(){}
   ~business_review_analyzer(){}
    void set_file_name(const std::string& file_name){ m_file_name = file_name; }
    void set_start_time_stamp(const time_t& start_time_stamp){ m_start_time_stamp = start_time_stamp; }
    void set_end_time_stamp(const time_t& end_time_stamp){ m_end_time_stamp = end_time_stamp; }
    int execute();
private:
    int reset_working_data();
    int read_review_file_bbb();
    int read_review_file_consumer_affairs();
    int read_review_file_google();
    int read_review_file_trustlink();
    int read_review_file_trustpilot();
    int read_review_file_yelp();
    int read_review_file_yelp_not_recommended();
    int attempt_equalize_timestamp_intervals();
    void sort_reviews_by_timestamp();
    void remove_reviews_violating_yelp_terms_of_service();
    void remove_invalid_reviews();
    void remove_invalid_reviews_outside_time_range();
    void init_city_state_from_full_location_name(business_review *review);
    int init_review_str_count_map();
    int init_review_count_str_vec();
    int init_star_tv_map();
    int write_city_bins();
    int write_first_letter_tallies();
    size_t get_review_count_in_period( const int& star_count,
        const time_t& period_start, const time_t& period_end ) const;
    void write_review_count_table(std::ostream *os) const;
    int write_review_count_table();
    void write_weekday_summary_table(std::ostream *os) const;
    int write_weekday_summary_table();
    void write_weekhour_summary_table(std::ostream *os) const;
    int write_weekhour_summary_table();
    void write_word_counts(std::ostream *os) const;
    int write_word_counts();
    void write_std_word_counts(std::ostream *os) const;
    int write_std_word_counts();
    void write_full_table(std::ostream *os) const;
    int write_full_table();
};


#endif /* #if !defined(BUSINESS_REVIEW_ANALYZER_H) */