#pragma once

typedef enum PacketType
{
    PING_PACKET = 0,

    // Requests
    LOGIN_PACKET = 1,
    REGISTER_PACKET,
    PROFILE_REQUEST_PACKET,
    SEARCH_PACKET,
    FRIEND_REQUEST_PACKET,
    FRIENDSHIP_STATUS_PACKET,
    GET_PRIVATE_CHANNEL_PACKET,
    SEND_MESSAGE_PACKET,
    GET_CHANNEL_MESSAGES_PACKET,

    // Respones
    AUTHENTICATED_PACKET = 200,
    SEARCH_RESULTS_PACKET,
    FRIEND_REQUEST_SENT_PACKET,
    FRIENDSHIP_STATUS_SET_PACKET,
    PROFILE_PACKET,
    PRIVATE_CHANNEL_PACKET,
    MESSAGE_ID_PACKET,
    CHANNEL_MESSAGES_PACKET,

    // On error
    ERROR_PACKET = 400,

    // On event
    EVENT_PACKET = 800,

    PONG_PACKET = 999
} PacketType;