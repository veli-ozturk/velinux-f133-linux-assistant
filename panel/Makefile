# F133 panel arayuzu (LVGL 9.2 + cJSON), statik riscv64. Cagri: scripts/28_build_panel_v1.sh
CC      := riscv64-linux-gnu-gcc
UI      ?= $(HOME)/f133/ui
LVGL    := $(UI)/lvgl
CJSON   := $(UI)/cJSON
OBJ     := $(UI)/obj
SRC     := $(UI)/src
OUT     ?= $(HOME)/f133/out/panel/panel

LVGL_SRCS := $(shell find $(LVGL)/src -name '*.c')
FONT_SRCS := $(wildcard $(SRC)/fonts/*.c)
OBJS := $(patsubst $(LVGL)/%.c,$(OBJ)/lvgl/%.o,$(LVGL_SRCS)) $(patsubst $(SRC)/fonts/%.c,$(OBJ)/fonts/%.o,$(FONT_SRCS)) $(OBJ)/cJSON.o $(OBJ)/main.o
CFLAGS := -O2 -DLV_CONF_INCLUDE_SIMPLE -I$(UI) -I$(CJSON) -ffunction-sections -fdata-sections \
	-Wall -Wno-unused-parameter -Wno-unused-function

$(OUT): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) -static -Wl,--gc-sections -o $@ $^ -lm -lpthread

$(OBJ)/lvgl/%.o: $(LVGL)/%.c $(UI)/lv_conf.h
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/cJSON.o: $(CJSON)/cJSON.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/main.o: $(SRC)/main.c $(UI)/lv_conf.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ)/fonts/%.o: $(SRC)/fonts/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
