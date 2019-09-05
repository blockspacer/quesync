#include "auth.h"

#include "client.h"

#include "../../../../shared/exception.h"
#include "../../../../shared/packets/login_packet.h"
#include "../../../../shared/packets/register_packet.h"
#include "../../../../shared/packets/session_auth_packet.h"

quesync::client::modules::auth::auth(std::shared_ptr<quesync::client::client> client)
    : module(client), _user(nullptr) {}

std::string quesync::client::modules::auth::login(std::string username, std::string password) {
    packets::login_packet login_packet(username, password);

    std::shared_ptr<response_packet> res_packet;

    // If the user is already authencticated
    if (_user) {
        throw exception(error::already_authenticated);
    }

    // Send the login packet to the server
    res_packet =
        _client->communicator()->send_and_verify(&login_packet, packet_type::authenticated_packet);

    // Init the user from the response
    _user = std::make_shared<user>(res_packet->json()["user"]);

    // Return the session id
    return res_packet->json()["sessionId"];
}

std::string quesync::client::modules::auth::signup(std::string username, std::string password,
                                          std::string email) {
    packets::register_packet register_packet(username, password, email, username);

    std::shared_ptr<response_packet> res_packet;

    // If the user is already authencticated
    if (_user) {
        throw exception(error::already_authenticated);
    }

    // Send the register packet to the server
    res_packet = _client->communicator()->send_and_verify(&register_packet,
                                                          packet_type::authenticated_packet);

    // Init the user from the response
    _user = std::make_shared<user>(res_packet->json()["user"]);

    // Return the session id
    return res_packet->json()["sessionId"];
}

void quesync::client::modules::auth::session_auth(std::string session_id) {
    packets::session_auth_packet session_auth_packet(session_id);

    std::shared_ptr<response_packet> res_packet;

    // If the user is already authencticated
    if (_user) {
        throw exception(error::already_authenticated);
    }

    // Send the session auth packet to the server
    res_packet = _client->communicator()->send_and_verify(&session_auth_packet,
                                                          packet_type::authenticated_packet);

    // Init the user from the response
    _user = std::make_shared<user>(res_packet->json()["user"]);
}

std::shared_ptr<quesync::user> quesync::client::modules::auth::get_user() { return _user; }

void quesync::client::modules::auth::clean() { _user = nullptr; }