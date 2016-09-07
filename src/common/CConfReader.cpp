/*
 ============================================================================
 Name        : CIniReader.cpp
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : ini file reader
 ============================================================================
 */

#include "CConfReader.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>


CConfReader::CConfReader(const c1_ *fileName) {
	if (null_v == fileName || 0 == fileName[0]) {
		log_fatal("configure file name is not right");
		exit(0);
	}

	FILE *file = fopen(fileName, "r");

	if (null_v == file) {
		log_fatal("configure file is not exist");
		exit(0);
	}

	ub4_ i = 0;

	do {
		_buffer[i] = fgetc(file);

		if ((c1_) EOF == _buffer[i]) {
			break;
		} else {
			i++;
		}
	} while (i < MAX_CONFIG_FILE_SIZE);

	if (MAX_CONFIG_FILE_SIZE <= i) {
		fclose(file);
		log_fatal("configure file is too big");
		exit(0);
	}

	_bufferSize = i;
	fclose(file);
}

CConfReader::~CConfReader() {
}

b4_ CConfReader::_isNewline(c1_ c) {
	return ('\n' == c || '\r' == c) ? 1 : 0;
}

b4_ CConfReader::_isEndOfString(c1_ c) {
	return '\0' == c ? 1 : 0;
}

b4_ CConfReader::_isLeftBarce(c1_ c) {
	return '[' == c ? 1 : 0;
}

b4_ CConfReader::_isRightBrace(b1_ c) {
	return ']' == c ? 1 : 0;
}

b4_ CConfReader::_parseFile(const c1_ *section, const c1_ *key, b4_ *secS,
		b4_ *secE, b4_ *keyS, b4_ *keyE, b4_ *valueS, b4_ *valueE) {
	b4_ i = 0;

	*secE = *secS = *keyE = *keyS = *valueS = *valueE = -1;

	while (!_isEndOfString(_buffer[i])) {
		// find the section
		if ((0 == i || _isNewline(_buffer[i - 1]))
				&& _isLeftBarce(_buffer[i])) {
			b4_ section_start = i + 1;

			// find the ']'
			do {
				i++;
			} while (!_isRightBrace(_buffer[i]) && !_isEndOfString(_buffer[i]));

			if (0
					== strncmp(_buffer + section_start, section,
							i - section_start)) {
				i++;

				//Skip over space char after ']'
				while (isspace(_buffer[i])) {
					i++;
				}

				//find the section
				*secS = section_start - 1;
				*secE = i - 1;

				while (!(_isNewline(_buffer[i - 1]) && _isLeftBarce(_buffer[i]))
						&& !_isEndOfString(_buffer[i])) {
					//get a new line
					b4_ newlineStart = i;
					b4_ j = i;

					while (!_isNewline(_buffer[i])
							&& !_isEndOfString(_buffer[i])) {
						i++;
					}

					// now i is equal to the end of the line
					if (';' != _buffer[j]) {
						while (j < i && '=' != _buffer[j]) {
							j++;

							if ('=' == _buffer[j]) {
								if (!strncmp(key, _buffer + newlineStart,
										j - newlineStart)) {
									//find the key ok
									*keyS = newlineStart;
									*keyE = j - 1;

									*valueS = j + 1;
									*valueE = i;

									return 0;
								}
							}
						}
					}

					i++;
				}
			}
		} else {
			i++;
		}
	}

	return 1;
}

b4_ CConfReader::readString(const c1_ *section, const c1_ *key, c1_ *value,
		ub4_ size) {
	if (null_v == section || 0 == section[0] || null_v == key || 0 == key[0]
			|| null_v == value || 0 == size) {
		return 1;
	}

	if (0 == _bufferSize) {
		return 2;
	}

	b4_ secS, secE, keyS, keyE, valueS, valueE;

	if (!_parseFile(section, key, &secS, &secE, &keyS, &keyE, &valueS,
			&valueE)) {
		ub4_ length = valueE - valueS;

		if (size - 1 < length) {
			length = size - 1;
		}

		memcpy(value, _buffer + valueS, length);
		value[length] = 0;
	} else {
		return 3;
	}

	return 0;
}

b4_ CConfReader::readByte(const c1_ *section, const c1_ *key, ub1_ *value) {
	c1_ val[32] = { 0 };
	b4_ ret = readString(section, key, val, 32);

	if (0 != ret) {
		return ret;
	}

	*value = (ub1_) atoi(val);

	return 0;
}

b4_ CConfReader::readShort(const c1_ *section, const c1_ *key, ub2_ *value) {
	c1_ val[32] = { 0 };
	b4_ ret = readString(section, key, val, 32);

	if (0 != ret) {
		return ret;
	}

	*value = (ub2_) atoi(val);

	return 0;
}

b4_ CConfReader::readInt(const c1_ *section, const c1_ *key, ub4_ *value) {
	c1_ val[32] = { 0 };
	b4_ ret = readString(section, key, val, 32);

	if (0 != ret) {
		return ret;
	}

	*value = atoi(val);

	return 0;
}
