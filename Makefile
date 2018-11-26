##########################################
# Makefile for funnel
#########################################

SRC_DIR=src

# adopt from https://github.com/AltraMayor/f3/issues/42
# xcode-select --install
# export CFLAGS="$CFLAGS -I/usr/local/include/"
# export LDFLAGS="$LDFLAGS -L/usr/local/lib/ -largp"
UNAME := $(shell uname)
test_osx:
ifeq ($(UNAME),Darwin)
	curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install | ruby
	brew install argp-standalone
endif

build: test_osx
	$(MAKE) -C $(SRC_DIR) build

test:
	$(MAKE) -C tests all

clean:
	$(MAKE) -C $(SRC_DIR) clean
	$(MAKE) -C tests clean
