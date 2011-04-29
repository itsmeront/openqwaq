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
 *  QSipCallbackHandler.cpp
 *  SipPlugin
 *
 */

#include "QSipCallbackHandler.hpp"

#include "qLogger.hpp"


void QSipCallbackHandler::onSuccess(ClientRegistrationHandle, const SipMessage& response)
{
	qLog() << "REGISTER::onSuccess" << flush;
}

void QSipCallbackHandler::onRemoved(ClientRegistrationHandle, const SipMessage& response)
{
	qLog() << "REGISTER::onRemoved" << flush;
}

int QSipCallbackHandler::onRequestRetry(ClientRegistrationHandle, int retrySeconds, const SipMessage& response)
{
	qLog() << "REGISTER::onRetry (FAILING AUTOMATICALLY FOR NOW)" << flush;
	return -1; // fail
}

void QSipCallbackHandler::onFailure(ClientRegistrationHandle, const SipMessage& response)
{
	qLog() << "REGISTER::onFailure" << flush;
}

void QSipCallbackHandler::onNewSession(ClientInviteSessionHandle, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
	qLog() << "INVITE(client)::onNewSession" << flush;
}

void QSipCallbackHandler::onNewSession(ServerInviteSessionHandle, InviteSession::OfferAnswerType oat, const SipMessage& msg)
{
	qLog() << "INVITE(server)::onNewSession" << flush;
}

void QSipCallbackHandler::onFailure(ClientInviteSessionHandle, const SipMessage& msg)
{
	qLog() << "INVITE::onFailure" << flush;
}

void QSipCallbackHandler::onProvisional(ClientInviteSessionHandle, const SipMessage&)
{
	qLog() << "INVITE::onProvisional" << flush;
}

void QSipCallbackHandler::onConnected(ClientInviteSessionHandle, const SipMessage& msg)
{
	qLog() << "INVITE(client)::onConnected" << flush;
}

void QSipCallbackHandler::onConnected(InviteSessionHandle, const SipMessage& msg)
{
	qLog() << "INVITE(server)::onConnected" << flush;
}

void QSipCallbackHandler::onEarlyMedia(ClientInviteSessionHandle, const SipMessage&, const SdpContents&)
{
	qLog() << "INVITE::onEarlyMedia" << flush;
}

void QSipCallbackHandler::onTerminated(InviteSessionHandle, InviteSessionHandler::TerminatedReason reason, const SipMessage* related)
{
	qLog() << "INVITE::onTerminated" << flush;
}

void QSipCallbackHandler::onForkDestroyed(ClientInviteSessionHandle)
{
	qLog() << "INVITE::onForkDestroyed" << flush;
}

void QSipCallbackHandler::onRedirected(ClientInviteSessionHandle, const SipMessage& msg)
{
	qLog() << "INVITE::onRedirected" << flush;
}

void QSipCallbackHandler::onAnswer(InviteSessionHandle, const SipMessage& msg, const SdpContents&)
{
	qLog() << "INVITE::onAnswer" << flush;
}

void QSipCallbackHandler::onOffer(InviteSessionHandle, const SipMessage& msg, const SdpContents&)
{
	qLog() << "INVITE::onOffer" << flush;
}
      
void QSipCallbackHandler::onOfferRequired(InviteSessionHandle, const SipMessage& msg)
{
	qLog() << "INVITE::onOfferRequired" << flush;
}
      
void QSipCallbackHandler::onOfferRejected(InviteSessionHandle, const SipMessage* msg)
{
	qLog() << "INVITE::onOfferRejected" << flush;
}

void QSipCallbackHandler::onInfo(InviteSessionHandle, const SipMessage& msg)
{
	qLog() << "INVITE::onInfo" << flush;
}

void QSipCallbackHandler::onInfoSuccess(InviteSessionHandle, const SipMessage& msg)
{
	qLog() << "INVITE::onInfoSuccess" << flush;
}

void QSipCallbackHandler::onInfoFailure(InviteSessionHandle, const SipMessage& msg)
{
	qLog() << "INVITE::onInfoFailure" << flush;
}

void QSipCallbackHandler::onMessage(InviteSessionHandle, const SipMessage& msg)
{
	qLog() << "INVITE::onMessage" << flush;
}

void QSipCallbackHandler::onMessageSuccess(InviteSessionHandle, const SipMessage& msg)
{
	qLog() << "INVITE::onMessageSuccess" << flush;
}

void QSipCallbackHandler::onMessageFailure(InviteSessionHandle, const SipMessage& msg)
{
	qLog() << "INVITE::onMessageFailure" << flush;
}

void QSipCallbackHandler::onRefer(InviteSessionHandle, ServerSubscriptionHandle, const SipMessage& msg)
{
	qLog() << "INVITE::onRefer" << flush;
}

void QSipCallbackHandler::onReferNoSub(InviteSessionHandle, const SipMessage& msg)
{
	qLog() << "INVITE::onReferNoSub" << flush;
}

void QSipCallbackHandler::onReferRejected(InviteSessionHandle, const SipMessage& msg)
{
	qLog() << "INVITE::onReferRejected" << flush;
}

void QSipCallbackHandler::onReferAccepted(InviteSessionHandle, ClientSubscriptionHandle, const SipMessage& msg)
{
	qLog() << "INVITE::onReferAccepted" << flush;
}

void QSipCallbackHandler::onSuccess(ClientOutOfDialogReqHandle, const SipMessage& successResponse)
{
	qLog() << "OUT-OF-DIALOG(client)::onSuccess" << flush;
}

void QSipCallbackHandler::onFailure(ClientOutOfDialogReqHandle, const SipMessage& errorResponse)
{
	qLog() << "OUT-OF-DIALOG(client)::onFailure" << flush;
}

void QSipCallbackHandler::onReceivedRequest(ServerOutOfDialogReqHandle, const SipMessage& request)
{
	qLog() << "OUT-OF-DIALOG(server)::onReceivedRequest" << flush;
}

