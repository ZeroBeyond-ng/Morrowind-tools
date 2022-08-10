CXX=g++
CXXFLAGS=$(shell pkg-config --cflags freetype2)
CXXLINKS=$(shell pkg-config --libs freetype2)

all:bin/generate_font_file

bin/generate_font_file:generate_font_file.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(CXXLINKS)
