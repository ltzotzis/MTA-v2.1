CXX = g++
CXXFLAGS = -static -std=c++11 -O3 -funroll-loops

MAKE = make

.PHONY: all tinyxml msh_gen pkxml diexml lef def

# Default build
all: msh_gen pkxml diexml


# -----------------------
# TinyXML
# -----------------------
tinyxml:
	@echo "Compiling tinyxml"
	@$(CXX) $(CXXFLAGS) -c tinyxml2.cpp -o lib/tinyxml2.o


# -----------------------
# msh_gen
# -----------------------
msh_gen: tinyxml
	@echo "Compiling msh_gen"
	@$(CXX) $(CXXFLAGS) -o msh_gen msh_gen.cpp lib/tinyxml2.o


# -----------------------
# pkxml
# -----------------------
pkxml: tinyxml
	@echo "Compiling pkxml"
	@$(CXX) $(CXXFLAGS) -o pkxml pkxml.cpp lib/tinyxml2.o


# -----------------------
# lef/def
# -----------------------
lef:
	$(MAKE) -C lefdef/lef

def:
	$(MAKE) -C lefdef/def


# -----------------------
# diexml
# -----------------------
diexml: tinyxml lef def
	@echo "Compiling diexml"
	@$(CXX) $(CXXFLAGS) -I./lefdef/def/include -I./lefdef/lef/include -L./lefdef/def/lib -L./lefdef/lef/lib -o diexml diexml.cpp lib/*.o -ldef -llef
