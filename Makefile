CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
USERID=16321200-16317727
CLASSES=

all: my-router

my-router: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp Router.cpp Socketwrapper.cpp

sender: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp Router.cpp Socketwrapper.cpp
	
receiver: $(CLASSES)
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp Socketwrapper.cpp

clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM web-server web-client *.tar.gz

tarball: clean
	tar -cvf $(USERID).tar.gz *
