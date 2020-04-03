#include "irc/message.h"
#include <algorithm>
#include <assert.h>
#include <memory>

using std::string;
using std::vector;
using std::optional;
using std::nullopt;
using std::make_optional;
using std::pair;
using std::make_pair;

using irc::message;
using params_t = irc::message::params_t;
using irc::tags_t;
using irc::prefix_t;

#define SKIP_WHITESPACES(it, end) do {} while ((++it) != (end) && *(it) == ' ')

#define SKIP_WHITESPACES_THROW_END(it, end, parsed_part) SKIP_WHITESPACES((it), (end));       \
	if ((it) == (end)) {                                                                      \
		throw message::parsing_error("Message ended while parsing " + string((parsed_part))); \
	}

namespace irc_parsing {
	static string parse_key(string::const_iterator& it, const string::const_iterator& end) {
		string key;
		while (isalnum(*it) || *it == '-' || *it == '/' || *it == '+') { // TODO make precisely as in rfc1459
			key.push_back(*it++);
			if (it == end) {
				throw message::parsing_error("Message ended while parsing tag key");
			}
		}

		if (key.empty()) {
			throw message::parsing_error("Empty key in tags parsing");
		}
		return key;
	}

	static string parse_tag_value(string::const_iterator& it, const string::const_iterator& end) {
		string tag_value;
		while (*it != ';' && *it != ' ' && *it != '\r' && *it != '\n' && *it != '\0') {
			if (*it == '\\') {
				if (++it == end) {
					throw message::parsing_error("Message ended while parsing tag value");
				}
				switch (*it++) {
					case ':':
						tag_value.push_back(';');
						break;
					case 's':
						tag_value.push_back(' ');
						break;
					case '\\':
						tag_value.push_back('\\');
						break;
					case 'r':
						tag_value.push_back('\r');
						break;
					case 'n':
						tag_value.push_back('\n');
						break;
					default:
						throw message::parsing_error("Unknown escape sequence");
				}
			} else {
				tag_value.push_back(*it++);
			}
			if (it == end) {
				throw message::parsing_error("Message ended while parsing tag value");
			}
		}
		return tag_value;
	}

	static pair<string, optional<string>> parse_tag(string::const_iterator& it, const string::const_iterator& end) {
		string key = parse_key(it, end);
		if (*it == '=') {
			if (++it == end) {
				throw message::parsing_error("Message ended while parsing tag");
			}
			return make_pair(key, parse_tag_value(it, end));
		} else {
			return make_pair(key, nullopt);
		}
	}

	static tags_t parse_tags(string::const_iterator& it, const string::const_iterator& end) {
		tags_t parsed_tags;
		if (*it != '@') {
			return parsed_tags;
		}
		do {
			if (++it == end) {
				throw message::parsing_error("Message ended while parsing tags");
			}
			parsed_tags.insert(parse_tag(it, end));
		} while (*it == ';');
		if (*it != ' ') {
			throw message::parsing_error("Tags do not terminate with space as they should");
		}
		SKIP_WHITESPACES_THROW_END(it, end, "tags")
		return parsed_tags;
	}

	static string parse_command(string::const_iterator& it, const string::const_iterator& end, bool crlf_included) {
		string command;
		if (isalpha(*it)) {
			do {
				command.push_back(*it++);
				if (it == end) {
					if (crlf_included) {
						throw message::parsing_error("Message ended while parsing command");
					}
					break;
				}
			} while (isalpha(*it));
		} else {
			if (!isdigit(*it)) {
				throw message::parsing_error("Message command is in wrong format");
			}
			command.push_back(*it++);
			if (it == end) {
				throw message::parsing_error("Message ended while parsing number command");
			}
			if (!isdigit(*it)) {
				throw message::parsing_error("Message command is in wrong format");
			}
			command.push_back(*it++);
			if (it == end) {
				throw message::parsing_error("Message ended while parsing number command");
			}
			if (!isdigit(*it)) {
				throw message::parsing_error("Message command is in wrong format");
			}
			command.push_back(*it++);
			if (it == end && crlf_included) {
				throw message::parsing_error("Message ended while parsing number command");
			}
		}
		return command;
	}

