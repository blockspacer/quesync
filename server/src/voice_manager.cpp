#include "voice_manager.h"

#include <algorithm>
#include <sole.hpp>

#include "server.h"
#include "session.h"

#include "../../shared/events/call_ended_event.h"
#include "../../shared/events/voice_state_event.h"
#include "../../shared/exception.h"
#include "../../shared/packets/voice_otp_packet.h"
#include "../../shared/packets/voice_packet.h"
#include "../../shared/packets/voice_participant_packet.h"
#include "../../shared/utils/encryption.h"
#include "../../shared/utils/memory.h"
#include "../../shared/utils/rand.h"

quesync::server::voice_manager::voice_manager(std::shared_ptr<quesync::server::server> server)
    : manager(server),
      _socket(server->get_io_context(), udp::endpoint(udp::v4(), VOICE_SERVER_PORT)),
      _voice_states_thread(&voice_manager::handle_voice_states, this) {
    // Detach voice states thread
    _voice_states_thread.detach();

    // Start receiving
    recv();
}

void quesync::server::voice_manager::recv() {
    _socket.async_receive_from(asio::buffer(_buf, MAX_DATA_LEN), _sender_endpoint,
                               [this](std::error_code ec, std::size_t bytes) {
                                   if (!ec && bytes > 0) {
                                       handle_packet(bytes);
                                   }

                                   recv();
                               });
}

void quesync::server::voice_manager::send(std::shared_ptr<char> buf, size_t length,
                                          udp::endpoint endpoint) {
    _socket.async_send_to(asio::buffer(buf.get(), length), endpoint,
                          [this, buf](std::error_code, std::size_t) {});
}

void quesync::server::voice_manager::handle_packet(std::size_t length) {
    packets::voice_otp_packet otp_packet;

    voice::encryption_info encryption_info;
    std::string user_session;
    std::shared_ptr<packets::voice_packet> packet;

    voice::encryption_info participant_encryption_info;
    std::string participant_session;
    udp::endpoint participant_endpoint;

    packets::voice_participant_packet participant_packet;
    std::string participant_packet_encrypted;
    std::shared_ptr<char> buf;

    std::unordered_map<std::string, voice::state> voice_states;

    std::lock_guard lk(_mutex);

    // If the packet is an OTP packet
    if (otp_packet.decode(std::string(_buf, length))) {
        try {
            // Get user's session
            user_session = _otps[otp_packet.otp()];

            // Save endpoint
            _session_endpoints[user_session] = _sender_endpoint;

            // Remove OTP
            _otps.erase(otp_packet.otp());
        } catch (...) {
        }

        return;
    }

    // Try to find the participant session by his endpoint and get it's
    // encryption info
    try {
        user_session = std::find_if(_session_endpoints.begin(), _session_endpoints.end(),
                                    [this](const std::pair<std::string, udp::endpoint>& p) {
                                        return p.second == _sender_endpoint;
                                    })
                           ->first;

        encryption_info = _session_keys.at(user_session);
    } catch (...) {
        return;
    }

    // Try to decrypt the voice packet
    packet = utils::encryption::decrypt_voice_packet<packets::voice_packet>(
        std::string(_buf, length), encryption_info.aes_key.get(), encryption_info.hmac_key.get());
    if (!packet) {
        return;
    }

    // Try to find the channel participants
    try {
        voice_states = _voice_channels.at(packet->channel_id())->voice_states;
    } catch (...) {
        return;
    }

    // If the user isn't a part of the channel
    if (!voice_states.count(packet->user_id())) {
        return;
    }

    if (packet->voice_data_len()) {
        // Create the pariticipant voice packet and encode it
        participant_packet = packets::voice_participant_packet(
            packet->user_id(), packet->voice_data(), packet->voice_data_len());

        // Send the participant voice packet to all other participants
        for (auto& participant : voice_states) {
            // If the given participant isn't our user
            if (participant.first != packet->user_id()) {
                try {
                    // Try to get the session, key and endpoint of the
                    // participant
                    participant_session = _sessions.at(participant.first);
                    participant_endpoint = _session_endpoints.at(participant_session);
                    participant_encryption_info = _session_keys.at(participant_session);
                } catch (...) {
                    continue;
                }

                // Encrypt the packet and convert it to buffer
                participant_packet_encrypted =
                    utils::encryption::encrypt_voice_packet<packets::voice_participant_packet>(
                        &participant_packet, participant_encryption_info.aes_key.get(),
                        participant_encryption_info.hmac_key.get());
                buf = utils::memory::convert_to_buffer<char>(participant_packet_encrypted);

                // Send the participant voice packet to the participant
                send(buf, participant_packet_encrypted.length(), participant_endpoint);
            }
        }
    }
}

