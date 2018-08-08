CFLAGS += -O0

PROGRAM = spectre
SOURCE  = spectre.c

all: $(PROGRAM) $(PROGRAM)-FAIL

GIT_SHELL_EXIT := $(shell git status --porcelain 2> /dev/null >&2 ; echo $$?)

# It can be non-zero when not in git repository or git is not installed.
# It can happen when downloaded using github's "Download ZIP" option.
ifeq ($(GIT_SHELL_EXIT),0)
# Check if working dir is clean.
GIT_STATUS := $(shell git status --porcelain)
ifndef GIT_STATUS
GIT_COMMIT_HASH := $(shell git rev-parse HEAD)
CFLAGS += -DGIT_COMMIT_HASH='"$(GIT_COMMIT_HASH)"'
endif
endif

$(PROGRAM): $(SOURCE) ; $(CC) $(CFLAGS) -o $(PROGRAM) $(SOURCE)
$(PROGRAM)-FAIL: $(SOURCE) ; $(CC) $(CFLAGS) -DTRAINOUTSIDE -o $(PROGRAM)-FAIL $(SOURCE)

clean: ; rm -f $(PROGRAM) $(PROGRAM)-FAIL
