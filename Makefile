CC=/usr/bin/clang
CXX=/usr/bin/clang++
LD=/usr/bin/clang++
LLVM_PROFDATA=/usr/bin/llvm-profdata-6.0
LLVM_COV=/usr/bin/llvm-cov-6.0

COVERAGE_FORMAT=report

GTEST=googletest/googletest

INCLUDES=-I$(GTEST) -I$(GTEST)/include
WARN_OPTS=-Wall -Werror -pedantic

COVERAGE=-fprofile-instr-generate -fcoverage-mapping

.PHONY: clean coverage all asm vm check

clean:
	find . -name '*.o' -delete


CFLAGS=$(WARN_OPTS) $(INCLUDES) -std=c11 $(COVERAGE)

CXXFLAGS=$(WARN_OPTS) $(INCLUDES) -std=c++17 $(COVERAGE)
LDFLAGS=-lm -lpthread $(COVERAGE)

SRC = \
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
		unit_tests/test_tokenized_reader.o

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
		$(SRC) \
		$(GTEST)/src/gtest-all.o \
		$(GTEST)/src/gtest_main.o \
		$(UNIT_TESTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

coverage: check
	LLVM_PROFILE_FILE=$^.profraw ./$^
	$(LLVM_PROFDATA) merge -sparse $^.profraw -o $^.profdata
	$(LLVM_COV) $(COVERAGE_FORMAT) \
		-instr-profile=$^.profdata $^

