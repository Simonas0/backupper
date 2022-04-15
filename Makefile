CC = g++
SRC_DIR := src
BIN_DIR := build

main: ${SRC_DIR}/main.cpp ${BIN_DIR}/backupper.o ${BIN_DIR}/logger.o ${BIN_DIR}/cli.o ${SRC_DIR}/debug.h
	${CC} -o main ${SRC_DIR}/main.cpp ${BIN_DIR}/backupper.o ${BIN_DIR}/logger.o ${BIN_DIR}/cli.o

${BIN_DIR}/backupper.o: ${SRC_DIR}/backupper.h ${SRC_DIR}/backupper.cpp ${SRC_DIR}/logger.h ${SRC_DIR}/debug.h
	mkdir -p ${BIN_DIR}
	${CC} -c ${SRC_DIR}/backupper.cpp -o ${BIN_DIR}/backupper.o

${BIN_DIR}/logger.o: ${SRC_DIR}/logger.h ${SRC_DIR}/logger.cpp ${SRC_DIR}/action.h ${SRC_DIR}/debug.h
	mkdir -p ${BIN_DIR}
	${CC} -c ${SRC_DIR}/logger.cpp -o ${BIN_DIR}/logger.o

${BIN_DIR}/cli.o: ${SRC_DIR}/cli.h ${SRC_DIR}/cli.cpp ${SRC_DIR}/logger.h
	mkdir -p ${BIN_DIR}
	${CC} -c ${SRC_DIR}/cli.cpp -o ${BIN_DIR}/cli.o

clean:
	rm -r ${BIN_DIR} main
