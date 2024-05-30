#
# Makefile
# WARNING: relies on invocation setting current working directory to Makefile location
# This is done in .vscode/task.json
#

APP_DIR 			:= apps/SL8_410x502


# select underlaying LCGL display driver (SDL2 || X11)
#LV_DRIVER          		:= X11
LV_DRIVER          		:= SDL2

PROJECT 			?= simulator
#MAKEFLAGS 			:= -j $(shell nproc)

CC 					?= gcc
GDB					?= gdb

SRC_DIR				:= .
WORKING_DIR			:= ./build
BUILD_DIR			:= $(WORKING_DIR)/obj
BIN_DIR				:= $(WORKING_DIR)/bin

WARNINGS 			:= -Wall -Wextra \
						-Wshadow -Wundef -Wmaybe-uninitialized -Wmissing-prototypes -Wno-discarded-qualifiers \
						-Wno-unused-function -Wno-error=strict-prototypes -Wpointer-arith -fno-strict-aliasing -Wno-error=cpp -Wuninitialized \
						-Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-nonliteral -Wno-cast-qual -Wunreachable-code -Wno-switch-default  \
					  	-Wreturn-type -Wmultichar -Wformat-security -Wno-ignored-qualifiers -Wno-error=pedantic -Wno-sign-compare -Wno-error=missing-prototypes -Wdouble-promotion -Wclobbered -Wdeprecated  \
						-Wempty-body -Wshift-negative-value -Wstack-usage=2048 \
            			-Wtype-limits -Wsizeof-pointer-memaccess -Wpointer-arith

CFLAGS 				:= -O0 -g $(WARNINGS) -DWINDOW_POS_X=10 -DWINDOW_POS_Y=800
#CFLAGS 				+= -fsanitize=address -fno-omit-frame-pointer 
OS_UNAME			:= $(shell uname)
ifeq "$(OS_UNAME)" "Darwin"
CFLAGS				+= -DUSE_MACOS
endif

# simulator library define
ifeq  "$(LV_DRIVER)" "SDL2"
LV_DRIVER_USE       := USE_SDL
else
LV_DRIVER_USE       := USE_$(LV_DRIVER)
endif

# Add simulator defines to allow modification of source
DEFINES				:= -D SIMULATOR=1 -D LV_BUILD_TEST=0 -D $(LV_DRIVER_USE)

ALKAID_UI           := alkaid_ui
ALKAID_MIDDLEWARE	:= alkaid_middleware

# Include simulator inc folder first so lv_conf.h from custom UI can be used instead
INC 				:= -I./ui/simulator/inc/ -I./ -I./lvgl/ -I/opt/homebrew/include/ #-I/usr/include/freetype2 -L/usr/local/lib
INC 				+= -I$(SRC_DIR)
INC 				+= -I$(ALKAID_UI)/include
INC 				+= -I$(ALKAID_UI)/include/generated
INC 				+= -I$(ALKAID_UI)/include/porting
INC 				+= -I$(ALKAID_UI)/include/pages_SL8_410x502
INC 				+= -I$(ALKAID_UI)/include/widgets
INC 				+= -I$(ALKAID_UI)/include/anim
INC 				+= -I$(ALKAID_UI)/include/page_mgr
INC 				+= -I$(ALKAID_UI)/include/res_mgr
INC 				+= -I$(ALKAID_UI)/include/storage
INC 				+= -I$(ALKAID_UI)/include/utils
INC 				+= -I$(ALKAID_UI)/include/proc
INC 				+= -I$(ALKAID_UI)/include/comm
INC 				+= -I$(ALKAID_UI)/include/sim
LDLIBS	 			:= -L/opt/homebrew/lib -l$(LV_DRIVER) -lpthread -lm #-lfreetype -lavformat -lavcodec -lavutil -lswscale -lm -lz
LDLIBS 				+= -llz4 
BIN 				:= $(BIN_DIR)/alkaidwatch_simulator

COMPILE				= $(CC) $(CFLAGS) $(INC) $(DEFINES)

# Automatically include all source files
SRCS 				:= $(shell find $(SRC_DIR)/$(ALKAID_UI)/src -type f -name '*.c' -not -path '*/\.*')
SRCS                += $(shell find $(SRC_DIR)/lv_drivers -type f -name '*.c' -not -path '*/sdl/*' -not -path '*/x11/*')
ifeq "$(LV_DRIVER)" "X11"
SRCS                += $(shell find $(SRC_DIR)/lv_drivers/x11 -type f -name '*.c')
else
SRCS                += $(shell find $(SRC_DIR)/lv_drivers/sdl -type f -name '*.c')
endif
SRCS                += $(shell find $(SRC_DIR)/lvgl -type f -name '*.c' -not -path '*/\.*')
SRCS                += $(shell find $(SRC_DIR)/main -type f -name '*.c' -not -path '*/\.*')
SRCS 				+= $(SRC_DIR)/assert.c
SRCS 				+= $(SRC_DIR)/key_proc.c

OBJECTS    			:= $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SRCS:.c=.o))


default: $(BIN)
	(cd $(ALKAID_UI)/include/generated; $(shell pwd)/$(BIN) $(SPEED))

test:
	@echo $(SRCS)
	@echo $(OBJECTS)

debug: $(BIN)
	@echo $(GDB) -cd=$(ALKAID_UI)/include/generated $(shell pwd)/$(BIN)
	@$(GDB) -cd=$(ALKAID_UI)/include/generated $(shell pwd)/$(BIN)

$(BIN): $(OBJECTS) 
	@mkdir -p $(BIN_DIR)
	$(CC) -o $(BIN) $(OBJECTS) $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c lv_demo_conf.h lv_conf.h
	@echo 'Building project file: $<'
	@mkdir -p $(dir $@)
	@$(COMPILE) -c -o "$@" "$<" 

clean:
	rm -rf $(WORKING_DIR)/bin
	rm -rf $(WORKING_DIR)/obj/$(ALKAID_UI)
	rm -rf $(WORKING_DIR)/obj/main

clean_all:
	rm -rf $(WORKING_DIR)

# install: ${BIN}
# 	install -d ${DESTDIR}/usr/lib/${PROJECT}/bin
# 	install $< ${DESTDIR}/usr/lib/${PROJECT}/bin/

font:
	(cd $(ALKAID_UI)/tools && make font)

img:
	(cd $(ALKAID_UI)/tools && make img)

imgsim:
	(cd $(ALKAID_UI)/tools && make imgsim)

str:
	(cd $(ALKAID_UI)/tools && make str)

tool:
	(cd $(ALKAID_UI)/tools && make tool)

clean_tool:
	(cd $(ALKAID_UI)/tools && make clean)

kill:
	@killall -9 alkaidwatch_simulator
