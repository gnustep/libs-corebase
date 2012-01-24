ifeq ($(GNUSTEP_MAKEFILES),)
 GNUSTEP_MAKEFILES := $(shell gnustep-config --variable=GNUSTEP_MAKEFILES 2>/dev/null)
  ifeq ($(GNUSTEP_MAKEFILES),)
    $(warning )
    $(warning Unable to obtain GNUSTEP_MAKEFILES setting from gnustep-config!)
    $(warning Perhaps gnustep-make is not properly installed,)
    $(warning so gnustep-config is not in your PATH.)
    $(warning )
    $(warning Your PATH is currently $(PATH))
    $(warning )
  endif
endif

ifeq ($(GNUSTEP_MAKEFILES),)
  $(error You need to set GNUSTEP_MAKEFILES before compiling!)
endif

include $(GNUSTEP_MAKEFILES)/common.make

all::
	@(echo If you want to run the gnustep-corebase testsuite, please type \'make check\')

TOP_DIR := $(shell dirname $(CURDIR))

check::
	(\
	ADDITIONAL_INCLUDE_DIRS="-I$(TOP_DIR)/Headers";\
	ADDITIONAL_LIB_DIRS="-L$(TOP_DIR)/Source/$(GNUSTEP_OBJ_DIR)";\
	LD_LIBRARY_PATH="$(TOP_DIR)/Source/$(GNUSTEP_OBJ_DIR):${LD_LIBRARY_PATH}";\
	PATH="$(TOP_DIR)/Source/$(GNUSTEP_OBJ_DIR):${PATH}";\
	export ADDITIONAL_INCLUDE_DIRS;\
	export ADDITIONAL_LIBS_DIR;\
	export LD_LIBRARY_PATH;\
	export PATH;\
	if [ "$(DEBUG)" = "" ]; then \
	  gnustep-tests;\
	else\
	  gnustep-tests --debug;\
	fi;\
	)

clean::
	gnustep-tests --clean

include $(GNUSTEP_MAKEFILES)/rules.make