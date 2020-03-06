include $(CLEAR_VARS)

TARGET_MODULE  := Demo
APP_NAME       := Demo

CPP_INCLUDE    := -I$(PROJECT_ROOT)/Demo/src \
                  -I$(SOURCE_ROOT)/Base
CPP_SRCDIR     := $(PROJECT_ROOT)/Demo/src

CPP_SRC        := $(shell find $(CPP_SRCDIR) -name "*.cpp")
C_SRC          := $(shell find $(CPP_SRCDIR) -name "*.c")

LIBS           += -lBase -lpthread

LIBS_DIR       += -L$(TARGET_ROOT)/$(PLATFORM)/$(PROJECT_NAME)/Release
LIBS_DIR_DEBUG += -L$(TARGET_ROOT)/$(PLATFORM)/$(PROJECT_NAME)/Debug

DEPS_LIBRARIES += libBase.so

include $(BUILD_EXECUTABLE)
