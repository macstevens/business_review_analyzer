// business_review_analyzer.cpp :

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

static const std::string& business_review_type_to_str(
    const business_review_type& t){
static const std::string empty_str;
static const std::string bbb_str = "bbb";
static const std::string consumer_affairs_str = "consumer_affairs";
static const std::string google_str = "google";
static const std::string trustlink_str = "trustlink";
static const std::string trustpilot_str = "trustpilot";
static const std::string yelp_str = "yelp";
static const std::string yelp_not_recommended_str = "yelp_not_recommended";

const std::string *result = &empty_str;
switch(t){
    case BUSINESS_REVIEW_TYPE_BBB:
        result = &bbb_str;
        break;
    case BUSINESS_REVIEW_TYPE_CONSUMER_AFFAIRS:
        result = &consumer_affairs_str;
        break;
    case BUSINESS_REVIEW_TYPE_GOOGLE:
        result = &google_str;
        break;
    case BUSINESS_REVIEW_TYPE_TRUSTLINK:
        result = &trustlink_str;
        break;
    case BUSINESS_REVIEW_TYPE_TRUSTPILOT:
        result = &trustpilot_str;
        break;
    case BUSINESS_REVIEW_TYPE_YELP:
        result = &yelp_str;
        break;
    case BUSINESS_REVIEW_TYPE_YELP_NOT_RECOMMENDED:
        result = &yelp_not_recommended_str;
        break;
    case BUSINESS_REVIEW_TYPE_COUNT:
    default:
        result = &empty_str;
        break;
    }
return *result;
}

business_review_type parse_file_determine_review_type(
    const std::string& file_name ){
struct str_type{ const char *m_str; business_review_type m_review_type; };
static const str_type indicator_str_types[] = {

    { "BBB.org",                        BUSINESS_REVIEW_TYPE_BBB },
    { "Review from",                    BUSINESS_REVIEW_TYPE_BBB },
    { "Better Business Bureaus, Inc.",  BUSINESS_REVIEW_TYPE_BBB },
    { "BBB Institute",                  BUSINESS_REVIEW_TYPE_BBB },
    { "BBB rating",                     BUSINESS_REVIEW_TYPE_BBB },

    { "Verified purchase",              BUSINESS_REVIEW_TYPE_CONSUMER_AFFAIRS }, 
    { "Consumers Unified, LLC",         BUSINESS_REVIEW_TYPE_CONSUMER_AFFAIRS }, 
    { "CONSUMERS UNIFIED, LLC",         BUSINESS_REVIEW_TYPE_CONSUMER_AFFAIRS }, 
    { "ConsumerAffairs",                BUSINESS_REVIEW_TYPE_CONSUMER_AFFAIRS }, 

    { "google.com",                     BUSINESS_REVIEW_TYPE_GOOGLE },
    { "</div></div></div>",             BUSINESS_REVIEW_TYPE_GOOGLE }, 

    { "TrustLink",                      BUSINESS_REVIEW_TYPE_TRUSTLINK }, 
    { "www.trustlink.org",              BUSINESS_REVIEW_TYPE_TRUSTLINK }, 
    { "trustlink",                      BUSINESS_REVIEW_TYPE_TRUSTLINK }, 
    { "RatingMed1_Rating1_A",           BUSINESS_REVIEW_TYPE_TRUSTLINK }, 

    { "www.trustpilot.com",             BUSINESS_REVIEW_TYPE_TRUSTPILOT },
    { "cdn.trustpilot.net",             BUSINESS_REVIEW_TYPE_TRUSTPILOT },
    { "rating",                         BUSINESS_REVIEW_TYPE_TRUSTPILOT },
    { "datePublished",                  BUSINESS_REVIEW_TYPE_TRUSTPILOT },

    { "Yelp Inc.",                      BUSINESS_REVIEW_TYPE_YELP },
    { "About Yelp",                     BUSINESS_REVIEW_TYPE_YELP },
    { "Photo of",                       BUSINESS_REVIEW_TYPE_YELP },

    { "%20-%20Yelp",                    BUSINESS_REVIEW_TYPE_YELP_NOT_RECOMMENDED },
    { "Not%20Recommended%20Reviews",    BUSINESS_REVIEW_TYPE_YELP_NOT_RECOMMENDED },
};
static const size_t indicator_str_type_count =
    sizeof(indicator_str_types)/sizeof(indicator_str_types[0]);


std::vector<size_t> indicator_count_vec(BUSINESS_REVIEW_TYPE_COUNT, 0);
std::ifstream ifs(file_name.c_str());
while(ifs.good()){
    std::string line;
    std::getline(ifs,line);
    for( size_t i = 0; i < indicator_str_type_count; ++i ){
        const str_type& indicator_str_type = indicator_str_types[i];
        const char *indicator_str = indicator_str_type.m_str;
        business_review_type t = indicator_str_type.m_review_type;
        if(line.find(indicator_str) != std::string::npos){
            ++(indicator_count_vec.at(static_cast<size_t>(t)));
            }
        }
    }

business_review_type result = BUSINESS_REVIEW_TYPE_COUNT;
size_t result_indicator_count = 0;
for( size_t j = 0; j < static_cast<size_t>(BUSINESS_REVIEW_TYPE_COUNT); ++j ){
    const size_t count = indicator_count_vec.at(j);
    if( count > result_indicator_count ){
        result = static_cast<business_review_type>(j);
        result_indicator_count = count;
        }
    }

return result;
}


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
    int init_city_bins();
    int init_first_letter_tallies();
    size_t get_review_count_in_period( const int& star_count,
        const time_t& period_start, const time_t& period_end ) const;
    void write_review_count_table(std::ostream *os) const;
    int write_review_count_table();
    void write_weekday_summary_table(std::ostream *os) const;
    int write_weekday_summary_table();
    void write_weekhour_summary_table(std::ostream *os) const;
    int write_weekhour_summary_table();
    void write_full_table(std::ostream *os) const;
    int write_full_table();
};



int business_review_analyzer::run_main( int argc, char *argv[] )
{
static const std::string start_date_tag = "--start-date-time";
static const std::string end_date_tag = "--end-date-time";
int err_cnt = 0;
std::string token;
if( argc > 1 ){
    time_t start_time_stamp = 0;
    time_t end_time_stamp = time(nullptr);
    std::vector<std::string> file_names;
    int arg_idx = 1;
    while( arg_idx < argc ){
        token.assign(argv[arg_idx]);
        const bool is_start_date_tag = ( token == start_date_tag );
        const bool is_end_date_tag = ( token == end_date_tag );
        if( is_start_date_tag || is_end_date_tag ){
            ++arg_idx;
            if( arg_idx < argc ){
                token.assign(argv[arg_idx]);
                std::shared_ptr<tm> tm_ptr = parse_date_t_time(token);
                if( (nullptr != tm_ptr.get()) && (tm_ptr->tm_year > 0) ){
                    const time_t t = mktime(tm_ptr.get());
                    if( is_start_date_tag ){
                        start_time_stamp = t;
                        }
                    else{
                        end_time_stamp = t;
                        }
                    }
                ++arg_idx;
                }
            }
        else{
            file_names.push_back(token);
            ++arg_idx;
            }
        }

    business_review_analyzer::input_params p;
    p.m_start_time_stamp = start_time_stamp;
    p.m_end_time_stamp = end_time_stamp;
    std::vector<std::string>::const_iterator file_name_itr = file_names.begin();
    for( ; file_names.end() != file_name_itr; ++file_name_itr ){
        const std::string& file_name = *file_name_itr;
        p.m_file_name = file_name;
        err_cnt += do_analysis( p );
        }
    }
return err_cnt;
}


int business_review_analyzer::do_analysis( const input_params& p ){

struct tm *start_tm = localtime( &(p.m_start_time_stamp) );
char start_time_buf[64];
strftime(start_time_buf, 64, "%FT%T", start_tm);

struct tm *end_tm = localtime( &(p.m_end_time_stamp) );
char end_time_buf[64];
strftime(end_time_buf, 64, "%FT%T", start_tm);

std::cout << "Analyze Business Reviews"
    << "   file:" << p.m_file_name 
    << "   start_time:" << start_time_buf 
    << "   end_time:" << end_time_buf 
    << "\n";
int err_cnt = 0;
business_review_analyzer a;
a.set_file_name(p.m_file_name);
a.set_start_time_stamp(p.m_start_time_stamp);
a.set_end_time_stamp(p.m_end_time_stamp);
err_cnt += a.execute();
return err_cnt;
}


void business_review_analyzer::trim_str( std::string *s){
static const std::string whitespace = " \t\f\v\r\n";
if( nullptr != s ){
    const std::string::size_type non_ws_start_pos = s->find_first_not_of(whitespace);
    if( std::string::npos != non_ws_start_pos ){
        s->erase(0,non_ws_start_pos);
        const std::string::size_type non_ws_final_pos = s->find_last_not_of(whitespace);
        if( std::string::npos != non_ws_final_pos ){
            s->resize(non_ws_final_pos+1);
            }
        }
    }
}


void business_review_analyzer::make_lower_str( std::string *s){
if(nullptr != s){
    std::string::iterator ch_itr = s->begin();
    for(; s->end() != ch_itr; ++ch_itr){
        if(((*ch_itr) >= 'A') && ((*ch_itr) <= 'Z')){
            (*ch_itr) += ('a' - 'A');
            }
        }
    }
}


void business_review_analyzer::split_str( const std::string& s,
    std::vector<std::string> *v){
std::string::const_iterator ch_itr = s.begin();
enum split_str_parse_mode{
    SSPM_SEEKING_TOKEN,
    SSPM_PARSING_TOKEN
    };

split_str_parse_mode mode = SSPM_SEEKING_TOKEN;
for(; s.end() != ch_itr; ++ch_itr ){
    const char& ch = *ch_itr;
    if ( (ch > 0) && isspace(ch) ){
        mode = SSPM_SEEKING_TOKEN;
        }
    else if( ('!' == ch) || (',' == ch ) || ('.' == ch ) || ('$' == ch ) ){
        v->push_back(std::string(1,ch));
        mode = SSPM_SEEKING_TOKEN;
        }
    else{
        switch(mode){
            case SSPM_SEEKING_TOKEN:
                v->push_back(std::string(1,ch));  
                mode = SSPM_PARSING_TOKEN;              
                break;
            case SSPM_PARSING_TOKEN:
                (v->back()).append(1, ch);
                break;            
            }
        }
    }
}

void business_review_analyzer::split_name( const std::string& s,
    std::vector<std::string> *v){
std::string name = s;
std::string::iterator name_char_itr = name.begin();
for(; name.end() != name_char_itr; ++name_char_itr){
    const char& name_char = *name_char_itr;
    if ((name_char < 0) || (name_char >= 127) || !isalpha(name_char)) {
        *name_char_itr = ' ';
        }
    }
v->clear();
std::vector<std::string> split_s_vec;
split_str(name, &split_s_vec);
switch( split_s_vec.size() ){
    case 0:
        *v = split_s_vec;
        break;
    case 1:
        v->push_back( split_s_vec.front() );
        v->push_back( std::string() );
        v->push_back( std::string() );
        break;
    case 2:
        v->push_back( split_s_vec.front() );
        v->push_back( std::string() );
        v->push_back( split_s_vec.back() );
        break;
    case 3:
    default:
        v->push_back( split_s_vec.front() );
        v->push_back( split_s_vec.at(1) );
        v->push_back( split_s_vec.back() );
        break;
    }
}
/*
Jan 6, 2024
*/
std::shared_ptr<tm> business_review_analyzer::parse_date_mon_day_year(
    const std::string& date_str ){

std::string date_str_trimmed = date_str;
trim_str(&date_str_trimmed);

/* replace punctuation with whitespace */
static const std::string punctuation = ",./;:[]{}-_!@#$%^&*()";
std::string::size_type punctuation_pos = date_str_trimmed.find_first_of(punctuation);
while( std::string::npos != punctuation_pos ){
    date_str_trimmed.replace(punctuation_pos, 1, " " );
    punctuation_pos = date_str_trimmed.find_first_of(punctuation);
    }

/* split into month/day/year */
std::vector<std::string> v;
split_str( date_str_trimmed, &v);
static const std::string empty_str;
std::string month_str = (v.size() > 0) ? v.at(0) : empty_str;
trim_str(&month_str);
std::string day_str = (v.size() > 1) ? v.at(1) : empty_str;
trim_str(&day_str);
std::string year_str = (v.size() > 2) ? v.at(2) : empty_str;
trim_str(&year_str);

/* month */
static std::map<std::string, int> month_int_map;
if(month_int_map.empty()){
    month_int_map["jan"] = 1;
    month_int_map["feb"] = 2;
    month_int_map["mar"] = 3;
    month_int_map["apr"] = 4;
    month_int_map["may"] = 5;
    month_int_map["jun"] = 6;
    month_int_map["jul"] = 7;
    month_int_map["aug"] = 8;
    month_int_map["sep"] = 9;
    month_int_map["oct"] = 10;
    month_int_map["nov"] = 11;
    month_int_map["dec"] = 12;
    }
std::string month3_str = month_str;
month3_str.resize(3);
make_lower_str(&month3_str);
const std::map<std::string, int>::const_iterator
    month_int_map_itr = month_int_map.find(month3_str);
const int month_int = (month_int_map.end() == month_int_map_itr) ?
    1 : month_int_map_itr->second;

/* day */
const int day_int = atoi(day_str.c_str());

/* year */
const int year_int = atoi(year_str.c_str());

/* output */
std::shared_ptr<tm> result(new struct tm);
tm *tms = result.get();
memset( tms, 0, sizeof(tm));
result->tm_year = year_int - 1900;
result->tm_mon =  month_int-1;
result->tm_mday = day_int;
result->tm_hour = 12;
result->tm_min = 0;
result->tm_sec = 0;

return result;
}

/*
2019-10-29
*/
std::shared_ptr<tm> business_review_analyzer::parse_date_ymd_dash(
    const std::string& date_str ){
const std::string::size_type first_dash_pos = date_str.find("-");
const std::string::size_type second_dash_pos =
    (std::string::npos == first_dash_pos ) ? std::string::npos :
    date_str.find( "-", first_dash_pos + 1 );

static const std::string empty_str;
const std::string year_str =  
    (std::string::npos == first_dash_pos ) ? empty_str:
    date_str.substr(0,first_dash_pos);
const std::string month_str =  
    (std::string::npos == second_dash_pos ) ? empty_str:
    date_str.substr(first_dash_pos + 1,(second_dash_pos - first_dash_pos)-1);
const std::string day_str = date_str.substr(second_dash_pos + 1);

const int year_int = atoi(year_str.c_str());
const int month_int = atoi(month_str.c_str());
const int day_int = atoi(day_str.c_str());
const int hour_int = 0;
const int minute_int = 0;
const int second_int = 0;


/* output */
std::shared_ptr<tm> result(new struct tm);
tm *tms = result.get();
memset( tms, 0, sizeof(tm));
result->tm_year = year_int - 1900;
result->tm_mon =  month_int-1;
result->tm_mday = day_int;
result->tm_hour = hour_int;
result->tm_min = minute_int;
result->tm_sec = second_int;

if( result->tm_year < 0 ){
    std::string break_point_str;
    }

return result;
}


/*
2/15/2023
*/
std::shared_ptr<tm> business_review_analyzer::parse_date_mdy_slash( const std::string& date_str ){
const std::string::size_type first_slash_pos = date_str.find("/");
const std::string::size_type second_slash_pos =
    (std::string::npos == first_slash_pos ) ? std::string::npos :
    date_str.find( "/", first_slash_pos + 1 );

static const std::string empty_str;
const std::string month_str =  
    (std::string::npos == first_slash_pos ) ? empty_str:
    date_str.substr(0,first_slash_pos);
const std::string day_str =  
    (std::string::npos == second_slash_pos ) ? empty_str:
    date_str.substr(first_slash_pos + 1,(second_slash_pos - first_slash_pos)-1);
const std::string year_str = date_str.substr(second_slash_pos + 1);

const int year_int = atoi(year_str.c_str());
const int month_int = atoi(month_str.c_str());
const int day_int = atoi(day_str.c_str());
const int hour_int = 12;
const int minute_int = 0;
const int second_int = 0;


/* output */
std::shared_ptr<tm> result(new struct tm);
tm *tms = result.get();
memset( tms, 0, sizeof(tm));
result->tm_year = year_int - 1900;
result->tm_mon =  month_int-1;
result->tm_mday = day_int;
result->tm_hour = hour_int;
result->tm_min = minute_int;
result->tm_sec = second_int;

if( result->tm_year < 0 ){
    std::string break_point_str;
    }

return result;
}


