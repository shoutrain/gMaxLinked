/*
 ============================================================================
 Name        : CNode.cpp
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description :
 ============================================================================
 */

#include "CNode.h"
#include "../config/Config.h"

#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "CNodeGroup.h"

CNode::CNode() :
		_transaction(this) {
	memset(_ip, 0, Size::IP_V4);
	_port = 0;
	_fd = 0;
	_group = NULL;
	_recvOffset = 0;
}

CNode::~CNode() {
}

void CNode::onAttach(CNodeGroup *group, const char *ip, unsigned short port,
		int fd) {
	_group = group;
	_recvOffset = 0;
	strncpy(_ip, ip, Length::IP_V4);
	_port = port;
	_fd = fd;
	_transaction.onAttach();
}

void CNode::onDetach() {
	_transaction.onDetach();
	_fd = 0;
	_port = 0;
	memset(_ip, 0, Size::IP_V4);
	_recvOffset = 0;
	_group = NULL;
}

bool CNode::recv() {
	ssize_t n;

	for (;;) {
		n = read(_fd, _recvBuffer + _recvOffset,
				Message::MSG_FIXED_LENGTH - _recvOffset);

		if (0 < n) {
			log_debug("[%p %s:%u]CNode::recv: got %lu bytes data",
					&_transaction, _ip, _port, n);

			_recvOffset += n;

			if (Message::MSG_FIXED_LENGTH == _recvOffset) {
				Message::TMsg *msg = (Message::TMsg *) _recvBuffer;

				msg->extra.transaction = (long unsigned int) &_transaction;
				getGroup()->putMessage(msg);
				_recvOffset = 0;
			}
		} else if (0 == n) {
			// the socket has been closed
			return false;
		} else {
			if (EAGAIN == errno) {
				break;
			}

			char error[8] = { 0 };

			switch (errno) {
			case EBADF:
				strncpy(error, "EBADF", 6);
				break;
			case EFAULT:
				strncpy(error, "EFAULT", 7);
				break;
			case EINTR:
				strncpy(error, "IINTR", 6);
				break;
			case EINVAL:
				strncpy(error, "EINVAL", 7);
				break;
			case EIO:
				strncpy(error, "EIO", 4);
				break;
			case EISDIR:
				strncpy(error, "EISDIR", 7);
				break;
			default:
				strncpy(error, "Unknown", 7);
				assert(false);
				break;
			}

			log_error("[%p %s:%u]CNode::recv: error-%s", &_transaction, _ip,
					_port, error);

			return false;
		}

	}

	return true;
}

bool CNode::send(const Message::TMsg *msg) {
	ssize_t n;
	unsigned char *buffer = (unsigned char *) msg;
	unsigned int offset = 0;

	do {
		n = write(_fd, buffer + offset, Message::MSG_FIXED_LENGTH - offset);

		if (0 < n) {
			log_debug("[%p %s:%u]CNode::send: send %lu bytes data",
					&_transaction, _ip, _port, n);
			offset += n;
		} else if (0 == n) {
			// the socket has been closed
			return false;
		} else {
			if (EAGAIN == errno) {
				sleep(0, 10);

				continue;
			}

			char error[8] = { 0 };

			switch (errno) {
			case EBADF:
				strncpy(error, "EBADF", 6);
				break;
			case EFAULT:
				strncpy(error, "EFAULT", 7);
				break;
			case EFBIG:
				strncpy(error, "EFBIG", 6);
				break;
			case EINTR:
				strncpy(error, "IINTR", 6);
				break;
			case EINVAL:
				strncpy(error, "EINVAL", 7);
				break;
			case EIO:
				strncpy(error, "EIO", 4);
				break;
			case ENOSPC:
				strncpy(error, "ENOSPC", 7);
				break;
			case EPIPE:
				strncpy(error, "EPIPE", 6);
				break;
			default:
				strncpy(error, "Unknown", 7);
				assert(false);
				break;
			}

			log_error("[%p %s:%d]CNode::send: error-%s"
					" when sending data.", &_transaction, _ip, _port, error);

			return false;
		}
	} while (Message::MSG_FIXED_LENGTH > offset);

	return true;
}
