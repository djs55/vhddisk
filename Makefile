.PHONY: all clean install build
all: build doc

NAME=grant-table
J=4

export OCAMLRUNPARAM=b

TESTS ?= --enable-tests
ifneq "$(MIRAGE_OS)" ""
TESTS := --disable-tests
endif


setup.bin: setup.ml
	@ocamlopt.opt -o $@ $< || ocamlopt -o $@ $< || ocamlc -o $@ $<
	@rm -f setup.cmx setup.cmi setup.o setup.cmo

setup.data: setup.bin
	@./setup.bin -configure $(TESTS)

build: setup.data setup.bin
	@./setup.bin -build -j $(J)

doc: setup.data setup.bin
	@./setup.bin -doc -j $(J)

install: setup.bin
	@./setup.bin -install

test: setup.bin build
	@./setup.bin -test

reinstall: setup.bin
	@ocamlfind remove $(NAME) || true
	@./setup.bin -reinstall

clean:
	@ocamlbuild -clean
	@rm -f setup.data setup.log setup.bin
