RM                 := rm -rf
MKDIR              := mkdir -p
COPY               := cp -f
STRIP              := strip
TARGET_ARCH        := 
CFLAGS             := -Wall -c -fmessage-length=0 -MMD -MP $(PLATFORM_MACRO) -fpermissive -O0 -g3
CFLAGS_DEBUG       := -Wall -c -fmessage-length=0 -MMD -MP $(PLATFORM_MACRO) -fpermissive -O0 -g3
LIBS               := -lm -lstdc++ -lglog
LIBS_DIR           := 
LIBS_DIR_DEBUG     := 
APP_NAME           :=
TARGET_MODULE      :=
DEPS_LIBRARIES     :=
APP_ROOT           := $(ROOT_PATH)/Project
SOURCE_ROOT        := $(ROOT_PATH)/src
PROJECT_ROOT       := $(ROOT_PATH)/Project
TARGET_ROOT        := $(ROOT_PATH)/target
IM_ROOT_PREX       := $(ROOT_PATH)/im_
