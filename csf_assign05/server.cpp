#include <pthread.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <set>
#include <vector>
#include <cctype>
#include <cassert>
#include "message.h"
#include "connection.h"
#include "user.h"
#include "room.h"
#include "guard.h"
#include "server.h"

////////////////////////////////////////////////////////////////////////
// Server implementation data types
////////////////////////////////////////////////////////////////////////

// TODO: add any additional data types that might be helpful
//       for implementing the Server member functions
struct ConnData
{
  Server *server;
  Connection *conn;
};

////////////////////////////////////////////////////////////////////////
// Client thread functions
////////////////////////////////////////////////////////////////////////

namespace
{

  // Helper function to validate username/room name (alphanumeric only, at least 1 char)
  bool is_valid_name(const std::string &name)
  {
    if (name.empty())
      return false;
    for (char c : name)
    {
      if (!std::isalnum(c))
        return false;
    }
    return true;
  }

  // function to handle chatting with receiver
  void chat_with_receiver(User *user, Connection *conn, Room *room)
  {
    while (true)
    {
      Message *msg = user->mqueue.dequeue();
      // wait for message
      if (!msg)
        continue;
      Message delivery(TAG_DELIVERY, msg->data);
      //break out on delivery failure
      if (!conn->send(delivery))
      {
        delete msg;
        break;
      }
      delete msg;
    }
    // Remove user from room when disconnecting
    if (room)
    {
      room->remove_member(user);
    }
  }

  // function to handle chatting with sender
  void chat_with_sender(User *sender, Server *server, Connection *conn, Room *&current_room)
  {
    while (true)
    {
      Message msg;
      if (!conn->receive(msg))
      {
        // Check if it's an invalid message (should send err) vs connection error (disconnect)
        if (conn->get_last_result() == Connection::INVALID_MSG)
        {
          Message err(TAG_ERR, "Invalid message format");
          if (!conn->send(err))
            break;
          continue;
        }
        // Otherwise, connection error - disconnect
        break;
      }

      // Check for empty tag (invalid message)
      if (msg.tag.empty())
      {
        Message err(TAG_ERR, "Invalid message format");
        if (!conn->send(err))
          break;
        continue;
      }

      if (msg.tag == TAG_SENDALL)
      {
        if (!current_room)
        {
          Message err(TAG_ERR, "Not in a room");
          if (!conn->send(err))
            break;
          continue;
        }
        current_room->broadcast_message(sender->username, msg.data);
        Message ok(TAG_OK, "Message sent");
        if (!conn->send(ok))
          break;
      }
      else if (msg.tag == TAG_JOIN)
      {
        if (!is_valid_name(msg.data))
        {
          Message err(TAG_ERR, "Invalid room name");
          if (!conn->send(err))
            break;
          continue;
        }
        // Leave current room if in one
        if (current_room)
        {
          // Senders don't need to be removed from room (only receivers are members)
        }
        // Join new room
        current_room = server->find_or_create_room(msg.data);
        Message ok(TAG_OK, "Joined room");
        if (!conn->send(ok))
          break;
      }
      else if (msg.tag == TAG_LEAVE)
      {
        if (!current_room)
        {
          Message err(TAG_ERR, "Not in a room");
          if (!conn->send(err))
            break;
          continue;
        }
        current_room = nullptr;
        Message ok(TAG_OK, "Left room");
        if (!conn->send(ok))
          break;
      }
      else if (msg.tag == TAG_QUIT)
      {
        Message ok(TAG_OK, "Goodbye");
        conn->send(ok);
        break;
      }
      else
      {
        Message err(TAG_ERR, "Unknown command");
        if (!conn->send(err))
          break;
      }
    }
  }

