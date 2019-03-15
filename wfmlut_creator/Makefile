BIN_DIR := bin
OBJ_DIR := obj
SRC_DIR := .
INC_DIR := .

BINS := $(BIN_DIR)/wfmlut_creator
SRCS := $(wildcard $(SRC_DIR)/*.c)
INCS := $(wildcard $(SRC_DIR)/*.h)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

LDFLAGS := -lm
override CFLAGS += -Wall -Werror -Wextra

all: $(BINS)

$(BINS): $(OBJS) | $(BIN_DIR)/
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INCS) | $(OBJ_DIR)/
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN_DIR)/ $(OBJ_DIR)/:
	mkdir -p $@

clean:
	rm -f $(BINS) $(OBJS)

distclean:
ifneq ("$(BIN_DIR)",".")
	rm -rf $(BIN_DIR)
endif
ifneq ("$(OBJ_DIR)",".")
	rm -rf $(OBJ_DIR)
endif

