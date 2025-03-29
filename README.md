# GameServer
## introduction
This is a simple Game Server Project, which I plan to finish in two periods. 
- **Phase 1**: A standalone game server with basic modules such as networking, database, and player management.
- **Phase 2**: Add advanced features to improve scalability and performance.

Through developping this project, I aim to review and enhance my back-end developping skills. I also hope this project could help other people in need.

The client for this game server is being developed in parallel:  
🔗 [GameClient Repository](https://github.com/RubyDuDo/GameClient)  
The server and client are built to ensure seamless communication between them.

If you're interested in **which features are already implemented** and **what’s planned**, see:  
📄 [`GameServer/doc/progress.md`](GameServer/doc/progress.md)

If you want to learn more about the **system design** or the **issues encountered during development**, check out:  
📄 [`GameServer/doc/system_design.md`](GameServer/doc/system_design.md)


## build
This project is managed using **CMake**.  
> ⚠️ Currently, it only supports **macOS**.

- Run `./build_xcode.sh` to generate an Xcode project.  
  Make sure to change the working directory to `./GameServer/bin` before running the server — the required config files are located there.

- Run `./build_mac.sh` to build the project directly and copy the executable to `./GameServer/bin`.  
  ⚠️ When launching the server by double-clicking, the working directory will be your home folder, and the config file will fail to load. So run it from the command line instead.



## Dependencies
This project currently relies on two external libraries:

- `protobuf` (for serialization)
- `mysql_connection` (for database access)

Both libraries are included in `./GameServer/thirdlibs`.