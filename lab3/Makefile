INCLUDE_PATH=./include
TARGET_DIR=./bin

SERVER_SRC_DIR=./src
SERVER_SRC=server.cpp process.cpp
SERVER_OBJ_DIR=./obj
SERVER_OBJS=$(addprefix $(SERVER_OBJ_DIR)/, $(patsubst %.cpp, %.o, $(SERVER_SRC)))

CLIENT_SRC_DIR=./src
CLIENT_SRC=client.cpp
CLIENT_OBJ_DIR=./obj
CLIENT_OBJS=$(addprefix $(CLIENT_OBJ_DIR)/, $(patsubst %.cpp, %.o, $(CLIENT_SRC)))

server:MAKE_NEEDED_DIR MAKE_SERVER
client:MAKE_NEEDED_DIR MAKE_CLIENT

MAKE_NEEDED_DIR:
	mkdir -p obj bin

MAKE_SERVER: $(SERVER_OBJS)
	$(CXX) -g $(SERVER_OBJS) -std=c++11 -o $(TARGET_DIR)/server.bin

MAKE_CLIENT: $(CLIENT_OBJS)
	$(CXX) -g $(CLIENT_OBJS) -std=c++11 -o $(TARGET_DIR)/client.bin

$(SERVER_OBJ_DIR)/%.o: $(SERVER_SRC_DIR)/%.cpp
	$(CXX) -I$(INCLUDE_PATH) -std=c++11 -c $< -g -o $@

$(CLIENT_OBJ_DIR)/%.o: $(CLIENT_SRC_DIR)/%.cpp
	$(CXX) -I$(INCLUDE_PATH) -std=c++11 -c $< -g -o $@

.PHONY: clean
clean:
	rm -rf $(SERVER_OBJ_DIR) $(CLIENT_OBJ_DIR)