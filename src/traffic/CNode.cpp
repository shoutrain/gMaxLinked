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
	_group = null_v;
	_recvOffset = 0;
	_recvDueSize = 0;
}

CNode::~CNode() {
}

none_ CNode::onAttach(CNodeGroup *group, const s1_ ip, ub2_ port, b4_ fd) {
	_group = group;
	_recvOffset = 0;
	_recvDueSize = 0;
	strncpy(_ip, ip, Length::IP_V4);
	_port = port;
	_fd = fd;
	_transaction.onAttach();
}

none_ CNode::onDetach() {
	_transaction.onDetach();
	_fd = 0;
	_port = 0;
	memset(_ip, 0, Size::IP_V4);
	_recvOffset = 0;
	_recvDueSize = 0;
	_group = null_v;
}

bool_ CNode::recv() {
	ssize_t n;

	for (;;) {
		if (0 == _recvDueSize) { // parser header
			n = read(_fd, _recvBuffer + _recvOffset,
					sizeof(Message::THeader) - _recvOffset);

			if (0 == n) {
				// the socket has been closed
				return false_v;
			}

			if (0 > n) {
				if (EAGAIN == errno) {
					break;
				}

				_handleRecvErrors();

				return false_v;
			}

			log_debug("[%p %s:%u]CNode::recv: got %lu bytes data",
					&_transaction, _ip, _port, n);
			_recvOffset += n;

			if (sizeof(Message::THeader) == _recvOffset) {
				_recvDueSize = (ub2_) _recvBuffer;

				// maybe the whole PDU is just a header
				if (sizeof(Message::THeader) == _recvDueSize) {
					Message::TMsg *msg = (Message::TMsg *) _recvBuffer;

					header->ext = &_transation;
					_group->putMessage(msg);
					_recvOffset = 0;
					_recvDueSize = 0;
				}
			}
		} else { // parser body and others
			n = read(_fd, _recvBuffer + _recvOffset,
					_recvDueSize - _recvOffset);

			if (0 == n) {
				// the socket has been closed
				return false_v;
			}

			if (0 > n) {
				if (EAGAIN == errno) {
					break;
				}

				_handleRecvErrors();

				return false_v;
			}

			log_debug("[%p %s:%u]CNode::recv: got %lu bytes data",
					&_transaction, _ip, _port, n);
			_recvOffset += n;

			if (_recvDueSize == _recvOffset) {
				Message::TMsg *msg = (Message::TMsg*) _recvBuffer;

				header->ext = &_transation;
				_group->putMessage(msg);
				_recvOffset = 0;
				_recvDueSize = 0;
			}
		}
	}

	return true_v;
}

bool_ CNode::send(const Message::TMsg *msg) {
	msg->ext = 0;

	ssize_t n;
	ub1_ *buffer = (ub1_ *) msg;
	ub4_ offset = 0;

	do {
		n = write(_fd, buffer + offset, msg->size - offset);

		if (0 < n) {
			log_debug("[%p %s:%u]CNode::send: send %lu bytes data",
					&_transaction, _ip, _port, n);
			offset += n;
		} else if (0 == n) {
			// the socket has been closed
			return false_v;
		} else {
			if (EAGAIN == errno) {
				sleep(0, 10);

				continue;
			}

			c1_ error[8] = { 0 };

			switch (errno) {
			case EBADF:
				strncpy(error, "EBADF", sizeof("EBADF"));
				break;
			case EFAULT:
				strncpy(error, "EFAULT", sizeof("EFAULT"));
				break;
			case EFBIG:
				strncpy(error, "EFBIG", sizeof("EFBIG"));
				break;
			case EINTR:
				strncpy(error, "IINTR", sizeof("IINTR"));
				break;
			case EINVAL:
				strncpy(error, "EINVAL", sizeof("EINVAL"));
				break;
			case EIO:
				strncpy(error, "EIO", sizeof("EIO"));
				break;
			case ENOSPC:
				strncpy(error, "ENOSPC", sizeof("ENOSPC"));
				break;
			case EPIPE:
				strncpy(error, "EPIPE", sizeof("EPIPE"));
				break;
			default:
				strncpy(error, "Unknown", sizeof("Unknown"));
				assert(false_v);
				break;
			}

			log_error("[%p %s:%d]CNode::send: error-%s"
					" when sending data.", &_transaction, _ip, _port, error);

			return false_v;
		}
	} while (msg->size > offset);

	return true_v;
}

none_ CNode::_handleRecvErrors() {
	c1_ error[8] = { 0 };

	switch (errno) {
	case EBADF:
		strncpy(error, "EBADF", sizeof("EBADF"));
		break;
	case EFAULT:
		strncpy(error, "EFAULT", sizeof("EFAULT"));
		break;
	case EINTR:
		strncpy(error, "IINTR", sizeof("IINTR"));
		break;
	case EINVAL:
		strncpy(error, "EINVAL", sizeof("EINVAL"));
		break;
	case EIO:
		strncpy(error, "EIO", sizeof("EIO"));
		break;
	case EISDIR:
		strncpy(error, "EISDIR", sizeof("EISDIR"));
		break;
	default:
		strncpy(error, "Unknown", sizeof("Unknown"));
		assert(false_v);
		break;
	}

	log_error("[%p %s:%u]CNode::recv: error-%s", &_transaction, _ip, _port,
			error);
}
