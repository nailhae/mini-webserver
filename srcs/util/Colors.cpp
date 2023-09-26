#include "Colors.hpp"

const std::string Colors::Reset = "\033[0m";
const std::string Colors::Red = "\033[31m";
const std::string Colors::Green = "\033[32m";
const std::string Colors::Yellow = "\033[33m";
const std::string Colors::Blue = "\033[34m";
const std::string Colors::Magenta = "\033[35m";
const std::string Colors::Cyan = "\033[36m";
const std::string Colors::BoldRed = "\033[1m\033[31m";
const std::string Colors::BoldGreen = "\033[1m\033[32m";
const std::string Colors::BoldYellow = "\033[1m\033[33m";
const std::string Colors::BoldBlue = "\033[1m\033[34m";
const std::string Colors::BoldMagenta = "\033[1m\033[35m";
const std::string Colors::BoldCyan = "\033[1m\033[36m";
const std::string Colors::BoldWhite = "\033[1m\033[37m";

std::string Colors::RedString(std::string target)
{
	return (Red + target + Reset);
}

std::string Colors::GreenString(std::string target)
{
	return (Green + target + Reset);
}

std::string Colors::YellowString(std::string target)
{
	return (Yellow + target + Reset);
}

std::string Colors::BlueString(std::string target)
{
	return (Blue + target + Reset);
}

std::string Colors::MagentaString(std::string target)
{
	return (Magenta + target + Reset);
}

std::string Colors::CyanString(std::string target)
{
	return (Cyan + target + Reset);
}

std::string Colors::BoldWhiteString(std::string target)
{
	return (BoldWhite + target + Reset);
}

std::string Colors::BoldRedString(std::string target)
{
	return (BoldRed + target + Reset);
}

std::string Colors::BoldGreenString(std::string target)
{
	return (BoldGreen + target + Reset);
}

std::string Colors::BoldYellowString(std::string target)
{
	return (BoldYellow + target + Reset);
}

std::string Colors::BoldBlueString(std::string target)
{
	return (BoldBlue + target + Reset);
}

std::string Colors::BoldMagentaString(std::string target)
{
	return (BoldMagenta + target + Reset);
}

std::string Colors::BoldCyanString(std::string target)
{
	return (BoldCyan + target + Reset);
}
