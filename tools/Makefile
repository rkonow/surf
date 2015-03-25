
# CONFIG REQUIRED: SET THIS VARIABLE TO THE PATH OF THE INDRI SOURCE CODE
#INDRISRC=/home/mpetri/dev/collection-parser/indri-5.6
# CONFIG REQUIRED: SET THIS TO THE SDSL INSTALL PATH
SDSLPREFIX=../build/external/sdsl-lite/

#include $(INDRISRC)/MakeDefns
SHARED=
INCPATH=-I$(INDRISRC)/include $(patsubst %, -I$(INDRISRC)/contrib/%/include, $(DEPENDENCIES)) -I $(SDSLPREFIX)/include/
LIBPATH=-L$(INDRISRC)/obj  $(patsubst %, -L$(INDRISRC)/contrib/%/obj, $(DEPENDENCIES)) -L $(SDSLPREFIX)/lib/
LIBS=-lindri $(patsubst %, -l%, $(DEPENDENCIES)) -lsdsl

surf2sea14: surf2sea14.cpp
	$(CXX) -O3 -std=c++11 surf2sea14.cpp -o surf2sea14 -L../build/external/sdsl-lite/lib -I../build/external/sdsl-lite/include -lsdsl
all: indri
	$(CXX) $(CXXFLAGS) -std=c++11 indri_to_surf.cpp -o indri_to_surf $(INCPATH) $(LIBPATH) $(LIBS) $(CPPLDFLAGS)
	$(CXX) $(CXXFLAGS) -std=c++11 indri_stem_krovetz.cpp -o indri_stem_krovetz $(INCPATH) $(LIBPATH) $(LIBS) $(CPPLDFLAGS)

indri:
	cd  $(INDRISRC); ./configure
	make -C $(INDRISRC) contrib

clean:
	rm -f $(APP)
