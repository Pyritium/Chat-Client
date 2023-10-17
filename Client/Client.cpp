#include <Networking.h>

using namespace Networking;

int main()
{
	Client c;
	c.Connect("127.0.0.1", 60000);
	std::string String = "";

	std::thread Message([&]()
		{
			while (1)
			{
				std::getline(std::cin, String);
				Message msg(nullptr, String);
				c.Send(msg);
			}
			
		});

	while (1)
	{
		c.Update();
	}

	Message.join();

	return 0;
}