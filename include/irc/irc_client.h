#ifndef TWITCH_IRC_IRCCLIENT_H
#define TWITCH_IRC_IRCCLIENT_H

#include <string>
#include <pplx/pplxtasks.h>
#include <cpprest/ws_client.h>
#include <thread>
#include "message.h"
#include "../../src/ts_queue.hpp"

namespace irc {
	class irc_client {
	  public:

		irc_client(const std::string& host, bool handle_ping);

		~irc_client();

		pplx::task<void> send_message(const message& message_to_send);

		message read_message(const std::chrono::milliseconds& timeout);

		message read_message();

	  private:

		pplx::task<void> create_infinite_receive_task(const pplx::cancellation_token& cancellation_token);

		const bool m_handle_ping;

		ts_queue<message> m_queued_messages;

		web::websockets::client::websocket_client m_client;

		pplx::task<void> m_receive_task;

		pplx::cancellation_token_source m_cancellation_token_source;

	};
}


#endif //TWITCH_IRC_IRCCLIENT_H