	static string
	parse_middle_param(string::const_iterator& it, const string::const_iterator& end, bool crlf_included) {
		string param;
		while (*it != ' ' && *it != '\r' && *it != '\n' && *it != '\0') {
			param.push_back(*it++);
			if (it == end) {
				if (crlf_included) {
					throw message::parsing_error("Message ended while parsing middle param");
				}
				break;
			}
		}
		if (param.empty()) {
			throw message::parsing_error("Middle param is empty");
		}
		return param;
	}

	static string
	parse_trailing_param(string::const_iterator& it, const string::const_iterator& end, bool crlf_included) {
		string param;
		while (*it != '\r' && *it != '\n' && *it != '\0') {
			param.push_back(*it++);
			if (it == end) {
				if (crlf_included) {
					throw message::parsing_error("Message ended while parsing trailing param");
				}
				break;
			}
		}
		return param;
	}

	static params_t parse_params(string::const_iterator& it, const string::const_iterator& end, bool crlf_included) {
		vector<string> parsed_params;
		if (it == end) {
			assert(!crlf_included);
			return parsed_params;
		}
		while (*it == ' ') {
			SKIP_WHITESPACES(it, end);
			if (it == end) {
				if (crlf_included) {
					throw message::parsing_error("Message ended while parsing params");
				}
				break;
			}
			if (*it == ':') {
				if (++it == end) {
					if (crlf_included) {
						throw message::parsing_error("Message ended while parsing params");
					}
					parsed_params.emplace_back("");
					break;
				}
				parsed_params.push_back(parse_trailing_param(it, end, crlf_included));
				break;
			}
			parsed_params.push_back(parse_middle_param(it, end, crlf_included));
			if (it == end) {
				assert(!crlf_included);
				break;
			}
		}
		return parsed_params;
	}

	static optional<prefix_t> parse_prefix(string::const_iterator& it, const string::const_iterator& end) {
		if (*it != ':') {
			return nullopt;
		}
		if (++it == end) {
			throw message::parsing_error("Message ended while parsing prefix");
		}
		string main;
		string user;
		optional<string> res_user;
		string host;
		optional<string> res_host;
		while (*it != ' ' && *it != '!' && *it != '@') { // Parse main
			main.push_back(*it++);
			if (it == end) {
				throw message::parsing_error("Message ended while parsing prefix");
			}
		}
		if (*it == '!') { // User part of prefix is present
			if (++it == end) {
				throw message::parsing_error("Message ended while parsing prefix");
			}
			while (*it != ' ' && *it != '@') { // Parse user part
				user.push_back(*it++);
				if (it == end) {
					throw message::parsing_error("Message ended while parsing prefix");
				}
			}
			if (user.empty()) {
				throw message::parsing_error("Empty user part in prefix");
			}
			res_user = make_optional(user);
		}
		if (*it == '@') { // Host part of prefix is present
			if (++it == end) {
				throw message::parsing_error("Message ended while parsing prefix");
			}
			while (*it != ' ') { // Parse host part
				host.push_back(*it++); // TODO validate for valid host by rfc952
				if (it == end) {
					throw message::parsing_error("Message ended while parsing prefix");
				}
			}
			if (host.empty()) {
				throw message::parsing_error("Empty host part in prefix");
			}
			res_host = make_optional(host);
		}
		if (*it != ' ') {
			throw message::parsing_error("Prefix does not terminate with space as it should");
		}
		SKIP_WHITESPACES_THROW_END(it, end, "prefix");
		return prefix_t(main, res_user, res_host);
	}
}

static string escape_tag_value(const string& tag_value) {
	string escaped_tag_value;
	for (char c : tag_value) {
		switch (c) {
			case ' ':
				escaped_tag_value += "\\s";
				break;
			case '\r':
				escaped_tag_value += "\\r";
				break;
			case '\n':
				escaped_tag_value += "\\n";
				break;
			case '\\':
				escaped_tag_value += "\\\\";
				break;
			case ';':
				escaped_tag_value += "\\:";
				break;
			default:
				escaped_tag_value.push_back(c);
		}
	}
	return escaped_tag_value;
}

