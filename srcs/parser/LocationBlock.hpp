#ifndef LOCATIONBLOCK_HPP
#define LOCATIONBLOCK_HPP

class LocationBlock
{
public:
	LocationBlock();
	virtual ~LocationBlock();

	LocationBlock(const LocationBlock& other);
	LocationBlock& operator=(const LocationBlock& other);

private:
	// Private members
};

#endif /* LOCATIONBLOCK_HPP */