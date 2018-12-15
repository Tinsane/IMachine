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

.PHONY: clean coverage all

clean:
	find . -name '*.o' -delete


CFLAGS=$(WARN_OPTS) $(INCLUDES) -std=c11 $(COVERAGE)

CXXFLAGS=$(WARN_OPTS) $(INCLUDES) -std=c++17 $(COVERAGE)
LDFLAGS=-lm -lpthread $(COVERAGE)

all: asm vm

asm: \
		asm.o \
		src/arg_parse.o \
		src/assembler.o \
		src/code_line.o \
		src/tokenized_reader.o \
		src/buffered_file_reader.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

vm: \
		vm.o \
		src/arg_parse.o \
		src/buffered_file_reader.o \
		src/instruction_set.o \
		src/subprocess.o \
		src/executer.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_long_number: \
		long_number.o \
		$(GTEST)/src/gtest-all.o \
		$(GTEST)/src/gtest_main.o \
		tests/test_long_number.o
	$(CXX) -o $@ $^ $(LDFLAGS)

$(GTEST)/src/gtest-all.o: \
	$(GTEST)/src/gtest-all.cc

$(GTEST)/src/gtest_main.o: \
	$(GTEST)/src/gtest_main.cc

test_long_number.o: \
	tests/test_long_number.cc

coverage: test_long_number
	LLVM_PROFILE_FILE=$^.profraw ./$^
	$(LLVM_PROFDATA) merge -sparse $^.profraw -o $^.profdata
	$(LLVM_COV) $(COVERAGE_FORMAT) \
		-instr-profile=$^.profdata $^

