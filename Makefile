CC = g++
SRC_DIR := src
BIN_DIR := build

main: ${SRC_DIR}/main.cpp ${BIN_DIR}/backupper.o ${SRC_DIR}/debug.h
	${CC} -o main ${SRC_DIR}/main.cpp ${BIN_DIR}/backupper.o

${BIN_DIR}/backupper.o: ${SRC_DIR}/backupper.h ${SRC_DIR}/backupper.cpp ${SRC_DIR}/debug.h  ${BIN_DIR}
	${CC} -c ${SRC_DIR}/backupper.cpp -o ${BIN_DIR}/backupper.o

${BIN_DIR}:
	mkdir ${BIN_DIR}

clean:
	rm -r ${BIN_DIR} main
