#ifndef NET_MINECRAFT_SharedConstants_H__
#define NET_MINECRAFT_SharedConstants_H__

#include <string>

enum LevelGeneratorVersion
{
	LGV_ORIGINAL = 0,
};

namespace Common {
	std::string getGameVersionString(const std::string& versionSuffix = "");
}

namespace SharedConstants
{
	// 0.5.0 uses NPv8
	// 0.6.0 uses NPv9
    inline constexpr int NetworkProtocolVersion = 9;
	inline constexpr int NetworkProtocolLowestSupportedVersion = 9;
	inline constexpr int GameProtocolVersion = 1;
	inline constexpr int GameProtocolLowestSupportedVersion = 1;

	inline constexpr int StorageVersion = 3;

	inline constexpr int MaxChatLength = 100;
    
	inline constexpr int TicksPerSecond = 20;

	inline constexpr int GeneratorVersion = static_cast<int>(LGV_ORIGINAL);
	//int FULLBRIGHT_LIGHTVALUE = 15 << 20 | 15 << 4;
}

#endif /*NET_MINECRAFT_SharedConstants_H__*/
