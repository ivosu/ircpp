#ifndef IRC_MESSAGE_H
#define IRC_MESSAGE_H


#include <string>
#include <map>
#include <vector>
#include <optional>

namespace irc {

	class prefix_t {
	  public:
		prefix_t(std::string main, std::optional<std::string> user,
				 std::optional<std::string> host) : m_main(std::move(main)), m_user(std::move(user)),
													m_host(std::move(host)) {}

		const std::string& main() const { return m_main; }

		const std::optional<std::string>& user() const { return m_user; }

		const std::optional<std::string>& host() const { return m_host; }

		std::string to_irc_prefix() const;

		bool operator==(const prefix_t& other) const {
			return m_main == other.m_main
				   && m_user == other.m_user
				   && m_host == other.m_host;
		}

		bool operator!=(const prefix_t& other) const { return !operator==(other); }

	  private:
		std::string m_main;

		std::optional<std::string> m_user;

		std::optional<std::string> m_host;
	};

	typedef std::map<std::string, std::optional<std::string>> tags_t;

	class message {
	  public:

		using params_t = std::vector<std::string>;

		message(const std::string& raw_message, bool crlf_included = true);

		message(tags_t tags,
				std::optional<prefix_t> prefix,
				std::string command,
				params_t params) : m_tags(std::move(tags)), m_prefix(std::move(prefix)), m_command(std::move(command)),
								   m_params(std::move(params)) {}

		message(std::string command, params_t params) : m_command(std::move(command)), m_params(std::move(params)) {}

		static message private_message(const std::string& message_text, const std::string& channel);

		static message pass_message(const std::string& password);

		static message nick_message(const std::string& nickname);

		static message
		join_message(const std::vector<std::string>& channels, const std::vector<std::string>& keys = {});

		static message part_message(const std::vector<std::string>& channels);

		static message pong_message(const std::string& server);

		static message pong_message(const std::string& server1, const std::string& server2);

		static message capability_request_message(const std::vector<std::string>& capabilities);

		static message capability_request_end_message();

		const std::optional<prefix_t>& prefix() const { return m_prefix; }

		const std::string& command() const { return m_command; }

		const params_t& params() const { return m_params; }

		const tags_t& tags() const { return m_tags; }

		std::string to_irc_message() const;

		bool operator==(const message& other) const {
			return m_tags == other.m_tags
				   && m_prefix == other.m_prefix
				   && m_command == other.m_command
				   && m_params == other.m_params;
		}

		bool operator!=(const message& other) const;

		class parsing_error : public std::exception {
		  public:
			parsing_error(const char* message) : m_message(message) {}

			parsing_error(std::string message) : m_message(std::move(message)) {}

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

		params_t m_params;
	};
}


#endif //IRC_MESSAGE_H
