#pragma once

#include "Resources.h"
#include "Connection.h"

namespace Networking
{
	class Client
	{
	public:
		Client()
		{};

		~Client()
		{
			Disconnect();
		}

		bool Connect(const std::string& ip, const uint16_t port)
		{
			try
			{
				asio::ip::tcp::resolver Resolver(IO);
				asio::ip::tcp::resolver::results_type Endpoints = Resolver.resolve(ip, std::to_string(port));

				Connector = std::make_shared<Connection>(Connection::Ownership::Client, IO, asio::ip::tcp::socket(IO), Buffer);
				Connector->ConnectToServer(Endpoints);

				Message Confirmation(nullptr, "Connected to:" + ip + " on port:" + std::to_string(port));
				Confirmation.Type = MessageType::Private;
				Buffer->Type = Confirmation.Type;
				Buffer->msg = Confirmation.msg;
				Buffer->Current = Connector->shared_from_this();
				//Send(Confirmation);
				if (IsConnected())
					Thread = std::thread([this]() { IO.run(); });
			}
			catch (std::exception& error)
			{
				std::cerr << error.what() << std::endl;
				return false;
			}

			return true;
		}

		

		void Send(Message& msg)
		{
			if (IsConnected())
				Connector->Send(msg);
				
		}

		bool IsConnected()
		{
			if (Connector)
				return Connector->IsConnected();
			else
				return false;
		}

		void Disconnect()
		{
			if (IsConnected())
				Connector->Disconnect();


			IO.stop();

			if (Thread.joinable())
				Thread.join();

			Connector.reset();
		}

		uint32_t GetID()
		{
			if (Connector)
				return Connector->GetID();
			else
				Disconnect();

			return 0;
		}

		bool Update()
		{
			if (IsConnected() && Buffer->Current)
				return true;
			

			return false;
		}

		std::shared_ptr<Message> PopBuffer()
		{
			std::shared_ptr<Message> Copy = Buffer;
			Buffer->Current = nullptr;
			return Copy;
		}

		
	protected:
	private:

		/*void OnMessage()
		{
			auto client = Buffer->Current;
			std::string Msg = Buffer->msg;
			MessageType Type = Buffer->Type;

			std::cout << "Message info from:" << client->GetAddress() << std::endl;

			Buffer->Current = nullptr;
		}*/

		asio::io_context IO;
		std::shared_ptr<Connection> Connector;
		//std::vector<char> Buffer = std::vector<char>(20 * 1024);

		// Maybe make it point to a mock up message for the time being?? no.. that wouldn't work, maybe have special conditions for if it is not pointing at anything to go off of just a fake message?
		std::shared_ptr<Message> Buffer = std::make_shared<Message>(Connector, std::string(""));

		std::thread Thread;
	};
}