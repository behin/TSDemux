################################
##
################################

CXX := g++
CXX_FLAGS := -fPIC -std=c++11 -O2 -Wall -Werror

RM := rm
AR := ar


###
SRC_FILES := main.cpp

OBJS := ${SRC_FILES:.cpp=.o}

###
EXE := TSDemux

### Rules
.PHONY: all
all: TSDemux
	@echo "Build Finished."

.PHONY: TSDemux
TSDemux: ${OBJS}
	${CXX} $^ -o ${EXE}

.PHONY: clean
clean:
	${RM} -f *.o *~ ${EXE}

%.o: %.cpp
	${CXX} ${CXX_FLAGS} -c $<
