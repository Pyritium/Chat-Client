#pragma once
#include "Resources.h"
#include "Connection.h"


namespace Networking
{
	class Server
	{
	public:
		Server(uint16_t port)
			: Acceptor(IO, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
		{}

		~Server()
		{
			Stop();
		}

		bool Start()
		{
			try 
			{
				ListenForConnections();
				Thread = std::thread([this]() {	IO.run(); });
			}
			catch (std::exception& e)
			{
				std::cerr << "[SERVER] Exception: " << e.what() << "\n";
				return false;
			}
			std::cout << "[SERVER] Started\n";
			return true;
		}

		void Stop()
		{

		}

		void Send()
		{

		}

		void MessageAllClients(Message& msg)
		{
			/*if (msg.Type == MessageType::Server)
			{
				Buffer->msg = msg.msg;
				//msg.Type = MessageType::Server;
				OnMessage();
				return;
			}*/

			//std::cout << bool(msg.Type == MessageType::Server) << '\n';
			bool DanglingConnection = false;
			for (auto& Client : Connections)
			{
				if (Client && Client->IsConnected())
				{
					// this will tell us whether or not it's dereferencing the message uint8_t vec or the connections i guess
					//if (Client != msg.Current)
						Client->Send(msg);

				}
				else
				{
					//Client->Disconnect();
					Client.reset();
					DanglingConnection = true;
				}
			}

			if (DanglingConnection)
				Connections.erase(std::remove(Connections.begin(), Connections.end(), nullptr), Connections.end());
		};


		// Almost done, just have to figure out how to send this data with the respect of who sent it... We figured out how to reciprocate it across clients (but it does crash), so just a little more...
		void Update()
		{
			if (Buffer->Current && (!Buffer->Locked))
			{
				OnMessage();
			}
		}

		void ListenForConnections()
		{
			Acceptor.async_accept([this](const asio::error_code& ec, asio::ip::tcp::socket Socket)
				{
					if (!ec)
					{
						std::cout << Socket.remote_endpoint() << " Connected!\n";

						std::shared_ptr<Connection> Connector = std::make_shared<Connection>(Connection::Ownership::Server,IO, std::move(Socket), Buffer);
					
						Connector->ConnectToClient(IDs++);

						Connections.push_back(Connector);
						
					}
					
					ListenForConnections();
				});
		}

	
	protected:
		std::deque<std::shared_ptr<Connection>> Connections;
	private:
		

		void OnMessage()
		{
			//std::cout << "On message fired\n";
			std::shared_ptr<Connection> client = Buffer->Current;
			MessageType Type = Buffer->Type;

			//std::string Speaker = ((client != nullptr ? "[" + client->GetAddress() + "]" : "[SERVER]"));
			//std::string Msg = Buffer->msg;
			//if (Buffer->Current != nullptr) std::cout << Buffer->Current->GetAddress() << " " << GetAddress() << std::endl;

			//std::fill(Buffer->msg.begin(), Buffer->msg.end(), '\0');
			//Buffer->length = 0;
			switch (Type)
			{
				case MessageType::Global:
				{
					
					std::string Sender = client != nullptr ? client->GetAddress() : "[SERVER]";
					//std::string RawMsg(Buffer->msg.begin(), Buffer->msg.begin() + Buffer->MessageLength());
					//std::string Msg = Sender + " Says:" + RawMsg;

					Message NewMessage(client, Buffer->msg);
					NewMessage = *Buffer;
					// Server custom variable ternary operator checks
					NewMessage.Color = Sender != "[SERVER]" ? Buffer->Color : T_RGB(0,0,0);
					NewMessage.Alias = Sender != "[SERVER]" ? Buffer->Alias : "";
					MessageAllClients(NewMessage);
				}
				break;
				case MessageType::RequestDataFromServer:
				{
					Message NewMessage(client, Buffer->msg);
					NewMessage.Type = MessageType::RequestDataFromServer;


					for (std::shared_ptr<Connection>& c : Connections)
					{
						//GET BUFFER VALUE SEND VALUE MESSAGE EACH ITERATION BUFFER UPDATE????
						Message RequestClientBuffer(nullptr, "");
						RequestClientBuffer.Type = MessageType::RequestDataFromClient;
						c->Send(RequestClientBuffer);
						
						Buffer->Current = nullptr;
						while (Buffer->Current == nullptr)
						{
							std::this_thread::sleep_for(std::chrono::milliseconds(1));
						}

						//std::cout << Buffer->Alias << std::endl;

						NewMessage.Sender = NewMessage.Sender + "(" + c->GetAddress() + "):" + Buffer->Alias + "[" + Buffer->GetColorAsString() + "] |";
					}
					client->Send(NewMessage);
				}
				break;
				case MessageType::Update:
				{
					
					Message NewMessage(client, Buffer->msg);
					NewMessage.Type = MessageType::RequestDataFromServer;
					for (std::shared_ptr<Connection>& c : Connections)
					{
						if (c && c->IsConnected())
						{
							Message RequestClientBuffer(nullptr, "");
							RequestClientBuffer.Type = MessageType::RequestDataFromClient;
							c->Send(RequestClientBuffer);

							Buffer->Current = nullptr;
							while (Buffer->Current == nullptr)
							{
								std::this_thread::sleep_for(std::chrono::milliseconds(1));
							}

							//std::cout << Buffer->Alias << std::endl;

							NewMessage.Sender = NewMessage.Sender + "(" + c->GetAddress() + "):" + Buffer->Alias + "[" + Buffer->GetColorAsString() + "] |";
						}
						//GET BUFFER VALUE SEND VALUE MESSAGE EACH ITERATION BUFFER UPDATE????
						
					}
					//std::cout << "Updating:" << NewMessage.Sender << std::endl;
					MessageAllClients(NewMessage);
				}
				break;
			};
			


			//if (Msg.data()) std::cout << "msg data:" << Msg.data() << std::endl;
			
			Buffer->Current = nullptr;
		}

		asio::io_context IO;
		asio::ip::tcp::acceptor Acceptor;
		std::thread Thread;
		//std::vector<char> Buffer = std::vector<char>(20 * 1024);
		//std::deque<Message> Buffer;
		std::shared_ptr<Message> Buffer = std::make_shared<Message>(nullptr, std::string(""));

		uint32_t IDs = 0;
	};
}