# GameServer
This is a simple Game Server Project, including some basic functions like network, db, player management. After developping this project, I hope I can have a better understanding about the whole picture of game server developping. I also hope this project could help other people in need.

The client for this game server is https://github.com/RubyDuDo/GameClient.

This Document will introduce some module of this server and list some problems I encounterred.

## General introduction
This project is running on Mac OS.

### function list
The functions are add in the following order:
* game run loop
* network 
* msg queue
* serialize
* database
* load server config, like socket port, db info

### functions in waiting list

* load game config data

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

### Considerations
#### DB Query Context
Different conditions may trigger a same db query, when we got db response, we need to know what to do next, so we need to save the context in some way. The context may not only include the original request from client, but also include current states of the dealing process. Moreover, db queries may not be triggered by client request. In summary, the way to save contexts must be very flexible to cover diverse conditions and params. 

In this project, we use ` map<int, std::function<void( const DBResponse&)> > m_mapDBRspFuns; `to store the context. The key is a unique query ID generated when send query to db. and the function is a void( const DBResponse&) type. In fact, when the response comes back, the dealing process must need some extra information to finish jobs. In order to get these diverse information, lambda is used to catch needed local information. 

In order to make the code looks cleaner, we can add a named function which contain params, and the lambda function invokes the named function and passes the local value captured to it. For example:  
```cpp
        addDBQuery( req , [sockID, this](const DBResponse& rsp ){
            dealAddRole( sockID ,  rsp );
        });
```