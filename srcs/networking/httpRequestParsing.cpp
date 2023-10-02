#include "ChangeList.hpp"
#include "Error.hpp"
#include "UserData.hpp"

static void trimWhiteSpace(std::string& target)
{
	size_t start = 0;

	while (start < target.size() && isspace(target[start]) == true)
	{
		start += 1;
	}
	target.erase(0, start);

	size_t end = target.size() - 1;
	while (end > 0 && isspace(target[end]) == true)
	{
		end -= 1;
	}
	target.erase(end + 1);
}

static int validHeader(std::string& content)
{
	for (std::string::iterator it = content.begin(); it != content.end(); it++)
	{
		if (isupper(*it) == true)
			*it += 32;
	}
	if (content == "connection")
		return (CONNECTION);
	else if (content == "content-type")
		return (CONTENT_TYPE);
	else if (content == "content-length")
		return (CONTENT_LENGTH);
	else if (content == "cache-control")
		return (CACHE_CONTROL);
	else if (content == "if-none-match")
		return (IF_NONE_MATCH);
	else if (content == "if-modified-since")
		return (IF_MODIFIED_SINCE);
	else
		return (NONE);
}

static int checkValueQuote(std::string& value)
{
	bool quoteFlag = false;
	size_t pos = 0;

	for (std::string::iterator it = value.begin(); it != value.end(); it++)
	{
		if (*it == '\"')
		{
			if (quoteFlag == false)
				quoteFlag = true;
			else
				quoteFlag = false;
		}
	}
	if (quoteFlag == true)
		return (ERROR);
	while ((pos = value.find('\"', pos)) != std::string::npos)
	{
		value.erase(pos, 1);
	}
	return (0);
}

static int checkValidHeaderKey(int headerKey, std::string& value)
{
	if (checkValueQuote(value) == ERROR)
		return (ERROR);
	if (headerKey == CONNECTION)
	{
		if (value != "close")
			value = "keep-alive";
	}
	else if (headerKey == CONTENT_TYPE)
	{
		/* MIME type 체크
			HTTP block의 TYPE map을 순회하고, 발견하지 못한 경우 application/octet-stream 으로 설정
		*/
	}
	else if (headerKey == CONTENT_LENGTH)
	{
		for (std::string::iterator it = value.begin(); it != value.end(); it++)
		{
			if (isdigit(*it) == false)
				return (ERROR);
		}
		// config에서 지정한 파일 크기보다 큰 파일의 경우
		// if (atoi(value.c_str) > max_contents)
		// {
		// 	return (ERROR);
		// }
	}
	else if (headerKey == CACHE_CONTROL)
	{
		if (value.substr(0, 7) != "max-age")
		{
			Error::Print(value.substr(0, 7) + " != max-age");
			return (0);
		}
		value.erase(0, 7);
		if (value.size() == 0 || value[0] != '=')
			return (ERROR);
		value.erase(0, 1);
		for (std::string::iterator it = value.begin(); it != value.end(); it++)
		{
			if (isdigit(*it) == false)
				return (ERROR);
		}
	}
	else if (headerKey == IF_MODIFIED_SINCE)
	{
		/*
			value가 올바른 날짜인지 확인하는 함수가 필요함.
			라이브러리 잘 적용시킬 것.
			보나스 할 때 추가하도록 하자.
		*/
	}
	return (0);
}

int UserData::ParseHeaderValue(int headerKey, std::string& value)
{
	if (headerKey == NONE)
		return (0);
	if (mHeaders[headerKey] != "" || checkValidHeaderKey(headerKey, value) == ERROR)
	{
		mStatusCode = 400;
		mStatusText = "Bad Request";
		return (ERROR);
	}
	else
		mHeaders[headerKey] = value;
	return (0);
}

int UserData::ParseHeaderKey(std::string& header)
{
	int headerKey;

	trimWhiteSpace(header);
	headerKey = validHeader(header);
	if (headerKey == NONE)
		return (NONE);
	else
		return (headerKey);
}

int UserData::ParseFirstLine(std::stringstream& request)
{
	std::string temp;

	request.seekg(std::ios::beg);
	request >> temp;
	if (temp == "GET")
		mMethod = GET;
	else if (temp == "HEAD")
		mMethod = HEAD;
	else if (temp == "POST")
		mMethod = POST;
	else if (temp == "DELETE")
		mMethod = DELETE;
	else
	{
		mMethod = ERROR;
		mStatusCode = 405;
		mStatusText = "Method is not allowed";
		// 이 경우 헤더에 Allow: GET, POST, DELETE 추가해야 함.
		return (ERROR);
	}
	request >> mUri;
	std::getline(request, temp);
	if (*(temp.end() - 1) == '\r')
		temp.erase(temp.size() - 1);
	trimWhiteSpace(temp);
	if (temp != "HTTP/1.1")
	{
		mStatusCode = 505;
		mStatusText = "HTTP Version Not Supported";
		return (ERROR);
	}
	return (0);
}

int UserData::ParseOneLine(std::string& oneLine)
{
	std::string key;
	int headerKey;

	for (std::string::iterator it = oneLine.begin(); it != oneLine.end(); it++)
	{
		if (*it == ':')
		{
			oneLine.erase(oneLine.begin(), it + 1);
			break;
		}
		else
			key += *it;
	}
	trimWhiteSpace(oneLine);
	headerKey = ParseHeaderKey(key);
	if (headerKey == ERROR)
		return (ERROR);
	trimWhiteSpace(oneLine);
	if (oneLine.size() == 0)
		return (ERROR);
	else if (ParseHeaderValue(headerKey, oneLine) == ERROR)
		return (ERROR);
	return (0);
}

int UserData::ParseRequest(std::stringstream& request)
{
	std::string temp;

	if (ParseFirstLine(request) == ERROR)
		return (ERROR);
	while (1)
	{
		std::getline(request, temp, '\n');
		if (*(temp.end() - 1) == '\r')
			temp.erase(temp.size() - 1);
		if (temp.size() == 0)
			break;
		else if (ParseOneLine(temp) == ERROR)
			return (ERROR);
	}
	if (mMethod == POST)
	{
		if (mHeaders[CONTENT_LENGTH] == "" || mHeaders[CONTENT_TYPE] == "")
		{
			mStatusCode = 400;
			mStatusText = "Bad Request";
			return (ERROR);
		}
	}
	return (0);
}
