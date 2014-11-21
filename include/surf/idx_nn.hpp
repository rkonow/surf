#ifndef SURF_IDX_NN_HPP
#define SURF_IDX_NN_HPP

#include "sdsl/suffix_trees.hpp"
#include "sdsl/k2_treap.hpp"
#include "sdsl/wt_topk.hpp"
#include "surf/df_sada.hpp"
#include "surf/rank_functions.hpp"
#include "surf/idx_d.hpp"
#include "surf/construct_col_len.hpp"
#include <algorithm>
#include <limits>
#include <queue>
#include <set>

namespace surf{

using range_type = sdsl::range_type;


template<typename t_select>
struct map_to_dup_type{
    const t_select* m_sel;

    map_to_dup_type(const t_select* select=nullptr) : 
        m_sel(select)
    {}

    range_type
    operator()(size_t sp, size_t ep)const{
        uint64_t y = (*m_sel)(ep);
        if ( y == 0 )
            return range_type(0, -1);
        uint64_t ep_ = (y + 1) - ep - 1; // # of 0 left to y - 1
        uint64_t sp_ = 0;          // # of 0 left to x
        if (0 == sp) {
        } else {
            uint64_t x = (*m_sel)(sp);
            sp_ = (x+1) - sp;
        }
        return std::make_pair(sp_, ep_);       
    }
};



/*! Class idx_nn consists of a 
 *   - CSA over the collection concatenation
 *   - H 
 */
template<typename t_csa,
         typename t_grid,
         typename t_rmq = sdsl::rmq_succinct_sct<>,
         typename t_border = sdsl::sd_vector<>,
         typename t_border_rank = typename t_border::rank_1_type,
         typename t_border_select = typename t_border::select_1_type,
         typename t_h = sdsl::rrr_vector<63>,
         typename t_h_select = typename t_h::select_1_type
         >
class idx_nn{
public:
    using size_type = sdsl::int_vector<>::size_type;
    typedef t_csa                                      csa_type;
    typedef t_border                                   border_type;
    typedef t_border_rank                              border_rank_type;
    typedef t_border_select                            border_select_type;
    typedef t_h                                        h_type;
    typedef t_h_select                                 h_select_type;
    typedef t_rmq                                      rmqc_type;
    typedef t_grid                                     grid_type;

    typedef typename t_csa::alphabet_category          alphabet_category;

    typedef map_to_dup_type<h_select_type> map_to_h_type;
public:
    csa_type           m_csa;
    border_type        m_border;
    border_rank_type   m_border_rank;
    border_select_type m_border_select;
    h_type            m_h;
    h_select_type     m_h_select;
    int_vector<>       m_doc; // documents in node lists
    rmqc_type          m_rmqc;
    grid_type       m_grid;
    map_to_h_type     m_map_to_h;

public:

    class top_k_iterator{
        public:
            typedef void(*t_mfptr)();
            typedef std::pair<uint64_t, double> t_doc_val;
            typedef std::stack<std::array<uint64_t,2>> t_stack_array;
            typedef decltype(top_k(std::declval<t_grid>(),{0ULL,0ULL},{0ULL,0ULL})) t_2d_range_iter;
        private:
            const idx_nn*      m_idx;
            uint64_t           m_sp;  // start point of lex interval
            uint64_t           m_ep;  // end point of lex interval
            t_doc_val          m_doc_val;  // stores the current result
            bool               m_valid = false;

            t_2d_range_iter    m_2d_range_iter;
            std::set<uint64_t> m_reported;
            std::set<uint64_t> m_singletons;
            t_stack_array      m_states;
            bool               m_multi_occ = false; // true, if document has to occur more than once
        public:
            top_k_iterator() = delete;
            top_k_iterator(const top_k_iterator&) = default;
            top_k_iterator(top_k_iterator&&) = default;
            top_k_iterator& operator=(const top_k_iterator&) = default;
            top_k_iterator& operator=(top_k_iterator&&) = default;

            template<typename t_pat_iter>
            top_k_iterator(const idx_nn* idx, t_pat_iter begin, t_pat_iter end, bool multi_occ, bool only_match) : 
                m_idx(idx), m_multi_occ(multi_occ) {
                m_valid = backward_search(m_idx->m_csa, 0, m_idx->m_csa.size()-1, begin, end, m_sp, m_ep) > 0;
                m_valid &= !only_match;
                if ( m_valid ){
                    auto h_range = m_idx->m_map_to_h(m_sp, m_ep);
                    if ( !empty(h_range) ) {
                        uint64_t depth = end-begin;
                        m_2d_range_iter = top_k(m_idx->m_grid, {std::get<0>(h_range) ,0}, {std::get<1>(h_range), depth-1});
                    }
                    m_states.push({m_sp, m_ep});
                    ++(*this);
                }
            }

