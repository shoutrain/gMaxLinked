/*
 ============================================================================
 Name        : CIniReader.cpp
 Author      : Rafael Gu
 Version     : 1.0
 Copyright   : GPL
 Description : ini file reader
 ============================================================================
 */

#include "CIniReader.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

CIniReader::CIniReader(const char *fileName) {
	if (NULL == fileName || 0 == fileName[0]) {
		log_fatal("configure file name is not right");
		exit(0);
	}

	FILE *file = fopen(fileName, "r");

	if (NULL == file) {
		log_fatal("configure file is not exist");
		exit(0);
	}

	unsigned int i = 0;

	do {
		_buffer[i] = fgetc(file);

		if ((char) EOF == _buffer[i]) {
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

CIniReader::~CIniReader() {
}

int CIniReader::isNewline(char c) {
	return ('\n' == c || '\r' == c) ? 1 : 0;
}

int CIniReader::isEndOfString(char c) {
	return '\0' == c ? 1 : 0;
}

int CIniReader::isLeftBarce(char c) {
	return '[' == c ? 1 : 0;
}

int CIniReader::isRightBrace(char c) {
	return ']' == c ? 1 : 0;
}

int CIniReader::parseFile(const char *section, const char *key, int *secS,
		int *secE, int *keyS, int *keyE, int *valueS, int *valueE) {
	int i = 0;

	*secE = *secS = *keyE = *keyS = *valueS = *valueE = -1;

	while (!isEndOfString(_buffer[i])) {
		// find the section
		if ((0 == i || isNewline(_buffer[i - 1])) && isLeftBarce(_buffer[i])) {
			int section_start = i + 1;

			// find the ']'
			do {
				i++;
			} while (!isRightBrace(_buffer[i]) && !isEndOfString(_buffer[i]));

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

				while (!(isNewline(_buffer[i - 1]) && isLeftBarce(_buffer[i]))
						&& !isEndOfString(_buffer[i])) {
					//get a new line
					int newlineStart = i;
					int j = i;

					while (!isNewline(_buffer[i]) && !isEndOfString(_buffer[i])) {
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

int CIniReader::readString(const char *section, const char *key, char *value,
		unsigned int size) {
	if (NULL == section || 0 == section[0] || NULL == key || 0 == key[0]
			|| NULL == value || 0 == size) {
		return 1;
	}

	if (0 == _bufferSize) {
		return 2;
	}

	int secS, secE, keyS, keyE, valueS, valueE;

	if (!parseFile(section, key, &secS, &secE, &keyS, &keyE, &valueS,
			&valueE)) {
		unsigned int length = valueE - valueS;

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

int CIniReader::readByte(const char *section, const char *key,
		unsigned char *value) {
	char val[32] = { 0 };
	int ret = readString(section, key, val, 32);

	if (0 != ret) {
		return ret;
	}

	*value = (unsigned char) atoi(val);

	return 0;
}

int CIniReader::readShort(const char *section, const char *key,
		unsigned short *value) {
	char val[32] = { 0 };
	int ret = readString(section, key, val, 32);

	if (0 != ret) {
		return ret;
	}

	*value = (unsigned short) atoi(val);

	return 0;
}

int CIniReader::readInt(const char *section, const char *key,
		unsigned int *value) {
	char val[32] = { 0 };
	int ret = readString(section, key, val, 32);

	if (0 != ret) {
		return ret;
	}

	*value = atoi(val);

	return 0;
}
