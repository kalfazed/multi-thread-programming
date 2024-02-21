CONFIG        :=  ../../.config/Makefile.config
CONFIG_LOCAL  :=  ./config/Makefile.config

include $(CONFIG)
include $(CONFIG_LOCAL)

BUILD_PATH    :=  build
SRC_PATH      :=  src
INC_PATH      :=  include

CXX_SRC       +=  $(wildcard $(SRC_PATH)/*.cpp)
APP_OBJS      :=  $(patsubst $(SRC_PATH)%, $(BUILD_PATH)%, $(CXX_SRC:.cpp=.cpp.o))
APP_MKS       :=  $(APP_OBJS:.o=.mk)

APP_DEPS      :=  $(CXX_SRC)
APP_DEPS      +=  $(wildcard $(SRC_PATH)/*.h)

CXXFLAGS      :=  -std=c++$(CXX_VER) -pthread -fPIC

INCS          :=  -I $(SRC_PATH) \
				  -I $(OPENCV_PATH) \
				  -I $(INC_PATH)

LIBS          :=  -lstdc++fs `pkg-config --libs opencv4`

ifeq ($(DEBUG),1)
CXXFLAGS      +=  -g -O0
else
CXXFLAGS      +=  -O3
endif

ifeq ($(SHOW_WARNING),1)
CXXFLAGS      +=  -Wall -Wunused-function -Wunused-variable -Wfatal-errors
else
CXXFLAGS      +=  -w
endif

all: 
	@mkdir -p bin
	@$(BEARCMD) $(MAKE) $(APP)
	@echo finished building $@. Have fun!!

run:
	@$(MAKE) update
	@./bin/$(APP)

update: $(APP)
	@echo finished updating $<

$(APP): $(APP_DEPS) $(APP_OBJS)
	@$(CXX) $(APP_OBJS) -o bin/$@ $(LIBS) $(INCS)

show: 
	@echo $(BUILD_PATH)
	@echo $(APP_DEPS)
	@echo $(INCS)
	@echo $(APP_OBJS)
	@echo $(APP_MKS)

clean:
	rm -rf $(APP)
	rm -rf build
	rm -rf config/compile_commands.json
	rm -rf bin

ifneq ($(MAKECMDGOALS), clean)
-include $(APP_MKS)
endif

# Compile CXX
$(BUILD_PATH)/%.cpp.o: $(SRC_PATH)/%.cpp 
	@echo Compile CXX $@
	@mkdir -p $(BUILD_PATH)
	@$(CXX) -o $@ -c $< $(CXXFLAGS) $(INCS)
$(BUILD_PATH)/%.cpp.mk: $(SRC_PATH)/%.cpp
	@echo Compile Dependence CXX $@
	@mkdir -p $(BUILD_PATH)
	@$(CXX) -M $< -MF $@ -MT $(@:.cpp.mk=.cpp.o) $(CXXFLAGS) $(INCS) 

.PHONY: all update show clean 
