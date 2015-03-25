include ../external/sdsl-lite/Make.helper

CXX_FLAGS = $(MY_CXX_FLAGS) $(MY_CXX_OPT_FLAGS) -I$(INC_DIR) -L$(LIB_DIR)  
LIBS = -lsdsl -ldivsufsort -ldivsufsort64

is_int_col=$(shell if [[ "$1" == *INT || "$1" == "gov2" || "$1" == "trec8" ]]; then echo "_int"; else echo ""; fi)
is_int_idx=$(shell if [[ "$1" == *_INT ]]; then echo "1"; else echo "0"; fi)

experiment1: results/experiment1.txt

experiment2: results/experiment2.txt

experiment3-sea14: results/experiment3-sea14.txt
experiment3-sea14-int: results/experiment3-sea14-int.txt

experiment3: results/experiment3.txt

experiment5: results/experiment5.txt

# Experiment 1
# Fix collection, pattern length, and index
# Vary k, index, collections
EXP1IDX:=$(call config_column,experiment1.IDX.config,1)
EXP1COL:=$(call config_column,experiment1.COL.config,1)
EXP1LEN:=$(call config_column,experiment1.config,1)
EXP1K:=$(call config_column,experiment1.K.config,1)
EXP1RES:=$(foreach K,$(EXP1K),\
		 	$(foreach IDX,$(EXP1IDX),\
				$(foreach COL,$(EXP1COL),\
					results/experiment1.$(K).$(IDX).$(COL).txt)))
EXP1:=$(foreach IDX,$(EXP1IDX),\
			$(foreach COL,$(EXP1COL),exp1.build.$(IDX).$(COL).byte))

EXP1PAT:=$(foreach COL,$(EXP1COL),pattern/$(COL).$(EXP1LEN).byte.txt)


results/experiment1.txt: exp1.txt $(EXP1PAT) $(EXP1) $(EXP1RES) 
	@cat $(EXP1RES) > $@

exp1.txt: $(EXP1) 

exp1.build.%.byte:
	rm -f exp1.txt
	$(eval IDX:=$(call dim,1,$*))
	$(eval COL:=$(call dim,2,$*))
	../build/surf_index-$(IDX) -c ../collections/$(COL) > exp1.txt

results/experiment1.%.txt: $(EXP1PAT)
	$(eval K:=$(call dim,2,$*))
	$(eval IDX:=$(call dim,2,$*))
	$(eval COL:=$(call dim,3,$*))
	$(eval ONLYMATCH:=$(call config_select,experiment1.K.config,$(K),2))
	../build/surf_query-$(IDX) -c ../collections/$(EXP1COL) \
		                -q pattern/$(COL).$(EXP1LEN).byte.txt -k $(K) \
						-m $(ONLYMATCH) > $@
	../build/surf_query-$(IDX) -c ../collections/$(EXP1COL) \
		                -q pattern/$(COL).$(EXP1LEN).byte.txt -k $(K) \
						$(ONLYMATCH) >> $@

pattern/%.byte.txt: ../build/gen_patterns
	$(eval COL:=$(call dim,1,$*)) 
	$(eval LEN:=$(call dim,2,$*)) 
	../build/gen_patterns$(SUF) -c ../collections/$(COL) -m $(LEN) -x 40000 > $@

pattern/%.int.txt: ../build/gen_patterns_int
	$(eval COL:=$(call dim,1,$*)) 
	$(eval LEN:=$(call dim,2,$*)) 
	../build/gen_patterns_int -c ../collections/$(COL) -m $(LEN) -x 40000 > $@



# Experiment 2
# Fix k, pattern length, 
# Vary index (IDX_NN, IDX_NNX), collections
EXP2IDX:=$(call config_column,experiment2.IDX.config,1)
EXP2IDXINT:=$(call config_column,experiment2.IDX_INT.config,1)
EXP2COL:=$(call config_column,experiment2.COL.config,1)
EXP2COLINT:=$(call config_column,experiment2.COL_INT.config,1)
EXP2K:=$(call config_column,experiment2.config,1)
EXP2LEN:=$(call config_column,experiment2.config,2)
EXP2LENINT:=$(call config_column,experiment2.config,3)
EXP2RES:=$(foreach IDX,$(EXP2IDX),\
			$(foreach COL,$(EXP2COL),results/experiment2.$(IDX).$(COL).byte.txt))
EXP2RESINT:=$(foreach IDX,$(EXP2IDXINT),\
			$(foreach COL,$(EXP2COLINT),results/experiment2.$(IDX).$(COL).int.txt))

