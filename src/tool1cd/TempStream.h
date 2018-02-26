//---------------------------------------------------------------------------

#ifndef TempStreamH
#define TempStreamH

#include <System.Classes.hpp>
#include <string>

class TTempStreamStaticInit
{
public:
	TTempStreamStaticInit();
	~TTempStreamStaticInit();
};


//---------------------------------------------------------------------------
class TTempStream : public TFileStream
{
public:
	static std::string tempcat;
	static long tempno;
	static std::string gettempname();

	TTempStream();
	virtual ~TTempStream();
};

//---------------------------------------------------------------------------
#endif
