#include "TStreamWriter.hpp"
#include "TFileStream.hpp"

namespace System {

namespace Classes {


TStreamWriter::TStreamWriter(const std::string &Filename, TEncoding *Encoding, int BufferSize)
	: stream(new TFileStream(Filename, fmOpenReadWrite))
{
}

TStreamWriter::TStreamWriter(TStream *stream, TEncoding* Encoding, int BufferSize)
	: stream(stream)
{
}

void TStreamWriter::Write(const std::string &s)
{
	// TODO: кодировка TEncoding::Convert()
	stream->Write(s.c_str(), s.size());
}

} // Classes

} // System
