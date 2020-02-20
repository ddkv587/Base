include $(CLEAR_VARS)

TARGET_MODULE  := Base

CPP_INCLUDE    := -I$(SOURCE_ROOT)/src/include \
                  -I$(SOURCE_ROOT)/src/third_include \
                  -I$(SOURCE_ROOT)/src/source/RenderEngine_GraphicsUtility \
                  -I$(SOURCE_ROOT)/src/source/RenderEngine_OpenGLES2 \
                  -I$(SYSROOT)/usr/include 
CPP_SRCDIR     := $(SOURCE_ROOT)/src/source/RenderEngine_OpenGLES2
PCH_H          := $(CPP_SRCDIR)/RenderEngine.h

CPP_SRC        := $(shell find $(CPP_SRCDIR) -name "*.cpp")

LIBS           += -lRenderEngine_GraphicsUtility -lOpenHMI_Native -lEGL -lGLESv2

LIBS_DIR       += -L$(SOURCE_ROOT)/target/qnx/$(PROJECT_NAME)/Release -L$(SOURCE_ROOT)/src/third_lib/qnx/$(PROJECT_NAME)
LIBS_DIR_DEBUG += -L$(SOURCE_ROOT)/target/qnx/$(PROJECT_NAME)/Debug -L$(SOURCE_ROOT)/src/third_lib/qnx/$(PROJECT_NAME)

include $(BUILD_SHARED_LIBRARY)