/*
2019-10-29T13:39:37.000Z
*/
std::shared_ptr<tm> business_review_analyzer::parse_date_t_time(
    const std::string& date_str ){

const std::string::size_type first_dash_pos = date_str.find("-");
const std::string::size_type second_dash_pos =
    (std::string::npos == first_dash_pos ) ? std::string::npos :
    date_str.find( "-", first_dash_pos + 1 );
const std::string::size_type capital_t_pos =
    (std::string::npos == second_dash_pos ) ? std::string::npos :
    date_str.find( "T", second_dash_pos + 1 );
const std::string::size_type first_colon_pos =
    (std::string::npos == capital_t_pos ) ? std::string::npos :
    date_str.find( ":", capital_t_pos + 1 );
const std::string::size_type second_colon_pos =
    (std::string::npos == first_colon_pos ) ? std::string::npos :
    date_str.find( ":", first_colon_pos + 1 );
const std::string::size_type period_pos =
    (std::string::npos == second_colon_pos ) ? std::string::npos :
    date_str.find( ".", second_colon_pos + 1 );
const std::string::size_type z_pos =
    (std::string::npos == period_pos ) ? std::string::npos :
    date_str.find( "Z", period_pos + 1 );

static const std::string empty_str;
const std::string year_str =  
    (std::string::npos == first_dash_pos ) ? empty_str:
    date_str.substr(0,first_dash_pos);
const std::string month_str =  
    (std::string::npos == second_dash_pos ) ? empty_str:
    date_str.substr(first_dash_pos + 1,(second_dash_pos - first_dash_pos)-1);
const std::string day_str =  
    (std::string::npos == capital_t_pos ) ? 
    ((std::string::npos == second_dash_pos ) ? empty_str : date_str.substr(second_dash_pos + 1)) :
    date_str.substr(second_dash_pos + 1,(capital_t_pos - second_dash_pos)-1);
const std::string hour_str =  
    (std::string::npos == first_colon_pos ) ? empty_str:
    date_str.substr(capital_t_pos + 1,(first_colon_pos - capital_t_pos)-1);
const std::string minute_str =  
    (std::string::npos == second_colon_pos ) ? empty_str:
    date_str.substr(first_colon_pos + 1,(second_colon_pos - first_colon_pos)-1);
const std::string second_str =  
    (std::string::npos == period_pos ) ? 
    ((std::string::npos == second_colon_pos ) ? empty_str : date_str.substr(second_colon_pos + 1)) :
    date_str.substr(second_colon_pos + 1,(period_pos - second_colon_pos)-1);
const std::string millisecond_str =  
    (std::string::npos == z_pos ) ? empty_str:
    date_str.substr(period_pos + 1,(z_pos - period_pos)-1);

const int year_int = atoi(year_str.c_str());
const int month_int = atoi(month_str.c_str());
const int day_int = atoi(day_str.c_str());
const int hour_int = atoi(hour_str.c_str());
const int minute_int = atoi(minute_str.c_str());
const int second_int = atoi(second_str.c_str());
const int millisecond_int = atoi(millisecond_str.c_str());


/* output */
std::shared_ptr<tm> result(new struct tm);
tm *tms = result.get();
memset( tms, 0, sizeof(tm));
result->tm_year = year_int - 1900;
result->tm_mon =  month_int-1;
result->tm_mday = day_int;
result->tm_hour = hour_int;
result->tm_min = minute_int;
result->tm_sec = second_int;

if( result->tm_year < 0 ){
    std::string break_point_str;
    }

return result;
}

int business_review_analyzer::execute(){
int err_cnt = 0;

err_cnt += reset_working_data();

m_business_review_type = parse_file_determine_review_type( m_file_name );
switch(m_business_review_type){
    case BUSINESS_REVIEW_TYPE_BBB:
        err_cnt += read_review_file_bbb();
        break;
    case BUSINESS_REVIEW_TYPE_CONSUMER_AFFAIRS:
        err_cnt += read_review_file_consumer_affairs();
        break;
    case BUSINESS_REVIEW_TYPE_GOOGLE:
        err_cnt += read_review_file_google();
        break;
    case BUSINESS_REVIEW_TYPE_TRUSTLINK:
        err_cnt += read_review_file_trustlink();
        break;
    case BUSINESS_REVIEW_TYPE_TRUSTPILOT:
        err_cnt += read_review_file_trustpilot();
        break;
    case BUSINESS_REVIEW_TYPE_YELP:
        err_cnt += read_review_file_yelp();
        break;
    case BUSINESS_REVIEW_TYPE_YELP_NOT_RECOMMENDED:
        err_cnt += read_review_file_yelp_not_recommended();
        break;

    case BUSINESS_REVIEW_TYPE_COUNT:
    default:
        break;
    }

remove_invalid_reviews_outside_time_range();

std::cout << "(within time range) review_count=" << m_reviews.size() << "\n";


err_cnt += init_review_str_count_map();
err_cnt += init_review_count_str_vec();
err_cnt += init_star_tv_map();
err_cnt += init_city_bins();
err_cnt += init_first_letter_tallies();
err_cnt += write_review_count_table();
err_cnt += write_weekday_summary_table();
err_cnt += write_weekhour_summary_table();
err_cnt += write_full_table();

return err_cnt;
}

int business_review_analyzer::reset_working_data(){
int err_cnt = 0;

m_business_review_type = BUSINESS_REVIEW_TYPE_COUNT;
m_reviews.clear();

m_review_str_count_map.clear();
m_review_count_str_vec.clear();

m_star_tv_map.clear();

return err_cnt;
}

int business_review_analyzer::read_review_file_bbb(){
int err_cnt = 0;

size_t line_count = 0;
std::ifstream ifs(m_file_name.c_str());

bbb_parse_state parse_state = BBB_PARSE_STATE_SEARCHING_FOR_REVIEW;
business_review *review = nullptr;
while(ifs.good()){
    std::string line;
    std::getline(ifs,line);
    ++line_count;

    const std::string::size_type start_non_ws_pos =
        line.find_first_not_of( " \n\t\v\f" );
    const std::string::size_type review_from_pos =
        line.find( "Review from" );
    const std::string::size_type stars_pos =
        line.find( "star" );
    const std::string::size_type first_digit_pos =
        line.find_first_of( "0123456789" );
    const std::string::size_type slash_pos =
        line.find( "/" );

    /* start new review */
    if( ( start_non_ws_pos != std::string::npos ) &&
        ( start_non_ws_pos == review_from_pos ) ){
        m_reviews.push_back(business_review());
        review = &(m_reviews.back());
        review->m_review_type = BUSINESS_REVIEW_TYPE_BBB;
        review->m_full_name = line.substr(review_from_pos + strlen("Review from"));
        split_name(review->m_full_name, &(review->m_parsed_name) );
        parse_state = BBB_PARSE_STATE_SEARCHING_FOR_STARS;
        }

    switch(parse_state){
        case BBB_PARSE_STATE_SEARCHING_FOR_REVIEW:
            /* skip */
            break;
        case BBB_PARSE_STATE_SEARCHING_FOR_STARS:
            if( stars_pos != std::string::npos ){
                const char star_count_ch = line.at(start_non_ws_pos);
                review->m_star_count = static_cast<int>(star_count_ch - '0');
                parse_state = BBB_PARSE_STATE_SEARCHING_FOR_DATE;                
                }
            break;
        case BBB_PARSE_STATE_SEARCHING_FOR_DATE:
            if( ( slash_pos != std::string::npos ) &&
                ( first_digit_pos != std::string::npos ) ){
                review->m_date_str = line.substr(first_digit_pos);
                const std::string month_str = (review->m_date_str).substr(0,2);
                const std::string day_str = (review->m_date_str).substr(3,2);
                const std::string year_str = (review->m_date_str).substr(6,4);
                const int month_int = atoi(month_str.c_str());
                const int day_int = atoi(day_str.c_str());
                const int year_int = atoi(year_str.c_str());
                (review->m_time_stamp_tm).reset( new tm );
                tm *tms = (review->m_time_stamp_tm).get();
                memset( tms, 0, sizeof(tm));
                tms->tm_year = year_int - 1900;
                tms->tm_mon =  month_int-1;
                tms->tm_mday = day_int;
                tms->tm_hour = 12;
                tms->tm_min = 0;
                tms->tm_sec = 0;
                review->m_time_stamp = mktime(tms);
                static const time_t time_err_seconds = 12*60*60;
                review->m_time_stamp_min = (review->m_time_stamp > time_err_seconds) ?
                    (review->m_time_stamp - time_err_seconds) : 0;
                review->m_time_stamp_max = review->m_time_stamp + time_err_seconds;
                struct tm *tms2 = localtime( &(review->m_time_stamp) );
                parse_state = BBB_PARSE_STATE_PARSING_REVIEW_STR;                
                }
            break;
        case BBB_PARSE_STATE_PARSING_REVIEW_STR:
            if( std::string::npos != start_non_ws_pos ){
                if( !(review->m_review_str).empty() ){
                    (review->m_review_str).append("\t");
                    }
                (review->m_review_str).append(line);
                }
            break;
        case BBB_PARSE_STATE_DONE:
            break;
        }
    }
std::cout << "total review_count=" << m_reviews.size() << "\n";
std::cout << "line_count=" << line_count << "\n";

sort_reviews_by_timestamp();

return err_cnt;
}

/*
Consumer Affairs Review

<span class="rvw__inf-nm" itemprop="name">Wayne</span>
<span class="rvw__inf-lctn">Durango, CO</span>
<meta itemprop="ratingValue" content="5">
Reviewed April 12, 2023

<div class="rvw__bd"><div class="rvw__top-text"><p>We 
were looking to roll over our 401K into a precious metal IRA. We chose 
GoldCo from a recommendation by Shari Raye on her podcast link, **. Upon
 filling out the information online, we were immediately called, by a 
very pleasant person, to discuss our plans. We were connected to Robert,
 who became our account executive, and he informed us about GoldCo, and 
the process of rolling over our 401K into a precious metal Traditional 
IRA. Robert was so courteous, patient, informative and helpful. He made 
the process go smoothly, kept in touch with us and guided us through all
 the paperwork and procedures. We really liked how everyone at GoldCo 
was extremely courteous and helpful. We felt relaxed and confident 
dealing with GoldCo and our investment with them. We are even using 
GoldCo with more investment of purchasing precious metals with our 
savings. Thank you to Robert and GoldCo for making the process smooth 
&amp; easy.</p></div></div>
*/
int business_review_analyzer::read_review_file_consumer_affairs(){
int err_cnt = 0;

size_t line_count = 0;
std::ifstream ifs(m_file_name.c_str());

consumer_affairs_parse_state parse_state = CONSUMER_AFFAIRS_PARSE_STATE_SEARCHING_FOR_REVIEW;
business_review *review = nullptr;
while(ifs.good()){
    std::string line;
    std::getline(ifs,line);
    ++line_count;

    static const std::string whitespace = " \n\t\v\f";
    static const std::string digits = "0123456789";

    static const std::string paragraph_open_tag = "<p>";
    static const std::string paragraph_close_tag = "</p>";    

    static const std::string name_open_tag = "<span class=\"rvw__inf-nm\" itemprop=\"name\">";
    static const std::string name_close_tag = "</span>";

    static const std::string location_open_tag = "<span class=\"rvw__inf-lctn\">";
    static const std::string location_close_tag = "</span>";

    static const std::string rating_open_tag = "<meta itemprop=\"ratingValue\" content=\"";
    static const std::string rating_close_tag = ">";

    static const std::string reviewed_tag = "Reviewed";

    static const std::string review_open_tag = "<div class=\"rvw__top-text\">";
    static const std::string review_close_tag = "</div>";


    const std::string::size_type start_non_ws_pos =
        line.find_first_not_of( whitespace );
    const std::string::size_type first_digit_pos =
        line.find_first_of( "0123456789" );

    const std::string::size_type paragraph_open_pos =
        line.find( paragraph_open_tag );
    const std::string::size_type paragraph_close_pos =
        line.find( paragraph_close_tag );

    const std::string::size_type name_open_pos =
        line.find( name_open_tag );
    const std::string::size_type name_close_pos =
        ( std::string::npos == name_open_pos ) ?
        std::string::npos : line.find( name_close_tag, name_open_pos );

    const std::string::size_type location_open_pos =
        line.find( location_open_tag );
    const std::string::size_type location_close_pos =
        ( std::string::npos == location_open_pos ) ?
        std::string::npos : line.find( location_close_tag, location_open_pos );

    const std::string::size_type rating_open_pos =
        line.find( rating_open_tag );
    const std::string::size_type rating_close_pos =
        ( std::string::npos == rating_open_pos ) ?
        std::string::npos : line.find( rating_close_tag, rating_open_pos );

    const std::string::size_type reviewed_pos =
        line.find( reviewed_tag );

    const std::string::size_type review_open_pos =
        line.find( review_open_tag );
    const std::string::size_type review_close_pos =
        line.find( review_close_tag );

    /* start new review */
    if( std::string::npos != name_open_pos ){
        m_reviews.push_back(business_review());
        review = &(m_reviews.back());
        review->m_review_type = BUSINESS_REVIEW_TYPE_CONSUMER_AFFAIRS;
        if( ( name_close_pos > name_open_pos ) &&
            ( std::string::npos != name_close_pos ) ){
            const size_t name_start_pos = name_open_pos + name_open_tag.length();
            const size_t name_len = ( name_start_pos < name_close_pos ) ?
                (name_close_pos - name_start_pos) : 0;
            review->m_full_name = line.substr(name_start_pos, name_len);
            split_name(review->m_full_name, &(review->m_parsed_name) );
            }

        if( ( location_close_pos > location_open_pos ) &&
            ( std::string::npos != location_close_pos ) ){
            const size_t location_start_pos = location_open_pos + location_open_tag.length();
            const size_t location_len = ( location_start_pos < location_close_pos ) ?
                (location_close_pos - location_start_pos) : 0;
            review->m_full_location_name = line.substr(location_start_pos, location_len);
            init_city_state_from_full_location_name(review);
            }
        parse_state = CONSUMER_AFFAIRS_PARSE_STATE_SEARCHING_FOR_RATING;
        }

    switch(parse_state){
        case CONSUMER_AFFAIRS_PARSE_STATE_SEARCHING_FOR_REVIEW:
            /* skip */
            break;
        case CONSUMER_AFFAIRS_PARSE_STATE_SEARCHING_FOR_RATING:
            if( ( rating_close_pos > rating_open_pos ) &&
                ( std::string::npos != rating_close_pos ) ){
                const size_t rating_start_pos = rating_open_pos + rating_open_tag.length();
                const size_t rating_len = ( rating_start_pos < rating_close_pos ) ?
                    (rating_close_pos - rating_start_pos) : 0;
                const std::string rating_str = line.substr(rating_start_pos, rating_len);
                review->m_star_count = (rating_str.empty()) ? 
                    0 : static_cast<int>(rating_str.at(0) - '0');
                parse_state = CONSUMER_AFFAIRS_PARSE_STATE_SEARCHING_FOR_DATE;  
                }
            break;
        case CONSUMER_AFFAIRS_PARSE_STATE_SEARCHING_FOR_DATE:
            if( reviewed_pos != std::string::npos ){
                review->m_date_str = line.substr(reviewed_pos + reviewed_tag.length());
                review->m_time_stamp_tm = parse_date_mon_day_year(review->m_date_str);
                review->m_time_stamp = mktime((review->m_time_stamp_tm).get());
                static const time_t time_err_seconds = 12*60*60;
                review->m_time_stamp_min = (review->m_time_stamp > time_err_seconds) ?
                    (review->m_time_stamp - time_err_seconds) : 0;
                review->m_time_stamp_max = review->m_time_stamp + time_err_seconds;
                struct tm *tms2 = localtime( &(review->m_time_stamp) );
                parse_state = CONSUMER_AFFAIRS_PARSE_STATE_SEARCHING_FOR_REVIEW_STR;                
                }
            break;
        case CONSUMER_AFFAIRS_PARSE_STATE_SEARCHING_FOR_REVIEW_STR:
            if( std::string::npos != review_open_pos ){
                std::string::size_type review_start_pos =
                    review_open_pos + review_open_tag.length();
                if( ( std::string::npos != paragraph_open_pos ) &&
                    (paragraph_open_pos >= review_start_pos ) ){
                    review_start_pos = paragraph_open_pos + paragraph_open_tag.length();
                    }
                if( ( std::string::npos != paragraph_close_pos ) &&
                    ( paragraph_close_pos > review_start_pos ) ){
                    review->m_review_str = line.substr(review_start_pos,
                        paragraph_close_pos-review_start_pos);
                    }
                else{
                    review->m_review_str = line.substr(review_start_pos);
                    }
                parse_state = CONSUMER_AFFAIRS_PARSE_STATE_PARSING_REVIEW_STR;
                }
            break;
        case CONSUMER_AFFAIRS_PARSE_STATE_PARSING_REVIEW_STR:
            if( std::string::npos != start_non_ws_pos ){
                if( !(review->m_review_str).empty() ){
                    (review->m_review_str).append(" ");
                    }
                if( std::string::npos != paragraph_close_pos ){
                    const std::string review_end = line.substr(0, paragraph_close_pos);
                    review->m_review_str.append(review_end);
                    parse_state = CONSUMER_AFFAIRS_PARSE_STATE_SEARCHING_FOR_REVIEW;
                    }
                else{
                    (review->m_review_str).append(line);
                    }                
                }
            break;
        case CONSUMER_AFFAIRS_PARSE_STATE_DONE:
            break;
        }
    }
std::cout << "total review_count=" << m_reviews.size() << "\n";
std::cout << "line_count=" << line_count << "\n";

sort_reviews_by_timestamp();

return err_cnt;
}


