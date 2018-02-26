#ifndef MESSENGER_H_INCLUDED
#define MESSENGER_H_INCLUDED

#include "MessageRegistration.h"

class Messenger : public MessageRegistrator
{
private:
	std::string logfile;
	bool noverbose;
	MessageState minimal_state;
public:
	bool has_error;
	Messenger();
	virtual void AddDetailedMessage(const std::string &message, const MessageState mstate,
									const TStringList *param = nullptr) override;
	virtual void Status(const std::string &message) override;
	void setlogfile(const std::string &_logfile);
	void setnoverbose(bool nv){noverbose = nv;}
	void set_log_level(const MessageState _minimal_state);
};

#endif // MESSENGER_H_INCLUDED
