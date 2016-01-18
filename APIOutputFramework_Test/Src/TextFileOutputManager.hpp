#ifndef __TEXTFILEOUTPUTMANAGER_HPP__
#define __TEXTFILEOUTPUTMANAGER_HPP__

#include "FileOutputManager.hpp"
#include "TextOProtocol.hpp"


class TextFileOutputManager : public GSAPI::FileOutputManager
{
private:
    TextFileOutputManager (const TextFileOutputManager& manager); // disabled
    TextFileOutputManager& operator= (const TextFileOutputManager& manager);  // disabled

public:
	TextFileOutputManager (const IO::Location& location) : GSAPI::FileOutputManager (location) { outFile.SetOutputProtocol (GS::textOProtocol); }
    virtual ~TextFileOutputManager (void);
};

#endif	// __TEXTFILEOUTPUTMANAGER_HPP__