/*
<reference_time>2024-04-03T20:00:00.000Z</reference_time>  <! download time = 2024/04/03 12:00 Pacific >

<script nonce="">(function(){var id='sjsuid_0';var index= 0 ;var offset= 0 ;var is_rtl= false ;var is_gecko= true ;var is_edge= false ;var init='cVrhhd';window[init](id,index,offset,is_rtl,is_gecko,is_edge);})();</script></g-scrolling-carousel></div><div class="aSzfg" jsaction="rcuQ6b:SFZoWd;zTZidf:NvbH5c;avTecb:bNdO0d;SM5Mre:AlfTDf;uQ7ONd:svVOG;ZdRM0c:PvLyDb;xXNcSc:U7Uzve;rGIWO:PuIsic;KsyVGf:PuIsic" jscontroller="AdYPFb" jsdata="TmTvS;_;$73"><div jsname="GmP9w" data-ved="0CCMQxyxqFwoTCKC6w6OXwIUDFQAAAAAdAAAAABAC">

<div class="bwb7ce" jsname="ShBeI" data-id="ChZDSUhNMG9nS0VJQ0FnSUREd19Uak1nEAE" jsdata="aQic9b;ChZDSUhNMG9nS0VJQ0FnSUREd19Uak1nEAE;$74" jscontroller="KUoFuf" data-hveid="2">

<div class="p7f0Nd"><a class="yC3ZMb" href="https://www.google.com/maps/contrib/117671072910842220541/reviews" jscontroller="t7jjzb" jsaction="mousedown:VacWff; touchstart:VacWff" data-redirect="/url?sa=t&amp;source=web&amp;rct=j&amp;url=https%3A%2F%2Fwww.google.com%2Fmaps%2Fcontrib%2F117671072910842220541%2Freviews&amp;ved=0CAMQzsEIahcKEwigusOjl8CFAxUAAAAAHQAAAAAQCQ&amp;opi=89978449">

<div class="wSokxc" role="img" style="background-image: url(&quot;https://lh3.googleusercontent.com/a/ACg8ocJXlAyEkgfSiKAtrxtpD_Ad78Ac214H7oK3JIq3CmuEaAr5FA=s64-c-rp-mo-br100&quot;);" aria-label="Connie Morgan">
<div class="ANWksb"></div></div>

<div><div class="Vpc5Fe">Connie Morgan</div><div class="GSM50">1 review</div></div>

</a><div style="display: flex;"><span jsaction="FzgWvd:Uzli5;OPLoIc:d2CJEf" jscontroller="bMNOfe"><div jsaction="JIbuQc:aj0Jcf(WjL7X); keydown:uYT2Vb(WjL7X);xDliB:oNPcuf;SM8mFd:li9Srb;iFFCZc:NSsOUb;Rld2oe:NSsOUb" jsshadow="" class="ekiqse a03r7b" jscontroller="wg1P6b"><div jsname="WjL7X" jsslot=""><button class="CeiTH yHy1rc eT1oJ mN1ivc" jscontroller="soHxf" jsaction="click:cOuCgd; mousedown:UX7yZ; mouseup:lbsD7e; mouseenter:tfO1Yc; mouseleave:JywGue; touchstart:p6p2H; touchmove:FwuNnf; touchend:yfqBxc; touchcancel:JMtRjd; focus:AHmuwe; blur:O22p3e; contextmenu:mg9Pef;mlnRJb:fLiPzd" data-idom-class="yHy1rc eT1oJ mN1ivc" aria-label="Review options for review by Connie Morgan"><div jsname="s3Eaab" class="ti8Tdd"></div><div class="z59NDd"></div><svg focusable="false" width="24" height="24" viewBox="0 0 24 24" class=" NMm5M"><path d="M12 8c1.1 0 2-.9 2-2s-.9-2-2-2-2 .9-2 2 .9 2 2 2zm0 2c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2zm0 6c-1.1 0-2 .9-2 2s.9 2 2 2 2-.9 2-2-.9-2-2-2z"></path></svg></button></div>
<div jsname="U0exHf" jsslot=""> <div class="Xmdwhc GSujlf q6oraf P77izf gqY1ne htHWJd" jscontroller="ywOR5c" jsaction="keydown:I481le;JIbuQc:j697N(rymPhb);XVaHYd:c9v4Fb(rymPhb);Oyo5M:b5fzT(rymPhb);DimkCe:TQSy7b(rymPhb);m0LGSd:fAWgXe(rymPhb);WAiFGd:kVJJuc(rymPhb)" data-is-hoisted="false" data-should-flip-corner-horizontally="true" data-stay-in-viewport="false" data-menu-uid="ucc-20"><ul class="XsNhfd DMZ54e NR1SI htHWJd" jsname="rymPhb" jscontroller="PHUIyb" jsaction="mouseleave:JywGue; touchcancel:JMtRjd; focus:AHmuwe; blur:O22p3e; keydown:I481le" role="menu" tabindex="-1" aria-label="Review options for review by Connie Morgan" data-disable-idom="true"><span aria-hidden="true" class="u31sFf NZp2ef"></span><a class="KgoSse" role="menuitem" href="https://www.google.com/local/review/rap/report?postId=ChZDSUhNMG9nS0VJQ0FnSUREd19Uak1nEAE&amp;entityid=ChZDSUhNMG9nS0VJQ0FnSUREd19Uak1nEi4KF0NJSE0wb2dLRUlDQWdJRER3X1Rqc2dFEhNDZ3dJa0p6cXNBWVE4TlhnckFFGi0KFkNJSE0wb2dLRUlDQWdJRER3X1RqY2cSE0Nnd0lrSnpxc0FZUThOWGdyQUUiEgkdda6OP5nCgBFaBBKPOVVmKyoTQ2d3SWtKenFzQVlROE5YZ3JBRQ&amp;wv=1&amp;d=286732336&amp;gsas=1">Report review</a></ul></div></div></div></span></div></div>

<div class="k0Ysuc"><div class="dHX2k " role="img" aria-label="Rated 5.0 out of 5"><svg focusable="false" width="12" height="12" viewBox="0 0 12 12" class="ePMStd NMm5M"><path d="M6 .6L2.6 11.1 11.4 4.7H.6L9.4 11.1Z" fill="#fabb05" stroke="#fabb05" stroke-linejoin="round" stroke-width="1"></path></svg>
<svg focusable="false" width="12" height="12" viewBox="0 0 12 12" class="ePMStd NMm5M"><path d="M6 .6L2.6 11.1 11.4 4.7H.6L9.4 11.1Z" fill="#fabb05" stroke="#fabb05" stroke-linejoin="round" stroke-width="1"></path></svg><svg focusable="false" width="12" height="12" viewBox="0 0 12 12" class="ePMStd NMm5M"><path d="M6 .6L2.6 11.1 11.4 4.7H.6L9.4 11.1Z" fill="#fabb05" stroke="#fabb05" stroke-linejoin="round" stroke-width="1"></path></svg><svg focusable="false" width="12" height="12" viewBox="0 0 12 12" class="ePMStd NMm5M"><path d="M6 .6L2.6 11.1 11.4 4.7H.6L9.4 11.1Z" fill="#fabb05" stroke="#fabb05" stroke-linejoin="round" stroke-width="1"></path></svg><svg focusable="false" width="12" height="12" viewBox="0 0 12 12" class="ePMStd NMm5M"><path d="M6 .6L2.6 11.1 11.4 4.7H.6L9.4 11.1Z" fill="#fabb05" stroke="#fabb05" stroke-linejoin="round" stroke-width="1"></path></svg></div>

<span class="y3Ibjb">8 hours ago</span>

<div class="t5YfZe"><span>New</span></div></div>

<div><div class="OA1nbd">Explained
 all my questions in detail.   I felt very comfortable going into this 
purchase of silver.    Also clarified why statements would reflect the 
the raw ore price vs the future index price.   Goldco account  reps were
 very knowledgeable in this silver and gold business.</div></div>
 
 <div class="svzjne"><div jsaction="JIbuQc:XSVqVe" jscontroller="ywSoFb" data-ved="0CAkQ9rwBahcKEwigusOjl8CFAxUAAAAAHQAAAAAQCQ">
 <div jsshadow="" role="button" class="U26fgb BMsU5c h5qmQc isUndragged" jscontroller="VXdfxd" jsaction="click:cOuCgd; mousedown:UX7yZ; mouseup:lbsD7e; mouseenter:tfO1Yc; mouseleave:JywGue; focus:AHmuwe; blur:O22p3e; contextmenu:mg9Pef;touchstart:p6p2H; touchmove:FwuNnf; touchend:yfqBxc(preventDefault=true); touchcancel:JMtRjd" aria-label="Mark review by Connie Morgan as helpful." aria-disabled="false" tabindex="0" data-tooltip="Mark review by Connie Morgan as helpful." aria-pressed="false" data-tooltip-vertical-offset="-12" data-tooltip-horizontal-offset="0">
 <svg focusable="false" width="20" height="20" viewBox="0 0 24 24" class="yduu4c NMm5M">
 <path d="M21 7h-6.31l.95-4.57.03-.32c0-.41-.17-.79-.44-1.06L14.17 0S7.08 6.85 7 7H2v13h16c.83 0 1.54-.5 1.84-1.22l3.02-7.05c.09-.23.14-.47.14-.73V9c0-1.1-.9-2-2-2zM7 18H4V9h3v9zm14-7l-3 7H9V8l4.34-4.34L12 9h9v2z"></path></svg><span class="XT8VNe" jsname="NnAfwf"></span></div></div>
 
 <span><div jscontroller="XHXkqb" jsdata="p5IYUb;_;$75" data-sf="14" data-sm="1" data-sp="0" data-shem="" data-ct="2"><a role="button" tabindex="0" jsname="YOuPgf" jsaction="RiZTIb;JIbuQc:RiZTIb" class="aeonxd" data-ved="0CAoQrcsEahcKEwigusOjl8CFAxUAAAAAHQAAAAAQCQ" data-hveid="10" aria-label="Share"><svg viewBox="0 0 24 24" focusable="false" height="20px" width="20px">
 
 <path d="M0 0h24v24H0z" fill="none"></path><path d="M18 16.08c-.76 0-1.44.3-1.96.77L8.91 12.7c.05-.23.09-.46.09-.7s-.04-.47-.09-.7l7.05-4.11c.54.5 1.25.81 2.04.81 1.66 0 3-1.34 3-3s-1.34-3-3-3-3 1.34-3 3c0 .24.04.47.09.7L8.04 9.81C7.5 9.31 6.79 9 6 9c-1.66 0-3 1.34-3 3s1.34 3 3 3c.79 0 1.5-.31 2.04-.81l7.12 4.16c-.05.21-.08.43-.08.65 0 1.61 1.31 2.92 2.92 2.92 1.61 0 2.92-1.31 2.92-2.92s-1.31-2.92-2.92-2.92z"></path></svg></a></div></span></div>
 
 <div><div class="PdaDLc"><div class="p7f0Nd"><span class="yC3ZMb">
<div class="wSokxc" role="img" style="background-image: url(&quot;https://lh3.googleusercontent.com/a/default-user=s32-cc&quot;);" aria-label="Goldco (Owner)"><div class="ANWksb"></div></div>
 
 <div><div class="PhaUTe">Goldco (Owner)</div>
 <div class="GSM50">7 hours ago</div></div></span></div>
 
 <div class="KmCjbd">Thank
 you so much for your kind words! Our team here at Goldco enjoyed 
working with you, and we look forward to working with you again in the 
future!  Should you ever need anything, please never hesitate to reach 
out to us!</div></div></div>

<div class="aUC40b" style="margin-left: 0px; margin-right: -16px;"></div></div>

<div class="bwb7ce" jsname="ShBeI" data-id="ChdDSUhNMG9nS0VJQ0FnSUREZ19TSG5nRRAB" jsdata="aQic9b;ChdDSUhNMG9nS0VJQ0FnSUREZ19TSG5nRRAB;$76" jscontroller="KUoFuf" data-hveid="14">
<div class="p7f0Nd"><a class="yC3ZMb" href="https://www.google.com/maps/contrib/100341548261237794761/reviews" jscontroller="t7jjzb" jsaction="mousedown:VacWff; touchstart:VacWff" data-redirect="/url?sa=t&amp;source=web&amp;rct=j&amp;url=https%3A%2F%2Fwww.google.com%2Fmaps%2Fcontrib%2F100341548261237794761%2Freviews&amp;ved=0CA8QzsEIahcKEwigusOjl8CFAxUAAAAAHQAAAAAQCQ&amp;opi=89978449">

<div class="wSokxc" role="img" style="background-image: url(&quot;https://lh3.googleusercontent.com/a/ACg8ocKzXpwlNadvN9Tv3eGLNwibrGaCZVVEfNgl3F3GrKcYVYx4ow=s64-c-rp-mo-br100&quot;);" aria-label="Gail Breckenridge"><div class="ANWksb"></div></div>

<div><div class="Vpc5Fe">Gail Breckenridge</div>
*/
int business_review_analyzer::read_review_file_google(){
int err_cnt = 0;


static const std::string tag_google_user_content =
    "https://lh3.googleusercontent.com/a/";
static const std::string tag_google_user_content_default_user =
    "https://lh3.googleusercontent.com/a/default-user=s32-cc&quot";

typedef std::pair<std::string, bool> str_bool;
typedef std::list<str_bool> str_bool_list;
typedef str_bool_list::const_iterator str_bool_list_citr;
str_bool_list raw_review_sb_list;

size_t line_count = 0;
std::ifstream ifs(m_file_name.c_str());

consumer_affairs_parse_state parse_state = CONSUMER_AFFAIRS_PARSE_STATE_SEARCHING_FOR_REVIEW;
business_review *review = nullptr;
size_t max_line_length = 0;
std::string first_line;
while(ifs.good()){
    std::string line;
    std::getline(ifs,line);

    if(first_line.empty()){
        first_line = line;
        }


    ++line_count;
    if(line.length() > max_line_length){
        max_line_length = line.length();
        }

    std::string::size_type pos = 0;
    std::string::size_type copy_pos = 0;
    while( pos < line.length() ){
        const std::string::size_type pos_a = line.find(tag_google_user_content, pos);
        const std::string::size_type pos_b = line.find(tag_google_user_content_default_user, pos);
        if( std::string::npos == pos_a ){
            if( !raw_review_sb_list.empty() ){
                str_bool *raw_review_sb = &(raw_review_sb_list.back());
                if( raw_review_sb->second ){
                    /* previous review raw string is complete */
                    }
                else{
                    /* raw review string is incomplete. append remainder of line*/
                    if(0 == copy_pos){
                        (raw_review_sb->first).append(" ");
                        }
                    (raw_review_sb->first).append(line,copy_pos);
                    }
                }
            pos = line.length();
            copy_pos =  line.length();
            }
        else if( pos_a == pos_b ){
            /* found default user, continue searching */            
            const std::string::size_type next_pos =
                pos_a + tag_google_user_content_default_user.size();            
            const std::string::size_type next_copy_pos = pos_a;
            if( !raw_review_sb_list.empty() ){
                str_bool *raw_review_sb = &(raw_review_sb_list.back());
                if( raw_review_sb->second ){
                    /* previous review raw string is complete */
                    }
                else{
                    /* raw review string is incomplete. append portion of line*/
                    (raw_review_sb->first).append(line,copy_pos, next_copy_pos-copy_pos);
                    }
                }


            pos = next_pos;
            copy_pos = next_copy_pos;
            }
        else{
            /* found reviewing user */            
            const std::string::size_type next_pos =
                pos_a + tag_google_user_content.size();           
            const std::string::size_type next_copy_pos = pos_a;
            if( !raw_review_sb_list.empty() ){
                str_bool *raw_review_sb = &(raw_review_sb_list.back());
                if( raw_review_sb->second ){
                    /* previous review raw string is complete */
                    }
                else{
                    /* raw review string is incomplete. append portion of line*/
                    (raw_review_sb->first).append(line,copy_pos, next_copy_pos-copy_pos);

                    /* raw review string is now complete */
                    (raw_review_sb->second) = true;
                    }
                }
            /* add new raw review */
            raw_review_sb_list.push_back(str_bool(std::string(),false));
            pos = next_pos;
            copy_pos = next_copy_pos;
            }

        if( ( pos_a != std::string::npos ) || ( pos_b != std::string::npos ) ){
            int breakpoint = 0;
            }
        }
    }

static const std::string reference_time_start_token = "<reference_time>";
static const std::string reference_time_end_token = "</reference_time>";

time_t reference_time = time(nullptr); /* download time */
if( !first_line.empty()){
    const std::string::size_type reference_time_start_token_pos = 
        first_line.find(reference_time_start_token);
    const std::string::size_type reference_time_end_token_pos = 
        first_line.find(reference_time_end_token, 
            reference_time_start_token_pos);
    if( (std::string::npos != reference_time_start_token_pos) &&
        (std::string::npos != reference_time_end_token_pos) ){
        const std::string::size_type reference_time_start_pos = 
            reference_time_start_token_pos + reference_time_start_token.length();
        const std::string::size_type reference_time_str_len =
            (reference_time_start_pos < reference_time_end_token_pos ) ?
            reference_time_end_token_pos - reference_time_start_pos : 0;
        const std::string reference_time_str = first_line.substr(
            reference_time_start_pos, reference_time_str_len );
        std::shared_ptr<tm> ref_tm = parse_date_t_time(reference_time_str);
        if( ( nullptr != ref_tm.get() ) && (ref_tm->tm_year > 0 ) ){
            static const time_t pacific_gm_time_offset_seconds = 8*60*60;
            reference_time = mktime(ref_tm.get()) -
                    pacific_gm_time_offset_seconds;
            }
        }
    }

static const std::string name_pre_token = "<div><div class=\"";
static const std::string name_start_token = "\">";
static const std::string name_end_token = "</div><div";

static const std::string rating_start_token = "aria-label=\"Rated";
static const std::string rating_end_token = "out of 5\">";

static const std::string ago_start_token = "\">";
static const std::string ago_partial_end_token = "ago</span>";

static const std::string review_text_pre_token = "<div><div class=\"";
static const std::string review_text_start_token = "\">";
static const std::string review_text_end_token = "</div><div";

str_bool_list_citr raw_itr = raw_review_sb_list.begin();
for(; raw_review_sb_list.end() != raw_itr; ++raw_itr){
    const std::string& raw_review_text = raw_itr->first;

    const std::string::size_type name_pre_token_pos = 
        raw_review_text.find(name_pre_token);
    const std::string::size_type name_start_token_pos = 
        raw_review_text.find(name_start_token, name_pre_token_pos);
    const std::string::size_type name_end_token_pos = 
        raw_review_text.find(name_end_token, name_start_token_pos);

    const std::string::size_type rating_start_token_pos = 
        raw_review_text.find(rating_start_token, name_end_token_pos);
    const std::string::size_type rating_end_token_pos = 
        raw_review_text.find(rating_end_token, rating_start_token_pos);

    const std::string::size_type ago_partial_end_token_pos = 
        raw_review_text.find(ago_partial_end_token, rating_end_token_pos);
    const std::string::size_type ago_start_token_pos = 
        raw_review_text.rfind(ago_start_token, ago_partial_end_token_pos);

    const std::string::size_type review_text_pre_token_pos = 
        raw_review_text.find(review_text_pre_token, ago_start_token_pos);
    const std::string::size_type review_text_start_token_pos = 
        raw_review_text.find(review_text_start_token, review_text_pre_token_pos);
    const std::string::size_type review_text_end_token_pos = 
        raw_review_text.find(review_text_end_token, review_text_start_token_pos);

    if( std::string::npos != name_end_token_pos ){
        m_reviews.push_back(business_review());
        review = &(m_reviews.back());
        review->m_review_type = BUSINESS_REVIEW_TYPE_GOOGLE;

        /* reviewer name */
        if(std::string::npos != name_start_token_pos){
            const std::string::size_type name_start_pos =
                name_start_token_pos + name_start_token.length();
            const std::string::size_type name_len = (name_start_pos < name_end_token_pos) ?
                name_end_token_pos - name_start_pos : 0;
            review->m_full_name = raw_review_text.substr(name_start_pos,name_len);
            split_name(review->m_full_name, &(review->m_parsed_name) );
            }
                
        /* rating */
        if(std::string::npos != rating_start_token_pos){
            const std::string::size_type rating_start_pos =
                rating_start_token_pos + rating_start_token.length();
            const std::string::size_type rating_len = (rating_start_pos < rating_end_token_pos) ?
                rating_end_token_pos - rating_start_pos : 0;
            const std::string rating_str = raw_review_text.substr(rating_start_pos,rating_len);
            const double rating_dbl = atof(rating_str.c_str());
            review->m_star_count = static_cast<int>(round(rating_dbl));
            }

        /* time stamp */
        if(std::string::npos != ago_start_token_pos){
            const std::string::size_type ago_start_pos =
                ago_start_token_pos + ago_start_token.length();
            const std::string::size_type ago_len = (ago_start_pos < ago_partial_end_token_pos) ?
                ago_partial_end_token_pos - ago_start_pos : 0;
            const std::string ago_str = raw_review_text.substr(ago_start_pos,ago_len);

            static const std::string a_keyword = "a ";
            const std::string::size_type a_pos = ago_str.find(a_keyword);

            const std::string digits = "0123456789";         
            const std::string::size_type final_digit_pos = ago_str.find_last_of(digits);
            const std::string ago_num_str = (std::string::npos == final_digit_pos) ?
                std::string() : ago_str.substr(0, final_digit_pos + 1 );
            const int ago_num = (std::string::npos == a_pos) ? atoi(ago_num_str.c_str()) : 1;

            static const std::string second_keyword = "second";
            static const std::string minute_keyword = "minute";
            static const std::string hour_keyword = "hour";
            static const std::string day_keyword = "day";
            static const std::string week_keyword = "week";
            static const std::string month_keyword = "month";
            static const std::string year_keyword = "year";

            const std::string::size_type second_pos = ago_str.find(second_keyword);
            const std::string::size_type minute_pos = ago_str.find(minute_keyword);
            const std::string::size_type hour_pos = ago_str.find(hour_keyword);
            const std::string::size_type day_pos = ago_str.find(day_keyword);
            const std::string::size_type week_pos = ago_str.find(week_keyword);
            const std::string::size_type month_pos = ago_str.find(month_keyword);
            const std::string::size_type year_pos = ago_str.find(year_keyword);

            time_t time_unit = 1;
            if(std::string::npos != second_pos){ time_unit = 1; }
            if(std::string::npos != minute_pos){ time_unit = 60; }
            if(std::string::npos != hour_pos){ time_unit = 60*60; }
            if(std::string::npos != day_pos){ time_unit = 24*60*60; }
            if(std::string::npos != week_pos){ time_unit = 7*24*60*60; }
            if(std::string::npos != month_pos){ time_unit = 30*24*60*60; }
            if(std::string::npos != year_pos){ time_unit = 365*24*60*60; }

            time_t time_ago = static_cast<time_t>(ago_num) * time_unit;
             
            review->m_date_str = ago_str + "ago";
            review->m_time_stamp = (time_ago < reference_time) ?
                (reference_time - time_ago) : 0;
            const time_t time_err_seconds_low = time_unit / 2;
            const time_t time_err_seconds_high = time_unit;
            review->m_time_stamp_min = (review->m_time_stamp > time_err_seconds_low) ?
                (review->m_time_stamp - time_err_seconds_low) : 0;
            review->m_time_stamp_max = review->m_time_stamp + time_err_seconds_high;
            }

        /* review text */
        if(std::string::npos != review_text_start_token_pos){
            const std::string::size_type review_start_pos =
                review_text_start_token_pos + review_text_start_token.length();
            const std::string::size_type review_len = (review_start_pos < review_text_end_token_pos) ?
                review_text_end_token_pos - review_start_pos : 0;
            review->m_review_str = raw_review_text.substr(review_start_pos, review_len);
            }

        int breakpoint = 0;

        }
    }

err_cnt += attempt_equalize_timestamp_intervals();

sort_reviews_by_timestamp();

/* finalize timestamps */
business_review_vec_itr bbbrv_itr = m_reviews.begin();
for(; m_reviews.end() != bbbrv_itr; ++bbbrv_itr){
    business_review *review = &(*bbbrv_itr);
    review->m_time_stamp;
    struct tm *tms2 = localtime( &(review->m_time_stamp) );
    review->m_time_stamp_tm.reset( new struct tm);
    memcpy( review->m_time_stamp_tm.get(), tms2, sizeof(tm));
    }

std::cout << "total review_count=" << m_reviews.size() << "\n";
std::cout << "line_count=" << line_count << "\n";

return err_cnt;
}



