CFLAGS=-Wall -O3 -g -Wextra -Wno-unused-parameter
CXXFLAGS=$(CFLAGS)
VPATH=../../src

# Where our library resides. You mostly only need to change the
# RGB_LIB_DISTRIBUTION, this is where the library is checked out.
RGB_LIB_DISTRIBUTION=../../..
RGB_INCDIR=$(RGB_LIB_DISTRIBUTION)/include
RGB_LIBDIR=$(RGB_LIB_DISTRIBUTION)/lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
LDFLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread

SRC=*.cpp
SRCLIB=../../src/*.cpp

# A directory to store object files (.o)
ODIR=./objects

# Unpack these lines from the end:
#  - grep all the .cpp files in SRC1 with wildcard
#  - add the prefix $(ODIR) to all the file names with addprefix
#  - replace .cpp in .o with patsubst
#  - remove all occurrences of ../../src/ from string
OBJLIB=$(subst ../../src/,,$(patsubst %.cpp,%.o,$(addprefix $(ODIR)/,$(wildcard $(SRCLIB)))))
OBJ=$(subst ../../src/,,$(patsubst %.cpp,%.o,$(addprefix $(ODIR)/,$(wildcard $(SRC)))))
BINARIES=$(patsubst %.cpp,%,$(wildcard $(SRC)))

all : $(BINARIES)

$(RGB_LIBRARY): FORCE
	$(MAKE) -C $(RGB_LIBDIR)

# Compile all the files in object files
# $@ refers to the rule name, here $(ODIR)/the_current_file.o
# $< refers to first prerequisite, here $(SRCLIB)/the_current_file.cpp
$(ODIR)/%.o : %.cpp
	$(CXX) -I$(RGB_INCDIR) -I$(VPATH) $(CXXFLAGS) $(LDFLAGS) -c -o $@ $<

# Specific dependencies for each binary need to be declared first, then a general target compiles them all
simplecrawl : $(ODIR)/simplecrawl.o $(ODIR)/crawler.o $(ODIR)/RGBMatrixRenderer.o 
gol : $(ODIR)/gol.o $(ODIR)/golife.o $(ODIR)/RGBMatrixRenderer.o 
sand : $(ODIR)/sand.o $(ODIR)/fallingsand.o $(ODIR)/RGBMatrixRenderer.o 

# All the binaries that have the same name as the object file.q
% : $(ODIR)/%.o $(RGB_LIBRARY)
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJ) $(OBJLIB) $(BINARIES)

rebuild: clean all

FORCE:
.PHONY: FORCE
