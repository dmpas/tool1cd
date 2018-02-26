#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "Messenger.h"

using namespace std;

Messenger::Messenger()
{
	has_error = false;
	logfile = "";
	noverbose = false;
	minimal_state = MessageState::Empty;
}

//---------------------------------------------------------------------------
void Messenger::Status(const string &message)
{
	cout << message << endl;
	AddMessage(message, MessageState::Empty);
}

//---------------------------------------------------------------------------
void Messenger::AddDetailedMessage(const string &message, const MessageState mstate, const TStringList *param)
{
	if (mstate < minimal_state) {
		return;
	}

	shared_ptr<ostream> output (&cerr, [](...){} );

	if (!logfile.empty()) {
		output = make_shared<boost::filesystem::ofstream>(boost::filesystem::path(logfile), std::ios_base::app);
	}
	*output << message << endl;
	if (param) {
		for (auto it : *param) {
			*output << "\t" << it << endl;
		}
	}
	output->flush();
}

//---------------------------------------------------------------------------
void Messenger::setlogfile(const string &_logfile)
{
	logfile = System::Ioutils::TPath::GetFullPath(_logfile);
	if(FileExists(logfile)) DeleteFile(logfile);
}

void Messenger::set_log_level(const MessageState _minimal_state)
{
	minimal_state = _minimal_state;
}