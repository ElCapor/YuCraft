#include "DataIO.h"

//
// BytesDataOutput
//

void BytesDataOutput::writeString( const std::string& v )
{
    int length = v.length() & 0x7fff;
	writeShort(length);
	writeBytes(v.c_str(), length);
	//LOGI("Writing: %d bytes as String: %s\n", v.length(), v.c_str());
}

//
// BytesDataInput
//
std::string BytesDataInput::readString() {
	const int len = readShort();
	// readShort() returns a signed short, so len is in [-32768, 32767].
	// A negative length is malformed data; zero is a valid empty string.
	if (len <= 0) return "";
	std::string out(static_cast<size_t>(len), '\0');
	readBytes(&out[0], len);
	return out;
}