/*
https://www.trustlink.org/Reviews/Goldco-206527051

id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucTLRatingMed1_Rating1_A" title="3" <== rated 3 stars

* 
* 
<div>
    <!--  id="divreview22" -->
    <div itemprop="review" itemscope="" itemtype="http://schema.org/Review">
        <table style="padding: 0px 5px 0px 5px; margin: 2px; width: 100%">
            <tbody><tr>
                <td style="border: thin solid #C0C0C0; vertical-align: top; margin-right: 5px;">
                    <table style="padding: 0px; margin: 0px; width: 100%">
                        <tbody><tr>
                            <td style="width: 113px; vertical-align: top;">
                                <table cellspacing="1" style="padding: 0px; margin: 0px; width: 100%">
                                    <tbody><tr>
                                        <td>
                                            <div id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucTLRatingMed1_Rating1">
							<input type="hidden" name="ctl00$MainSection$gvReviews$ctl08$ucReview221$ucTLRatingMed1$Rating1_RatingExtender_ClientState" id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucTLRatingMed1_Rating1_RatingExtender_ClientState" value="3">
    <a href="javascript:void(0)" id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucTLRatingMed1_Rating1_A" title="3" style="text-decoration:none"><span id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucTLRatingMed1_Rating1_Star_1" class="ratingStar medfilledRatingStar" style="float:left;">&nbsp;</span><span id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucTLRatingMed1_Rating1_Star_2" class="ratingStar medfilledRatingStar" style="float:left;">&nbsp;</span><span id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucTLRatingMed1_Rating1_Star_3" class="ratingStar medfilledRatingStar" style="float:left;">&nbsp;</span><span id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucTLRatingMed1_Rating1_Star_4" class="ratingStar medemptyRatingStar" style="float:left;">&nbsp;</span><span id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucTLRatingMed1_Rating1_Star_5" class="ratingStar medemptyRatingStar" style="float:left;">&nbsp;</span></a>
						</div>


                                            Review
                                            Posted <meta itemprop="datePublished" content="2020-06-02">6/2/2020<br>
                                            
<table style="width: 115px">
    <tbody><tr>
        <td id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucReviewerProfile1_tdReviewer">
                <div id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucReviewerProfile1_pnlDisplayName">
							
                    <span itemprop="author" itemscope="" itemtype="https://schema.org/Person"><span itemprop="name">william c.</span></span>
                
						</div>
                
            </td>
						
    </tr>
    <tr>
        
    </tr>
</tbody></table>
<div id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucReviewerProfile1_pnlPopup" style="border-color: black; border-width: 1px; border-style: solid; display: none; visibility: hidden; position: absolute;">
							
    <table style="background-color: #FFFFFF; width: 299px;">
        <tbody><tr>
            <td style="width: 31px">
                <img src="2024_04_17_trustlink_goldco_reviews_pg01_files/tl_icon_review.gif" alt="Reviews" border="0">
            </td>
            <td style="width: 88px">
                <span id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucReviewerProfile1_lblNumReviewsp">1 Review</span>
            </td>

            <td style="width: 31px">&nbsp;<img src="2024_04_17_trustlink_goldco_reviews_pg01_files/tl_icon_question.gif" alt="Question" border="0"></td>
            <td style="width: 122px">
                <span id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucReviewerProfile1_lblNumQuestionsp">0 Question</span>
            </td>
        </tr>
        <tr>
            <td style="width: 31px">
                <img src="2024_04_17_trustlink_goldco_reviews_pg01_files/tl_icon_prof_views.gif" alt="Profile Views" border="0">
            </td>
            <td style="width: 88px">
                <span id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucReviewerProfile1_lblProfileViewsp">0 Profile View</span>
            </td>

            <td style="width: 31px">&nbsp;<img src="2024_04_17_trustlink_goldco_reviews_pg01_files/tl_icon_answer.gif" alt="Answers" border="0"></td>
            <td style="width: 122px">
                <span id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucReviewerProfile1_lblNumAnswersp">0 Answer</span>
            </td>
        </tr>
        <tr>
            <td colspan="4">
                <span id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucReviewerProfile1_lblTrustlinkerSince">Trustlinker since 06/02/20</span>
            </td>
        </tr>
        <tr>
            <td colspan="4">
                <span id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucReviewerProfile1_lblLocation">BERWYN, PA</span>
            </td>
        </tr>
        <tr>
            <td colspan="4">
                <span id="ctl00_MainSection_gvReviews_ctl08_ucReview221_ucReviewerProfile1_lblDisplayName"></span>
            </td>
        </tr>
    </tbody></table>

						</div>

                                        </td>
                                    </tr>
                                </tbody></table>
                            </td>
                            <td style="vertical-align: top; text-align: left">
                                <table style="padding: 0px; margin: 0px; width: 100%">
                                    <tbody><tr style="height: 20px">
                                        <td style="width: 113px; vertical-align: top;">
                                            
                                            <a id="ctl00_MainSection_gvReviews_ctl08_ucReview221_hlkHeadline" class="reviewheaderwb" href="https://www.trustlink.org/Review/Goldco-252733"><span itemprop="name">Goldco Transfer</span></a><br>
                                        </td>
                                    </tr>
                                    <tr>
                                        <td style="vertical-align: top; padding-right: 10px; color: #000000;">
                                            <span itemprop="reviewBody">I
 was put in touch with David Taylor when I needed information on what 
would be involved in transferring money out of mutual funds in my IRA to
 gold or silver. This would be a first for me and I was nervous. David 
spent so much time with me that I...<a class="url" href="https://www.trustlink.org/Review/Goldco-252733"> Read More »</a></span>
                                        </td>
                                    </tr>
                                    <tr>
                                        <td>
                                            <table cellspacing="1" style="padding: 0px; margin: 0px; width: 100%">
                                                <tbody><tr>
                                                    <td>
                                                        
                                                    </td>
                                                    <td style="width: 155px">&nbsp;</td>
                                                    <td style="text-align: right">
                                                        <table cellpadding="0" cellspacing="1" style="padding: 0px; margin: 0px; width: 100%">
                                                            <tbody><tr>
                                                                <td>
                                                                    
                                                                </td>
                                                                <td>
                                                                    
                                                                </td>
                                                                <td style="width: 2px">&nbsp;</td>
                                                            </tr>
                                                        </tbody></table>
                                                    </td>
                                                </tr>
                                            </tbody></table>
                                        </td>
                                    </tr>
                                    
                                </tbody></table>
                            </td>
                        </tr>
                    </tbody></table>
                    
                </td>
                <td style="padding: 0px; margin: 0px; width: 4px;">&nbsp;
                </td>
                <td style="margin: 0px; border: thin solid #C0C0C0; vertical-align: top; text-align: left; padding-left: 5px; width: 150px;">
                    
                </td>
            </tr>
        </tbody></table>
    </div>
    <!-- Review -->
</div>
*/
int business_review_analyzer::read_review_file_trustlink(){
int err_cnt = 0;

//static const std::string token_review_start_tag = "<div itemprop=\"review\">";
static const std::string token_review_start_tag = "\"review\"";

//static const std::string token_rating_pre_open_tag =
//    "\"id=\"ctl00_MainSection_gvReviews_ctl08_ucReview221_ucTLRatingMed1_Rating1_A\"";
static const std::string token_rating_pre_open_tag = "RatingMed1_Rating1_A\"";
static const std::string token_rating_open_tag = "title=\"";
static const std::string token_rating_close_tag = "\"";

static const std::string token_date_pre_open_tag = "\"datePublished\"";
static const std::string token_date_open_tag = "content=\"";
static const std::string token_date_close_tag = "\">";

static const std::string token_author_pre_open_tag = "\"author\"";
static const std::string token_author_open_tag = "<span itemprop=\"name\">";
static const std::string token_author_close_tag = "</span>";

static const std::string token_review_body_open_tag = "<span itemprop=\"reviewBody\">";
static const std::string token_review_body_close_tag = "<a";

size_t line_count = 0;
std::ifstream ifs(m_file_name.c_str());

trustlink_parse_state parse_state = TRUSTLINK_PARSE_STATE_SEARCHING_FOR_REVIEW;
business_review *review = nullptr;
size_t max_line_length = 0;
while(ifs.good()){
    std::string line;
    std::getline(ifs,line);
    ++line_count;
    if(line.length() > max_line_length){
        max_line_length = line.length();
        }

    const std::string::size_type token_review_start_tag_pos = 
        line.find(token_review_start_tag);
    const std::string::size_type token_date_pre_open_tag_pos = 
        line.find(token_date_pre_open_tag);
    const std::string::size_type token_author_pre_open_tag_pos = 
        line.find(token_author_pre_open_tag);
    const std::string::size_type token_review_body_open_tag_pos = 
        line.find(token_review_body_open_tag);
    const std::string::size_type token_review_body_close_tag_pos = 
        (std::string::npos == token_review_body_open_tag_pos) ?
        line.find(token_review_body_close_tag) : 
        line.find(token_review_body_close_tag,
            token_review_body_open_tag_pos + token_review_body_open_tag.length());

    /* start new review */
    if( std::string::npos != token_review_start_tag_pos ){
        m_reviews.push_back(business_review());
        review = &(m_reviews.back());
        review->m_review_type = BUSINESS_REVIEW_TYPE_TRUSTLINK;
        parse_state = TRUSTLINK_PARSE_STATE_SEARCHING_FOR_RATING;
        }
    else if( std::string::npos != token_date_pre_open_tag_pos ){
        parse_state = TRUSTLINK_PARSE_STATE_SEARCHING_FOR_DATE;
        }
    else if( std::string::npos != token_author_pre_open_tag_pos ){
        parse_state = TRUSTLINK_PARSE_STATE_SEARCHING_FOR_AUTHOR;
        }
    else if( std::string::npos != token_review_body_open_tag_pos ){
        parse_state = TRUSTLINK_PARSE_STATE_SEARCHING_FOR_REVIEW_STR;
        }


    switch( parse_state ){
    case TRUSTLINK_PARSE_STATE_SEARCHING_FOR_REVIEW:
        break;
    case TRUSTLINK_PARSE_STATE_SEARCHING_FOR_RATING:
            {
            const std::string::size_type token_rating_pre_open_tag_pos = 
                line.find(token_rating_pre_open_tag);
            const std::string::size_type token_rating_open_tag_pos = 
                (std::string::npos == token_rating_pre_open_tag_pos) ?
                std::string::npos : line.find(token_rating_open_tag,
                token_rating_pre_open_tag_pos + token_rating_pre_open_tag.length() );
            const std::string::size_type token_rating_start_pos =
                (std::string::npos == token_rating_open_tag_pos) ?
                std::string::npos :
            token_rating_open_tag_pos + token_rating_open_tag.length(); 
            const std::string::size_type token_rating_close_tag_pos = 
                (std::string::npos == token_rating_start_pos) ?
                std::string::npos :  
                line.find(token_rating_close_tag, token_rating_start_pos);
            if( (std::string::npos != token_rating_start_pos) &&
                (std::string::npos != token_rating_close_tag_pos) ){
                const std::string::size_type token_rating_str_len =
                    (token_rating_start_pos < token_rating_close_tag_pos ) ?
                    token_rating_close_tag_pos - token_rating_start_pos : 0;
                const std::string token_rating_str =
                    line.substr(token_rating_start_pos, token_rating_str_len);
                review->m_star_count = atoi(token_rating_str.c_str());
                parse_state = TRUSTLINK_PARSE_STATE_SEARCHING_FOR_DATE;
                }
            }
        break;
    case TRUSTLINK_PARSE_STATE_SEARCHING_FOR_DATE:
            {
            const std::string::size_type token_date_open_tag_pos = 
                (std::string::npos == token_date_pre_open_tag_pos) ?
                std::string::npos : line.find(token_date_open_tag,
                token_date_pre_open_tag_pos + token_date_pre_open_tag.length() );
            const std::string::size_type token_date_start_pos =
                (std::string::npos == token_date_open_tag_pos) ?
                std::string::npos :
                token_date_open_tag_pos + token_date_open_tag.length(); 
            const std::string::size_type token_date_close_tag_pos = 
                (std::string::npos == token_date_start_pos) ?
                std::string::npos :  
                line.find(token_date_close_tag, token_date_start_pos);
            if( (std::string::npos != token_date_start_pos) &&
                (std::string::npos != token_date_close_tag_pos) ){
                const std::string::size_type token_date_str_len =
                    (token_date_start_pos < token_date_close_tag_pos ) ?
                    token_date_close_tag_pos - token_date_start_pos : 0;
                review->m_date_str =
                    line.substr(token_date_start_pos, token_date_str_len);
                review->m_time_stamp_tm = parse_date_ymd_dash( review->m_date_str );
                review->m_time_stamp = mktime((review->m_time_stamp_tm).get());
                static const time_t time_err_seconds = 1;
                review->m_time_stamp_min = (review->m_time_stamp > time_err_seconds) ?
                    (review->m_time_stamp - time_err_seconds) : 0;
                review->m_time_stamp_max = review->m_time_stamp + time_err_seconds;
                struct tm *tms2 = localtime( &(review->m_time_stamp) );
                parse_state = TRUSTLINK_PARSE_STATE_SEARCHING_FOR_AUTHOR;
                }
            }
        break;
    case TRUSTLINK_PARSE_STATE_SEARCHING_FOR_AUTHOR:
            {
            const std::string::size_type token_author_open_tag_pos = 
                (std::string::npos == token_author_pre_open_tag_pos) ?
                std::string::npos : line.find(token_author_open_tag,
                token_author_pre_open_tag_pos + token_author_pre_open_tag.length() );
            const std::string::size_type token_author_start_pos =
                (std::string::npos == token_author_open_tag_pos) ?
                std::string::npos :
                token_author_open_tag_pos + token_author_open_tag.length(); 
            const std::string::size_type token_author_close_tag_pos = 
                (std::string::npos == token_author_start_pos) ?
                std::string::npos :  
                line.find(token_author_close_tag, token_author_start_pos);
            if( (std::string::npos != token_author_start_pos) &&
                (std::string::npos != token_author_close_tag_pos) ){
                const std::string::size_type token_author_str_len =
                    (token_author_start_pos < token_author_close_tag_pos ) ?
                    token_author_close_tag_pos - token_author_start_pos : 0;
                review->m_full_name =
                    line.substr(token_author_start_pos, token_author_str_len);
                trim_str(&(review->m_full_name));
                split_name(review->m_full_name, &(review->m_parsed_name) );
                parse_state = TRUSTLINK_PARSE_STATE_SEARCHING_FOR_REVIEW_STR;
                }
            }
        break;
    case TRUSTLINK_PARSE_STATE_SEARCHING_FOR_REVIEW_STR:
            {
            if( std::string::npos != token_review_body_open_tag_pos ){
                const std::string::size_type review_body_start_pos =
                    token_review_body_open_tag_pos + 
                    token_review_body_open_tag.length();
                if( review_body_start_pos < line.length() ){
                    std::string review_first_line;
                    if( std::string::npos == token_review_body_close_tag_pos ){
                        review_first_line = line.substr(review_body_start_pos);
                        parse_state = TRUSTLINK_PARSE_STATE_PARSING_REVIEW_STR;
                        }
                    else{
                        if( review_body_start_pos < token_review_body_close_tag_pos){
                            const std::string::size_type review_first_line_len =
                                token_review_body_close_tag_pos - review_body_start_pos;
                            review_first_line = line.substr(review_body_start_pos,
                                review_first_line_len);
                            }
                        parse_state = TRUSTLINK_PARSE_STATE_SEARCHING_FOR_REVIEW;                      
                        }

                    trim_str(&review_first_line);
                    review->m_review_str = review_first_line; 
                    }
                else{
                    parse_state = TRUSTLINK_PARSE_STATE_PARSING_REVIEW_STR;
                    }
                }
            }
        break;
    case TRUSTLINK_PARSE_STATE_PARSING_REVIEW_STR:
            {
            std::string review_line;
            if( std::string::npos == token_review_body_close_tag_pos ){
                review_line = line;
                parse_state = TRUSTLINK_PARSE_STATE_PARSING_REVIEW_STR;
                }
            else{
                review_line = line.substr(0, token_review_body_close_tag_pos);
                parse_state = TRUSTLINK_PARSE_STATE_SEARCHING_FOR_REVIEW;                      
                }
            trim_str(&review_line);
            if(!review_line.empty()){
                review->m_review_str.append( " " );
                }
            review->m_review_str.append(review_line); 
            }
        break;

    case TRUSTLINK_PARSE_STATE_DONE:
        break;
        }

    }

sort_reviews_by_timestamp();

std::cout << "total review_count=" << m_reviews.size() << "\n";
std::cout << "line_count=" << line_count << "\n";

return err_cnt;
}


