#include <string>
#include "irc/message.h"
#include "irc/client.h"

std::string get_user_name_private_message(const irc::message& message) {
	auto tags = message.tags();
	auto display_name_it = tags.find("display-name");
	if (display_name_it != tags.end() && display_name_it->second.has_value() &&
		!display_name_it->second.value().empty()) {
		return display_name_it->second.value();
	}
	else {
		return message.prefix().value().main();
	}
}

int main() {
	// your twitch nickname
	std::string nickname = "<nickname>";
	// your oauth token (you can use https://twitchapps.com/tmi/)
	std::string auth = "<oauth token>";
	// join some channel
	std::string channel = "<some twitch channel>";
	// connect to twitches irc
	irc::client client(U("wss://irc-ws.chat.twitch.tv:443"), true);

	// request capability for tags
	client.send_message(irc::message::capability_request_message({"twitch.tv/tags"})).wait();
	// send auth
	client.send_message(irc::message::pass_message(auth)).wait();
	// send nickname
	client.send_message(irc::message::nick_message(nickname)).wait();
	// join given channel
	client.send_message(irc::message::join_message({channel})).wait();
	std::cout << "CHAT:" << std::endl;
	while(true) {
		irc::message msg = client.read_message();
		if (msg.command() == "PRIVMSG") {
			std::string sender = get_user_name_private_message(msg);
			std::string message = *msg.params().rbegin();
			std::string channel = msg.params().front();
			// remove first character (#)
			channel.erase(channel.begin());
			std::cout << sender << "@" << channel << ": " << message << std::endl;
		}
	}
	return 0;
}
