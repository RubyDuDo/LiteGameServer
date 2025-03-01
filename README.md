# GameServer
This is a simple Game Server Project, including some basic functions like network, db, player management. After developping this project, I hope I can have a better understanding about the whole picture of game server developping. I also hope this project could help other people in need.

The client for this game server is https://github.com/RubyDuDo/GameClient.

This Document will introduce some module of this server and list some problems I encounterred.

## General introduction
This project is running on Mac OS.

### function list
The functions are add in the following order:
1. game run loop
2. network 
3. msg queue
4. serialize
5. database

### threads
* main thread: game update
* network thread( listen, send, receive)
* db query thread: 


## Network


## Serialize
When sending message between client and server, we need to serialize the messages instead of send the data directly, because two main reasons:
* the difference between network order and host order. Some host using big endian order while others use small endian order. If the client and server has different choices, then they may misunderstand each other. This mainly effects data type like int, short.
* the messages needed to send may have string, or vector data members, which's real data is somewhere out of their memory. If just send the data memory directly, these data are lost.

For serialize, I chose protobuf, which is widely used for serializing.

## DB(Mysql)