std::pair<std::string, quesync::voice::encryption_info>
quesync::server::voice_manager::create_voice_session(std::string user_id) {
    std::lock_guard lk(_mutex);

    // If a session doesn't exists for the user
    if (!_sessions.count(user_id)) {
        _sessions[user_id] = sole::uuid4().str();
    }

    // Create the aes key and hmac key for the session
    _session_keys[_sessions[user_id]] =
        voice::encryption_info{utils::rand::bytes(AES_KEY_SIZE), utils::rand::bytes(HMAC_KEY_SIZE)};

    return *_session_keys.find(_sessions[user_id]);
}

void quesync::server::voice_manager::delete_voice_session(std::string user_id) {
    std::lock_guard lk(_mutex);

    try {
        // Remove OTP if exists
        _otps.erase(_sessions[user_id]);
    } catch (...) {
    }

    try {
        _session_endpoints.erase(_sessions[user_id]);
        _session_keys.erase(_sessions[user_id]);
        _sessions.erase(user_id);
    } catch (...) {
        throw exception(error::voice_not_connected);
    }
}

std::string quesync::server::voice_manager::generate_otp(std::string session_id) {
    std::string otp;

    std::lock_guard lk(_mutex);

    // Generate random bytes for the OTP
    std::shared_ptr<unsigned char> bytes = utils::rand::bytes(OTP_SIZE);

    // Save the OTP
    otp = std::string((char*)bytes.get(), OTP_SIZE);
    _otps[otp] = session_id;

    return otp;
}

std::shared_ptr<quesync::call_details> quesync::server::voice_manager::init_voice_channel(
    std::string caller_id, std::string channel_id, std::vector<std::string> users) {
    std::unordered_map<std::string, voice::state> user_states;

    std::lock_guard lk(_mutex);

    // If the voice channel already started
    if (_voice_channels.count(channel_id)) {
        throw exception(error::call_already_started);
    }

    // Create user states for all the users as PENDING
    for (auto& user : users) {
        user_states[user] = voice::state(voice::state_type::pending, false, false);
    }
    _voice_channels[channel_id] =
        std::make_shared<call_details>(create_call(caller_id, channel_id), user_states);

    // Duplicate the call details and return it
    return std::make_shared<call_details>(*_voice_channels[channel_id]);
}

bool quesync::server::voice_manager::is_voice_channel_active(std::string channel_id) {
    std::lock_guard lk(_mutex);

    return _voice_channels.count(channel_id);
}

void quesync::server::voice_manager::join_voice_channel(std::string user_id, std::string channel_id,
                                                        bool muted, bool deafen) {
    std::lock_guard lk(_mutex);

    // If the user is already in a channel, leave the channel
    if (_joined_voice_channels.count(user_id)) {
        leave_voice_channel(user_id);
    }

    // Check if the channel exists and check for active call
    if (!_voice_channels.count(channel_id)) {
        throw exception(error::channel_not_found);
    }

    // Add the participant to the call
    add_participant_to_call(channel_id, user_id);

    // Connect the user to the channel
    _joined_voice_channels[user_id] = channel_id;
    _voice_channels[channel_id]->voice_states[user_id] =
        voice::state(voice::state_type::connected, muted, deafen);
    trigger_voice_state_event(channel_id, user_id,
                              _voice_channels[channel_id]->voice_states[user_id]);
}

