#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string server_hostname = argv[1];
  int server_port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room_name = argv[4];

  //connect to server
  Connection conn;
  conn.connect(server_hostname, server_port);
  
  //if connection not open, throw error
  if (!conn.is_open()) { 
    std::cerr << "Error: failed to connect to server\n";
    return 1;
  }

  //send rlogin message
  Message login_msg(TAG_RLOGIN, username);
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

  //check if login was successful, but if response is error then throw error
  if (response.tag == TAG_ERR) {
    std::cerr << response.data << "\n";
    return 1;
  }

  //send join message, but if can't send successfully throw error
  Message join_msg(TAG_JOIN, room_name);
  if (!conn.send(join_msg)) {
    std::cerr << "Error: failed to send join message\n";
    return 1;
  }

  //receive response to the join, but if didn't receive correctly throw error
  if (!conn.receive(response)) {
    std::cerr << "Error: failed to receive join response\n";
    return 1;
  }

  //check if join successful, but if response error then throw error
  if (response.tag == TAG_ERR) {
    std::cerr << response.data << "\n";
    return 1;
  }

  //loop waiting for messages from server
  while (true) {
    Message msg;
    //if message not received successfully, exit loop
    if (!conn.receive(msg)) { 
      //connection closed or error occurred
      break;
    }

    //check if message is delivery message
    if (msg.tag == TAG_DELIVERY) {
      //parse message
      std::string payload = msg.data;
      //find first colon in message
      size_t first_colon = payload.find(':');
      //if first colon not found, skip message
      if (first_colon == std::string::npos) { 
        continue;
      }
      //find second colon in message
      size_t second_colon = payload.find(':', first_colon + 1);
      //if second colon not found, skip message
      if (second_colon == std::string::npos) {
        continue;
      }

      //isolate sender and message text by using colons 
      std::string sender = payload.substr(first_colon + 1, second_colon - first_colon - 1);
      //isolate message text by using second colon
      std::string message_text = payload.substr(second_colon + 1);

      //display message as "sender: message_text"
      std::cout << sender << ": " << message_text << "\n";
    } else if (msg.tag == TAG_ERR) {
      //if error message, print to std:cerr
      std::cerr << msg.data << "\n";
    }
  }

  return 0;
}
