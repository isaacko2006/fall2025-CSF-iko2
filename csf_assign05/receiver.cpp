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

  //connect to the server
  Connection conn;
  conn.connect(server_hostname, server_port);
  
  if (!conn.is_open()) { //if the connection is not open, throw an error
    std::cerr << "Error: failed to connect to server\n";
    return 1;
  }

  //send the rlogin message
  Message login_msg(TAG_RLOGIN, username);
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

  //send the join message
  Message join_msg(TAG_JOIN, room_name);
  if (!conn.send(join_msg)) { //if the join message was not sent successfully, throw an error
    std::cerr << "Error: failed to send join message\n";
    return 1;
  }

  //receive the response to the join
  if (!conn.receive(response)) { //if the join response was not received successfully, throw an error
    std::cerr << "Error: failed to receive join response\n";
    return 1;
  }

  //check if the join was successful
  if (response.tag == TAG_ERR) { //if the response is an error, throw an error
    std::cerr << response.data << "\n";
    return 1;
  }

  //loop waiting for messages from the server
  while (true) {
    Message msg;
    if (!conn.receive(msg)) { //if the message was not received successfully, exit the loop
      //connection closed or error occurred
      break;
    }

    //check if the message is a delivery message
    if (msg.tag == TAG_DELIVERY) {
      //parse the message
      std::string payload = msg.data;
      //find the first colon in the message
      size_t first_colon = payload.find(':');
      if (first_colon == std::string::npos) { //if the first colon is not found, skip message
        continue;
      }
      //find the second colon in the message
      size_t second_colon = payload.find(':', first_colon + 1);
      if (second_colon == std::string::npos) { //if the second colon is not found, skip this message
        continue;
      }

      //isolate the sender and message text by using the colons 
      std::string sender = payload.substr(first_colon + 1, second_colon - first_colon - 1);
      //isolate the message text by using the second colon
      std::string message_text = payload.substr(second_colon + 1);

      //display the message as "sender: message_text"
      std::cout << sender << ": " << message_text << "\n";
    } else if (msg.tag == TAG_ERR) {
      //if there is an error message, print the error message to stdcerr
      std::cerr << msg.data << "\n";
    }
  }

  return 0;
}