void quesync::server::voice_manager::leave_voice_channel(std::string user_id) {
    std::string channel_id;

    std::shared_ptr<events::call_ended_event> call_ended_event;

    std::lock_guard lk(_mutex);

    // Check if in voice channel
    if (!_joined_voice_channels.count(user_id)) {
        throw exception(error::voice_not_connected);
    }

    // Get the channel id of the user
    channel_id = _joined_voice_channels[user_id];

    // Remove the user from the map of joined voice channels
    _joined_voice_channels.erase(user_id);
    _voice_channels[channel_id]->voice_states[user_id] = voice::state_type::disconnected;
    trigger_voice_state_event(channel_id, user_id,
                              _voice_channels[channel_id]->voice_states[user_id]);

    // Check for others connected to the voice channel
    for (auto& join_pair : _joined_voice_channels) {
        if (join_pair.second == channel_id) {
            return;
        }
    }

    // Send call ended event for all participants of the call
    call_ended_event = std::make_shared<events::call_ended_event>(channel_id);
    for (auto& user : _voice_channels[channel_id]->voice_states) {
        if (user.first != user_id)
            _server->event_manager()->trigger_event(
                std::static_pointer_cast<quesync::event>(call_ended_event), user.first);
    }

    // Close the call in the channel
    close_call(channel_id);

    // If the channel has no one connected to it, remove it
    _voice_channels.erase(channel_id);
}

std::unordered_map<std::string, quesync::voice::state>
quesync::server::voice_manager::get_voice_states(std::string channel_id) {
    std::lock_guard lk(_mutex);

    // Check if the channel exists
    if (!_voice_channels.count(channel_id)) {
        throw exception(error::channel_not_found);
    }

    return _voice_channels[channel_id]->voice_states;
}

void quesync::server::voice_manager::handle_voice_states() {
    while (true) {
        // Sleep for half a second
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        std::lock_guard lk(_mutex);

        std::time_t current_time = std::time(nullptr);

        // For each channel
        for (auto& channel : _voice_channels) {
            // For each user
            for (auto& user : channel.second->voice_states) {
                // If pending time exceeded, change the user's state
                if (user.second == voice::state_type::pending &&
                    current_time - user.second.change_time() > MAX_PENDING_SECONDS) {
                    _voice_channels[channel.first]->voice_states[user.first] =
                        voice::state_type::disconnected;
                    trigger_voice_state_event(
                        channel.first, user.first,
                        _voice_channels[channel.first]->voice_states[user.first]);
                }
            }
        }
    }
}

void quesync::server::voice_manager::set_voice_state(std::string user_id, bool muted, bool deafen) {
    std::string channel_id;

    std::lock_guard lk(_mutex);

    // Check if in voice channel
    if (!_joined_voice_channels.count(user_id)) {
        throw exception(error::voice_not_connected);
    }

    // Get the channel id of the user
    channel_id = _joined_voice_channels[user_id];

    if (muted) {
        _voice_channels[channel_id]->voice_states[user_id].mute();
    } else {
        _voice_channels[channel_id]->voice_states[user_id].unmute();
    }

    if (deafen) {
        _voice_channels[channel_id]->voice_states[user_id].deaf();
    } else {
        _voice_channels[channel_id]->voice_states[user_id].undeaf();
    }

    // Send event to all participants
    trigger_voice_state_event(channel_id, user_id,
                              _voice_channels[channel_id]->voice_states[user_id]);
}

