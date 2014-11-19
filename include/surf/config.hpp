#ifndef SURF_CONFIG_HPP
#define SURF_CONFIG_HPP

#include "sdsl/config.hpp"
#include <string>
#include <vector>

namespace surf{

const std::string TEXT_FILENAME = "text_int_SURF.sdsl";
const std::string TEXT_FILENAME_BYTE = "text_SURF.sdsl";
const std::string DICT_FILENAME = "dict.txt";
const std::string URL2ID_FILENAME = "url2id.txt";
const std::string DOCNAMES_FILENAME = "doc_names.txt";
const std::string SPACEUSAGE_FILENAME = "space_usage";

const std::string KEY_DOCWEIGHT = "docweights";
const std::string KEY_DARRAY = "darray";
const std::string KEY_U = "U";
const std::string KEY_WTU = "wtu";
const std::string KEY_DOCPERM = "docperm";
const std::string KEY_SADADF = "sadadf";
const std::string KEY_WTD = "wtd";
const std::string KEY_C = "C";
const std::string KEY_RMQC = "RMQC";
const std::string KEY_RMQW = "RMQW";
const std::string KEY_WTC = "wtc";
const std::string KEY_TMPCST = "tempcst";
const std::string KEY_TMPDUP = "tmpdup";
const std::string KEY_WTDUP  = "wtdup";
const std::string KEY_WTDP  = "wtdp";
const std::string KEY_DUP  = "dup";
const std::string KEY_P   = "P";
const std::string KEY_WTP   = "WTP";
const std::string KEY_DOCCNT  = "doccnt";
const std::string KEY_COLLEN  = "collen";
const std::string KEY_DOCBORDER = "docborder";
const std::string KEY_DOCBORDER_RANK = "docborder_rank";
const std::string KEY_DOCBORDER_SELECT = "docborder_select";
const std::string KEY_DOC_LENGTHS = "doclengths";
const std::string KEY_INVFILE_TERM_RANGES = "invfile_term_ranges";
const std::string KEY_INVFILE_PLISTS = "invfile_postings_lists";
const std::string KEY_INVFILE_DOCPERM = "invfile_docperm";
const std::string KEY_INVFILE_IDOCPERM = "invfile_inv_docperm";
const std::string KEY_F_T = "Ft";
const std::string KEY_H = "H";
const std::string KEY_H_SELECT  = "H_select";              
const std::string KEY_CSA = "csa";
const std::string KEY_MAXTF = "maxtf";
const std::string KEY_MAXDOCLEN = "maxdoclen";
const std::string KEY_MAXCSTDEPTH = "maxcstdepth";
const std::string KEY_WEIGHTS = "weights";
const std::string KEY_W_AND_P = "W_and_P";
const std::string KEY_PERMUTED_DOC = "perm_doc";


std::vector<std::string> storage_keys = {KEY_DOCCNT,
										 KEY_DARRAY,
										 KEY_DOCPERM,
										 KEY_SADADF,
										 KEY_WTD,
										 KEY_C,
										 KEY_WTC,
										 KEY_TMPCST,
										 KEY_TMPDUP,
										 KEY_DUP,
										 KEY_WTDUP,
                                         KEY_MAXTF,
										 KEY_DOCCNT,
										 KEY_DOC_LENGTHS,
                                         KEY_COLLEN,
										 KEY_INVFILE_TERM_RANGES,
										 KEY_INVFILE_PLISTS,
                                         KEY_H,
                                         KEY_U,
                                         KEY_WTU,
                                         KEY_CSA,
                                         sdsl::conf::KEY_TEXT,
                                         sdsl::conf::KEY_TEXT_INT,
                                         sdsl::conf::KEY_SA,
                                         sdsl::conf::KEY_LCP,
                                         sdsl::conf::KEY_BWT,
                                         sdsl::conf::KEY_BWT_INT,
                                         sdsl::conf::KEY_PSI
               };

} // end namespace
#endif
