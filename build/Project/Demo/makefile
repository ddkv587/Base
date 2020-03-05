include $(CLEAR_VARS)

TARGET_MODULE  := Demo
APP_NAME       := Demo

CPP_INCLUDE    := -I$(PROJECT_ROOT)/Demo \
                  -I$(SOURCE_ROOT)/Base
CPP_SRCDIR     := $(PROJECT_ROOT)/Demo

CPP_SRC        := $(shell find $(CPP_SRCDIR) -name "*.cpp")
C_SRC          := $(shell find $(CPP_SRCDIR) -name "*.c")

LIBS           += 

LIBS_DIR       += -L$(TARGET_ROOT)/Release
LIBS_DIR_DEBUG += -L$(TARGET_ROOT)/Debug

DEPS_LIBRARIES += libBase.so

include $(BUILD_EXECUTABLE)