            top_k_iterator& operator++(){
                if ( m_valid ){
                    m_valid = false;
                    if ( m_2d_range_iter ) { // multiple occurrence result exists
                        auto xy_w       = *m_2d_range_iter;
                        uint64_t doc_id = m_idx->m_doc[real(xy_w.first)]; 
                        m_doc_val = t_doc_val(doc_id, xy_w.second+1);
                        m_reported.insert(doc_id);
                        m_valid = true;
                        ++m_2d_range_iter;
                    } else { // search for singleton results
                        while ( !m_multi_occ and !m_states.empty() ) {
                            auto state = m_states.top();
                            m_states.pop();
                            uint64_t min_idx = m_idx->m_rmqc(state[0], state[1]);
                            uint64_t doc_id  = m_idx->m_border_rank(m_idx->m_csa[min_idx]);
                            if ( m_singletons.find(doc_id) == m_singletons.end() ){
                                m_singletons.insert(doc_id);
                                if ( min_idx + 1 <= state[1] )
                                    m_states.push({min_idx+1, state[1]});
                                if ( state[0] + 1 <= min_idx )
                                    m_states.push({state[0], min_idx-1});
                                if ( m_reported.find(doc_id) == m_reported.end() ){
                                    m_doc_val = t_doc_val(doc_id, 1);
                                    m_reported.insert(doc_id);
                                    m_valid = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                return *this;
            }

            t_doc_val operator*() const{
                return m_doc_val; 
            }

            operator t_mfptr() const{
                return (t_mfptr)(m_valid);
            }
    };

    template<typename t_pat_iter>
    top_k_iterator topk(t_pat_iter begin, t_pat_iter end, bool multi_occ=false, bool only_match=false) const{
        return top_k_iterator(this, begin, end, multi_occ, only_match);
    }

    result search(const std::vector<query_token>& qry,size_t k,bool ranked_and = false,bool profile = false) const {
        result res;
        if ( qry.size() > 0 ){
            auto res_iter = topk(qry[0].token_ids.begin(), qry[0].token_ids.end());
            size_t i = 0;
            while ( i < k and res_iter ){
                ++i;
                auto docid_weight = *res_iter;
                res.list.emplace_back(docid_weight.first, docid_weight.second);
                ++res_iter;
            }
        }
        return res;
    }

    auto doc(uint64_t doc_id) -> decltype(extract(m_csa,0,0)) {
        size_type doc_begin=0;
        if ( doc_id ) {
            doc_begin = m_border_select(doc_id)+1;
        }
        size_type doc_end=m_border_select(doc_id+1)-1;
        auto res = extract(m_csa, doc_begin, doc_end);
        return res;
    }

    uint64_t doc_cnt() const{
        return m_border_rank(m_csa.size());
    }

    uint64_t word_cnt() const {
        return m_csa.size() - doc_cnt();
    }


    void load(sdsl::cache_config& cc){
        load_from_cache(m_csa, surf::KEY_CSA, cc, true);
        if (!t_grid::permuted_x)
            load_from_cache(m_doc, surf::KEY_DUP, cc);
        else
            load_from_cache(m_doc, surf::KEY_PERMUTED_DOC, cc);

        load_from_cache(m_border, surf::KEY_DOCBORDER, cc, true); 
        load_from_cache(m_border_rank, surf::KEY_DOCBORDER_RANK, cc, true); 
        m_border_rank.set_vector(&m_border);
        load_from_cache(m_border_select, surf::KEY_DOCBORDER_SELECT, cc, true); 
        m_border_select.set_vector(&m_border);
        load_from_cache(m_h, surf::KEY_H, cc, true);
        load_from_cache(m_h_select, surf::KEY_H_SELECT, cc, true);
        m_h_select.set_vector(&m_h);
        m_map_to_h = map_to_h_type(&m_h_select);
        load_from_cache(m_rmqc, surf::KEY_RMQC, cc, true); 
        load_from_cache(m_grid, surf::KEY_W_AND_P, cc, true); 
    }

    size_type serialize(std::ostream& out, structure_tree_node* v=nullptr, std::string name="")const {
        structure_tree_node* child = structure_tree::add_child(v, name, util::class_name(*this));
        size_type written_bytes = 0;
        written_bytes += m_csa.serialize(out, child, "CSA");
        written_bytes += m_doc.serialize(out, child, "DOC");
        written_bytes += m_border.serialize(out, child, "BORDER");
        written_bytes += m_border_rank.serialize(out, child, "BORDER_RANK");
        written_bytes += m_h.serialize(out, child, "H");
        written_bytes += m_h_select.serialize(out, child, "H_SELECT");
        written_bytes += m_rmqc.serialize(out, child, "RMQ_C");
        written_bytes += m_grid.serialize(out, child, "W_AND_P");
        structure_tree::add_size(child, written_bytes);
        return written_bytes;
    }

    void mem_info()const{}
};

template<typename t_cst,
         typename t_select>
struct map_node_to_dup_type{
    typedef typename t_cst::node_type t_node;
    const map_to_dup_type<t_select> m_map;
    const t_cst* m_cst;

    map_node_to_dup_type(const t_select* select, const t_cst* cst):
        m_map(select), m_cst(cst)
    { }

    range_type
    operator()(const t_node& v)const{
        auto left    = 1;
        auto left_rb = m_cst->rb(m_cst->select_child(v, left));
        return m_map(left_rb, left_rb+1);
    }
    // id \in [1..n-1]
    uint64_t id(const t_node& v)const{
        auto left    = 1;
        return m_cst->rb(m_cst->select_child(v, left))+1;
    }
};



int_vector<> sorted_perm(int_vector<>& v)
{
    int_vector<> perm(v.size(), 0, bits::hi(v.size())+1);
    util::set_to_id(perm);
    sort(perm.begin(), perm.end(), [&v](const size_t& i,
            const size_t& j) {
        auto x = v[i], y = v[j];
        if (x == y)
            return i < j;
        return x < y;
    });
    return std::move(perm);
}


template<typename t_csa,
         typename t_grid,
         typename t_rmq,
         typename t_border,
         typename t_border_rank,
         typename t_border_select,
         typename t_h,
         typename t_h_select
         >
void construct(idx_nn<t_csa,t_grid,t_rmq,t_border,t_border_rank,t_border_select,t_h,t_h_select>& idx,
               const std::string&,
               sdsl::cache_config& cc, uint8_t num_bytes)
{    
    using namespace sdsl;
    using namespace std;
    using t_df = DF_TYPE;
    using cst_type = typename t_df::cst_type;
    using t_wtd = WTD_TYPE;

    construct_col_len<t_df::alphabet_category::WIDTH>(cc);

    cout<<"...CSA"<<endl; // CSA to get the lex. range
    if ( !cache_file_exists<t_csa>(surf::KEY_CSA, cc) )
    {
        t_csa csa;
        construct(csa, "", cc, 0);
        store_to_cache(csa, surf::KEY_CSA, cc, true);
    }
    cout<<"...WTD"<<endl; // Document array and wavelet tree of it
    if (!cache_file_exists<t_wtd>(surf::KEY_WTD, cc) ){
        construct_darray<t_csa::alphabet_type::int_width>(cc, false);
        t_wtd wtd;
        construct(wtd, cache_file_name(surf::KEY_DARRAY, cc), cc);
        cout << "wtd.size() = " << wtd.size() << endl;
        cout << "wtd.sigma = " << wtd.sigma << endl;
        store_to_cache(wtd, surf::KEY_WTD, cc, true);
    }
    cout<<"...DF"<<endl; // 
    if (!cache_file_exists<t_df>(surf::KEY_SADADF, cc))
    {
        t_df df;
        construct(df, "", cc, 0);
        store_to_cache(df, surf::KEY_SADADF, cc, true);
        bit_vector h;
        load_from_cache(h, surf::KEY_H, cc);
        t_h hrrr = h;
        store_to_cache(hrrr, surf::KEY_H, cc, true);
        t_h_select h_select(&hrrr);
        store_to_cache(h_select, surf::KEY_H_SELECT, cc, true);
    }
    cout<<"...DOC_BORDER"<<endl;
    if ( !cache_file_exists<t_border>(surf::KEY_DOCBORDER,cc) or
         !cache_file_exists<t_border_rank>(surf::KEY_DOCBORDER_RANK,cc) or
         !cache_file_exists<t_border_select>(surf::KEY_DOCBORDER_SELECT,cc) )
    {
        bit_vector doc_border;
        load_from_cache(doc_border, surf::KEY_DOCBORDER,cc);
        t_border sd_doc_border(doc_border);
        store_to_cache(sd_doc_border, surf::KEY_DOCBORDER, cc, true);
        t_border_rank doc_border_rank(&sd_doc_border);
        store_to_cache(doc_border_rank, surf::KEY_DOCBORDER_RANK, cc, true);
        t_border_select doc_border_select(&sd_doc_border);
        store_to_cache(doc_border_select, surf::KEY_DOCBORDER_SELECT, cc, true);
    }
// P corresponds to up-pointers
    cout<<"...P"<<endl;
    if (!cache_file_exists(surf::KEY_P, cc))
    {
        uint64_t max_depth = 0;
        load_from_cache(max_depth, surf::KEY_MAXCSTDEPTH, cc);

        int_vector<> dup;
        load_from_cache(dup, surf::KEY_DUP, cc);
        cout<<"dup.size()="<<dup.size()<<endl;
        if ( dup.size() < 20 ){
            cout << dup << endl;
        }

        std::string P_file = cache_file_name(surf::KEY_P, cc);
        
        int_vector_buffer<> P_buf(P_file, std::ios::out, 1<<20, sdsl::bits::hi(max_depth)+1);

        t_wtd wtd;
        load_from_cache(wtd, surf::KEY_WTD, cc, true);
        
        t_h hrrr;
        load_from_cache(hrrr, surf::KEY_H, cc, true);
        t_h_select h_select;
        load_from_cache(h_select, surf::KEY_H_SELECT, cc, true);
        h_select.set_vector(&hrrr);
        cst_type cst;
        load_from_file(cst, cache_file_name<cst_type>(surf::KEY_TMPCST, cc));
        map_node_to_dup_type<cst_type, t_h_select> map_node_to_dup(&h_select, &cst);

        uint64_t doc_cnt = 1;
        load_from_cache(doc_cnt, KEY_DOCCNT, cc);
        typedef stack<uint32_t,vector<uint32_t>> t_stack;
        //  HELPER to build the pointer structure
        vector<t_stack> depths(doc_cnt, t_stack(vector<uint32_t>(1,0)));// doc_cnt stack for last depth
        uint64_t depth=0;
        // DFS traversal of CST
        for (auto it=cst.begin(); it!=cst.end(); ++it) {
            auto v = *it; // get the node by dereferencing the iterator
            if ( !cst.is_leaf(v) ) {
                if (it.visit() == 1) {  // node visited the first time
                    depth = cst.depth(v);
                    range_type r = map_node_to_dup(v);
                    if ( !empty(r) ){
                        for(size_t i=r.first; i<=r.second; ++i){
                            depths[dup[i]].push(depth);
                        }
                    }
                } else { // node visited the second time 
                    range_type r = map_node_to_dup(v);
                    if ( !empty(r) ){
                         for(size_t i=r.first; i<=r.second; ++i){
                            depths[dup[i]].pop();
                            P_buf[i] = depths[dup[i]].top();
                        }                       
                    }
                }
            }
        }
        P_buf.close();
    }
    cout<<"...RMQ_C"<<endl;
    if (!cache_file_exists<t_rmq>(surf::KEY_RMQC,cc))
    {
        int_vector<> C;
        load_from_cache(C, surf::KEY_C, cc);
        t_rmq rmq_c(&C);
        store_to_cache(rmq_c, surf::KEY_RMQC, cc, true); 
    }
    cout<<"...W_AND_P"<<endl;
    if (!cache_file_exists<t_grid>(surf::KEY_W_AND_P, cc)) {
        int_vector_buffer<> P_buf(cache_file_name(surf::KEY_P, cc));
        std::string W_and_P_file = cache_file_name(surf::KEY_W_AND_P, cc);
        cout << "P_buf.size()=" << P_buf.size() << endl;
        {
            int_vector<> id_v(P_buf.size(), 0, bits::hi(P_buf.size()) + 1);
            util::set_to_id(id_v);
            store_to_file(id_v, W_and_P_file + ".x");
        }
        {
            int_vector<> P;
            load_from_cache(P, surf::KEY_P, cc);
            store_to_file(P, W_and_P_file + ".y");
        }
        {
            int_vector<> W;
            load_from_cache(W, surf::KEY_WEIGHTS, cc);
            store_to_file(W, W_and_P_file + ".w");
        }
        {
            t_grid grid;
            if (t_grid::permuted_x == true) {
                int_vector<> D;
                load_from_cache(D, surf::KEY_DUP, cc);
                int_vector<> P;
                load_from_cache(P, surf::KEY_P, cc);
                int_vector<> perm = sorted_perm(P);
                int_vector<> permuted_d(P.size());
                for (size_t i=0; i<P.size(); ++i) {
                    permuted_d[i] = D[perm[i]];
                }
                store_to_cache(permuted_d, surf::KEY_PERMUTED_DOC, cc);
            }
            cout << "build grid" << endl;
            construct(grid, cache_file_name(surf::KEY_W_AND_P, cc));
            store_to_cache(grid, surf::KEY_W_AND_P, cc, true);
            sdsl::remove(W_and_P_file + ".x");
            sdsl::remove(W_and_P_file + ".y");
            sdsl::remove(W_and_P_file + ".w");

        }
    }
}

} // end namespace surf

#endif
