#ifndef __HTTP_SETTINGS_H__
#define __HTTP_SETTINGS_H__

#include <iostream>
#include <map>
#include <string>
#include "http_parser.h"
#include "deal_http_msg.h"


class deal_http_msg;


size_t get_end_pos(const char *src, size_t maxlen);

size_t strlncat(char *dst, size_t dst_len, const char *src, size_t len);

int message_begin_cb(http_parserc * hparser);

int request_url_cb(http_parserc *hparser, const char *buf, size_t len);

int status_cb(http_parserc *hparser);

int header_field_cb(http_parserc *hparser, const char *buf, size_t len);

int header_value_cb(http_parserc *hparser, const char *buf, size_t len);

int headers_complete_cb(http_parserc *hparser);

int body_cb(http_parserc *hparser, const char *buf, size_t len);

int message_complete_cb(http_parserc *hparser);

extern http_parser_settings settings;
#endif