/*
* {"@type":"Review",
"@id":"https://www.trustpilot.com/#/schema/Review/goldco.com/5db84119604858035c8f512a",
"itemReviewed":
{"@id":"https://www.trustpilot.com/#/schema/Organization/goldco.com"},
"author":{"@type":"Person","name":"Juli Mock Campbell","url":"https://www.trustpilot.com/users/5db8410fcf562213914e4a15/"},
"datePublished":"2019-10-29T13:39:37.000Z",
"headline":"While it took 2-1/2 months for my union…",
"reviewBody":"While it took 2-1/2 months for my union to release my 401K funds, I want to say that Sean Dozier, Patricia Loya, and particularly, Joyce (whom helped me wait while the union processed my paperwork), I just want to say, Goldco couldn't have a better team to help me understand what I was doing. I've never worked worked with precious metals purchasing, and Sean helped me understand what I was doing with my rollover process. I'm a happy precious metals investor now!!",
"reviewRating":{"@type":"Rating","bestRating":"5","worstRating":"1","ratingValue":"5"},
"publisher":{"@id":"https://www.trustpilot.com/#/schema/Organization/1"},"inLanguage":"en"},

{"@type":"Review","@id":"https://www.trustpilot.com/#/schema/Review/goldco.com/5da12f9b6b04580a8c339699","itemReviewed":{"@id":"https://www.trustpilot.com/#/schema/Organization/goldco.com"},"author":{"@type":"Person","name":"customer","url":"https://www.trustpilot.com/users/5d9de60988e053395371d89c/"},"datePublished":"2019-10-12T01:42:51.000Z","headline":"Outstanding service","reviewBody":"Outstanding qualified personal keeping you informed on the market and are there to assist you with your purchase decision.","reviewRating":{"@type":"Rating","bestRating":"5","worstRating":"1","ratingValue":"5"},"publisher":{"@id":"https://www.trustpilot.com/#/schema/Organization/1"},"inLanguage":"en"},
{"@type":"Review","@id":"https://www.trustpilot.com/#/schema/Review/goldco.com/5d88e5ae3585c7069897c8fc","itemReviewed":{"@id":"https://www.trustpilot.com/#/schema/Organization/goldco.com"},"author":{"@type":"Person","name":"Jo Ann Levitt","url":"https://www.trustpilot.com/users/5c365a4fd8b15b754a4e3179/"},"datePublished":"2019-09-23T15:33:02.000Z","headline":"I am a total fan","reviewBody":"I am a total fan! I have received so much inspiration and wisdom, especially from my coach and guide, 
* 
* */
int business_review_analyzer::read_review_file_trustpilot(){
int err_cnt = 0;

size_t line_count = 0;
std::ifstream ifs(m_file_name.c_str());

business_review *review = nullptr;
size_t max_line_length = 0;
while(ifs.good()){
    std::string line;
    std::getline(ifs,line);
    ++line_count;
    if(line.length() > max_line_length){
        max_line_length = line.length();
        }

    std::string::size_type review_start_pos = 0;
    while( review_start_pos < line.length() ){

        static const std::string review_start_tag = "{\"@type\":\"Review\"";

        static const std::string name_open_tag = "\"author\":{\"@type\":\"Person\",\"name\":\"";
        static const std::string name_close_tag = "\"";

        static const std::string date_published_open_tag = "\"datePublished\":\"";
        static const std::string date_published_close_tag = "\"";

        static const std::string review_body_open_tag = "\"reviewBody\":\"";
        static const std::string review_body_close_tag = "\"";

        static const std::string rating_open_tag = "\"ratingValue\":\"";
        static const std::string rating_close_tag = "\"";

        review_start_pos = ( std::string::npos == review_start_pos ) ?
            std::string::npos : line.find( review_start_tag, review_start_pos + 1);

        const std::string::size_type name_open_pos =
            ( std::string::npos == review_start_pos ) ?
            std::string::npos : line.find( name_open_tag, review_start_pos+1);
        const std::string::size_type name_close_pos =
            ( std::string::npos == name_open_pos ) ?
            std::string::npos : line.find( name_close_tag, 
                name_open_pos + name_open_tag.length() );

        const std::string::size_type date_published_open_pos =
            ( std::string::npos == name_close_pos ) ?
            std::string::npos : line.find( date_published_open_tag, name_close_pos+1);
        const std::string::size_type date_published_close_pos =
            ( std::string::npos == date_published_open_pos ) ?
            std::string::npos : line.find( date_published_close_tag, 
                date_published_open_pos + date_published_open_tag.length() );

        const std::string::size_type review_body_open_pos =
            ( std::string::npos == date_published_close_pos ) ?
            std::string::npos : line.find( review_body_open_tag, date_published_close_pos+1);
        const std::string::size_type review_body_close_pos =
            ( std::string::npos == review_body_open_pos ) ?
            std::string::npos : line.find( review_body_close_tag, 
                review_body_open_pos + review_body_open_tag.length() );

        const std::string::size_type rating_open_pos =
            ( std::string::npos == review_body_close_pos ) ?
            std::string::npos : line.find( rating_open_tag, review_body_close_pos+1);
        const std::string::size_type rating_close_pos =
            ( std::string::npos == rating_open_pos ) ?
            std::string::npos : line.find( rating_close_tag, 
                rating_open_pos + rating_open_tag.length() );

        /* start new review */
        if( std::string::npos != name_open_pos ){
            m_reviews.push_back(business_review());
            review = &(m_reviews.back());
            review->m_review_type = BUSINESS_REVIEW_TYPE_TRUSTPILOT;
            if( ( name_close_pos > name_open_pos ) &&
                ( std::string::npos != name_close_pos ) ){
                const size_t name_start_pos = name_open_pos + name_open_tag.length();
                const size_t name_len = ( name_start_pos < name_close_pos ) ?
                    (name_close_pos - name_start_pos) : 0;
                review->m_full_name = line.substr(name_start_pos, name_len);
                split_name(review->m_full_name, &(review->m_parsed_name) );
                }

            if( ( date_published_close_pos > date_published_open_pos ) &&
                ( std::string::npos != date_published_close_pos ) ){
                const size_t date_published_start_pos = 
                    date_published_open_pos + date_published_open_tag.length();
                const size_t date_published_len =
                    ( date_published_start_pos < date_published_close_pos ) ?
                    (date_published_close_pos - date_published_start_pos) : 0;
                review->m_date_str = line.substr(date_published_start_pos, date_published_len);
                std::shared_ptr<tm> gm_time_stamp_tm = parse_date_t_time(review->m_date_str);
                static const time_t pacific_gm_time_offset_seconds = 8*60*60;
                review->m_time_stamp = mktime(gm_time_stamp_tm.get())-
                    pacific_gm_time_offset_seconds;
                static const time_t time_err_seconds = 1;
                review->m_time_stamp_min = (review->m_time_stamp > time_err_seconds) ?
                    (review->m_time_stamp - time_err_seconds) : 0;
                review->m_time_stamp_max = review->m_time_stamp + time_err_seconds;
                struct tm *tms2 = localtime( &(review->m_time_stamp) );
                review->m_time_stamp_tm.reset( new struct tm);
                memcpy( review->m_time_stamp_tm.get(), tms2, sizeof(tm));
                }

            if( ( review_body_close_pos > review_body_open_pos ) &&
                ( std::string::npos != review_body_close_pos ) ){
                const size_t review_body_start_pos = review_body_open_pos + review_body_open_tag.length();
                const size_t review_body_len =
                    ( review_body_start_pos < review_body_close_pos ) ?
                    (review_body_close_pos - review_body_start_pos) : 0;
                review->m_review_str = line.substr(review_body_start_pos, review_body_len);
                }

            if( ( rating_close_pos > rating_open_pos ) &&
                ( std::string::npos != rating_close_pos ) ){
                const size_t rating_start_pos = rating_open_pos + rating_open_tag.length();
                const size_t rating_len = ( rating_start_pos < rating_close_pos ) ?
                    (rating_close_pos - rating_start_pos) : 0;
                const std::string rating_str = line.substr(rating_start_pos, rating_len);
                const char rating_char = (rating_str.empty()) ? '0' : rating_str.at(0);
                review->m_star_count = ( (rating_char >= '0') && (rating_char <= '5')) ? 
                    static_cast<int>(rating_char - '0') : 0;
                } 
            }
        }
    }
std::cout << "total review_count=" << m_reviews.size() << "\n";
std::cout << "line_count=" << line_count << "\n";
std::cout << "max_line_length=" << max_line_length << "\n";

sort_reviews_by_timestamp();

return err_cnt;
}