  void *worker(void *arg)
  {
    pthread_detach(pthread_self());

    // TODO: use a static cast to convert arg from a void* to
    //       whatever pointer type describes the object(s) needed
    //       to communicate with a client (sender or receiver)
    ConnData *data = static_cast<ConnData *>(arg);
    Server *server = data->server;
    Connection *conn = data->conn;
    delete data;

    // TODO: read login message (should be tagged either with
    //       TAG_SLOGIN or TAG_RLOGIN), send response
    Message login;
    if (!conn->receive(login))
    {
      // If invalid message format, send error before disconnecting
      if (conn->get_last_result() == Connection::INVALID_MSG)
      {
        Message err(TAG_ERR, "Invalid message format");
        conn->send(err);
      }
      delete conn;
      return nullptr;
    }

    // Validate login tag
    if (login.tag != TAG_SLOGIN && login.tag != TAG_RLOGIN)
    {
      Message err(TAG_ERR, "Invalid login tag");
      conn->send(err);
      delete conn;
      return nullptr;
    }

    // Validate username
    if (!is_valid_name(login.data))
    {
      Message err(TAG_ERR, "Invalid username");
      conn->send(err);
      delete conn;
      return nullptr;
    }

    // TODO: depending on whether the client logged in as a sender or
    //       receiver, communicate with the client (implementing
    //       separate helper functions for each of these possibilities
    //       is a good idea)
    if (login.tag == TAG_RLOGIN)
    {
      User *user = new User(login.data);
      Message ok(TAG_OK, "Logged in as receiver");
      if (!conn->send(ok))
      {
        delete user;
        delete conn;
        return nullptr;
      }

      // Wait for join message from receiver
      Message join_msg;
      if (!conn->receive(join_msg))
      {
        // If invalid message format, send error before disconnecting
        if (conn->get_last_result() == Connection::INVALID_MSG)
        {
          Message err(TAG_ERR, "Invalid message format");
          conn->send(err);
        }
        delete user;
        delete conn;
        return nullptr;
      }

      if (join_msg.tag != TAG_JOIN)
      {
        Message err(TAG_ERR, "Expected join message");
        conn->send(err);
        delete user;
        delete conn;
        return nullptr;
      }

      // Validate room name
      if (!is_valid_name(join_msg.data))
      {
        Message err(TAG_ERR, "Invalid room name");
        conn->send(err);
        delete user;
        delete conn;
        return nullptr;
      }

      // Join the room
      Room *room = server->find_or_create_room(join_msg.data);
      room->add_member(user);
      Message join_ok(TAG_OK, "Joined room");
      if (!conn->send(join_ok))
      {
        room->remove_member(user);
        delete user;
        delete conn;
        return nullptr;
      }

      chat_with_receiver(user, conn, room);
      delete user;
      delete conn;
    }
    else if (login.tag == TAG_SLOGIN)
    {
      User *sender = new User(login.data);
      Message ok(TAG_OK, "Logged in as sender");
      if (!conn->send(ok))
      {
        delete sender;
        delete conn;
        return nullptr;
      }

      // Sender starts with no room
      Room *current_room = nullptr;
      chat_with_sender(sender, server, conn, current_room);
      delete sender;
      delete conn;
    }
    else
    {
      Message err(TAG_ERR, "Invalid login tag");
      conn->send(err);
      delete conn;
    }

    return nullptr;
  }

}

////////////////////////////////////////////////////////////////////////
// Server member function implementation
////////////////////////////////////////////////////////////////////////

Server::Server(int port)
    : m_port(port), m_ssock(-1)
{
  // TODO: initialize mutex
  pthread_mutex_init(&m_lock, nullptr);
}

Server::~Server()
{
  // TODO: destroy mutex
  pthread_mutex_destroy(&m_lock);
  for (auto &pair : m_rooms)
    delete pair.second;
}

bool Server::listen()
{
  // TODO: use open_listenfd to create the server socket, return true
  //       if successful, false if not
  std::string port_str = std::to_string(m_port);
  m_ssock = open_listenfd(port_str.c_str());
  if (m_ssock < 0)
    return false;
  return true;
}

void Server::handle_client_requests()
{
  // TODO: infinite loop calling accept or Accept, starting a new
  //       pthread for each connected client
  while (true)
  {
    sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int csock = accept(m_ssock, (sockaddr *)&cli_addr, &cli_len);
    if (csock < 0)
      continue;

    Connection *conn = new Connection(csock);
    ConnData *data = new ConnData{this, conn};

    pthread_t tid;
    pthread_create(&tid, nullptr, worker, data);
  }
}

Room *Server::find_or_create_room(const std::string &room_name)
{
  // TODO: return a pointer to the unique Room object representing
  //       the named chat room, creating a new one if necessary
  Guard g(m_lock);
  auto it = m_rooms.find(room_name);
  if (it != m_rooms.end())
    return it->second;

  Room *room = new Room(room_name);
  m_rooms[room_name] = room;
  return room;
}
