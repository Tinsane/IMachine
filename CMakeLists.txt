cmake_minimum_required(VERSION 3.10)
project(IMachine C)

set(CMAKE_C_STANDARD 11)

add_executable(IMachine asm.c vm.c src/instructions.h src/assembler.h src/instruction_set.h src/instruction_set.c src/macro.h src/executer.c src/executer.h src/subprocess.c src/subprocess.h src/buffered_file_reader.c src/buffered_file_reader.h src/arg_parse.c src/arg_parse.h src/util.c src/util.h src/assembler.c src/code_line.c src/code_line.h src/tokenized_reader.c src/tokenized_reader.h)