/*
Photo of Kingfish S.
Kingfish S.
San Francisco, CA
0
8
Apr 3, 2023

I had previously bought PMs from Birch. My shipment arrived in about a week. About two months ago, I sent 30k to Goldco for an intended purchase. They got back to me about two weeks later, after my original contact had fallen ill. The next man up took my order and did a good job explaining my options. This order was placed mid-February. They said 4-6 weeks for delivery and I was promised two weeks ago that it was to ship this past week. It did not, and if I had not inquired, I seriously doubt I would have been updated as to its status. My recommendation is RUN AS FAR FROM THIS PLACE AS YOU CAN! I can't remember at which point in this debacle that I was made aware of this preposterous delivery time, but I will not be making this mistake again.

Business owner information
Photo of Kevin D.

Kevin D.

Business Manager

Apr 4, 2023

We apologize for the experience you had. As you can see from the 1000s of 5-Star reviews we have, this isn’t how we handle our customers. If you don’t mind, we would like to find out more about what happened during your experience. Our Customer Care team will reach out to you to ensure that you are a satisfied customer. Also, thank you for taking the time to write a review. All reviews help us improve our business practices and policies.


Photo of Nanki S.
Nanki S.
Santa Rosa, CA
0
5
rating: 5 Stars
Nov 18, 2014
First to Review

I recently purchased precious metals from Goldco with the sound advice of my representative, Zeve Katz. At first, I was unsure of what exactly to purchase for my portfolio, however, after consulting with Mr. Katz, I was confident that his advice was exactly what I was looking for. Not only was Mr. Katz extremely knowledgable about precious metals, he also earned my trust because of his personal attention. He was always on top of my order and kept me informed all the way. My entire experience was really something that was extraordinary. It left me thinking that I will most definitely purchase more precious metals very soon and with Zeve Katz as my portfolio consultant. I believe Goldco to be a reputable, sound and honest company and it was a pleasure doing business with them.
Photo of Jessica C.
Jessica C.
SYLMAR, CA
0
18
4
rating: 5 Stars
Nov 18, 2014

When I called, I spoke with Josh Leslie. He was very enthusiastic but never pushy. I felt comfortable trusting him with my information and he gave me good advice. He presented me all of my options and gave me his opinion on how I should transfer over my assets and what I could and should invest on. Gold is going up in the market more than it is ever going down so I am happy I called goldco and I want to thank Mr. Leslie for everything. Thanks guys!
1
2
2 of 2
*/
int business_review_analyzer::read_review_file_yelp(){
int err_cnt = 0;

size_t line_count = 0;
std::ifstream ifs(m_file_name.c_str());
yelp_recommended_parse_state parse_state = YELP_RECOMENDED_PARSE_STATE_SEARCHING_FOR_REVIEW;
business_review *review = nullptr;
size_t max_line_length = 0;
std::string prev_line;
while(ifs.good()){
    std::string line;
    std::getline(ifs,line);
    ++line_count;
    if(line.length() > max_line_length){
        max_line_length = line.length();
        }

    /*
    Photo of <author>
    <author>
    */
    static const std::string photo_of_token = "Photo of ";    
    const std::string str_a = photo_of_token + line;
    if(str_a == prev_line){
        m_reviews.push_back(business_review());
        review = &(m_reviews.back());
        review->m_review_type = BUSINESS_REVIEW_TYPE_YELP;
        review->m_full_name = line;
        split_name(review->m_full_name, &(review->m_parsed_name) );
        parse_state = YELP_RECOMENDED_PARSE_STATE_PARSING_LOCATION;
        }
    else{
        switch(parse_state){
            case YELP_RECOMENDED_PARSE_STATE_SEARCHING_FOR_REVIEW:
                break;
            case YELP_RECOMENDED_PARSE_STATE_PARSING_LOCATION:
                review->m_full_location_name = line;
                init_city_state_from_full_location_name(review);
                parse_state = YELP_RECOMENDED_PARSE_STATE_SEARCHING_FOR_RATING;
                break;
            case YELP_RECOMENDED_PARSE_STATE_SEARCHING_FOR_RATING:
                {
                static const std::string rating_token = "rating:";
                static const std::string star_token = "Star";
                const std::string::size_type rating_token_pos = line.find(rating_token);
                if( std::string::npos != rating_token_pos ){
                    const std::string::size_type rating_start_pos =
                        rating_token_pos + rating_token.length();
                    const std::string::size_type stars_token_pos =
                        line.find(star_token, rating_start_pos);
                    if( ( rating_start_pos < stars_token_pos ) &&
                        ( std::string::npos != stars_token_pos ) ){
                        const std::string::size_type rating_len =
                            stars_token_pos - rating_start_pos;
                        std::string rating_str = line.substr(rating_start_pos, rating_len);
                        trim_str(&rating_str);
                        review->m_star_count = atoi(rating_str.c_str());
                        }
                    parse_state = YELP_RECOMENDED_PARSE_STATE_PARSING_DATE;
                    }
                }
                break;
            case YELP_RECOMENDED_PARSE_STATE_PARSING_DATE:
                {
                review->m_date_str = line;
                review->m_time_stamp_tm = parse_date_mon_day_year(review->m_date_str);
                review->m_time_stamp = mktime((review->m_time_stamp_tm).get());
                static const time_t time_err_seconds = 12*60*60;
                review->m_time_stamp_min = (review->m_time_stamp > time_err_seconds) ?
                    (review->m_time_stamp - time_err_seconds) : 0;
                review->m_time_stamp_max = review->m_time_stamp + time_err_seconds;
                parse_state = YELP_RECOMENDED_PARSE_STATE_PARSING_REVIEW_STR;
                }
                break;
            case YELP_RECOMENDED_PARSE_STATE_PARSING_REVIEW_STR:
                if( line == "1" ||
                    line == "Business owner information"  ||
                    line.find(photo_of_token) == 0 ){
                    parse_state = YELP_RECOMENDED_PARSE_STATE_SEARCHING_FOR_REVIEW;
                    }
                else{
                    if( !(review->m_review_str).empty() ){
                        (review->m_review_str).append(" ");
                        }
                    (review->m_review_str).append(line);
                    }
                break;

            case YELP_RECOMENDED_PARSE_STATE_DONE:
            break;
            }
        }
    
    prev_line = line;
    }


remove_invalid_reviews();
sort_reviews_by_timestamp();

std::cout << "total review_count=" << m_reviews.size() << "\n";
std::cout << "line_count=" << line_count << "\n";

return err_cnt;
}



/*
        </div>
        <div class="media-story">
                <ul class="user-passport-info">
        <li class="user-name">
                    <span class="user-display-name" data-hovercard-id="eG69Fl_0b8sAo5nP_vUHEw">David G.</span>

        </li>
        <li class="user-location responsive-hidden-small">
            <b>Fremont, CA</b>
        </li>
    </ul>

            

    <ul class="user-passport-stats">
        <li class="friend-count responsive-small-display-inline-block">
            <span aria-hidden="true" style="fill: #f15c00; width: 16px; height: 16px;" class="icon icon--18-friends icon--size-18">
    <svg role="img" class="icon_svg">
        <use xlink:href="#18x18_friends"></use>
    </svg>
</span>
            <b>1</b> friend
        </li>
        <li class="review-count responsive-small-display-inline-block">
            <span aria-hidden="true" style="fill: #f15c00; width: 16px; height: 16px;" class="icon icon--18-review icon--size-18">
    <svg role="img" class="icon_svg">
        <use xlink:href="#18x18_review"></use>
    </svg>
</span>
            <b>1</b> review
        </li>
        
    </ul>

        </div>
    </div>

        </div>
    </div>

        <div class="review-wrapper">
                <div class="review-content">
            <div class="biz-rating biz-rating-large biz-rating-large--wrap clearfix">
        <div class="biz-rating__stars">
                    



    <div class="i-stars i-stars--regular-1 rating-large" title="1.0 star rating">
        <img class="offscreen" height="303" src="Not%20Recommended%20Reviews%20for%20Goldco%20-%20Yelp_2024_04_17__pg00_files/stars.png" width="84" alt="1.0 star rating">
    </div>



        </div>
            <span class="rating-qualifier">
        2/8/2023
    </span>

    </div>

        
                <p lang="en">Never again. Forget their glossy brochures 
and supposed relations with paid Hollywood stars. In the end they are an
 aggressive sales orgranization that tell you what you want to hear in 
order to sell you overpriced unmarked unknown gold and silver that has 
little market vale.</p> 
            
    </div>
        <div class="review-footer clearfix">
    </div>

    
                <div class="island biz-owner-reply clearfix">
        <div class="biz-owner-reply-header arrange arrange--6">
            <div class="arrange_unit biz-owner-reply-photo">
                    <div class="photo-box pb-30s">
                <a href="https://s3-media0.fl.yelpcdn.com/buphoto/a9QOZN5-TYUuImzikhwayw/o.jpg">
                <img alt="Kevin D." class="photo-box-img" height="30" loading="lazy" src="Not%20Recommended%20Reviews%20for%20Goldco%20-%20Yelp_2024_04_17__pg00_files/30s.jpg" srcset="Not%20Recommended%20Reviews%20for%20Goldco%20-%20Yelp_2024_04_17__pg00_files/ss.jpg 1.33x, Not%20Recommended%20Reviews%20for%20Goldco%20-%20Yelp_2024_04_17__pg00_files/90s_004.jpg 3x" width="30">

        </a>

    </div>

            </div>
            <div class="arrange_unit arrange_unit--fill embossed-text-white">
                <strong>
                    Comment from Kevin D. of Goldco
                </strong>
                <br>
                Business Manager
            </div>


        </div>
        <span class="bullet-after">
            2/9/2023
        </span>
                <span class="js-expandable-comment comment-truncated" data-component-bound="true">
            <span class="js-content-toggleable">We apologize for the experience you had. &nbsp;As you can see from the 1000s of 5-Star reviews we have,…</span>
            <span class="js-content-toggleable hidden">We apologize for 
the experience you had. &nbsp;As you can see from the 1000s of 5-Star 
reviews we have, this isn’t how we handle our customers. &nbsp;If you 
don’t mind, we would like to find out more about what happened during 
your experience. &nbsp;A manager will reach out to you to ensure that 
you are a satisfied customer. &nbsp;Also, thank you for taking the time 
to write a review. &nbsp;All reviews help us improve our business 
practices and policies.</span>
            <a href="#" class="read-more js-review-expander">Read more</a>
            <div class="review-footer js-content-toggleable hidden clearfix">
                
            </div>
        </span>

    </div>



        </div>
    </div>

        </li>

------------------------------------

user_name
        <li class="user-name">
                    <span class="user-display-name" data-hovercard-id="eG69Fl_0b8sAo5nP_vUHEw">David G.</span>

user-location
        <li class="user-location responsive-hidden-small">
            <b>Fremont, CA</b>

rating
    <div class="i-stars i-stars--regular-1 rating-large" title="1.0 star rating">

date
    <span class="rating-qualifier">
        2/8/2023

review_str
    <p lang="en">Never again. Forget their glossy brochures 
    and supposed relations with paid Hollywood stars. In the end they are an
        aggressive sales orgranization that tell you what you want to hear in 
    order to sell you overpriced unmarked unknown gold and silver that has 
    little market vale.</p> 
*/
int business_review_analyzer::read_review_file_yelp_not_recommended(){
int err_cnt = 0;

size_t line_count = 0;
std::ifstream ifs(m_file_name.c_str());
yelp_nrr_parse_state parse_state = YELP_NRR_PARSE_STATE_SEARCHING_FOR_REVIEW;
business_review *review = nullptr;
size_t max_line_length = 0;
while(ifs.good()){
    std::string line;
    std::getline(ifs,line);
    ++line_count;
    if(line.length() > max_line_length){
        max_line_length = line.length();
        }

    static const std::string name_pre_open_tag = "<span class=\"user-display-name\"";
    static const std::string name_open_tag = "\">";
    static const std::string name_close_tag = "</span>";

    static const std::string location_pre_open_tag = "class=\"user-location";
    static const std::string location_open_tag = "<b>";
    static const std::string location_close_tag = "</b>";

    static const std::string rating_pre_open_tag = "class=\"i-stars";
    static const std::string rating_open_tag = "title=\"";
    static const std::string rating_close_tag = "star rating\">";

    static const std::string date_pre_open_tag = "class=\"rating-qualifier\">";

    static const std::string review_str_open_tag = "<p lang=\"en\">";
    static const std::string review_str_close_tag = "</p>";


    const std::string::size_type name_pre_open_tag_pos = line.find(name_pre_open_tag);
    const std::string::size_type location_pre_open_tag_pos = line.find(location_pre_open_tag);
    const std::string::size_type rating_pre_open_tag_pos = line.find(rating_pre_open_tag);
    const std::string::size_type date_pre_open_tag_pos = line.find(date_pre_open_tag);
    const std::string::size_type review_str_open_tag_pos = line.find(review_str_open_tag);

        
    bool should_create_review = false;
    if(std::string::npos != name_pre_open_tag_pos ){
        parse_state = YELP_NRR_PARSE_STATE_SEARCHING_FOR_USER_NAME;
        should_create_review = true;
        }
    else if(std::string::npos != location_pre_open_tag_pos ){
        parse_state = YELP_NRR_PARSE_STATE_SEARCHING_FOR_LOCATION;
        should_create_review = (nullptr == review);
        }
    else if(std::string::npos != rating_pre_open_tag_pos ){
        parse_state = YELP_NRR_PARSE_STATE_SEARCHING_FOR_RATING;
        should_create_review = (nullptr == review);
        }
    else if(std::string::npos != date_pre_open_tag_pos ){
        parse_state = YELP_NRR_PARSE_STATE_SEARCHING_FOR_DATE;
        should_create_review = (nullptr == review);
        }
    else if(std::string::npos != review_str_open_tag_pos ){
        parse_state = YELP_NRR_PARSE_STATE_SEARCHING_FOR_REVIEW_STR;
        should_create_review = (nullptr == review);
        }

    if(should_create_review){
        m_reviews.push_back(business_review());
        review = &(m_reviews.back());
        review->m_review_type = BUSINESS_REVIEW_TYPE_YELP_NOT_RECOMMENDED;
        }

    switch(parse_state){
        case YELP_NRR_PARSE_STATE_SEARCHING_FOR_REVIEW:
            break;
        case YELP_NRR_PARSE_STATE_SEARCHING_FOR_USER_NAME:
            if(std::string::npos != name_pre_open_tag_pos ){
                const std::string::size_type name_open_tag_pos = 
                    line.find(name_open_tag,
                        name_pre_open_tag_pos + name_pre_open_tag.length());
                const std::string::size_type name_start_pos =
                    (std::string::npos == name_open_tag_pos) ? std::string::npos :
                    name_open_tag_pos + name_open_tag.length();
                const std::string::size_type name_close_tag_pos =
                    (std::string::npos == name_start_pos) ? std::string::npos : 
                    line.find(name_close_tag, name_start_pos);
                if( (std::string::npos != name_start_pos) &&
                    (std::string::npos != name_close_tag_pos) &&
                    (name_start_pos < name_close_tag_pos) ){
                    const std::string::size_type name_len =
                        name_close_tag_pos - name_start_pos;
                    review->m_full_name = line.substr(name_start_pos, name_len);
                    split_name(review->m_full_name, &(review->m_parsed_name) );
                    }
                parse_state = YELP_NRR_PARSE_STATE_SEARCHING_FOR_LOCATION;
                }
            break;
        case YELP_NRR_PARSE_STATE_SEARCHING_FOR_LOCATION:
            {
            if(std::string::npos != location_pre_open_tag_pos ){
                parse_state = YELP_NRR_PARSE_STATE_PARSING_LOCATION;
                }
            }
            break;
        case YELP_NRR_PARSE_STATE_PARSING_LOCATION:
            {
            const std::string::size_type location_open_tag_pos = 
                line.find(location_open_tag);
            const std::string::size_type location_start_pos =
                (std::string::npos == location_open_tag_pos) ? std::string::npos :
                location_open_tag_pos + location_open_tag.length();
            const std::string::size_type location_close_tag_pos =
                (std::string::npos == location_start_pos) ? std::string::npos : 
                line.find(location_close_tag, location_start_pos);
            if( (std::string::npos != location_start_pos) &&
                (std::string::npos != location_close_tag_pos) &&
                (location_start_pos < location_close_tag_pos) ){
                const std::string::size_type location_len =
                    location_close_tag_pos - location_start_pos;
                review->m_full_location_name =
                    line.substr(location_start_pos, location_len);
                init_city_state_from_full_location_name(review);
                parse_state = YELP_NRR_PARSE_STATE_SEARCHING_FOR_RATING;
                }
            }
            break;
        case YELP_NRR_PARSE_STATE_SEARCHING_FOR_RATING:
            if(std::string::npos != rating_pre_open_tag_pos ){
                const std::string::size_type rating_open_tag_pos = 
                    line.find(rating_open_tag,
                        rating_pre_open_tag_pos + rating_pre_open_tag.length());
                const std::string::size_type rating_start_pos =
                    (std::string::npos == rating_open_tag_pos) ? std::string::npos :
                    rating_open_tag_pos + rating_open_tag.length();
                const std::string::size_type rating_close_tag_pos =
                    (std::string::npos == rating_start_pos) ? std::string::npos : 
                    line.find(rating_close_tag, rating_start_pos);
                if( (std::string::npos != rating_start_pos) &&
                    (std::string::npos != rating_close_tag_pos) &&
                    (rating_start_pos < rating_close_tag_pos) ){
                    const std::string::size_type rating_len =
                        rating_close_tag_pos - rating_start_pos;
                    const std::string rating_str = 
                        line.substr(rating_start_pos, rating_len);
                    const double rating_dbl = atof(rating_str.c_str());
                    review->m_star_count = static_cast<int>(round(rating_dbl));
                    }
                parse_state = YELP_NRR_PARSE_STATE_SEARCHING_FOR_DATE_PRE_TOKEN;
                }
            break;
        case YELP_NRR_PARSE_STATE_SEARCHING_FOR_DATE_PRE_TOKEN:
            if(std::string::npos != date_pre_open_tag_pos ){
                parse_state = YELP_NRR_PARSE_STATE_SEARCHING_FOR_DATE;
                }
            break;
        case YELP_NRR_PARSE_STATE_SEARCHING_FOR_DATE:
            {
            /* 2/15/2023 */
            const std::string::size_type first_slash_pos = line.find("/");
            const std::string::size_type second_slash_pos = 
                (std::string::npos == first_slash_pos) ?
                std::string::npos : line.find("/", first_slash_pos + 1);
            if( (std::string::npos != first_slash_pos) &&
                (std::string::npos != second_slash_pos) &&
                (first_slash_pos > 2 ) &&
                (second_slash_pos + 4 < line.length() ) ){
                const std::string::size_type date_start_pos = first_slash_pos - 2;
                const std::string::size_type date_len = 
                    (second_slash_pos + 5) - date_start_pos;                
                review->m_date_str = line.substr(date_start_pos, date_len);
                review->m_time_stamp_tm = parse_date_mdy_slash( review->m_date_str );
                review->m_time_stamp = mktime((review->m_time_stamp_tm).get());
                static const time_t time_err_seconds = 12*60*60;
                review->m_time_stamp_min = (review->m_time_stamp > time_err_seconds) ?
                    (review->m_time_stamp - time_err_seconds) : 0;
                review->m_time_stamp_max = review->m_time_stamp + time_err_seconds;
                parse_state = YELP_NRR_PARSE_STATE_SEARCHING_FOR_REVIEW_STR;
                }
            }
            break;
        case YELP_NRR_PARSE_STATE_SEARCHING_FOR_REVIEW_STR:
            if( std::string::npos != review_str_open_tag_pos ){
                const std::string::size_type review_str_start_pos =
                    review_str_open_tag_pos + review_str_open_tag.length();
                const std::string::size_type review_str_close_tag_pos =
                    line.find(review_str_close_tag, review_str_start_pos );   
                std::string review_str_line;             
                if( std::string::npos == review_str_close_tag_pos ){
                    review_str_line = line.substr(review_str_start_pos);
                    }
                else if( review_str_start_pos < review_str_close_tag_pos ){
                    const std::string::size_type review_str_line_length =
                        review_str_close_tag_pos - review_str_start_pos;
                    review_str_line = line.substr(review_str_start_pos, review_str_line_length);
                    parse_state = YELP_NRR_PARSE_STATE_SEARCHING_FOR_REVIEW;
                    }
                trim_str(&review_str_line);
                review->m_review_str = review_str_line;
                parse_state = YELP_NRR_PARSE_STATE_PARSING_REVIEW_STR;
                }
            break;
        case YELP_NRR_PARSE_STATE_PARSING_REVIEW_STR:
                {
                const std::string::size_type review_str_close_tag_pos =
                    line.find(review_str_close_tag);   
                std::string review_str_line;             
                if( std::string::npos == review_str_close_tag_pos ){
                    review_str_line = line;
                    }
                else{
                    review_str_line = line.substr(0, review_str_close_tag_pos);
                    parse_state = YELP_NRR_PARSE_STATE_SEARCHING_FOR_REVIEW;
                    }
                trim_str(&review_str_line);
                if( !review_str_line.empty() ){
                    review->m_review_str.append(" ");
                    review->m_review_str.append(review_str_line);
                    }
                }
            break;

        case YELP_NRR_PARSE_STATE_DONE:
        default:
            break;
        }
    }

remove_reviews_violating_yelp_terms_of_service();

sort_reviews_by_timestamp();

std::cout << "total review_count=" << m_reviews.size() << "\n";
std::cout << "line_count=" << line_count << "\n";

return err_cnt;
}



