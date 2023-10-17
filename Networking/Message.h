#pragma once

#include "Resources.h"


namespace Networking
{
	class Client;
	class Connection;

	enum class MessageType
	{
		Server,
		Global,
		Private,
		RequestDataFromServer,
		RequestDataFromClient,
		Null,
		Update
	};

	class T_RGB
	{
	public:
		T_RGB(float r = 0.0f, float g = 0.0f, float b = 0.0f)
			: R(r), G(g), B(b)
		{};

		float R;
		float G;
		float B;
	};

	struct Message
	{
		std::shared_ptr<Connection> Current = nullptr;
		std::vector<uint8_t> msg = std::vector<uint8_t>(1024 * 20);
		std::string Sender;
		std::string Alias;
		int ID = 0;
		T_RGB Color;

		//wxColour Color;

		//int length = 0;
		MessageType Type = MessageType::Global;
		bool Locked = false;
		Message(std::shared_ptr<Connection> connection, const std::string message)
			: Current(connection)
		{

			SetMsg(message);

		}

		Message(std::shared_ptr<Connection> connection, const std::vector<uint8_t>& message)
			: Current(connection), msg(message) {}

		std::string Serialize()
		{

			//msg.resize(MessageLength());
			nlohmann::json SerializedData;
			
			SerializedData["msg"] = std::string(msg.begin(), msg.begin() + MessageLength());
			//SerializedData["Color"] = std::string("HWOWWW");
			SerializedData["Sender"] = Sender;
			SerializedData["Alias"] = Alias;
			SerializedData["Type"] = Type;
			SerializedData["ID"] = ID;
			//nlohmann::json ColorArray = { Color.R, Color.G, Color.B };

			SerializedData["Color"]["R"] = Color.R;
			SerializedData["Color"]["G"] = Color.G;
			SerializedData["Color"]["B"] = Color.B;

			return SerializedData.dump();
			//return SerializedData;
		}

		static Message Deserialize(const std::string& SerializedData)
		{
			nlohmann::json json = nlohmann::json::parse(SerializedData);
			std::string m = json["msg"].get<std::string>();

			Message MSG(nullptr, m);
			
			//nlohmann::json ColorArray = json["Color"];
			//MSG.Color = T_RGB(ColorArray[1], ColorArray[2], ColorArray[3]);
			MSG.Sender = json["Sender"].get<std::string>();
			MSG.Alias = json["Alias"].get<std::string>();
			MSG.Type = json["Type"].get<MessageType>();
			MSG.ID = json["ID"].get<int>();
			MSG.Color = T_RGB(json["Color"]["R"], json["Color"]["G"], json["Color"]["B"]);
			
			return MSG;
		}

		

		size_t size() const
		{
			return msg.size();
		}

		int MessageLength() const
		{
			int length = 0;
			
			for (char c : msg)
			{
				length += c == '\0' ? 0 : 1;
			}

			return length;
		}


		void SetMsg(const std::string& message)
		{
			size_t MessageLength = message.size();
			//length = MessageLength;
			for (size_t i = 0; i < MessageLength; i++)
			{
				msg[i] = message[i];
			}


		}

		std::string GetColorAsString()
		{
			return std::to_string(Color.R) + "," + std::to_string(Color.G) + "," + std::to_string(Color.B);
		}

		Message operator=(const Message& m)
		{
			ID = m.ID;
			Type = m.Type;
			Sender = m.Alias == "" ? m.Sender : m.Alias;
			Alias = m.Alias;
			msg = m.msg;
			Color = m.Color;
			//Current = m.Current;
			return *this;
		};

	private:
		

	};

	class PrivateMesasge
	{
		Message* PM;
		std::shared_ptr<Connection> Recipient;
	};

	
}