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
  rio_readinitb(&m_fdbuf, m_fd); //initialize read buffer
}

void Connection::connect(const std::string &hostname, int port) {
  //convert integer to string for open_clientfd
  std::string port_str = std::to_string(port);
  
  //connect to server
  m_fd = open_clientfd(hostname.c_str(), port_str.c_str());
  
  //initialize read buffer
  rio_readinitb(&m_fdbuf, m_fd);
}

Connection::~Connection() {
  close(); //close connection
}

bool Connection::is_open() const {
  return m_fd >= 0; //if file descriptor greater than 0, connection open
}

void Connection::close() {
  //if connection vlaid, close connection and mark file descriptor closed
  if (is_open()) {
    ::close(m_fd);
    m_fd = -1;
  }
}

bool Connection::send(const Message &msg) {
  //if connection not open, throw error
  if (!is_open()) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  //encode message with tag and data
  std::string encoded = msg.tag + ":" + msg.data + "\n";
  
  //check if message length valid
  if (encoded.length() > Message::MAX_LEN) {
    m_last_result = INVALID_MSG;
    return false;
  }

  //send message
  ssize_t n = rio_writen(m_fd, encoded.c_str(), encoded.length());

  //check if message sent successfully, if not throw error
  if (n < 0 || (size_t)n != encoded.length()) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  //successful send
  m_last_result = SUCCESS;
  return true;
}

//if connectio not open, throw error
bool Connection::receive(Message &msg) {
  if (!is_open()) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  //create buffer to store message (+1 to includ newline character)
  char buf[Message::MAX_LEN + 1];
  //read message from connection
  ssize_t n = rio_readlineb(&m_fdbuf, buf, Message::MAX_LEN + 1);
  
  //if message not read successfully, throw error
  if (n <= 0) { 
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  //trim off newline chars
  std::string line(buf);
  //while line not empty and last character is newline or carriage return, remove last char
  while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) {
    line.pop_back();
  }

  // Check if message is empty (invalid)
  if (line.empty()) {
    m_last_result = INVALID_MSG;
    return false;
  }

  // Check if message is too long (including newline that was removed)
  // The original message with newline should be <= MAX_LEN
  if (n > (ssize_t)Message::MAX_LEN) {
    m_last_result = INVALID_MSG;
    return false;
  }

  //find colon in message and split message into tag and data
  size_t colon_pos = line.find(':');
  //if no colon, throw error
  if (colon_pos == std::string::npos) {
    m_last_result = INVALID_MSG;
    return false;
  }

  //everything before colon is tag
  msg.tag = line.substr(0, colon_pos);
  //everything after colon is data
  msg.data = line.substr(colon_pos + 1);

  //successful receival
  m_last_result = SUCCESS;
  return true;
}
