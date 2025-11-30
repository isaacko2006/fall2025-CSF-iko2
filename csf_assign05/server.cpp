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

  // function to handle chatting with receiver
  void chat_with_receiver(User *user, Connection *conn)
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
  }

  // function to handle chatting with sender
  void chat_with_sender(User *sender, Server *server, Connection *conn, Room *room)
  {
    while (true)
    {
      Message msg;
      if (!conn->receive(msg))
        break;

      if (msg.tag == TAG_SENDALL)
      {
        room->broadcast_message(sender->username, msg.data);
        Message ok(TAG_OK, "Message sent");
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

      chat_with_receiver(user, conn);
      delete user;
      delete conn;
    }
    else if (login.tag == TAG_SLOGIN)
    {
      User sender(login.data);
      Message ok(TAG_OK, "Logged in as sender");
      if (!conn->send(ok))
      {
        delete conn;
        return nullptr;
      }

      Room *room = server->find_or_create_room("main");
      chat_with_sender(&sender, server, conn, room);
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
  m_ssock = socket(AF_INET, SOCK_STREAM, 0);
  if (m_ssock < 0)
    return false;

  sockaddr_in serv_addr{};
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(m_port);

  int opt = 1;
  setsockopt(m_ssock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  if (::bind(m_ssock, (sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    return false;
  if (::listen(m_ssock, 10) < 0)
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
