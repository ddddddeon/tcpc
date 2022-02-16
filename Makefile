SERVER_NAME=server
CLIENT_NAME=client
LIBS=-lpthread -lcrypto -lleveldb
CFLAGS=-g -Wall -fuse-ld=lld $(LIBS) #-fsanitize=address -fsanitize=memory -fsanitize=thread

CC=clang++
SERVER_OUTFILE=bin/$(SERVER_NAME)
CLIENT_OUTFILE=bin/$(CLIENT_NAME)
SERVER_INFILES=$(wildcard src/$(SERVER_NAME)/*.cpp)
CLIENT_INFILES=$(wildcard src/$(CLIENT_NAME)/*.cpp)
TEST_INFILES=$(wildcard test/*.cpp)
TEST_OUTFILE=bin/test
SHARED_LIBFILES=$(wildcard src/lib/*.cpp)

.PHONY: default

default: $(SERVER_NAME) $(CLIENT_NAME)

$(SERVER_NAME): 
	set -e; \
	if [ ! -d bin ]; then mkdir bin; fi; \
	$(CC) -o $(SERVER_OUTFILE) $(SERVER_INFILES) $(SHARED_LIBFILES) $(CFLAGS); 

$(CLIENT_NAME): 
	set -e; \
	if [ ! -d bin ]; then mkdir bin; fi; \
	$(CC) -o $(CLIENT_OUTFILE) $(CLIENT_INFILES) $(SHARED_LIBFILES) $(CFLAGS); 

tests:
	@set -e; \
	if [ ! -d bin ]; then mkdir bin; fi; \
	$(CC) -o $(TEST_OUTFILE) $(TEST_INFILES) $(SHARED_LIBFILES) $(CFLAGS);

flush: flush-db flush-keys

flush-db:
	rm -rf db;

flush-keys:
	rm id_rsa*;

clean:	find-bin
	@rm -rf bin;

find-bin:
	@[ -d bin ];

install:
	@mv bin/$(SERVER_NAME) /usr/bin/$(SERVER_NAME); \
	chmod a+x /usr/bin/$(SERVER_NAME); \
	echo "[OK] installed to /usr/bin/$(SERVER_NAME)"; \
	mv bin/$(CLIENT_NAME) /usr/bin/$(CLIENT_NAME); \
	chmod a+x /usr/bin/$(CLIENT_NAME); \
	echo "[OK] installed to /usr/bin/$(CLIENT_NAME)";

format:
	@clang-format -i -style=google src/**/*.cpp src/**/*.h

tidy:
	@clang-tidy --checks=google-* -header-filter=.* src/**/*.cpp src/**/*.h

lint:
	@cpplint src/**/*.cpp src/**/*.h

check: check-$(SERVER_NAME) check-$(CLIENT_NAME)

check-$(SERVER_NAME):
	@valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./bin/$(SERVER_NAME);

trace-$(SERVER_NAME):
	@strace ./bin/$(SERVER_NAME)

check-$(CLIENT_NAME):
	@valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./bin/$(CLIENT_NAME)

trace-$(CLIENT_NAME):
	@strace ./bin/$(CLIENT_NAME)

sloc:
	@git ls-files | xargs wc -l

all: default find-bin install

rebuild: clean default install

