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

  //connect to the server
  Connection conn;
  conn.connect(server_hostname, server_port);
  
  if (!conn.is_open()) { //if the connection is not open, throw an error
    std::cerr << "Error: failed to connect to server\n";
    return 1;
  }

  //send the login message
  Message login_msg(TAG_SLOGIN, username); 
  if (!conn.send(login_msg)) { //if the login message was not sent successfully, throw an error
    std::cerr << "Error: failed to send login message\n";
    return 1;
  }

  //receive the response to the login
  Message response;
  if (!conn.receive(response)) { //if the login response was not received successfully, throw an error
    std::cerr << "Error: failed to receive login response\n";
    return 1;
  }

  //check if the login was successful
  if (response.tag == TAG_ERR) { //if the response is an error, throw an error
    std::cerr << response.data << "\n";
    return 1;
  }

  //this is the loop to read commands from the user
  std::string line;
  while (std::getline(std::cin, line)) { //read a line until EOF
    //trim whitespace from the line
    std::string trimmed = trim(line);
    
    //skip reading the empty lines
    if (trimmed.empty()) {
      continue;
    }

    //create a message to send to the server
    Message msg;
    //create a boolean to check if the user wants to quit
    bool should_quit = false;

    //parse the commands
    if (trimmed == "/quit") {
      msg = Message(TAG_QUIT, "");
      if (!conn.send(msg)) { //throw error if not sent successfully
        std::cerr << "Error: failed to send quit message\n";
        return 1;
      }
      should_quit = true;
    } else if (trimmed == "/leave") {
      //send the leave message
      msg = Message(TAG_LEAVE, "");
      if (!conn.send(msg)) { //throw error if not sent successfully
        std::cerr << "Error: failed to send leave message\n";
        return 1;
      }
    } else if (trimmed.substr(0, 6) == "/join ") { //if the line starts with "/join"
      //get the room name by trimming everything after /join
      std::string room_name = trim(trimmed.substr(6));
      if (room_name.empty()) { //if the room name is empty, throw an error
        std::cerr << "Error: room name cannot be empty\n";
        continue;
      }
      //create a join message with the name
      msg = Message(TAG_JOIN, room_name);
      if (!conn.send(msg)) { //throw error if not sent successfully
        std::cerr << "Error: failed to send join message\n";
        return 1;
      }
    } else {
      //otherwise, it is a regular messagse that is sent to all users in the room
      msg = Message(TAG_SENDALL, trimmed);
      if (!conn.send(msg)) { //throw error if not sent successfully
        std::cerr << "Error: failed to send message\n";
        return 1;
      }
    }

    //if the should_quit is true, exit the loop
    if (should_quit) {
      break;
    }

    //receive the response from the server
    if (!conn.receive(response)) {
      std::cerr << "Error: failed to receive server response\n";
      return 1;
    }

    //check if there was an error from the server
    if (response.tag == TAG_ERR) {
      std::cerr << response.data << "\n";
    }
  }

  return 0;
}
