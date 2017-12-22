main : FriendList.o LatestMessage.o User.o Verify.o main.o World.o FriendList.o 
	g++ -pthread -std=c++11 -o catchat LatestMessage.o User.o Verify.o main.o World.o FriendList.o 

LatestMessage.o : LatestMessage.h LatestMessage.cpp
	g++ -std=c++11 -c -o LatestMessage.o LatestMessage.cpp

FriendList.o : FriendList.h FriendList.cpp
	g++ -std=c++11 -c -o FriendList.o FriendList.cpp

User.o : User.h User.cpp World.h
	g++ -std=c++11 -c -o User.o User.cpp

World.o : World.h World.cpp User.h
	g++ -std=c++11 -c -o World.o World.cpp

Verify.o : Verify.h Verify.cpp
	g++ -std=c++11 -c -o Verify.o Verify.cpp

main.o : main.cpp World.h User.h
	g++ -std=c++11 -c -o main.o main.cpp