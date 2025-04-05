# Network
## Three Layers

For a game server, network functionality is one of the essential features, so it was the first component I added to my LiteGameServer. In a game server, the network functionality is usually divided into three layers:

**Socket Layer**  
This is the lowest layer, which uses sockets to manage connections and send/receive messages. It establishes communication between the client and the server. In this layer, you should create a socket that listens on a specific port and accepts new connections. You should also use multiplexing techniques, such as select and epoll, to manage multiple sockets within a single thread, handling tasks like sending, receiving, disconnecting, and detecting disconnections. In this project, I use epoll on Linux and select on macOS (macOS is used only for testing purposes). The specific design of this layer will be discussed in another article.

**Buffer Layer**  
TCP is widely used for client connections. In LiteGameServer, I also chose to support TCP as the primary connection method. Since TCP is a stream protocol, the data received from the socket interface is usually not a complete message or may contain parts of multiple messages. Therefore, we need a buffer to store the incoming data until the entire message has arrived, after which we can reconstruct the complete message. Once a message is fully constructed, it is forwarded to the game logic. One widely used method is to send the length of the message before sending the actual data. The receiver first reads a fixed-size length field, and then uses that information to determine the size of the message. If there is enough data in the buffer, the message is then constructed. When sending lengths, pay attention to the byte order. Otherwise, an incorrect length received could lead to serious errors.

**Application Protocol Layer**  
After receiving a message, it's time to interpret its contents, which means determining the application protocol. There are three common ways to encode a message:

**Text Format (e.g., JSON):**  
It is simple and easy to read; since it is just text, you do not have to worry about byte order issues. However, text-based messages are typically longer than their binary counterparts, consuming more bandwidth. Moreover, they are easier to intercept, making them less secure.

**Custom Message Types with Serialization and Deserialization:**  
This approach is safe and efficient, and since you have complete control over the format, it is more flexible. The disadvantage is that it requires more effort; you must implement a serialization mechanism and functions for every message. Additionally, it is your responsibility to ensure that everything is correctly handled.

**Third-Party Libraries, such as Protobuf:**  
Third-party serialization libraries like Protobuf are robust and efficient; moreover, they offer advanced features, such as optional fields, which are quite useful for sophisticated applications. Additionally, they support multiple programming languages. For example, if your client is written in C# (Unity) and your game server in C++, it is very convenient. All you need to do is write a configuration file and use the provided tools to generate code for different languages. The drawback is that it introduces a dependency on an external project, which increases the overall complexity of your project.

I don't want to manage my own serialization, and the benefits of supporting multiple languages and version updates with Protobuf are quite appealing, so I chose to use Protobuf in this project.
