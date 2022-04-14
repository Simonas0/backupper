CC = g++
SRC_DIR := src
BIN_DIR := build

main: ${SRC_DIR}/main.cpp ${BIN_DIR}/backupper.o ${BIN_DIR}/logger.o ${SRC_DIR}/debug.h
	${CC} -o main ${SRC_DIR}/main.cpp ${BIN_DIR}/backupper.o ${BIN_DIR}/logger.o

${BIN_DIR}/backupper.o: ${SRC_DIR}/backupper.h ${SRC_DIR}/backupper.cpp
	mkdir -p ${BIN_DIR}
	${CC} -c ${SRC_DIR}/backupper.cpp -o ${BIN_DIR}/backupper.o

${BIN_DIR}/logger.o: ${SRC_DIR}/logger.h ${SRC_DIR}/logger.cpp ${SRC_DIR}/action.h
	mkdir -p ${BIN_DIR}
	${CC} -c ${SRC_DIR}/logger.cpp -o ${BIN_DIR}/logger.o

clean:
	rm -r ${BIN_DIR} main
