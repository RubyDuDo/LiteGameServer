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
It supports **macOS** and **linux** .

- Run `./build_xcode.sh` to generate an Xcode project.  
  Make sure to change the working directory to `./GameServer/bin` before running the server — the required config files are located there.

- Run `./build_mac.sh` to build the project directly and copy the executable to `./GameServer/bin`.  
  ⚠️ When launching the server by double-clicking, the working directory will be your home folder, and the config file will fail to load. So run it from the command line instead.

- Run `./build_linux.sh` to build the project directly and copy the executable to `./GameServer/bin`.  


## Dependencies
This project currently relies on three external libraries:

- `protobuf` (for serialization)
- `mysql_connection` (for database access)
- `spdlog` (for log system)

mysql_connection are included in `./GameServer/thirdlibs`. Protobuf and Spdlog are managed using `fetch content` function in CMake, so it will be download during configuring. So if you download this project, it may need network and git to build the first time. 


## Running environment
This project connect to local database, so you need create the database before running.

There are two sql scripts to setup the database environments, which are both under `./GameServer/script` directory.

- `db.sql` will create the database MyGame, and add two table to it. 
- `createrole.sql` will add one account to db, name: `ruby`, password:`111`.

However, you need to create the user and grant the authority to the user you create. And then, you need to change the `[DB]` configure in `config.ini`, which is under `./GameServer/bin`
