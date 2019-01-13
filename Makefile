CC=/usr/bin/clang
CXX=/usr/bin/clang++
LD=/usr/bin/clang++
LLVM_PROFDATA=/usr/bin/llvm-profdata-6.0
LLVM_COV=/usr/bin/llvm-cov-6.0

COVERAGE_FORMAT=report

GTEST=googletest/googletest

INCLUDES=-I$(GTEST) -I$(GTEST)/include
WARN_OPTS=-Wall -Werror -pedantic

SANITIZER=-fsanitize=address,undefined -fno-sanitize-recover=all

COVERAGE=-fprofile-instr-generate -fcoverage-mapping

CFLAGS=$(WARN_OPTS) $(INCLUDES) $(SANITIZER) -std=c11 $(COVERAGE)

CXXFLAGS=$(WARN_OPTS) $(INCLUDES) $(SANITIZER) -std=c++17 $(COVERAGE)
LDFLAGS=$(SANITIZER) -lm -lpthread $(COVERAGE)

SRC = \
		src/parse_number.c \
		src/assembler.c \
		src/code_line.c \
		src/tokenized_reader.c \
		src/buffered_file_reader.c \
		src/instruction_set.c \
		src/subprocess.c \
		src/executer.c

SRCO = \
		src/parse_number.o \
		src/assembler.o \
		src/code_line.o \
		src/tokenized_reader.o \
		src/buffered_file_reader.o \
		src/instruction_set.o \
		src/subprocess.o \
		src/executer.o

UNIT_TESTS = \
		unit_tests/test_parse_number.o \
		unit_tests/test_instruction_set.o \
		unit_tests/test_subprocess.o \
		unit_tests/test_non_jump_instructions.o \
		unit_tests/test_tokenized_reader.o

.PHONY: clean coverage all check

clean:
	find . -name '*.o' -delete
	rm asm
	rm vm
	rm check
	rm *.profdata
	rm *.profraw

all: asm vm check

asm: \
		asm.o \
		src/parse_number.o \
		src/assembler.o \
		src/code_line.o \
		src/tokenized_reader.o \
		src/buffered_file_reader.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

vm: \
		vm.o \
		src/parse_number.o \
		src/buffered_file_reader.o \
		src/instruction_set.o \
		src/subprocess.o \
		src/executer.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

check: \
		$(SRCO) \
		$(GTEST)/src/gtest-all.o \
		$(GTEST)/src/gtest_main.o \
		$(UNIT_TESTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

coverage: check
	LLVM_PROFILE_FILE=$^.profraw ./$^
	$(LLVM_PROFDATA) merge -sparse $^.profraw -o $^.profdata
	$(LLVM_COV) $(COVERAGE_FORMAT) \
		-instr-profile=$^.profdata $^ $(SRC) 

