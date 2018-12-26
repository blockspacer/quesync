{
  "targets": [
    {
      "target_name": "hello",
      "sources": [ 
        "backend/hello.cc", 
        "backend/socket-manager.cpp", 
        "backend/voice-chat.cpp", 
        "backend/include/socket-manager.hpp",
        "backend/include/socket-error.hpp",
        "backend/include/voice-chat.hpp" ],
      "include_dirs": [ "../include" ],
      "cflags": [
        "-std=c++17",
        "-stdlib=libc++",
      ],
      'link_settings': {
        'libraries': [
          '-lopus',
          '-lbass',
          '-lws2_32',
          '-lOpenAL32'
        ],
        'library_dirs': [
          '../lib',
        ],
      },
    }
  ]
}