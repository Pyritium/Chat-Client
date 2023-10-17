#pragma once

#include "Resources.h"
#include "Message.h"

namespace Networking
{
	class Connection : public std::enable_shared_from_this<Connection>
	{
	public:
		enum class Ownership 
		{
			Server,
			Client
		};

		// Make sockets on the client & server, send over to this connection class
		Connection(Ownership OT, asio::io_context& io, asio::ip::tcp::socket socket, std::shared_ptr<Message>& buf)
			: Owner(OT), IO(io), Socket(std::move(socket)), Buffer(buf)
		{}

		~Connection()
		{};

		void UpdateConnectionsForAll()
		{
			Buffer->SetMsg("");
			Buffer->Type = MessageType::Update;
			Buffer->Current = this->shared_from_this();
		}

		void ConnectToClient(uint32_t UID = 0)
		{
			if (Socket.is_open())
			{
				ID = UID;
				ReadHandler();
				UpdateConnectionsForAll();
			}
		}

		void ConnectToServer(asio::ip::tcp::resolver::results_type& Endpoints)
		{
			asio::async_connect(Socket, Endpoints,
				[&](const asio::error_code& ec, asio::ip::tcp::endpoint Endpoint)
				{
					if (!ec)
					{
						std::cout << "New Connection:" << Socket.remote_endpoint() << " (" << ID << ")" << std::endl;
						ReadHandler();
					}
				});
		};
		
		void Disconnect()
		{
			Socket.close();
			IO.stop();
		}

		bool IsConnected()
		{
			return Socket.is_open();
		}

		void Send(Message& message)
		{

			//Buffer->length = message.length;
			//Buffer->msg = message.msg;
			//Buffer->sender = message.sender;*/
			//std::shared_ptr<Connection> client = message.Current;

			
			//message.SetMsg(Msg);
			
			// The problem with the solution below is that it's overlapping with each other sometimes and it's not actually in an "ORDER"(?)
			//message.Sender = message->GetAddress();
			
			std::string SerializedMessage = message.Serialize();
			Socket.async_write_some(asio::buffer(SerializedMessage.data(), SerializedMessage.size()),
				[&](const std::error_code& ec, size_t length)
				{
					/*if (!ec)
						std::cout << "Data successfully written!\n";
					else
						std::cout << "error...\n";*/
					ReadHandler();
					//asio::write(Socket, asio::buffer(&message.Type, sizeof(MessageType)));
					//std::cout << Socket.remote_endpoint() << '\n';

				});
		}


		std::string GetAlias()
		{
			return Alias;
		}

		uint32_t GetID()
		{
			return ID;
		};


		std::string GetAddress()
		{
			return Socket.remote_endpoint().address().to_string() + ":" + std::to_string(Socket.remote_endpoint().port());
		}

	protected:
	private:

		/*void AddToQueue()
		{
			// Actually queue the messages instead of just dealing with the raw data, so we can interpret who and where it comes from
			std::string Str(Buffer.begin(), Buffer.end());
			if (Owner == Ownership::Server)
				InMessages.push_back({ this->shared_from_this(), Str });
			else
				InMessages.push_back({ nullptr, Str });
		}

		void WriteHandler(size_t length)
		{
			asio::async_write(Socket, asio::buffer(Buffer.data(), length),
				[this](const std::error_code& ec, size_t bytes_transferred)
				{
					if (!ec) {
						ReadHandler();
					}
					else {
						std::cerr << "Error in writing data: " << ec.message() << std::endl;
						Socket.close();
					}
				});
		}*/


