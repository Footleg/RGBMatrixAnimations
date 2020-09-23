CFLAGS=-Wall -O3 -g -Wextra -Wno-unused-parameter
CXXFLAGS=$(CFLAGS)
OBJECTS=RGBMatrixRenderer.o golife.o gol.o crawler.o simplecrawl.o fallingsand.o sand.o
BINARIES=gol simplecrawler sand

# Where our library resides. You mostly only need to change the
# RGB_LIB_DISTRIBUTION, this is where the library is checked out.
RGB_LIB_DISTRIBUTION=..
RGB_INCDIR=$(RGB_LIB_DISTRIBUTION)/include
RGB_LIBDIR=$(RGB_LIB_DISTRIBUTION)/lib
RGB_LIBRARY_NAME=rgbmatrix
RGB_LIBRARY=$(RGB_LIBDIR)/lib$(RGB_LIBRARY_NAME).a
LDFLAGS+=-L$(RGB_LIBDIR) -l$(RGB_LIBRARY_NAME) -lrt -lm -lpthread

all : $(BINARIES)

$(RGB_LIBRARY): FORCE
	$(MAKE) -C $(RGB_LIBDIR)
gol : RGBMatrixRenderer.o golife.o gol.o 
	$(CXX) -o $@ $^ $(LDFLAGS)

simplecrawler : RGBMatrixRenderer.o crawler.o simplecrawl.o 
	$(CXX) -o $@ $^ $(LDFLAGS)

sand : RGBMatrixRenderer.o fallingsand.o sand.o 
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o : %.cpp
	$(CXX) -I$(RGB_INCDIR) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(BINARIES)

rebuild: clean all

FORCE:
.PHONY: FORCE
