# ircpp
ircpp is a simple C++17 open-source library for basic IRCv3 message parsing and client using websocket.

## Requirements
Client uses [Microsoft's CPP REST SDK](https://github.com/microsoft/cpprestsdk) because of it's tasks and websocket. I plan on either replacing them completely or find another libraries with similar functionality.

## Contribution
Feel free to create pull requests, I am aware that this is not polished as it could be.

## Examples
See examples folder, there is a really simple example for reading Twitch chat.

## TODO's
Main TODO's are:
  * Replacing mentioned CPP REST SDK
  * Adding examples
  * Better commented/documented code
  * Fully comply to all specifications (namely tags key parsing and rfc952 for hostnames)
