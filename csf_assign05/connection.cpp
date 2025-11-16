#include <sstream>
#include <string>
#include <cctype>
#include <cassert>
#include <cstring>
#include "csapp.h"
#include "message.h"
#include "connection.h"

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  rio_readinitb(&m_fdbuf, m_fd); //initialize the read buffer
}

void Connection::connect(const std::string &hostname, int port) {
  //convert the integer to a string for open_clientfd
  std::string port_str = std::to_string(port);
  
  //connect to the server
  m_fd = open_clientfd(hostname.c_str(), port_str.c_str());
  
  //initialize the read buffer
  rio_readinitb(&m_fdbuf, m_fd);
}

Connection::~Connection() {
  close(); //close the connection
}

bool Connection::is_open() const {
  return m_fd >= 0; //if the file descriptor is greater than 0, the connection is open
}

void Connection::close() {
  if (is_open()) { //if the connection is valid
    ::close(m_fd); //close the connection
    m_fd = -1; //mark the file descriptor as closed
  }
}

bool Connection::send(const Message &msg) {
  if (!is_open()) { //if the connection is not open
    m_last_result = EOF_OR_ERROR; //throw an error
    return false;
  }

  //encode the message with the tag and the data
  std::string encoded = msg.tag + ":" + msg.data + "\n";
  
  //check if the message's length is valid
  if (encoded.length() > Message::MAX_LEN) {
    m_last_result = INVALID_MSG;
    return false;
  }

  //send the message
  ssize_t n = rio_writen(m_fd, encoded.c_str(), encoded.length());

  //check if the message was sent successfully
  if (n < 0 || (size_t)n != encoded.length()) { //if the full message was not sent, throw an error
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  m_last_result = SUCCESS; //the send succeeded
  return true;
}

bool Connection::receive(Message &msg) {
  if (!is_open()) { //if the connection is not open, throw an error
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  //create a buffer to store the message (+1 to inclue the newline character)
  char buf[Message::MAX_LEN + 1];
  //read the message from the connection
  ssize_t n = rio_readlineb(&m_fdbuf, buf, Message::MAX_LEN + 1);
  
  if (n <= 0) { //if the message was not read successfully, throw an error
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  //trim off newling characters
  std::string line(buf);
  //while the line is not empty and the last character is a newline or carriage return, remove the last character
  while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
    line.pop_back();
  }

  //find the colon in the message and split the message into the tag and the data
  size_t colon_pos = line.find(':');
  if (colon_pos == std::string::npos) { //if the colon is not found, throw an error
    m_last_result = INVALID_MSG;
    return false;
  }

  //everything before the colon is the tag
  msg.tag = line.substr(0, colon_pos);
  //everything after the colon is the data
  msg.data = line.substr(colon_pos + 1);

  m_last_result = SUCCESS; //the receive succeeded
  return true;
}
