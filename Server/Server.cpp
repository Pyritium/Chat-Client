#include <Networking.h>

using namespace Networking;

uint32_t RequestPort()
{
	std::string Port;
	std::cout << "Enter the port you want to host your server on:" << std::endl;

	std::getline(std::cin, Port);

	int len = Port.length();
	if (std::stoi(Port) && len <= 5)
	{
		int Difference = 5 - len;

		for (int x = 0; x < Difference; x++)
		{
			Port = Port + '0';
		}
		std::cout << "Starting connection with port:" + Port + '\n';
		return std::stoi(Port);
	}
	else
	{
		std::cout << "Not a valid port." << std::endl;
		RequestPort();
	}
	
}

int main()
{
	uint32_t Port = RequestPort();

	Server s(Port);
	s.Start();

	std::string String = "";

	std::thread OutgoingMessages([&]()
		{
			while (1)
			{
				std::getline(std::cin, String);
				Message msg(nullptr, String);
				msg.Sender = "[SERVER]";
				msg.Type = MessageType::Server;
				s.MessageAllClients(msg);
			}	
		});


	while (1)
	{
		s.Update();
	}

	OutgoingMessages.join();

	return 0;
}