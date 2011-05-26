/**
 * Project OpenQwaq
 *
 * Copyright (c) 2005-2011, Teleplace, Inc., All Rights Reserved
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
 *  QSipCallbackHandler.hpp
 *  SipPlugin
 *
 */

#ifndef __Q_SIP_CALLBACK_HANDLER_HPP__
#define __Q_SIP_CALLBACK_HANDLER_HPP__

#include "resip/dum/InviteSessionHandler.hxx"
#include "resip/dum/RegistrationHandler.hxx"
#include "resip/dum/OutOfDialogHandler.hxx"
#include "resip/dum/ClientRegistration.hxx"

using namespace resip;

class QSipCallbackHandler : 
	public InviteSessionHandler, 
	public ClientRegistrationHandler,
	public OutOfDialogHandler
{
public:
	QSipCallbackHandler() {}
	~QSipCallbackHandler() {}

	// ****************	
	// Callback methods declared in ClientRegistrationHandler
	virtual void onSuccess(ClientRegistrationHandle, const SipMessage& response);
	virtual void onRemoved(ClientRegistrationHandle, const SipMessage& response);
	virtual int onRequestRetry(ClientRegistrationHandle, int retrySeconds, const SipMessage& response);
	virtual void onFailure(ClientRegistrationHandle, const SipMessage& response);
	
	// ****************
	// Callback methods declared in InviteSessionHandler
	// (commented-out methods are already implemented in superclass)
	virtual void onNewSession(ClientInviteSessionHandle, InviteSession::OfferAnswerType oat, const SipMessage& msg);
	virtual void onNewSession(ServerInviteSessionHandle, InviteSession::OfferAnswerType oat, const SipMessage& msg);
	virtual void onFailure(ClientInviteSessionHandle, const SipMessage& msg);
	virtual void onEarlyMedia(ClientInviteSessionHandle, const SipMessage&, const SdpContents&);
	//virtual void onEarlyMedia(ClientInviteSessionHandle, const SipMessage&, const Contents&);
	virtual void onProvisional(ClientInviteSessionHandle, const SipMessage&);
	virtual void onConnected(ClientInviteSessionHandle, const SipMessage& msg);
	virtual void onConnected(InviteSessionHandle, const SipMessage& msg);
	//virtual void onConnectedConfirmed(InviteSessionHandle, const SipMessage &msg);
	//virtual void onStaleCallTimeout(ClientInviteSessionHandle h);
	//virtual void terminate(ClientInviteSessionHandle h);
	
	virtual void onTerminated(InviteSessionHandle, InviteSessionHandler::TerminatedReason reason, const SipMessage* related=0);
	virtual void onForkDestroyed(ClientInviteSessionHandle);
	virtual void onRedirected(ClientInviteSessionHandle, const SipMessage& msg);
	//virtual void onReadyToSend(InviteSessionHandle, SipMessage& msg);
	virtual void onAnswer(InviteSessionHandle, const SipMessage& msg, const SdpContents&);
	//virtual void onAnswer(InviteSessionHandle, const SipMessage& msg, const Contents&);
	virtual void onOffer(InviteSessionHandle, const SipMessage& msg, const SdpContents&);      
	//virtual void onOffer(InviteSessionHandle, const SipMessage& msg, const Contents&);      
	//virtual void onRemoteSdpChanged(InviteSessionHandle, const SipMessage& msg, const SdpContents&);
	//virtual void onRemoteAnswerChanged(InviteSessionHandle, const SipMessage& msg, const Contents&);  
	//virtual void onOfferRequestRejected(InviteSessionHandle, const SipMessage& msg);
	virtual void onOfferRequired(InviteSessionHandle, const SipMessage& msg);      
	virtual void onOfferRejected(InviteSessionHandle, const SipMessage* msg);
	virtual void onInfo(InviteSessionHandle, const SipMessage& msg);
	virtual void onInfoSuccess(InviteSessionHandle, const SipMessage& msg);
	virtual void onInfoFailure(InviteSessionHandle, const SipMessage& msg);
	virtual void onMessage(InviteSessionHandle, const SipMessage& msg);
	virtual void onMessageSuccess(InviteSessionHandle, const SipMessage& msg);
	virtual void onMessageFailure(InviteSessionHandle, const SipMessage& msg);
	virtual void onRefer(InviteSessionHandle, ServerSubscriptionHandle, const SipMessage& msg);
	virtual void onReferNoSub(InviteSessionHandle, const SipMessage& msg);
	virtual void onReferRejected(InviteSessionHandle, const SipMessage& msg);
	virtual void onReferAccepted(InviteSessionHandle, ClientSubscriptionHandle, const SipMessage& msg);
	//virtual void onAckReceived(InviteSessionHandle, const SipMessage& msg);
	//virtual void onAckNotReceived(InviteSessionHandle);
	//virtual void onStaleReInviteTimeout(InviteSessionHandle h);
	//virtual void onIllegalNegotiation(InviteSessionHandle, const SipMessage& msg);     
	//virtual void onSessionExpired(InviteSessionHandle);
	
	// ****************	
	// Callback methods declared in ClientRegistrationHandler
	virtual void onSuccess(ClientOutOfDialogReqHandle, const SipMessage& successResponse);
	virtual void onFailure(ClientOutOfDialogReqHandle, const SipMessage& errorResponse);
	virtual void onReceivedRequest(ServerOutOfDialogReqHandle, const SipMessage& request);
};

#endif // #ifndef __Q_SIP_CALLBACK_HANDLER_HPP__
