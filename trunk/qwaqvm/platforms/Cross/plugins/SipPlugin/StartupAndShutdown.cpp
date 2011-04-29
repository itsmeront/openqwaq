/**
 * Project OpenQwaq
 *
 * Copyright (c) 2005-20011, Teleplace, Inc., All Rights Reserved
 *
 * Redistributions in source code form must reproduce the above
 * copyright and this condition.
 *
 * The contents of this file are subject to the GNU General Public
 * License, Version 2 (the "License"); you may not use this file
 * except in compliance with the License. A copy of the License is
 * available at http://www.opensource.org/licenses/gpl-2.0.php.
 *
 */

/*
 *  StartupAndShutdown.cpp
 *  SipPlugin
 *
 */

#include "SipPlugin.h"

#include "qLogger.hpp"
#include "QSipCallbackHandler.hpp"

#include "resip/stack/SipStack.hxx"
#include "resip/stack/StackThread.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/DumShutdownHandler.hxx"
#include "resip/dum/ClientAuthManager.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/dum/AppDialogSet.hxx"
#include "resip/dum/AppDialogSetFactory.hxx"

#include <memory>

using namespace resip;
using namespace std;


DialogUsageManager* dum;
static SipStack* stack;
static StackThread* stack_thread;
static QSipCallbackHandler callback_handler;

class QDumShutdownHandler : public DumShutdownHandler
{
public:
	QDumShutdownHandler(const Data& n) : name(n) {}
	virtual ~QDumShutdownHandler() {}
	virtual void onDumCanBeDeleted() { qLog() << "Shutting down DUM"; }
private:
	Data name;	
};


// Called either to shutdown the module, or to 
// clean up after a failed module-initialization.
int qShutdownOrCleanupModule(bool shutDown)
{
	if (dum) {
		QDumShutdownHandler h("");
		dum->shutdown(&h);
		delete dum;
		dum = NULL;
		qLog() << "Deleted DUM";
	
	}
	if (stack_thread) {
		stack_thread->shutdown();
		stack_thread->join();
		delete stack_thread;
		stack_thread = NULL;
		qLog() << "Deleted StackThread";
	}
	if (stack) {
		delete stack;
		stack = NULL;
		qLog() << "Deleted SipStack";
	}
	
	qLog() << "Finished shutdown of SipPlugin" << flush;
	qShutdownLogging();

	return 1;
}


int qInitModule(void)
{
	stack = NULL;
	dum = NULL;
	
	qInitLogging();
	qLogToFile("SipPlugin.log");

	// These need to happen before any "goto errorQInitModule", otherwise it's
	// a compiler error.
	SharedPtr<MasterProfile> masterProfile(new MasterProfile);
	auto_ptr<ClientAuthManager> authManager(new ClientAuthManager);
//	auto_ptr<AppDialogSetFactory> dialogSetFactory(new AppDialogSetFactory);
				
	stack = new SipStack();
	if (!stack) {
		qLog() << "SipPlugin... Failed to instantiate SipStack";
		goto errorQInitModule;
	}
	
	stack_thread = new StackThread(*stack);
	if (!stack_thread) {
		qLog() << "SipPlugin... Failed to instantiate StackThread";
		goto errorQInitModule;
	}
	
	dum = new DialogUsageManager(*stack);
	if (!dum) {
		qLog() << "SipPlugin... Failed to instantiate DUM";
		goto errorQInitModule;	
	}

	// Add componets to DUM
	if (!masterProfile) {
		qLog() << "SipPlugin... Failed to instantiate MasterProfile";
		goto errorQInitModule;
	}
	if (!authManager.get()) {
		qLog() << "SipPlugin... Failed to instantiate ClientAuthManager";
		goto errorQInitModule;
	}
//	if (!dialogSetFactory.get()) {
//		qLog() << "SipPlugin... Failed to instantiate AppDialogSetFactory";
//		goto errorQInitModule;
//	}
	
	dum->setMasterProfile(masterProfile);
	dum->setClientAuthManager(authManager);
	dum->setClientRegistrationHandler(&callback_handler);
	dum->addOutOfDialogHandler(OPTIONS, &callback_handler);
//	dum->setAppDialogSetFactory(dialogSetFactory);
	
	dum->addTransport(UDP, 31234);
	stack_thread->run();
	
	qLog() << "SipPlugin... successfully initialized!" << flush;
	
	return 1;
	
errorQInitModule:
	qShutdownOrCleanupModule(false);
	qLog() << "Finished cleanup after failed initialization of SipPlugin" << flush;
	return 0;	
}


int qShutdownModule()
{
	qShutdownOrCleanupModule(true);
	qLog() << "Finished shutdown of SipPlugin" << flush;
	qShutdownLogging();
	return 1;
}

void qProcessDUM()
{
	if (!dum) {
		qLog() << "qProcessDUM()... DUM is NULL" << flush;
		return;
	}
	qLog(0) << "qProcessDUM()... starting dum->process()";
	dum->process();
	qLog(0) << "qProcessDUM()... finished dum->process()" << flush;
}

void qFakeREGISTER()
{
	SharedPtr<UserProfile> profile(new UserProfile);
//	NameAddr addressOfRecord("sip:1002@10.0.1.15");
	NameAddr addressOfRecord("sip:1002@10.10.2.138");
	Data passwd("st00p1d");
	
//	profile->setDefaultFrom(&addressOfRecord);
	profile->setDefaultRegistrationTime(70);
	profile->setDigestCredential(addressOfRecord.uri().host(), addressOfRecord.uri().user(), passwd);
	
	SharedPtr<SipMessage> regMessage = dum->makeRegistration(addressOfRecord, profile);
	qLog() << "qFakeREGISTER()... just created REGISTER message, about to send";
	qLog() << "\ttransaction-id: " << regMessage->getTransactionId().c_str() << flush;
	dum->send(regMessage);
}
