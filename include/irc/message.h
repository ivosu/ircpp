#ifndef TWITCH_IRC_MESSAGE_H
#define TWITCH_IRC_MESSAGE_H


#include <string>
#include <map>
#include <vector>
#include <optional>
#include <memory>

namespace irc {

	class prefix_t {
	  public:
		prefix_t(const std::string& main, const std::optional<std::string>& user,
				 const std::optional<std::string>& host) : m_main(main), m_user(user), m_host(host) {}

		const std::string& main() const { return m_main; }

		const std::optional<std::string>& user() const { return m_user; }

		const std::optional<std::string>& host() const { return m_host; }

		std::string to_irc_prefix() const;

		bool operator==(const prefix_t& other) const;

		bool operator!=(const prefix_t& other) const;

	  private:
		std::string m_main;
		std::optional<std::string> m_user;
		std::optional<std::string> m_host;
	};

	typedef std::map<std::string, std::optional<std::string>> tags_t;

	class message {
	  public:
		message(const std::string& raw_message, bool crlf_included = true);

		message(const tags_t& tags,
				const std::optional<prefix_t>& prefix,
				const std::string& command,
				const std::vector<std::string>& params) : m_tags(tags), m_prefix(prefix), m_command(command),
														  m_params(params) {};

		static message private_message(const std::string& message_text, const std::string& channel);

		static message pass_message(const std::string& password);

		static message nick_message(const std::string& nickname);

		static message
		join_message(const std::vector<std::string>& channels, const std::vector<std::string>& keys = {});

		static message part_message(const std::vector<std::string>& channels);

		static message pong_message(const std::string& daemon);

		static message pong_message(const std::string& daemon1, const std::string& deamon2);

		static message capability_request_message(const std::vector<std::string>& capabilities);

		static message& capability_request_end_message();

		const std::optional<prefix_t>& prefix() const { return m_prefix; }

		const std::string& command() const { return m_command; }

		const std::vector<std::string>& params() const { return m_params; }

		const tags_t& tags() const { return m_tags; }

		std::string to_irc_message() const;

		bool operator==(const message& other) const;

		bool operator!=(const message& other) const;

		class parsing_error : public std::exception {
		  public:
			parsing_error(const char* message) : m_message(message) {}

			parsing_error(const std::string& message) : m_message(message) {}

			const char* what() const noexcept final {
				return "Message parsing error";
			}

			const std::string& message() const {
				return m_message;
			}

		  private:
			std::string m_message;
		};

	  private:
		tags_t m_tags;

		std::optional<prefix_t> m_prefix;

		std::string m_command;

		std::vector<std::string> m_params;
	};
}


#endif //TWITCH_IRC_MESSAGE_H