message::message(const string& raw_message, bool crlf_included) {
	auto it = raw_message.cbegin();
	auto end = raw_message.cend();
	if (it == end) {
		throw message::parsing_error("Message is empty");
	}
	m_tags = irc_parsing::parse_tags(it, end);
	m_prefix = irc_parsing::parse_prefix(it, end);
	m_command = irc_parsing::parse_command(it, end, crlf_included);
	m_params = irc_parsing::parse_params(it, end, crlf_included);
	if (crlf_included) {
		if (it == end) {
			throw message::parsing_error("Message ends before CRLF sequence");
		}
		if (*it++ != '\r') {
			throw message::parsing_error("Expected CR character");
		}
		if (it == end) {
			throw message::parsing_error("Message ends before LF");
		}
		if (*it++ != '\n') {
			throw message::parsing_error("Expected LF character");
		}
	}
	if (it != end) {
		throw message::parsing_error("Message does not end properly");
	}
}

message message::private_message(const string& message_text, const string& channel) {
	return message("PRIVMSG", {"#" + channel, message_text});
}

message message::pass_message(const string& password) {
	return message("PASS", {password});
}

message message::nick_message(const string& nickname) {
	return message("NICK", {nickname});
}

message message::join_message(const vector<string>& channels, const vector<string>& keys) {
	string channels_param;
	assert(!channels.empty());
	auto c_it = channels.cbegin();
	channels_param = "#" + *c_it;
	for (++c_it; c_it != channels.cend(); ++c_it) {
		channels_param += ",#" + *c_it;
	}
	string keys_param;
	if (!keys.empty()) {
		auto k_it = keys.cbegin();
		keys_param = *k_it;
		for (++k_it; k_it != keys.cend(); ++k_it) {
			keys_param += "," + *k_it;
		}
	}
	return message("JOIN", {channels_param, keys_param});
}

message message::part_message(const vector<string>& channels) {
	string channels_param;
	assert(!channels.empty());
	auto c_it = channels.cbegin();
	channels_param = "#" + *c_it;
	for (++c_it; c_it != channels.cend(); ++c_it) {
		channels_param += ",#" + *c_it;
	}
	return message("PART", {channels_param});
}

message message::pong_message(const string& server) {
	return message("PONG", {server});
}

message message::pong_message(const string& server1, const string& server2) {
	return message("PONG", {server1, server2});
}

message message::capability_request_message(const vector<string>& capabilities) {
	vector<string> params({"REQ"});
	assert(!capabilities.empty());
	auto c_it = capabilities.cbegin();
	string capabilities_str = *c_it;

	for (++c_it; c_it != capabilities.cend(); ++c_it) {
		capabilities_str += ' ' + *c_it;
	}
	params.push_back(capabilities_str);
	return message("CAP", params);
}

string message::to_irc_message() const {
	string raw_message;
	if (!m_tags.empty()) {
		auto it = m_tags.begin();
		raw_message = '@' + it->first;
		if (it->second.has_value())
			raw_message += '=' + escape_tag_value(it->second.value());
		for (++it; it != m_tags.end(); ++it) {
			raw_message += ';' + it->first;
			if (it->second.has_value()) {
				raw_message += '=' + escape_tag_value(it->second.value());
			}
		}
		raw_message += ' ';
	}
	if (m_prefix.has_value()) {
		raw_message += ":" + m_prefix.value().to_irc_prefix() + ' ';
	}
	raw_message += m_command;
	for (auto it = m_params.begin(); it != m_params.end(); ++it) {
		raw_message += ' ';
		if (it->find(' ') != string::npos) {
			assert(next(it) == m_params.end());
			raw_message += ":";
		}
		raw_message += *it;
	}
	raw_message += "\r\n";
	return raw_message;
}

message message::capability_request_end_message() {
	return message("CAP", {string("END")});
}

string prefix_t::to_irc_prefix() const {
	string prefix = m_main;
	if (m_user.has_value()) {
		prefix += "!" + m_user.value();
	}
	if (m_host.has_value()) {
		prefix += "@" + m_host.value();
	}
	return prefix;
}
