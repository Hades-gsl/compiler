CC = gcc
FLEX = flex
BISON = bison
CFLAGS = -std=c99

ifdef ENABLE_DEBUG 
    CFLAGS += -DDEBUG -g
endif

CFILES = $(shell find ./ -name "*.c")
OBJS = $(CFILES:.c=.o)
LFILE = $(shell find ./ -name "*.l")
YFILE = $(shell find ./ -name "*.y")
LFC = $(shell find ./ -name "*.l" | sed s/[^/]*\\.l/lex.yy.c/)
YFC = $(shell find ./ -name "*.y" | sed s/[^/]*\\.y/syntax.tab.c/)
LFO = $(LFC:.c=.o)
YFO = $(YFC:.c=.o)

parser: syntax $(filter-out $(LFO),$(OBJS))
	$(CC) -o parser $(filter-out $(LFO),$(OBJS)) -lfl $(CFLAGS)

syntax: lexical syntax-c
	$(CC) -c $(YFC) -o $(YFO) $(CFLAGS)

lexical: $(LFILE)
	$(FLEX) -o $(LFC) $(LFILE)

syntax-c: $(YFILE)
	$(BISON) -o $(YFC) -d -v $(YFILE)

-include $(patsubst %.o, %.d, $(OBJS))


.PHONY: clean test package clean-package mv
test: parser
	./parser ../Test/test1.cmm
clean:
	rm -f parser lex.yy.c syntax.tab.c syntax.tab.h syntax.output
	rm -f $(OBJS) $(OBJS:.o=.d)
	rm -f $(LFC) $(YFC) $(YFC:.c=.h)
	rm -f *~


PACKAGE := 211220079.zip
PACKAGEDIR := Code Test
PACKAGEFILES := README.md report.pdf parser

package:
	@echo "Creating package..."
	@make && mv parser ../ && make clean
	@cd .. && zip -r $(PACKAGE) $(PACKAGEDIR) $(PACKAGEFILES)
	@echo "Package created: $(PACKAGE)"

clean-package:
	@echo "Cleaning package..."
	@cd .. && rm -f $(PACKAGE)
	@echo "Package cleaned."

mv: package
	@echo "Moving package..."
	@mv ../$(PACKAGE) /mnt/c/Users/砳砳/Desktop
	@echo "Package moved."