EXP2:=$(foreach IDX,$(EXP2IDX),\
			$(foreach COL,$(EXP2COL),exp2.build.$(IDX).$(COL).byte))
EXP2INT:=$(foreach IDX,$(EXP2IDXINT),\
			$(foreach COL,$(EXP2COLINT),exp2.build.$(IDX).$(COL).int))

EXP2PAT:=$(foreach COL,$(EXP2COL),pattern/$(COL).$(EXP2LEN).byte.txt)
EXP2PATINT:=$(foreach COL,$(EXP2COLINT),pattern/$(COL).$(EXP2LENINT).int.txt)



results/experiment2.%.byte.txt: 
	$(eval IDX:=$(call dim,1,$*))
	$(eval COL:=$(call dim,2,$*))
	../build/surf_query-$(IDX) -c ../collections/$(COL) \
		-q pattern/$(COL).$(EXP2LEN).byte.txt -k $(EXP2K) \
		> $@

results/experiment2.%.int.txt: 
	$(eval IDX:=$(call dim,1,$*))
	$(eval COL:=$(call dim,2,$*))
	../build/surf_query-$(IDX) -c ../collections/$(COL) \
		-q pattern/$(COL).$(EXP2LENINT).int.txt -k $(EXP2K) \
		> $@

results/experiment2.txt: exp2.txt $(EXP2PAT) $(EXP2RES) $(EXP2PATINT) $(EXP2RESINT)
	@cat $(EXP2RES) $(EXP2RESINT) > $@

exp2.txt: $(EXP2) $(EXP2INT)

exp2.build.%.byte:
	rm -f exp2.txt
	$(eval IDX:=$(call dim,1,$*))
	$(eval COL:=$(call dim,2,$*))
	../build/surf_index-$(IDX) -c ../collections/$(COL) > exp2.txt

exp2.build.%.int:
	rm -f exp2.txt
	$(eval IDX:=$(call dim,1,$*))
	$(eval COL:=$(call dim,2,$*))
	../build/surf_index-$(IDX) -c ../collections/$(COL) > exp2.txt


# Experiment 3
# As in Figure 2+3 in Plug&Play paper.
# Fix k=10, index=IDX_NNX
# Vary index, pattern length, collection

EXP3IDX:=$(call config_column,experiment3.config,1)
EXP3IDXINT:=$(call config_column,experiment3.config,2)
EXP3K:=$(call config_column,experiment3.config,3)
EXP3LEN:=$(call config_column,experiment3.PAT.config,1)
EXP3COL:=$(call config_column,experiment3.COL.config,1)
EXP3COLINT:=$(call config_column,experiment3.COL_INT.config,1)
EXP3RES:=$(foreach COL,$(EXP3COL),\
			$(foreach LEN,$(EXP3LEN),\
				$(foreach IDX,$(EXP3IDX),\
				results/experiment3.$(IDX).$(COL).$(LEN).byte.txt)))
EXP3RESINT:=$(foreach COL,$(EXP3COLINT),\
			$(foreach LEN,$(EXP3LEN),\
			$(foreach IDX,$(EXP3IDXINT),\
				results/experiment3.$(IDX).$(COL).$(LEN).int.txt)))
		
EXP3:=$(foreach COL,$(EXP3COL),\
		$(foreach IDX,$(EXP3IDX),exp3.build.$(IDX).$(COL).byte))
EXP3INT:=$(foreach COL,$(EXP3COLINT),\
			$(foreach IDX,$(EXP3IDXINT),exp3.build.$(IDX).$(COL).int))
EXP3PAT:=$(foreach COL,$(EXP3COL),\
			$(foreach LEN,$(EXP3LEN),pattern/$(COL).$(LEN).byte.txt))
EXP3PATINT:=$(foreach COL,$(EXP3COLINT),\
			$(foreach LEN,$(EXP3LEN),pattern/$(COL).$(LEN).int.txt))


EXP3SEA14COL:=$(call config_column,experiment3sea14.COL.config,1)
EXP3SEA14:=$(foreach COL,$(EXP3SEA14COL),\
			$(foreach LEN,$(EXP3LEN),\
				results/experiment3-sea14.$(COL).$(LEN).byte.txt))

EXP3SEA14COLINT:=$(call config_column,experiment3sea14.COL_INT.config,1)
EXP3SEA14INT:=$(foreach COL,$(EXP3SEA14COLINT),\
			$(foreach LEN,$(EXP3LEN),\
				results/experiment3-sea14.$(COL).$(LEN).int.txt))