std::vector<quesync::call> quesync::server::voice_manager::get_channel_calls(
    std::shared_ptr<quesync::server::session> sess, std::string channel_id, int amount,
    int offset) {
    std::vector<call> calls;

    sql::Session sql_sess = _server->get_sql_session();
    sql::Table calls_table(_server->get_sql_schema(sql_sess), "calls");
    sql::RowResult res;
    sql::Row row;

    // Check if the session is authenticated
    if (!sess->authenticated()) {
        throw exception(error::not_authenticated);
    }

    // Check if the channel exists
    if (!_server->channel_manager()->does_channel_exists(channel_id)) {
        throw exception(error::channel_not_found);
    }

    // Check if the user is a member of the wanted channel
    if (!_server->channel_manager()->is_user_member_of_channel(sess->user()->id, channel_id)) {
        throw exception(error::not_member_of_channel);
    }

    // Check if amount surpasses the limit
    if (amount > MAX_CALLS_AMOUNT) {
        throw exception(error::amount_exceeded_max);
    }

    try {
        // Try to get the calls from the table using the amount and offset
        res = calls_table
                  .select("id", "caller_id", "channel_id", "unix_timestamp(start_date)",
                          "unix_timestamp(end_date)")
                  .where("channel_id = :channel_id")
                  .orderBy("start_date DESC")
                  .limit(amount)
                  .offset(offset)
                  .bind("channel_id", channel_id)
                  .execute();
    } catch (...) {
        throw exception(error::unknown_error);
    }

    // For each row in the result, create a call
    while ((row = res.fetchOne())) {
        // Get the call participants and create the call from the row
        calls.push_back(call((std::string)row[0], (std::string)row[1], (std::string)row[2],
                             (int)row[3], row[4].isNull() ? 0 : (int)row[4],
                             user_joined_call((std::string)row[0], sess->user()->id)));
    }

    return calls;
}

void quesync::server::voice_manager::trigger_voice_state_event(std::string channel_id,
                                                               std::string user_id,
                                                               quesync::voice::state voice_state) {
    auto voice_states = _voice_channels[channel_id]->voice_states;

    std::shared_ptr<events::voice_state_event> evt(
        std::make_shared<events::voice_state_event>(user_id, voice_state));

    // For each user in the channel
    for (auto& user : voice_states) {
        // Check if the user is connected to the channel
        if (user.first != user_id && user.second == voice::state_type::connected) {
            _server->event_manager()->trigger_event(std::static_pointer_cast<quesync::event>(evt),
                                                    user.first);
        }
    }
}

quesync::call quesync::server::voice_manager::create_call(std::string caller_id,
                                                          std::string channel_id) {
    std::string call_id = sole::uuid4().str();

    sql::Session sql_sess = _server->get_sql_session();
    sql::Table calls_table(_server->get_sql_schema(sql_sess), "calls");

    try {
        // Insert to the calls table the new call
        calls_table.insert("id", "caller_id", "channel_id")
            .values(call_id, caller_id, channel_id)
            .execute();
    } catch (...) {
        throw exception(error::unknown_error);
    }

    // Create the call object
    return call(call_id, caller_id, channel_id, std::time(nullptr), false);
}

void quesync::server::voice_manager::add_participant_to_call(std::string channel_id,
                                                             std::string participant_id) {
    try {
        // Try to insert the participant to the call participants table (ignore if already exists)
        _server->get_sql_session()
            .sql(
                "INSERT IGNORE INTO quesync.call_participants(call_id, participant_id) VALUES(?, "
                "?)")
            .bind(_voice_channels[channel_id]->call.id)
            .bind(participant_id)
            .execute();
    } catch (...) {
        throw exception(error::unknown_error);
    }
}

void quesync::server::voice_manager::close_call(std::string channel_id) {
    try {
        // Set end date
        _server->get_sql_session()
            .sql("UPDATE calls SET end_date = FROM_UNIXTIME(?) WHERE id = ?")
            .bind(std::time(nullptr))
            .bind(_voice_channels[channel_id]->call.id)
            .execute();
    } catch (...) {
        throw exception(error::unknown_error);
    }
}

bool quesync::server::voice_manager::user_joined_call(std::string call_id, std::string user_id) {
    std::vector<std::string> call_participants;

    sql::Session sql_sess = _server->get_sql_session();
    sql::Table call_participants_table(_server->get_sql_schema(sql_sess), "call_participants");
    sql::RowResult res;

    try {
        // Try to get the call participants
        res = call_participants_table.select("1")
                  .where("call_id = :call_id AND participant_id = :user_id")
                  .bind("call_id", call_id)
                  .bind("user_id", user_id)
                  .execute();
    } catch (...) {
        throw exception(error::unknown_error);
    }

    return res.count() != 0;
}