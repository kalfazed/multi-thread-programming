CC           := g++
SRC_PATH     := src
CONFIG_PATH  := config
BUILD_PATH   := build
BIN_PATH     := bin
INC_PATH     := include
APP          := ${BIN_PATH}/app
COMPILE_CMDS := config/compile_commands.json

CPP_SRC      := $(shell find $(SRC_PATH) -name "*.cpp")
CPP_OBJ      := $(CPP_SRC:${SRC_PATH}/%.cpp=${BUILD_PATH}/%.cpp.o)
CPP_MK       := $(CPP_OBJ:%.cpp.o=%.cpp.mk)

CPP_INC    := -std=c++14 -fopenmp -pthread -g -O0 -w

ifneq ("$(wildcard $(COMPILE_CMDS))","")
	COMPILE_CMDS_EXISTS :=1
else
	COMPILE_CMDS_EXISTS :=0
endif


all:
	@if [ $(COMPILE_CMDS_EXISTS) -eq 0 ]; then \
		bear --output ${COMPILE_CMDS} -- $(MAKE) ${APP}; \
	else \
		$(MAKE) ${APP}; \
	fi

run: ${APP}
	@./${APP}

dir:
	@mkdir -p ${BIN_PATH}
	@mkdir -p ${BUILD_PATH}

deps: ${CPP_OBJ}
	@echo Linking ${APP}
	@${CC} $^ -o ${APP}

show:
	@echo ${CPP_SRC}
	@echo ${CPP_OBJ}
	@echo ${CPP_MK}

${APP} : dir deps
	@echo Finished building $@, have fun!!


ifneq (${MAKECMDGALS}, clean)
-include ${CPP_MK}
endif

${BUILD_PATH}/%.cpp.o: ${SRC_PATH}/%.cpp
	@echo Compiling CXX $<
	@mkdir -p ${dir $@}
	@${CC} -c $< -o $@ ${CPP_INC} -I ${INC_PATH}

${BUILD_PATH}/%.cpp.mk: ${SRC_PATH}/%.cpp
	@echo Compiling dependence $<
	@mkdir -p ${dir $@}
	@${CC} -M $< -MF $@ -MT ${@:.cpp.mk=.cpp.o} ${CPP_INC} -I ${INC_PATH}

clean:
	@rm -rf ${BUILD_PATH}
	@rm -rf ${BIN_PATH}
	@rm -rf ${CONFIG_PATH}/compile_commands.json

