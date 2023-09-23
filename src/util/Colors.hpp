#pragma once
#ifndef _COLORS_HPP_
#define _COLORS_HPP_
#include <iostream>
#include <string>

class Colors
{

public:
	static const std::string Reset;
	static const std::string Red;
	static const std::string Green;
	static const std::string Yellow;
	static const std::string Blue;
	static const std::string Magenta;
	static const std::string Cyan;
	static const std::string BoldRed;
	static const std::string BoldGreen;
	static const std::string BoldYellow;
	static const std::string BoldBlue;
	static const std::string BoldMagenta;
	static const std::string BoldCyan;
	static const std::string BoldWhite;
	
	static std::string RedString(std::string target);
	static std::string GreenString(std::string target);
	static std::string YellowString(std::string target);
	static std::string BlueString(std::string target);
	static std::string MagentaString(std::string target);
	static std::string CyanString(std::string target);
	static std::string BoldRedString(std::string target);
	static std::string BoldWhiteString(std::string target);
	static std::string BoldGreenString(std::string target);
	static std::string BoldYellowString(std::string target);
	static std::string BoldBlueString(std::string target);
	static std::string BoldMagentaString(std::string target);
	static std::string BoldCyanString(std::string target);

};

#endif