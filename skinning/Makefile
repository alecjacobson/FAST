include Makefile.conf

INC=-I. ${EIGEN3_INC} ${ANT_INC} ${LIBIGL_INC} ${PUPPET_INC} ${TETGEN_INC} ${MOSEK_INC} 
LIB=${ANT_LIB} ${PUPPET_LIB} ${TETGEN_LIB} ${MOSEK_LIB} ${OPENGL_LIB} ${GLUT_LIB} ${LIBIGL_LIB} 

# Make file for GLUT skinning app
.PHONY: all
all: skinning

PBS_KAAN=defined

CPP_FILES=$(wildcard ./*.cpp)
ifdef PBS_KAAN
	INC+=-I./pbs_project/
	CFLAGS+=-DPBS_KAAN
	CPP_FILES+=$(wildcard ./pbs_project/*.cpp)
endif
#C_FILES=$(wildcard ./*.c)
OBJ_FILES=$(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o))) 
LIBFASTCPP_FILES=arap_dof.cpp arap_rhs.cpp arap_linear_block.cpp
LIBFASTOBJ_FILES=$(addprefix obj/,$(notdir $(LIBFASTCPP_FILES:.cpp=.o))) 

#$(addprefix obj/,$(notdir $(C_FILES:.c=.o)))
CFLAGS+=-DSHADER_DIR="\"./GLSL\""

skinning: obj $(OBJ_FILES)
	g++ -o skinning $(CFLAGS) $(OBJ_FILES) $(LIB)

libfast.a: $(LIBFASTOBJ_FILES)
	rm -f $@
	ar cqs $@ $^


obj:
	mkdir -p obj

#obj/%.o: %.c %.h
#	gcc $(CFLAGS) -o $@ -c $< $(INC)
obj/%.o: %.cpp %.h
	g++ $(CFLAGS) -o $@ -c $< $(INC)

	
obj/%.o: ./pbs_project/%.cpp ./pbs_project/%.h
	g++ $(CFLAGS) -o $@ -c $< $(INC)

# Special rule for Ladislav's MT19937.h MT19937F.cpp pair
obj/%.o: %.cpp
	g++ $(CFLAGS) -o $@ -c $< $(INC)

.PHONY: clean
clean:
	rm -f $(OBJ_FILES)
	rm -f skinning
