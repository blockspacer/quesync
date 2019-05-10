#include <iostream>

#include "session.h"
#include "../shared/utils.h"
#include "../shared/packets/error_packet.h"

Session::Session(tcp::socket socket, Quesync *server) : _socket(std::move(socket)), // Copy the client's socket
                                                        _server(server),            // Save the server for data transfer,
                                                        _user(nullptr)
{
}

Session::~Session()
{
    // If the user authenticated
    if (_user)
    {
        // Unauthenticate session and free the user object
        _server->userManagement().unauthenticateSession(_user->id());
        delete _user;
    }

    // Close the client's socket in case it's not closed
    try
    {
        _socket.close();
    }
    catch (...)
    {
    }
}

void Session::start()
{
    // Start receiving from the client
    recv();
}

void Session::recv()
{
    auto self(shared_from_this());

    // Get a request from the user
    _socket.async_read_some(asio::buffer(_data, MAX_DATA_LEN),
                            [this, self](std::error_code ec, std::size_t length) {
                                Packet *packet;
                                std::string response;

                                // If no error occurred, parse the request
                                if (!ec)
                                {
                                    // Parse the packet
                                    packet = Utils::ParsePacket(_data);

                                    // If the packet has parsed successfully handle it
                                    if (packet)
                                    {
                                        // Handle the client's request and get a respond
                                        response = packet->handle(this);

                                        delete packet;
                                    }
                                    else
                                    {
                                        // Return an invalid packet error packet
                                        response = ErrorPacket(INVALID_PACKET).encode();
                                    }

                                    // Send the server's response to the server
                                    send(response);
                                }
                                else
                                {
                                    // If the client closed the connection, close the client
                                    if (ec == asio::error::misc_errors::eof)
                                    {
                                        std::cout << "The client " << _socket.remote_endpoint().address().to_string() << ":" << (int)_socket.remote_endpoint().port() << " disconnected!" << std::endl;
                                    }
                                    else
                                    {
                                        // Print error
                                        std::cout << "An error occurred: " << ec << std::endl;
                                    }
                                }
                            });
}

void Session::send(std::string data)
{
    auto self(shared_from_this());

    // Create a buffer with the size of the data and copy the data to it
    std::shared_ptr<char> buf(new char[data.length() + 1]);
    Utils::CopyString(data, buf.get());

    // Send the data to the client
    asio::async_write(_socket, asio::buffer(buf.get(), data.length() + 1),
                      [this, self, buf](std::error_code ec, std::size_t) {
                          // If no error occurred, return to the receiving function
                          if (!ec)
                          {
                              recv();
                          }
                          else
                          {
                              // If the client closed the connection, close the client
                              if (ec == asio::error::misc_errors::eof)
                              {
                                  std::cout << "The client " << _socket.remote_endpoint().address().to_string() << ":" << (int)_socket.remote_endpoint().port() << " disconnected!" << std::endl;
                              }
                              else
                              {
                                  // Print error
                                  std::cout << "An error occurred: " << ec << std::endl;
                              }
                          }
                      });
}

void Session::sendOnly(std::string data)
{
    auto self(shared_from_this());

    // Create a buffer with the size of the data and copy the data to it
    std::shared_ptr<char> buf(new char[data.length() + 1]);
    Utils::CopyString(data, buf.get());

    // Send the data to the client
    asio::async_write(_socket, asio::buffer(buf.get(), data.length() + 1),
                      [this, self, buf](std::error_code ec, std::size_t) {
                          // If an error occurred during sending the data, print the error(only if not a disconnect error)
                          if (ec && ec != asio::error::misc_errors::eof)
                          {
                              // Print error
                              std::cout << "An error occurred: " << ec << std::endl;
                          }
                      });
}

Quesync *Session::server() const
{
    return _server;
}

std::shared_ptr<Session> Session::getShared()
{
    return shared_from_this();
}

void Session::setUser(User *user)
{
    _user = user;
}

bool Session::authenticated() const
{
    return (_user != nullptr);
}

User *Session::user() const
{
    return _user;
}