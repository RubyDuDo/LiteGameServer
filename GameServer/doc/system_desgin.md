<!-- toc -->
- [System Overview](#system-overview)
  - [System Structure](#system-structure)
  - [Threads](#threads)
  - [Module and Directory](#module-and-directory)
- [Network](#network)
  - [Introduction](#introduction)
  - [Three Layers](#three-layers)
  - [EPOLL Mode](#epoll-mode)
- [Database](#database)
- [Build System (CMake)](#build-system-cmake)
- [Configuration](#configuration)
  - [Game Config](#game-config)
  - [Server Config](#server-config)
  - [Data Hot Reload](#data-hot-reload)
- [Online State Management](#online-state-management)
- [Role ID Generation](#role-id-generation)
- [Gracefully Exit](#gracefully-exit)
  - [Trigger: Singal Handle](#trigger-singal-handle)
  - [Exit Waiting thread](#exit-waiting-thread)
- [Logging \& Metrics](#logging--metrics)
  - [Log System](#log-system)
  - [Statistic Event Log](#statistic-event-log)
    - [Storage Options](#storage-options)
    - [Event Log Format](#event-log-format)
    - [Common Info to Include](#common-info-to-include)
    - [How to Add a Event Log Type](#how-to-add-a-event-logtype)
- [Test](#test)
  - [Unit Tests](#unit-tests)
  - [Integration Tests](#integration-tests)
- [Deployment \& Operations](#deployment--operations)
  - [Run Scripts](#run-scripts)
  - [CI/CD Pipeline](#cicd-pipeline)
- [Utils](#utils)
  - [Memory Pool](#memory-pool)

<!-- /toc -->

# System Overview
## System Structure
## Threads
## Module and Directory
# Network
## Introduction


## Three Layers

For a game server, network functionality is one of the essential features, so it was the first component I added to my LiteGameServer. In a game server, the network functionality is usually divided into three layers:

**Socket Layer**  
This is the lowest layer, which uses sockets to manage connections and send/receive messages. It establishes communication between the client and the server. In this layer, you should create a socket that listens on a specific port and accepts new connections. You should also use multiplexing techniques, such as select and epoll, to manage multiple sockets within a single thread, handling tasks like sending, receiving, disconnecting, and detecting disconnections. In this project, I use epoll on Linux and select on macOS (macOS is used only for testing purposes). The specific design of this layer will be discussed in another article.

**Buffer Layer**  
TCP is widely used for client connections. In LiteGameServer, I also chose to support TCP as the primary connection method. Since TCP is a stream protocol, the data received from the socket interface is usually not a complete message or may contain parts of multiple messages. Therefore, we need a buffer to store the incoming data until the entire message has arrived, after which we can reconstruct the complete message. Once a message is fully constructed, it is forwarded to the game logic. One widely used method is to send the length of the message before sending the actual data. The receiver first reads a fixed-size length field, and then uses that information to determine the size of the message. If there is enough data in the buffer, the message is then constructed. When sending lengths, pay attention to the byte order. Otherwise, an incorrect length received could lead to serious errors.

**Application Protocol Layer**  
After receiving a message, it's time to interpret its contents, which means determining the application protocol. There are three common ways to encode a message:

***Text Format (e.g., JSON):***  
It is simple and easy to read; since it is just text, you do not have to worry about byte order issues. However, text-based messages are typically longer than their binary counterparts, consuming more bandwidth. Moreover, they are easier to intercept, making them less secure.

**Custom Message Types with Serialization and Deserialization:**  
This approach is safe and efficient, and since you have complete control over the format, it is more flexible. The disadvantage is that it requires more effort; you must implement a serialization mechanism and functions for every message. Additionally, it is your responsibility to ensure that everything is correctly handled.

**Third-Party Libraries, such as Protobuf:**  
Third-party serialization libraries like Protobuf are robust and efficient; moreover, they offer advanced features, such as optional fields, which are quite useful for sophisticated applications. Additionally, they support multiple programming languages. For example, if your client is written in C# (Unity) and your game server in C++, it is very convenient. All you need to do is write a configuration file and use the provided tools to generate code for different languages. The drawback is that it introduces a dependency on an external project, which increases the overall complexity of your project.

I don't want to manage my own serialization, and the benefits of supporting multiple languages and version updates with Protobuf are quite appealing, so I chose to use Protobuf in this project.
## EPOLL Mode

# Database

# Build System (CMake)

# Configuration
## Game Config

## Server Config

## Data Hot Reload

# Online State Management

# Role ID Generation

# Gracefully Exit
## Trigger: Singal Handle
## Exit Waiting thread

# Logging & Metrics
## Log System
## Statistic Event Log
**Files**: `EventLog`

This module is used for collecting data for statistic analysis. It could include online data, system performance data, or other events infomation like login, logout, enter room and spend money.

### Storage Options
Usually, statistic log have three options:
**Write to file**
This is quite efficient. However, you need more work to process the data because it has no structure and type information.

**Write to DB**
Relational database or NoSQL database. Using this method will be more time-consuming and put some stress on db.

**Send to network**
The receiver uses a db/file to record the logs. It's also efficient, but it adds complexity as you need to add network communication and a receiver.

In this project, we choose logging into files, as its both simple and efficient. How to deal with the event log files is out of the scope of this project. However, there are some agents to collection event log from files with low delay, so we can leave this part to consider later.

Our log system is used to write backend logs to files.

All the event log types will be written to a single file, as it's more convenient to trace the sequence of events, and you can simply split these events by grep event type.

### Event Log Format
About the log format, there are two options.
For example: a login event contain roleID = 1, time = 135

- Raw Content:
  
    `1,135`

    Content using this method is quite efficient as the message is short. However, it's hard to read and it rely heavily on the order which may cause problems.

- Structured content(json):
`{roleID=1,time=135}`

    Using this method, the data can explain itself, and support nested data, the shortage is the message is much longer.

In our project, we choose json as our log format.

### Common Info to Include

There are some information that are included in most event types:

`EventType`: As we put all events in a single file, it's necessary.

`time`: When the event happen.

`ret`: Error Code (0)-success, (other)-error reason. 

`roleID`: 



### How to Add a Event Log Type

Just add a member function to EventLogs

```c++
void onEventLogin( int ret, int roleID );
```
Then ,you can invoke it:

```c++
EventLogs::getInstance()->onEventLogin( MsgErr_OK, query.roleid());
```
# Test
## Unit Tests
## Integration Tests

# Deployment & Operations
## Run Scripts
## CI/CD Pipeline

# Utils
## Memory Pool





