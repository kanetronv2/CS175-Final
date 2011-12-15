BASE = asst10

all: $(BASE)

OS := $(shell uname -s)

ifdef OPT 
  #turn on optimization
  CXXFLAGS += -O2
else 
  #turn on debugging
  CXXFLAGS += -g
endif

CXX = g++ 

OBJ = $(BASE).o ppm.o raycast.o scene.o

$(BASE): $(OBJ)
	$(LINK.cpp) -o $@ $^

clean:
	rm -f $(OBJ) $(BASE)