int business_review_analyzer::attempt_equalize_timestamp_intervals(){
int err_cnt = 0;

if( m_reviews.size() > 2 ){
    static const size_t iteration_limit = 100000;
    static const time_t threshold_delta_time_seconds = 15;

    size_t iteration = 0;
    time_t max_delta_time_seconds = threshold_delta_time_seconds * 2;
    while( (iteration < iteration_limit) && 
         ( max_delta_time_seconds > threshold_delta_time_seconds ) ){
        max_delta_time_seconds = 0;
        
        business_review_vec_itr bbbrv_itr_a = m_reviews.begin();
        business_review_vec_itr bbbrv_itr_b = bbbrv_itr_a + 1;
        business_review_vec_itr bbbrv_itr_c = bbbrv_itr_b + 1;
        while(bbbrv_itr_c != m_reviews.end()){
            const time_t t_a     = bbbrv_itr_a->m_time_stamp;
            const time_t t_b_min = bbbrv_itr_b->m_time_stamp_min;
            const time_t t_b     = bbbrv_itr_b->m_time_stamp;
            const time_t t_b_max = bbbrv_itr_b->m_time_stamp_max;
            const time_t t_c     = bbbrv_itr_c->m_time_stamp;
            const time_t t_ac_mid = (t_a + t_c) / 2;
            const time_t t_b_new = (t_ac_mid < t_b_min) ? t_b_min :
                (t_ac_mid > t_b_max) ? t_b_max : t_ac_mid; 
            const time_t delta_time_seconds = 
                (t_b < t_b_new) ? t_b_new - t_b : t_b - t_b_new;
            if( delta_time_seconds > max_delta_time_seconds ) {
                max_delta_time_seconds = delta_time_seconds;
                }
            
            bbbrv_itr_b->m_time_stamp = t_b_new;

            /* advance */
            bbbrv_itr_a = bbbrv_itr_b;
            bbbrv_itr_b = bbbrv_itr_c;
            ++bbbrv_itr_c;
            }
        /* advance */
        ++iteration;
        }

    }

return err_cnt;
}

void business_review_analyzer::sort_reviews_by_timestamp(){
typedef std::pair<time_t, size_t> sort_item;
typedef std::vector<sort_item> sort_vec;
typedef sort_vec::const_iterator sort_vec_citr;

sort_vec v;
v.reserve(m_reviews.size());
for(size_t i = 0; i < m_reviews.size(); ++i){
    const business_review& r = m_reviews.at(i);
    const sort_item s(r.m_time_stamp, i);
    v.push_back(s);
    }

std::sort(v.begin(), v.end());

business_review_vec unsorted_reviews;
std::swap(unsorted_reviews, m_reviews);
assert(m_reviews.empty());

sort_vec_citr v_itr = v.begin();
for(; v.end() != v_itr; ++v_itr){    
    m_reviews.push_back(unsorted_reviews.at(v_itr->second));
    }
}

/* <p lang="en">This review has been removed for violating our Terms of Service</p> */
void business_review_analyzer::remove_reviews_violating_yelp_terms_of_service(){

static const std::string viol_str = "removed for violating our Terms of Service";

business_review_vec unfiltered_reviews;
std::swap(unfiltered_reviews, m_reviews);
assert(m_reviews.empty());
business_review_vec_citr r_itr = unfiltered_reviews.begin();
for(; unfiltered_reviews.end() != r_itr; ++r_itr) { 
    const business_review& r = *r_itr;
     if( r.m_review_str.find(viol_str) != std::string::npos){
        int breakpoint = 0;
        }
    else{
        m_reviews.push_back(r);
        }
    }
}


void business_review_analyzer::remove_invalid_reviews(){

business_review_vec unfiltered_reviews;
std::swap(unfiltered_reviews, m_reviews);
assert(m_reviews.empty());
business_review_vec_citr r_itr = unfiltered_reviews.begin();
for(; unfiltered_reviews.end() != r_itr; ++r_itr) { 
    const business_review& r = *r_itr;
     if( 0 == r.m_time_stamp || r.m_full_name.empty()){
        int breakpoint = 0;
        }
    else{
        m_reviews.push_back(r);
        }
    }

}


void business_review_analyzer::remove_invalid_reviews_outside_time_range(){

business_review_vec unfiltered_reviews;
std::swap(unfiltered_reviews, m_reviews);
assert(m_reviews.empty());
business_review_vec_citr r_itr = unfiltered_reviews.begin();
for(; unfiltered_reviews.end() != r_itr; ++r_itr) { 
    const business_review& r = *r_itr;
    if ( (r.m_time_stamp < m_start_time_stamp ) || 
         (r.m_time_stamp > m_end_time_stamp ) ){
        int breakpoint = 0;
        }
    else{
        m_reviews.push_back(r);
        }
    }

}


void business_review_analyzer::init_city_state_from_full_location_name(
    business_review *review){
const std::string::size_type comma_pos =
    (review->m_full_location_name).rfind(",");
if( std::string::npos == comma_pos ){
    review->m_city = review->m_full_location_name;
    }
else{
    review->m_city = (review->m_full_location_name).substr(0, comma_pos);
    review->m_state = (review->m_full_location_name).substr(comma_pos + 1);
    }
trim_str(&(review->m_city));
trim_str(&(review->m_state));
}


int business_review_analyzer::init_review_str_count_map(){
int err_cnt = 0;
m_review_str_count_map.clear();
business_review_vec_citr bbbrv_itr = m_reviews.begin();
for(; bbbrv_itr != m_reviews.end(); ++bbbrv_itr){
    const business_review& review = *bbbrv_itr;
    std::set<std::string> ss;
    std::vector<std::string> sv;
    split_str( review.m_review_str, &sv );
    std::vector<std::string>::const_iterator s_itr = sv.begin();
    for( ; sv.end() != s_itr; ++s_itr ){
        const std::string& s = *s_itr;
        if( ss.insert(s).second ){
            /* count each string once per review */
            if( m_review_str_count_map.find(s) == m_review_str_count_map.end()){
                m_review_str_count_map[s] = 1;
                }
            else{
                ++(m_review_str_count_map[s]);
                }
            }
        }
    }

return err_cnt;
}

int business_review_analyzer::init_review_count_str_vec(){
int err_cnt = 0;
m_review_count_str_vec.clear();
str_szt_map_citr map_itr = m_review_str_count_map.begin();
for(; m_review_str_count_map.end() != map_itr; ++map_itr ){
    const szt_str_pair x(map_itr->second, map_itr->first);
    m_review_count_str_vec.push_back(x);
    }
std::sort(m_review_count_str_vec.begin(), m_review_count_str_vec.end());

//std::cout << "review string, review count\n";
//for( size_t i = 0; i < 100 && i < m_review_count_str_vec.size(); ++i ){
//    const szt_str_pair& xx = m_review_count_str_vec.at(m_review_count_str_vec.size() - i -1);
//    std::cout << xx.second << "," << xx.first << "\n";
//    }
return err_cnt;
}


int business_review_analyzer::init_star_tv_map(){
int err_cnt = 0;

m_star_tv_map.clear();
time_t_vec *tv = nullptr;
business_review_vec_citr bbbrv_itr = m_reviews.begin();
for(; bbbrv_itr != m_reviews.end(); ++bbbrv_itr){
    const business_review& review = *bbbrv_itr;
    tv = &(m_star_tv_map[review.m_star_count]);
    tv->push_back(review.m_time_stamp);
    }

/* sort */
int_tv_map_itr map_itr = m_star_tv_map.begin();
for(; m_star_tv_map.end() != map_itr; ++map_itr ){
    tv = &(map_itr->second);
    std::sort(tv->begin(), tv->end());
    }

return err_cnt;
}


int business_review_analyzer::init_city_bins(){
int err_cnt = 0;

city_bins cb;

cb.init(20);

city_and_state cs;
business_review_vec_citr bbbrv_itr = m_reviews.begin();
for(; bbbrv_itr != m_reviews.end(); ++bbbrv_itr){
    const business_review& review = *bbbrv_itr;
    cs.m_city = review.m_city;
    cs.m_state = get_state_abbr(review.m_state);
    cb.add_to_bin_tally(cs);
    }

std::string out_file_name = m_file_name + ".citypop.csv";
std::ofstream ofs(out_file_name);


ofs << "bin,small city,state,population,large city,state,population,review tally\n";

for( size_t j = 0; j < cb.get_bin_count(); ++j ){
    const city_bin& b = cb.get_bin(j);
    //std::cout << "city_bin[" << j << "]"
    //    << "  small city=" << b.m_small_city.m_city << "," << b.m_small_city.m_state
    //    << "(" << b.m_small_city_population << ") "
    //    << "  large city=" << b.m_large_city.m_city << "," << b.m_large_city.m_state
    //    << "(" << b.m_large_city_population << ") "
    //    << "  review tally=" << b.m_tally 
    //    << "\n";
    ofs << j << ","
      << b.m_small_city.m_city << "," 
      << b.m_small_city.m_state << ","  
      << b.m_small_city_population << "," 
      << b.m_large_city.m_city << "," 
      << b.m_large_city.m_state << ","  
      << b.m_large_city_population << ","
       << b.m_tally << "\n"; 
    }



std::string cityunk_file_name = m_file_name + ".cityunk.csv";
std::ofstream ufs(cityunk_file_name);
ufs << "unrecognized city and state\n";
for(size_t k = 0; k < cb.get_unrecognized_cs_count(); ++k ){
    const city_and_state& ucs = cb.get_unrecognized_city_state(k);
    ufs << k << "," << ucs.m_city << "," << ucs.m_state << "\n";
    }

return err_cnt;
}

