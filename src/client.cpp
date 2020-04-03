#include "irc/client.h"

using std::string;
using std::cerr;
using std::endl;
using std::istringstream;
using std::chrono::milliseconds;

using irc::client;
using irc::message;

using pplx::task_status;
using pplx::task;
using pplx::create_task;
using pplx::cancellation_token;
using pplx::cancel_current_task;

using web::websockets::client::websocket_incoming_message;
using web::websockets::client::websocket_outgoing_message;
using web::websockets::client::websocket_exception;

task<void> client::create_infinite_receive_task(const cancellation_token& cancellation_token) {
	return create_task(
	  [=]() -> void {
		  while (true) {
			  if (cancellation_token.is_canceled())
				  cancel_current_task();
			  try {
				  if (m_client.receive().then([](websocket_incoming_message msg) {
					  return msg.extract_string();
				  }, cancellation_token).then([=](string body) {
					  istringstream is(body);
					  string tmp;
					  while (getline(is, tmp)) {
						  if (tmp.back() == '\r') {
							  //Remove \r from back
							  tmp.pop_back();
						  }
						  try {
							  if (m_handle_ping) {
								  message message(tmp, false);
								  if (message.command() == "PING") {
									  send_message(message::pong_message(*message.params().begin()));
								  } else {
									  m_queued_messages.emplace(message);
								  }
							  } else {
								  m_queued_messages.emplace(tmp, false);
							  }
						  } catch (const message::parsing_error& e) {
							  // TODO handling this error
							  cerr << tmp << " : " << e.message() << endl;
							  continue;
						  }
					  }
				  }, cancellation_token).wait() == task_status::canceled)
					  cancel_current_task();
			  } catch (const websocket_exception& e) {
				  cancel_current_task();
			  }
		  }
	  }
	);
}

client::client(const string& host, bool handle_ping) : m_handle_ping(handle_ping) {
	if (m_client.connect(host).wait() != task_status::completed) {
		throw "Not connected";
	}
	m_receive_task = create_infinite_receive_task(m_cancellation_token_source.get_token());
}

task<void> client::send_message(const message& message_to_send) {
	websocket_outgoing_message websocket_message;
	websocket_message.set_utf8_message(message_to_send.to_irc_message());
	return m_client.send(websocket_message);
}

message client::read_message(const milliseconds& timeout) {
	auto tmp = m_queued_messages.pop(timeout);
	if (tmp.has_value()) {
		return tmp.value();
	}
	throw "Timeout";
}

message client::read_message() {
	return m_queued_messages.pop();
}

irc::client::~client() {
	m_client.close().wait();
	m_cancellation_token_source.cancel();
	assert(m_receive_task.wait() == task_status::canceled);
}