results/experiment3-sea14.txt: ../external/sdsl-lite/benchmark/document_retrieval/bin/build_idx_GREEDY ../external/sdsl-lite/benchmark/document_retrieval/bin/build_idx_SORT ../external/sdsl-lite/benchmark/document_retrieval/bin/query_idx_SORT ../external/sdsl-lite/benchmark/document_retrieval/bin/query_idx_GREEDY $(EXP3PAT) $(EXP3SEA14)
	@cat $(EXP3SEA14) > $@


results/experiment3-sea14-int.txt: ../external/sdsl-lite/benchmark/document_retrieval/bin/build_int_idx_GREEDYINT ../external/sdsl-lite/benchmark/document_retrieval/bin/build_int_idx_SORTINT ../external/sdsl-lite/benchmark/document_retrieval/bin/query_int_idx_SORTINT ../external/sdsl-lite/benchmark/document_retrieval/bin/query_int_idx_GREEDYINT $(EXP3PATINT) $(EXP3SEA14INT)
	@cat $(EXP3SEA14INT) > $@

../external/sdsl-lite/benchmark/document_retrieval/bin/build_idx_%:
	cd ../external/sdsl-lite/ && ./install.sh ..
	cd ../external/sdsl-lite/benchmark/document_retrieval && make bin/build_idx_$*

../external/sdsl-lite/benchmark/document_retrieval/bin/query_idx_%:
	cd ../external/sdsl-lite/ && ./install.sh ..
	cd ../external/sdsl-lite/benchmark/document_retrieval && make bin/query_idx_$*

../external/sdsl-lite/benchmark/document_retrieval/bin/build_int_idx_%:
	cd ../external/sdsl-lite/ && ./install.sh ..
	cd ../external/sdsl-lite/benchmark/document_retrieval && make bin/build_int_idx_$*

../external/sdsl-lite/benchmark/document_retrieval/bin/query_int_idx_%:
	cd ../external/sdsl-lite/ && ./install.sh ..
	cd ../external/sdsl-lite/benchmark/document_retrieval && make bin/query_int_idx_$*

results/experiment3-sea14.%.byte.txt:
	$(eval COL:=$(call dim,1,$*))
	$(eval LEN:=$(call dim,2,$*))
	@echo "COL= $(COL)"
#	@cd ../tools && make surf2sea14
#	@echo "surf2sea generated"
	make ../collections/$(COL)/sea14.txt
	make ../collections/$(COL)/index/SORT.sdsl
	make ../collections/$(COL)/index/GREEDY.sdsl
	make pattern/$(COL).$(LEN).byte.txt
	../external/sdsl-lite/benchmark/document_retrieval/bin/query_idx_SORT ../collections/$(COL)/index/SORT.sdsl pattern/$(COL).$(LEN).byte.txt > $@
	../external/sdsl-lite/benchmark/document_retrieval/bin/query_idx_GREEDY ../collections/$(COL)/index/GREEDY.sdsl pattern/$(COL).$(LEN).byte.txt >> $@
	

../collections/%/index/SORT.sdsl:
	$(eval COL:=$(call dim,1,$*))
	mkdir -p ../collections/$(COL)/tmp
	../external/sdsl-lite/benchmark/document_retrieval/bin/build_idx_SORT ../collections/$(COL)/sea14.txt ../collections/$(COL)/tmp/ ../collections/$(COL)/index/SORT.sdsl

../collections/%/index/GREEDY.sdsl:
	$(eval COL:=$(call dim,1,$*))
	mkdir -p ../collections/$(COL)/tmp
	../external/sdsl-lite/benchmark/document_retrieval/bin/build_idx_GREEDY ../collections/$(COL)/sea14.txt ../collections/$(COL)/tmp/ ../collections/$(COL)/index/GREEDY.sdsl


../collections/%/sea14.txt: ../tools/surf2sea14
	../tools/surf2sea14 ../collections/$*/text_SURF.sdsl $@


../collections/%/sea14.sdsl: ../tools/surf2sea14
	../tools/surf2sea14 ../collections/$*/text_int_SURF.sdsl $@

../tools/surf2sea14:
	cd ../tools && make surf2sea14

results/experiment3-sea14.%.int.txt:
	$(eval COL:=$(call dim,1,$*))
	$(eval LEN:=$(call dim,2,$*))
	@echo "COL= $(COL)"
