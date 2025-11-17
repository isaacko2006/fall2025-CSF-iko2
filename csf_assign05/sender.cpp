#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  std::string server_hostname;
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

  //connect to server
  Connection conn;
  conn.connect(server_hostname, server_port);
  
  //if connection not open, throw error
  if (!conn.is_open()) {
    std::cerr << "Error: failed to connect to server\n";
    return 1;
  }

  //send login message
  Message login_msg(TAG_SLOGIN, username); 
  //if login message not sent successfully, throw error
  if (!conn.send(login_msg)) {
    std::cerr << "Error: failed to send login message\n";
    return 1;
  }

  //receive response to login
  Message response;
  //if login response not received successfully, throw error
  if (!conn.receive(response)) {
    std::cerr << "Error: failed to receive login response\n";
    return 1;
  }

  //check if login successful, but if response error then throw error
  if (response.tag == TAG_ERR) {
    std::cerr << response.data << "\n";
    return 1;
  }
  //if response is not ok, something unexpected happened
  if (response.tag != TAG_OK) {
    std::cerr << "Error: unexpected response to login\n";
    return 1;
  }

  //loop to read commands from user
  std::string line;
  //read line until EOF
  while (std::getline(std::cin, line)) {
    //trim whitespace from line
    std::string trimmed = trim(line);
    
    //skip reading empty lines
    if (trimmed.empty()) {
      continue;
    }

    //create message to send to server
    Message msg;
    //create boolean to check if user wants to quit
    bool should_quit = false;

    //parse commands
    if (trimmed == "/quit") {
      msg = Message(TAG_QUIT, "");
      //throw error if not sent successfully
      if (!conn.send(msg)) {
        std::cerr << "Error: failed to send quit message\n";
        return 1;
      }
      should_quit = true;
    } else if (trimmed == "/leave") {
      //send leave message
      msg = Message(TAG_LEAVE, "");
      //throw error if not sent successfully
      if (!conn.send(msg)) {
        std::cerr << "Error: failed to send leave message\n";
        return 1;
      }
      //if the line starts with "/join"
    } else if (trimmed.substr(0, 6) == "/join ") {
      //get room name by trimming everything after /join
      std::string room_name = trim(trimmed.substr(6));
      //if room name empty, throw error
      if (room_name.empty()) {
        std::cerr << "Error: room name cannot be empty\n";
        continue;
      }
      //create join message with name
      msg = Message(TAG_JOIN, room_name);
      //throw error if not sent successfully
      if (!conn.send(msg)) {
        std::cerr << "Error: failed to send join message\n";
        return 1;
      }
    } else {
      //otherwise, regular messagse that sent to all users in room
      msg = Message(TAG_SENDALL, trimmed);
      //throw error if not sent successfully
      if (!conn.send(msg)) {
        std::cerr << "Error: failed to send message\n";
        return 1;
      }
    }

    //receive response from server
    if (!conn.receive(response)) {
      //if we're quitting, server may close connection immediately, which is acceptable
      if (should_quit) {
        break;
      }
      //otherwise, this is an error
      std::cerr << "Error: failed to receive server response\n";
      return 1;
    }

    //check if error from the server
    if (response.tag == TAG_ERR) {
      std::cerr << response.data << "\n";
    } else if (response.tag != TAG_OK) {
      //if response is not ok or err, something unexpected happened
      std::cerr << "Error: unexpected response from server\n";
    }
    //if response is ok, silently continue (success)

    //if should_quit is true, exit loop after receiving response
    if (should_quit) {
      break;
    }
  }

  return 0;
}
