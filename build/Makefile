LIBS:=  -lwebsockets -lmysqlclient -lpthread -lrt -lcjson
LIBPATH:= -L/usr/local/lib/ -L/usr/lib64/mysql/
INCLUDE:= -I/usr/include/mysql -I../include
OBJ:= main.o mesh_tcp.o mesh_ws.o mesh_sql.o data_solve.o json_check.o
CFLAG:= -std=c99 -g
DFLAG:= -D _XOPEN_SOURCE=500
TARGET:=meshserver.out
${TARGET}:${OBJ}
	gcc -o $@ ${OBJ} ${CFLAG} ${LIBS} ${LIBPATH} ${INCLUDE} ${DFLAG}
main.o: ../src/main.c
	gcc -c ${CFLAG} $^ ${DFLAG}
mesh_tcp.o: ../src/mesh_tcp.c 
	gcc -c ${CFLAG} $^ ${DFLAG}
mesh_ws.o: ../src/mesh_ws.c
	gcc -c ${CFLAG} $^ ${DFLAG}
mesh_sql.o:../src/mesh_sql.c
	gcc -c ${CFLAG} $^ -I/usr/include/mysql ${DFLAG}
data_solve.o:../src/data_solve.c
	gcc -c ${CFLAG} $^ ${DFLAG}
json_check.o:../src/json_check.c
	gcc -c ${CFLAG} $^ -I../include ${DFLAG}
clean:
	@rm -rf *.o