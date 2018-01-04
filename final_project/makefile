all:
	g++ project.cpp SockOp.c project_func.cpp Sock.cpp MC.cpp WL.cpp POS.cpp -o Server -pthread -std=c++11
	g++ PXASimu.cpp SockOp.c -o Client -pthread
test:
	g++ test.cpp SockOp.c project_func.cpp Sock.cpp MC.cpp WL.cpp POS.cpp -o testServer -pthread -fopenmp
