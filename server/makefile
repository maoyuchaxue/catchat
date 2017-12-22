main : FriendList.o LatestMessage.o User.o Verify.o main.o World.o
	g++ -pthread -o catchat LatestMessage.o User.o Verify.o main.o World.o

LatestMessage.o : LatestMessage.h LatestMessage.cpp
	g++ -c -o LatestMessage.o LatestMessage.cpp

FriendList.o : FriendList.h FriendList.cpp
	g++ -c -o FriendList.o FriendList.cpp

User.o : User.h User.cpp World.h
	g++ -c -o User.o User.cpp

World.o : World.h World.cpp User.h
	g++ -c -o World.o World.cpp

Verify.o : Verify.h Verify.cpp
	g++ -c -o Verify.o Verify.cpp

main.o : main.cpp World.h User.h
	g++ -c -o main.o main.cpp