#	@cd ../tools && make surf2sea14
#	@echo "surf2sea generated"
	make ../collections/$(COL)/sea14.sdsl
	make ../collections/$(COL)/index/SORTINT.sdsl
	make ../collections/$(COL)/index/GREEDYINT.sdsl
	make pattern/$(COL).$(LEN).int.txt
	../external/sdsl-lite/benchmark/document_retrieval/bin/query_int_idx_SORTINT ../collections/$(COL)/index/SORTINT.sdsl pattern/$(COL).$(LEN).int.txt > $@
	../external/sdsl-lite/benchmark/document_retrieval/bin/query_int_idx_GREEDYINT ../collections/$(COL)/index/GREEDYINT.sdsl pattern/$(COL).$(LEN).int.txt >> $@
	
../collections/%/index/SORTINT.sdsl:
	$(eval COL:=$(call dim,1,$*))
	mkdir -p ../collections/$(COL)/tmp
	../external/sdsl-lite/benchmark/document_retrieval/bin/build_int_idx_SORTINT ../collections/$(COL)/sea14.sdsl ../collections/$(COL)/tmp/ ../collections/$(COL)/index/SORTINT.sdsl

../collections/%/index/GREEDYINT.sdsl:
	$(eval COL:=$(call dim,1,$*))
	mkdir -p ../collections/$(COL)/tmp
	../external/sdsl-lite/benchmark/document_retrieval/bin/build_int_idx_GREEDYINT ../collections/$(COL)/sea14.sdsl ../collections/$(COL)/tmp/ ../collections/$(COL)/index/GREEDYINT.sdsl


results/experiment3.%.txt:
	$(eval IDX:=$(call dim,1,$*))
	$(eval COL:=$(call dim,2,$*))
	$(eval LEN:=$(call dim,3,$*))
	$(eval ALPH:=$(call dim,4,$*))
	../build/surf_query-$(IDX) -c ../collections/$(COL) \
		-q pattern/$(COL).$(LEN).$(ALPH).txt -k $(EXP3K) \
		> $@

results/experiment3.txt: exp3.txt $(EXP3PAT) $(EXP3RES) $(EXP3PATINT) $(EXP3RESINT)
	@cat $(EXP3RES) $(EXP3RESINT) > $@

exp3.txt: $(EXP3) $(EXP3INT)

exp3.build.%.byte:
	rm -f exp3.txt
	$(eval IDX:=$(call dim,1,$*))
	$(eval COL:=$(call dim,2,$*))
	../build/surf_index-$(IDX) -c ../collections/$(COL) > exp3.txt

exp3.build.%.int:
	rm -f exp3.txt
	$(eval IDX:=$(call dim,1,$*))
	$(eval COL:=$(call dim,2,$*))
	../build/surf_index-$(IDX) -c ../collections/$(COL) > exp3.txt

# Experiment 5
# How much faster is the solution for indexes with different
# CSA implementations?

EXP5COL:=$(call config_column,experiment5.config,1)
EXP5K:=$(call config_column,experiment5.config,2)
EXP5IDX:=$(call config_column,experiment5.IDX.config,1)
EXP5LEN:=$(call config_column,experiment5.PAT.config,1)
EXP5RES:=$(foreach IDX,$(EXP5IDX),\
			$(foreach LEN,$(EXP5LEN),\
				results/experiment5.$(IDX).$(EXP5COL).$(LEN).byte.txt))
		
EXP5:=$(foreach IDX,$(EXP5IDX),exp5.build.$(IDX).$(EXP5COL).byte)
EXP5PAT:=$(foreach LEN,$(EXP5LEN),pattern/$(EXP5COL).$(LEN).byte.txt)

results/experiment5.%.txt:
	$(eval IDX:=$(call dim,1,$*))
	$(eval COL:=$(call dim,2,$*))
	$(eval LEN:=$(call dim,3,$*))
	$(eval ALPH:=$(call dim,4,$*))
	../build/surf_query-$(IDX) -c ../collections/$(COL) \
		-q pattern/$(COL).$(LEN).$(ALPH).txt -k $(EXP5K) \
		> $@

results/experiment5.txt: exp5.txt $(EXP5PAT) $(EXP5RES) 
	@cat $(EXP5RES) > $@

exp5.txt: $(EXP5) 

exp5.build.%.byte:
	rm -f exp5.txt
	$(eval IDX:=$(call dim,1,$*))
	$(eval COL:=$(call dim,2,$*))
	../build/surf_index-$(IDX) -c ../collections/$(COL) > exp5.txt

