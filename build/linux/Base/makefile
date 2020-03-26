include $(CLEAR_VARS)

TARGET_MODULE  := Base

CPP_INCLUDE    := -I$(SOURCE_ROOT)/Base \
                  -I$(SYSROOT)/usr/include 
CPP_SRCDIR     := $(SOURCE_ROOT)/Base
PCH_H          := $(CPP_SRCDIR)/CBase.hpp

#CPP_SRC        := $(shell find $(CPP_SRCDIR) -name "*.cpp")
CPP_SRC        := $(shell find $(CPP_SRCDIR) -name "CMemAllocator.cpp")
CPP_SRC        += $(shell find $(CPP_SRCDIR) -name "CMemoryManager.cpp")
CPP_SRC        += $(shell find $(CPP_SRCDIR) -name "Memory.cpp")
CPP_SRC        += $(shell find $(CPP_SRCDIR) -name "CSampleThreadPool.cpp")
C_SRC          := $(shell find $(CPP_SRCDIR) -name "*.c")

LIBS           += 

LIBS_DIR       += -L$(TARGET_ROOT)/Release
LIBS_DIR_DEBUG += -L$(TARGET_ROOT)/Debug

include $(BUILD_SHARED_LIBRARY)
