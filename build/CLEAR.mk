RM                 := rm -rf
MKDIR              := mkdir -p
COPY               := cp -f
STRIP              := strip
TARGET_ARCH        := 
CFLAGS             := -Wall -c -fmessage-length=0 -MMD -MP $(PLATFROM_MACRO) -fpermissive -flto -Os
CFLAGS_DEBUG       := -Wall -c -fmessage-length=0 -MMD -MP $(PLATFROM_MACRO) -fpermissive -O0 -g3
LIBS               := -lm -lstdc++
LIBS_DIR           := 
LIBS_DIR_DEBUG     := 
APP_NAME           :=
TARGET_MODULE      :=
DEPS_LIBRARIES     :=
