#include "surf/config.hpp"
#include "surf/indexes.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <sstream>

using namespace std;
using namespace sdsl;
using namespace surf;

typedef struct cmdargs {
    std::string collection_dir = "";
    std::string query_file = "";
    uint64_t k = 10;
    bool verbose = false;
    bool multi_occ = false;
    bool match_only = false;
    bool correctness = false;
} cmdargs_t;

void
print_usage(char* program)
{
    fprintf(stdout,"%s -c <collection directory> -q <query file> other options\n", program);
    fprintf(stdout,"where\n");
    fprintf(stdout,"  -c <collection directory>  : the directory the collection is stored.\n");
    fprintf(stdout,"  -q <query file>  : the queries to be performed.\n");
    fprintf(stdout,"  -k <top-k>  : the top-k documents to be retrieved for each query.\n");
    fprintf(stdout,"  -v <verbose>  : verbose mode.\n");
    fprintf(stdout,"  -m <multi_occ>  : only retrieve documents which contain the term more than once.\n");
    fprintf(stdout,"  -o <multi_occ>  : only match pattern; no document retrieval.\n");
    fprintf(stdout,"  -f <correctness>  : correctness check (super-slow).\n");
};

cmdargs_t
parse_args(int argc,char* const argv[])
{
    cmdargs_t args;
    int op;
    args.collection_dir = "";
    args.query_file = "";
    args.k = 10;
    while ((op=getopt(argc,argv,"c:q:k:vmof")) != -1) {
        switch (op) {
            case 'c':
                args.collection_dir = optarg;
                break;
            case 'q':
                args.query_file = optarg;
                break;
            case 'k':
                args.k = std::strtoul(optarg,NULL,10);
                break;
            case 'v':
                args.verbose = true;
                break;
            case 'm':
                args.multi_occ = true;
                break;
            case 'o':
                args.match_only = true;
                break;
            case 'f':
                args.correctness = true;
                break;
            case '?':
            default:
                print_usage(argv[0]);
        }
    }
    if (args.collection_dir==""||args.query_file=="") {
        std::cerr << "Missing command line parameters.\n";
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    return args;
}



using idx_type = INDEX_TYPE;

const size_t buf_size=1024*128;
char   buffer[buf_size];

template<typename X>
struct myline {
    static string parse(char* str) {
        return string(str);
    }
};

template<>
struct myline<sdsl::int_alphabet_tag> {
    static vector<uint64_t> parse(char* str) {
        vector<uint64_t> res;
        stringstream ss(str);
        uint64_t x;
        while (ss >> x) {
            res.push_back(x);
        }
        return res;
    }
};


struct correctness_check {
    public:
        typedef tuple<size_t, size_t> t_result;
        cache_config                    m_collection;
        idx_type                        m_idx;
        string                          m_query;
        vector<t_result>                m_result;
        idx_type::csa_type              m_csa;
        idx_type::border_type           m_border;
        idx_type::border_rank_type      m_border_rank;
        idx_type::border_select_type    m_border_select;



    correctness_check(const idx_type& idx, const vector<t_result>& res) :
            m_result(res), m_csa(idx.m_csa), m_border(idx.m_border),
            m_border_rank(idx.m_border_rank), m_border_select(idx.m_border_select)

        { }

        template<typename t_pat_iter>
        bool check(t_pat_iter begin, t_pat_iter end, bool multi_occ, bool only_match) {
            vector<t_result> check_result;
            map<uint64_t, uint64_t> matches;

            vector<t_result> r;
            uint64_t m_sp, m_ep;

            bool valid = backward_search(m_csa, 0, m_csa.size() - 1, begin, end, m_sp, m_ep) > 0;
            if (valid) {
                for (size_t i = m_sp; i <= m_ep; i++) {
                    auto document = m_border_rank(m_csa[i]);
                    matches[document]++;
                }
            }
            for (const auto& x : matches) {
                r.push_back({get<0>(x), get<1>(x)});
            }

            size_t k = 0;
            if (r.size() < m_result.size()) {
                cerr << "Error there are less real results!" << endl;
                return false;
            }
            if (r.size() == 0 and m_result.size() == 0) {
                return true;
            }

            for (const auto& it : m_result) {
                auto grid_doc_id = get<0>(it);
                auto grid_freq = get<1>(it);
                auto pos = lower_bound(r.begin(),r.end(),grid_doc_id, [](const t_result& l, size_t r) {
                    return get<0>(l) < r;
                });

                if (get<0>(*pos) != grid_doc_id) {
                    cerr << "Error at doc result k = " << k <<  " document " << grid_doc_id << " not found" << endl;
                    return false;
                } else {
                    if (get<1>(*pos) != grid_freq) {
                        cerr << "Error at freq result k = " << k << " freq does not  " << get<1>(*pos) << " != " << " grid_freq" << endl;
                        return false;
                    }
                }
                k++;
            }
            return true;
        }
};


int main(int argc, char* argv[])
{

    cmdargs_t args = parse_args(argc,argv);
    idx_type idx;
    typedef tuple<size_t,size_t> t_result;
    vector<t_result> result;

    using timer = chrono::high_resolution_clock;

    if (!args.verbose) {
        cout<<"# collection_file = "<< args.collection_dir <<endl;
        cout<<"# index_name = "<< IDXNAME << endl;
    }
    auto cc = surf::parse_collection<idx_type::alphabet_category>(args.collection_dir);
    idx.load(cc);
    if (!args.verbose) {
        cout<<"# pattern_file = "<<args.query_file<<endl;
        cout<<"# doc_cnt = "<<idx.doc_cnt()<<endl;
        cout<<"# word_cnt = "<<idx.word_cnt()<<endl;
        cout<<"# k = "<<args.k <<endl;
        cout<<"# match_only = "<<args.match_only<<std::endl;
        cout<<"# multi_occ = "<<args.multi_occ<<std::endl;
    }
    ifstream in(args.query_file);
    if (!in) {
        cerr << "Could not load pattern file" << endl;
        return 1;
    }

    using timer = chrono::high_resolution_clock;
    size_t q_len = 0;
    size_t q_cnt = 0;
    size_t sum = 0;
    size_t sum_fdt = 0;
    bool tle = false; // flag: time limit exceeded
    auto start = timer::now();
    while (!tle and in.getline(buffer, buf_size)) {
        auto q_start = timer::now();
        auto query = myline<idx_type::alphabet_category>::parse(buffer);
        q_len += query.size();
        ++q_cnt;
        size_t x = 0;
        auto res_it = idx.topk(query.begin(), query.end(),args.multi_occ,args.match_only);
        while ( x < args.k and res_it ){
            ++x;
            sum_fdt += (*res_it).second;
            if ( args.verbose ) {
                cout<<q_cnt<<";"<<x<<";"<<(*res_it).first<< ";"<<(*res_it).second << endl;
            }
            if (args.correctness) {
                result.push_back({(*res_it).first,(*res_it).second});
            }
            if ( x < args.k ) 
                ++res_it;
        }
        if (args.correctness) {
            correctness_check ck(idx,result);
            ck.check(query.begin(), query.end(), args.multi_occ, args.match_only);
            result.clear();
            if (q_cnt % 100 == 0) {
                cout << q_cnt << endl;
            }
        }
        sum += x;
        auto q_time = timer::now()-q_start;
        // single query should not take more then 5 seconds
        if (chrono::duration_cast<chrono::seconds>(q_time).count() > 5 and !args.correctness) {
            tle = true;
        }
    }
    auto stop = timer::now();
    auto elapsed = stop-start;
    if ( !args.verbose ){
        cout<<"# TLE = " << tle << endl;
        cout<<"# query_len = "<<q_len/q_cnt<<endl;
        cout<<"# queries = " <<q_cnt <<endl;
        auto exec_time = chrono::duration_cast<chrono::microseconds>(elapsed).count();
        cout<<"# time_per_query = "<< exec_time/q_cnt <<endl;
        auto doc_time = sum == 0 ? 0.0 : ((double)exec_time)/(sum);
        cout<<"# time_per_doc = " << doc_time << endl;
        cout<<"# check_sum = "<<sum<<endl;
        cout<<"# check_sum_fdt = "<<sum_fdt<<endl;
    }
}
