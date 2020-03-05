# build execute target
RELEASE_TARGET          := $(TARGET_MODULE)
DEBUG_TARGET            := $(TARGET_MODULE)_DEBUG

CPP_TARGETDIR           := $(APP_ROOT)/$(APP_NAME)/target/Release
CPP_TARGETDIR_DEBUG     := $(APP_ROOT)/$(APP_NAME)/target/Debug
CPP_OBJECT_MIDDLE       := $(APP_ROOT)/im_$(PLATFORM)_$(PROJECT_NAME)/$(APP_NAME)/$(TARGET_MODULE)/Release
CPP_OBJECT_MIDDLE_DEBUG := $(APP_ROOT)/im_$(PLATFORM)_$(PROJECT_NAME)/$(APP_NAME)/$(TARGET_MODULE)/Debug
SDK_LIBRARIES           := $(TARGET_ROOT)/$(PLATFORM)/$(PROJECT_NAME)/Release
SDK_LIBRARIES_DEBUG     := $(TARGET_ROOT)/$(PLATFORM)/$(PROJECT_NAME)/Debug

COPY_LIBRARIES          := $(DEPS_LIBRARIES:%.so=$(SDK_LIBRARIES)/%.so)
COPY_LIBRARIES_DEBUG    := $(DEPS_LIBRARIES:%.so=$(SDK_LIBRARIES_DEBUG)/%.so)

COPY_LIBRARIES_THIRD    := $(DEPS_LIBRARIES_THIRD:%.so=$(THIRD_LIBRARY_PATH)/%.so)

CPP_APP_OBJECT          := $(foreach filename,$(CPP_SRC:%.cpp=%.o),$(subst $(CPP_SRCDIR),$(CPP_OBJECT_MIDDLE),$(filename)))
CPP_APP_OBJECT_DEBUG    := $(foreach filename,$(CPP_SRC:%.cpp=%.o),$(subst $(CPP_SRCDIR),$(CPP_OBJECT_MIDDLE_DEBUG),$(filename)))

CPP_APP_D               := $(foreach filename,$(CPP_SRC:%.cpp=%.d),$(subst $(CPP_SRCDIR),$(CPP_OBJECT_MIDDLE),$(filename)))
CPP_APP_D_DEBUG         := $(foreach filename,$(CPP_SRC:%.cpp=%.d),$(subst $(CPP_SRCDIR),$(CPP_OBJECT_MIDDLE_DEBUG),$(filename)))

.PHONY: all clean debug release

all: release debug
release: $(RELEASE_TARGET)
debug: $(DEBUG_TARGET)

ifneq ($(PCH_H),)
PCH       := $(CPP_OBJECT_MIDDLE)/$(notdir $(PCH_H)).gch
PCH_DEBUG := $(CPP_OBJECT_MIDDLE_DEBUG)/$(notdir $(PCH_H)).gch

$(PCH) : $(PCH_H)
	@echo "> Generating PCH file: $(notdir $@)"
	@$(MKDIR) $(dir $@)
	@$(CPP) $(TARGET_ARCH) $(CFLAGS) -x c++-header -c $(PCH_H) -o $@ $(CPP_INCLUDE)
	@echo ' '
	@echo '> Compiling...'

$(PCH_DEBUG) : $(PCH_H)
	@echo "> Generating PCH file: $(notdir $@)"
	@$(MKDIR) $(dir $@)
	@$(CPP) $(TARGET_ARCH) $(CFLAGS_DEBUG) -x c++-header -c $(PCH_H) -o $@ $(CPP_INCLUDE)
	@echo ' '
	@echo '> Compiling...'
else
PCH       :=
PCH_DEBUG :=
endif

ifneq ($(C_SRC),)
C_APP_OBJECT       := $(foreach filename,$(C_SRC:%.c=%.o),$(subst $(CPP_SRCDIR),$(CPP_OBJECT_MIDDLE),$(filename)))
C_APP_OBJECT_DEBUG := $(foreach filename,$(C_SRC:%.c=%.o),$(subst $(CPP_SRCDIR),$(CPP_OBJECT_MIDDLE_DEBUG),$(filename)))

