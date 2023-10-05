#include "ChangeList.hpp"
#include "Error.hpp"
#include "MethodGet.hpp"
#include "UserData.hpp"
#include "dataSet.hpp"

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
	else if (content == "host")
		return (HOST);
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

int UserData::ParseFirstLine(std::string& firstLine)
{
	std::stringstream ss(firstLine);
	std::string line;

	ss >> line;
	if (line == "GET")
		mMethod = new MethodGet(GET);
	else if (line == "HEAD")
		mMethod = new MethodGet(HEAD);
	else if (line == "POST")
		mMethod = new MethodGet(POST);
	else if (line == "DELETE")
		mMethod = new MethodGet(DELETE);
	else
	{
		mMethod = new MethodGet(ERROR);
		mStatusCode = 405;
		mStatusText = "Method is not allowed";
		// 이 경우 헤더에 Allow: GET, POST, DELETE 추가해야 함.
		return (ERROR);
	}
	ss >> mUri;
	std::getline(ss, line);
	if (*(line.end() - 1) == '\r')
		line.erase(line.size() - 1);
	trimWhiteSpace(line);
	if (line != "HTTP/1.1")
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

int UserData::ParseRequest(std::vector<unsigned char>& request)
{
	std::vector<unsigned char>::iterator pos = request.begin();
	std::string line;


	for (std::vector<unsigned char>::iterator it = request.begin(); it != request.end();)
	{
		pos = std::find(pos, request.end(), '\n');
		if (pos == request.end())
			return (false);
		line.assign(it, pos);
		if (it == request.begin() && ParseFirstLine(line) == ERROR)
			return (ERROR);
		if (*(line.end() - 1) == '\r')
			line.erase(line.size() - 1);
		if (line.size() == 0)
			break ;
		else if (ParseOneLine(line) == ERROR)
			return (ERROR);
		else
		{
			pos += 1;
			it = pos;
		}
	}
	mReceived.erase(request.begin(), pos);
	if (mMethod->GetType() == POST)
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