/* count first name letters and last name letters 
   
https://math.answers.com/statistics/What_is_the_percent_distribution_of_first_letters_in_last_names_in_the_US

http://answers.google.com/answers/threadview/id/347668.html
*/
int business_review_analyzer::init_first_letter_tallies(){
int err_cnt = 0;

static const double first_name_expected_ratio[] = {
    0.0650821166,   /* A */
    0.0464808765,   /* B */
    0.0722381492,   /* C */
    0.0740493811,   /* D */
    0.0461586328,   /* E */
    0.0174344956,   /* F */
    0.032635509,    /* G */
    0.0225348357,   /* H */
    0.0073449341,   /* I */
    0.1329088606,   /* J */
    0.0412138587,   /* K */
    0.0565259906,   /* L */
    0.1008289442,   /* M */
    0.0183567793,   /* N */
    0.0045225237,   /* O */
    0.0313465342,   /* P */
    0.0003555793,   /* Q */
    0.0794608529,   /* R */
    0.0620374692,   /* S */
    0.042169478,    /* T */
    0.000222237,    /* U */
    0.0149676645,   /* V */
    0.0275240572,   /* W */
    0.0001111185,   /* X */
    0.0022779296,   /* Y */
    0.0012111919    /* Z */
    };

static const double last_name_expected_ratio[] = {
    0.0374962504,   /* A */
    0.0895910409,   /* B */
    0.0637936206,   /* C */
    0.0564943506,   /* D */
    0.0202979702,   /* E */
    0.0362963704,   /* F */
    0.0533946605,   /* G */
    0.0558944106,   /* H */
    0.0075992401,   /* I */
    0.0135986401,   /* J */
    0.0569943006,   /* K */
    0.0523947605,   /* L */
    0.0827917208,   /* M */
    0.0212978702,   /* N */
    0.0162983702,   /* O */
    0.0526947305,   /* P */
    0.00259974,     /* Q */
    0.0479952005,   /* R */
    0.1092890711,   /* S */
    0.0387961204,   /* T */
    0.00469953,     /* U */
    0.0254974503,   /* V */
    0.0345965403,   /* W */
    0.00019998,     /* X */
    0.0064993501,   /* Y */
    0.0128987101    /* Z */
    };





m_first_name_letter_tallies.clear();
m_first_name_letter_tallies.resize(26,0);
double first_name_letter_total_tally = 0;
m_last_name_letter_tallies.clear();
m_last_name_letter_tallies.resize(26,0);
double last_name_letter_total_tally = 0;

business_review_vec_citr bbbrv_itr = m_reviews.begin();
for(; bbbrv_itr != m_reviews.end(); ++bbbrv_itr){
    const business_review& review = *bbbrv_itr;
    if( !review.m_parsed_name.empty()){
        const std::string& first_name = review.m_parsed_name.front();
        const char ch1 = (first_name.empty()) ? ' ' : first_name.at(0);
        size_t i = 26;
        if( ( 'a' <= ch1 ) && ( ch1 <= 'z' ) ){
            i = static_cast<size_t>(ch1 - 'a');
            }
        else if( ( 'A' <= ch1 ) && ( ch1 <= 'Z' ) ){
            i = static_cast<size_t>(ch1 - 'A');
            }
        if( i < 26 ){
            ++(m_first_name_letter_tallies.at(i));
            ++first_name_letter_total_tally;
            }

        if( review.m_parsed_name.size() > 1){
            const std::string& last_name = review.m_parsed_name.back();
            const char ch2 = (last_name.empty()) ? ' '  : last_name.at(0);
            size_t j = 26;
            if( ( 'a' <= ch2 ) && ( ch2 <= 'z' ) ){
                j = static_cast<size_t>(ch2 - 'a');
                }
            else if( ( 'A' <= ch2 ) && ( ch2 <= 'Z' ) ){
                j = static_cast<size_t>(ch2 - 'A');
                }
            if( j < 26 ){
                ++(m_last_name_letter_tallies.at(j));
                ++last_name_letter_total_tally;
                }
            }
        }
    }

std::string out_file_name = m_file_name + ".namltr.csv";
std::ofstream ofs(out_file_name);

ofs << "letter,first name count,expected,last name count,expected\n";

for( size_t k = 0; k < 26; ++k ){
    const char ch = 'A' + static_cast<char>(k);
    const double expected_first_name_tally = first_name_expected_ratio[k] *
        static_cast<double>(first_name_letter_total_tally);
    const double expected_last_name_tally = last_name_expected_ratio[k] *
        static_cast<double>(last_name_letter_total_tally);
    ofs << ch << ","
      << m_first_name_letter_tallies.at(k) << "," 
      << expected_first_name_tally << ","  
      << m_last_name_letter_tallies.at(k) << "," 
      << expected_last_name_tally << "\n"; 
    }


return err_cnt;
}

size_t business_review_analyzer::get_review_count_in_period(
    const int& star_count, const time_t& period_start,
    const time_t& period_end ) const {
size_t result = 0;
const int_tv_map_citr map_itr = m_star_tv_map.find(star_count);
if( m_star_tv_map.end() != map_itr ){
    const time_t_vec * const tv = &(map_itr->second);
    time_t_vec_citr tv_itr_a =
        std::lower_bound(tv->begin(), tv->end(), period_start);
    time_t_vec_citr tv_itr_b =
        std::lower_bound(tv->begin(), tv->end(), period_end);
    result = static_cast<size_t>(std::distance(tv_itr_a, tv_itr_b));
    }
return result;
}


void business_review_analyzer::write_review_count_table(std::ostream *os) const{
if( nullptr != os ){
    int star_count_low = 0;
    int star_count_high = 5;
    int star_count;

    /* header */
    (*os) << "period_end_time_t,period_end_date";
    for( star_count = star_count_low; star_count <= star_count_high; ++star_count ){
        (*os) << "," << star_count;
        }
    (*os) << "\n";

    /* time range (min,max) */
    static const time_t time_stamp_undefined = static_cast<time_t>(-1);
    time_t time_stamp_min = time_stamp_undefined;
    time_t time_stamp_max = time_stamp_undefined;
    int_tv_map_citr map_itr = m_star_tv_map.begin();
    for(; m_star_tv_map.end() != map_itr; ++map_itr ){
        const time_t_vec * const tv = &(map_itr->second);
        if( !(tv->empty()) ){
            time_t t_min = tv->front();
            time_t_vec_citr t_itr = tv->begin();
            while( (tv->end() != t_itr) && ( 0 == t_min ) ){
                ++t_itr;
                }
            if( ( ( time_stamp_undefined == time_stamp_min ) ||
                  ( t_min < time_stamp_min) ) && (t_min > 0 ) ){
                time_stamp_min = t_min;
                }
            const time_t& t_max = tv->back();
            if( ( time_stamp_undefined == time_stamp_max ) ||
                ( t_max > time_stamp_max ) ){
                time_stamp_max = t_max;
                }
            }
        }

    /* constrain by input parameters */
    if( m_start_time_stamp > 0 ){
        time_stamp_min = m_start_time_stamp;
        }

    if( m_end_time_stamp > time_stamp_min ){
        time_stamp_max = m_end_time_stamp;
        }

    /* */
    if( time_stamp_undefined != time_stamp_min ){   
        static const time_t interval = 60 * 60 * 24 * 1; /* 1 day */

        static const time_t t_start = ( time_stamp_min < interval ) ?
            0 : (interval * ((time_stamp_min / interval) -  1));
        time_t ta = t_start;
        time_t tb = ta + interval;        
        for( time_t ta = t_start; ta <= time_stamp_max; ta=tb, tb += interval ){
            (*os) << tb;            
            struct tm *ts = localtime( &tb );
            static const size_t time_buf_sz = 32;
            char time_buf[time_buf_sz];
            strftime(time_buf, time_buf_sz, "%Y/%m/%d", ts);
            (*os) << "," << time_buf;
            for( star_count = star_count_low;
                star_count <= star_count_high; ++star_count ){
                const size_t review_count =
                    get_review_count_in_period( star_count, ta, tb);
                (*os) << "," << review_count;
                }
            (*os) << "\n";
            }
        }    
    }
}


int business_review_analyzer::write_review_count_table(){
int err_cnt = 0;

std::string out_file_name = m_file_name + ".rvcnt.csv";
std::ofstream ofs(out_file_name);
write_review_count_table(&ofs);
return err_cnt;
}


void business_review_analyzer::write_weekday_summary_table(std::ostream *os) const{
static const char *wkday_names[] = 
    { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

typedef std::vector<size_t> szt_vec;
typedef std::vector<szt_vec> szt_vec_vec;

if( nullptr != os ){
    static const szt_vec six_zeroes(6, 0);
    szt_vec_vec svv(7, six_zeroes);

    /* count weekdays by star count */
    business_review_vec_citr bbbrv_itr = m_reviews.begin();
    for(; bbbrv_itr != m_reviews.end(); ++bbbrv_itr){
        const business_review& review = *bbbrv_itr;
        const int wkday = (nullptr != review.m_time_stamp_tm) ?
             review.m_time_stamp_tm->tm_wday : 0;
        szt_vec *sv = &(svv.at(wkday % 7 ) );
        const int star_count =
            ( review.m_star_count > 5 ) ? 5 : review.m_star_count;
        ++(sv->at(star_count));
        }

    (*os) << "stars";
    for( int s = 0; s < 6; ++ s ){
        (*os) << "," << s;
        }
    (*os) << "\n";
    for( size_t w = 0; w < 7; ++w ){

        (*os) << wkday_names[w];
        const szt_vec *csv = &(svv.at(w) );
        for( size_t ss = 0; ss < 6; ++ss ){
            const size_t c = csv->at(ss);
            (*os) << "," << c;
            }
        (*os) << "\n";
        }
    }
}


int business_review_analyzer::write_weekday_summary_table(){
int err_cnt = 0;

std::string out_file_name = m_file_name + ".wkday.csv";
std::ofstream ofs(out_file_name);
write_weekday_summary_table(&ofs);
return err_cnt;
}


void business_review_analyzer::write_weekhour_summary_table(std::ostream *os) const{
static const char *wkday_names[] = 
    { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

static const size_t hours_per_week = 7 * 24;

typedef std::vector<size_t> szt_vec;
typedef std::vector<szt_vec> szt_vec_vec;

if( nullptr != os ){
    static const szt_vec six_zeroes(6, 0);
    szt_vec_vec svv(hours_per_week, six_zeroes);

    /* count weekdays by star count */
    business_review_vec_citr bbbrv_itr = m_reviews.begin();
    for(; bbbrv_itr != m_reviews.end(); ++bbbrv_itr){
        const business_review& review = *bbbrv_itr;
        const int wkday = (nullptr != review.m_time_stamp_tm) ?
             review.m_time_stamp_tm->tm_wday : 0;
        const int hour = (nullptr != review.m_time_stamp_tm) ?
             review.m_time_stamp_tm->tm_hour : 0;
        const int wh = (24*wkday) + hour;
        
        szt_vec *sv = &(svv.at(wh % hours_per_week ) );
        const int star_count =
            ( review.m_star_count > 5 ) ? 5 : review.m_star_count;
        ++(sv->at(star_count));
        }

    (*os) << "wkhr,day,hour";
    for( int s = 0; s < 6; ++ s ){
        (*os) << "," << s << ((1 == s) ? "star" : "stars");
        }
    (*os) << "\n";
    for( size_t wh = 0; wh < hours_per_week; ++wh ){
        (*os) << wh;
        const size_t w = wh/24;
        (*os) << "," << wkday_names[w%7];
        const size_t hr = wh%24;
        (*os) << "," << hr;
        const szt_vec *csv = &(svv.at(wh) );
        for( size_t ss = 0; ss < 6; ++ss ){
            const size_t c = csv->at(ss);
            (*os) << "," << c;
            }
        (*os) << "\n";
        }
    }
}

int business_review_analyzer::write_weekhour_summary_table(){
int err_cnt = 0;

std::string out_file_name = m_file_name + ".wkhr.csv";
std::ofstream ofs(out_file_name);
write_weekhour_summary_table(&ofs);
return err_cnt;
}

void business_review_analyzer::write_full_table(std::ostream *os) const{

static const char *wkday_names[] = 
    { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

if( nullptr != os ){
    static const std::string delim = "\t";

    (*os) << "index" 
        << delim << "review_type"
        << delim << "time_stamp_s"
        << delim << "time_stamp_s_minus"
        << delim << "time_stamp_ss_plus"
        << delim << "date"
        << delim << "weekday"
        << delim << "year"
        << delim << "month" 
        << delim << "day"
        << delim << "hour"
        << delim << "minute"
        << delim << "second"
        << delim << "reviewer_first_name" 
        << delim << "reviewer_middle_name"
        << delim << "reviewer_last_name"
        << delim << "rating"
        << delim << "city"
        << delim << "state" 
        << delim << "review" 
        << "\n";

    for( size_t i = 0; i < m_reviews.size(); ++i ){
        const business_review& r = m_reviews.at(i);

        const std::string& review_type_str =
            business_review_type_to_str(r.m_review_type);

        int wday_int = 0;
        int year_int = 0;
        int month_int = 0;
        int day_int = 0;
        int hour_int = 12;
        int minute_int = 0;
        int second_int = 0;
        if( r.m_time_stamp_tm.get() != nullptr ){
            wday_int = r.m_time_stamp_tm->tm_wday;
            year_int = r.m_time_stamp_tm->tm_year + 1900;
            month_int = r.m_time_stamp_tm->tm_mon + 1;
            day_int = r.m_time_stamp_tm->tm_mday;
            hour_int = r.m_time_stamp_tm->tm_hour;
            minute_int = r.m_time_stamp_tm->tm_min;
            second_int = r.m_time_stamp_tm->tm_sec;
            }
        char date_buf[32];
        sprintf(date_buf, "%04i/%02i/%02i", year_int, month_int, day_int);
        const std::string date_str(date_buf);

        const std::string weekday_str( wkday_names[wday_int % 7] );

        const size_t time_tol_seconds_minus =
            (r.m_time_stamp_min < r.m_time_stamp) ?
            r.m_time_stamp - r.m_time_stamp_min : 0;
        const size_t time_tol_seconds_plus =
            (r.m_time_stamp < r.m_time_stamp_max) ?
            r.m_time_stamp_max - r.m_time_stamp : 0;

        static const std::string empty_str;        
        const std::string& reviewer_first_name = 
            (!r.m_parsed_name.empty()) ? r.m_parsed_name.front() : empty_str;
        const std::string& reviewer_middle_name = 
            (r.m_parsed_name.size() > 1) ? r.m_parsed_name.at(1) : empty_str;
        const std::string& reviewer_last_name = 
            (r.m_parsed_name.size() > 2) ? r.m_parsed_name.at(2) : empty_str;

        (*os) << (i+1)
            << delim << review_type_str
            << delim << r.m_time_stamp
            << delim << time_tol_seconds_minus
            << delim << time_tol_seconds_plus
            << delim << date_str
            << delim << weekday_str
            << delim << year_int
            << delim << month_int
            << delim << day_int
            << delim << hour_int
            << delim << minute_int
            << delim << second_int
            << delim << reviewer_first_name
            << delim << reviewer_middle_name
            << delim << reviewer_last_name
            << delim << r.m_star_count
            << delim << r.m_city
            << delim << r.m_state
            << delim << r.m_review_str
            << "\n";
        }
    }
}

int business_review_analyzer::write_full_table(){
int err_cnt = 0;

std::string out_file_name = m_file_name + ".full.tsv";
std::ofstream ofs(out_file_name);
write_full_table(&ofs);
return err_cnt;
}



/*
command line:
  business_review_analyzer.exe --start-date-time 2012-01-01T12:00:00 --end-date-time 2024-04-20T12:00:00 file1.txt file2.htm file3.txt 
*/
int main( int argc, char *argv[] )
{
int result = business_review_analyzer::run_main(argc, argv);
return result;
}