		void ReadHandler()
		{
			//std::fill(Buffer->msg.begin(), Buffer->msg.end(), '\0');
			//

			/* note to self in case json encoding doesn't work:
				the old buffer was like asio::buffer(Buffer->msg.data(), Buffer->msg.size()), [&](){});
				and for the send function it was like asio::buffer(message.msg, message.size()), [&](){});
			*/

			// fill?
			//std::fill(StreamBuffer.begin(), StreamBuffer.end(), '\0');
			std::fill(StreamBuffer.begin(), StreamBuffer.end(), '\0');
			Socket.async_read_some(asio::buffer(StreamBuffer.data(), StreamBuffer.size()),
				[&](const asio::error_code& ec, size_t length) {
					if (!ec)
					{
						// convert the string to json then interact with the variables there! :D
						
						//std::cout << GetAddress() << std::endl;
						

						if (length > 0)
						{
							std::string ReceivedData(StreamBuffer.begin(), StreamBuffer.end());
							Message ReceivedMessage = Message::Deserialize(ReceivedData);
							ReceivedMessage;

							//std::cout << ReceivedMessage.GetColorAsString() << std::endl;

							*Buffer = ReceivedMessage;
							
							//Buffer->Current = nullptr;
							Buffer->Sender = Buffer->Sender != "" ? Buffer->Sender : GetAddress(); // hotfix to relay messages on server side
							if (Owner == Ownership::Server)
							{
								Buffer->ID = ID;
							}
								
								
							/*Buffer->Alias = ReceivedMessage.Alias;
							Buffer->Sender = Buffer->Alias == "" ? ReceivedMessage.Sender : Buffer->Alias;
							
							Buffer->Color = ReceivedMessage.Color;*/

							int len = ReceivedMessage.MessageLength();
							std::string Msg(ReceivedMessage.msg.begin(), ReceivedMessage.msg.begin() + len);

							if (Buffer->Type == MessageType::Global)
								std::cout << ID + " (" + Buffer->Sender + ") Says:" + Msg << std::endl;

							//if (Owner == Ownership::Server)
								
							
							//Buffer->SetMsg(Buffer->Sender + " Says:" + Msg);
							//size_t found = Msg.find(Buffer->Sender);
							Buffer->Locked = true;
							Buffer->msg = ReceivedMessage.msg;
							Buffer->Current = this->shared_from_this();
							
							/*if (Owner == Ownership::Server)
							{
								
								
								/*if (Buffer->Current == nullptr)
									Buffer->Type = MessageType::Server;
							}

							//asio::read(Socket, asio::buffer(&Buffer->Type, sizeof(MessageType)));
							
							
							
							//Msg = (Buffer->Current != nullptr ? ((Msg.find(Buffer->Current->GetAddress()) != std::string::npos) ? Msg : Buffer->Current->GetAddress() + " Says:" + Msg) : (Owner == Ownership::Server) ? Msg : "[SERVER] Says:" + Msg);
							//Msg = (Buffer->Current != nullptr ? ((Msg.find(Buffer->Current->GetAddress()) != std::string::npos) ? Msg : Buffer->Current->GetAddress() + " Says:" + Msg) : Owner == Ownership::Client ? "[SERVER] Says:" + Msg : Msg);
							Msg = (Buffer->Type != MessageType::Private ? (Buffer->Current != nullptr ? Buffer->Current->GetAddress() + " Says:" + Msg : Msg) : Msg);
							std::cout << Msg << '\n';*/
							/* 
							* i literally did this for no reason btw LOL because it just works now soooo awesome
							* For the ternary operator, to help explain what's going on here (it's a little daunting at first!)
									
											Where is this being replicated to?
											                 |
															 |
											  (Client) ------|------ (Server)
													   |           |
										      Is message type    Concatenate
												  server?        buffer address
													|              with Msg
													|
									  (False) ------|------ (True)
											  |           |
											 Msg	 Concatenate
											       (server address)
													      /
												    (server name)
													  with Msg

							*/

							//std::string Msg(Buffer->msg.begin(), Buffer->msg.begin()+len);
						
							//std::string Speaker = ((client != nullptr ? client->GetAddress() : "[SERVER]"));
							
							//std::cout << "Speaker:" + Sender + " Msg data: '" + RawMsg + "'\n";

							

							
							
						}
						
						Buffer->Locked = false;
						
						ReadHandler();
					}
					else
					{
						// Handle read error
						if (IsConnected())
						{
							std::string Result = "[" + GetAddress() + "](" + std::to_string(ID) + "):" + ec.message();
							std::cerr << Result << "\n";
							Socket.close();

							UpdateConnectionsForAll();
						}
					}
				});
			
			
			
			
		}

		
		Ownership Owner;
		std::shared_ptr<Message>& Buffer;
		asio::io_context& IO;
		asio::ip::tcp::socket Socket;
		uint16_t ID = 0;
		std::string Alias;

		std::vector<uint8_t> StreamBuffer = std::vector<uint8_t>(1024 * 20);

	};		
}