C_APP_D            := $(foreach filename,$(C_SRC:%.c=%.d),$(subst $(CPP_SRCDIR),$(CPP_OBJECT_MIDDLE),$(filename)))
C_APP_D_DEBUG      := $(foreach filename,$(C_SRC:%.c=%.d),$(subst $(CPP_SRCDIR),$(CPP_OBJECT_MIDDLE_DEBUG),$(filename)))

$(C_APP_OBJECT): $(CPP_OBJECT_MIDDLE)%.o: $(CPP_SRCDIR)%.c
	@echo $(notdir $<)
	@$(MKDIR) $(dir $@)
	@$(CPP) $(TARGET_ARCH) -I$(CPP_OBJECT_MIDDLE) $(CPP_INCLUDE) $(CFLAGS) -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

$(C_APP_OBJECT_DEBUG): $(CPP_OBJECT_MIDDLE_DEBUG)%.o: $(CPP_SRCDIR)%.c
	@echo $(notdir $<)
	@$(MKDIR) $(dir $@)
	@$(CPP) $(TARGET_ARCH) -I$(CPP_OBJECT_MIDDLE_DEBUG) $(CPP_INCLUDE) $(CFLAGS_DEBUG) -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
else
C_APP_OBJECT       :=
C_APP_OBJECT_DEBUG :=
endif

$(CPP_APP_OBJECT): $(CPP_OBJECT_MIDDLE)%.o: $(CPP_SRCDIR)%.cpp $(PCH)
	@echo $(notdir $<)
	@$(MKDIR) $(dir $@)
	@$(CPP) $(TARGET_ARCH) -I$(CPP_OBJECT_MIDDLE) $(CPP_INCLUDE) $(CFLAGS) -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

$(CPP_APP_OBJECT_DEBUG): $(CPP_OBJECT_MIDDLE_DEBUG)%.o: $(CPP_SRCDIR)%.cpp $(PCH_DEBUG)
	@echo $(notdir $<)
	@$(MKDIR) $(dir $@)
	@$(CPP) $(TARGET_ARCH) -I$(CPP_OBJECT_MIDDLE_DEBUG) $(CPP_INCLUDE) $(CFLAGS_DEBUG) -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

$(RELEASE_TARGET): $(CPP_APP_OBJECT) $(C_APP_OBJECT)
	@echo '> Linking: $@'
	@$(MKDIR) $(CPP_TARGETDIR)
	@$(COPY) $(COPY_LIBRARIES) $(CPP_TARGETDIR)/
	@pwd
	@$(CPP) $(TARGET_ARCH) -o $@ $(CPP_APP_OBJECT) $(C_APP_OBJECT) $(LIBS_DIR) $(LIBS)
	@$(COPY) $(RELEASE_TARGET) $(CPP_TARGETDIR)/$(RELEASE_TARGET)
	@-$(RM) $(RELEASE_TARGET)
	@echo 'Finished building target: $@'
	@echo ' '

$(DEBUG_TARGET): $(CPP_APP_OBJECT_DEBUG) $(C_APP_OBJECT_DEBUG)
	@echo '> Linking: $@'
	@$(MKDIR) $(CPP_TARGETDIR_DEBUG)
	@$(COPY) $(COPY_LIBRARIES_DEBUG) $(CPP_TARGETDIR_DEBUG)/
	@$(CPP) $(TARGET_ARCH) -o $@ $(CPP_APP_OBJECT_DEBUG) $(C_APP_OBJECT_DEBUG) $(LIBS_DIR_DEBUG) $(LIBS)
	@$(COPY) $(DEBUG_TARGET) $(CPP_TARGETDIR_DEBUG)/$(RELEASE_TARGET)
	@-$(RM) $(DEBUG_TARGET)
	@echo 'Finished building target: $@'
	@echo ' '

clean:
	@echo 'clean-$(TARGET_MODULE)'
	@-$(RM) $(CPP_OBJECT_MIDDLE)
	@-$(RM) $(CPP_OBJECT_MIDDLE_DEBUG)
	@-$(RM) $(CPP_TARGETDIR)/$(RELEASE_TARGET)
	@-$(RM) $(CPP_TARGETDIR_DEBUG)/$(DEBUG_TARGET)
	@-$(RM) $(CPP_TARGETDIR)
	@-$(RM) $(CPP_TARGETDIR_DEBUG)
