#pragma once

//#include <gzstream.h>

enum optValue {
	oPROGR,
	oFRAG_LEN,
	oSPACE,
	oTIME,
	oHELP
};

typedef USHORT wigval;

class WigReg
{
private:
	string		_declLine;		// current declaration line without value of span
	streambuf*	_initStream;	// pointer to initial ostream
	ofstream	_outFile;
	bool	_empty;				// true if no value is added
	//ogzstream	_outzFile;

	// Replaces file name and correct description
	void		CorrectDef(const char* line, const char* fName, BYTE space);
	// Outputs declaration line
	inline void	PrintDeclLine(chrlen span)		{ cout << _declLine << span << EOL; }
	// Outputs data line
	inline void	PrintLine(chrlen pos, int val)	{ cout << pos << TAB << val << EOL; _empty = false; }
	// Outputs declaration and data line
	inline void PrintRecord(chrlen pos, chrlen span, int val) {
		PrintDeclLine(span);
		PrintLine(pos, val);
	}
public:
	WigReg(const char* inFileName, const char* outFileName);

	~WigReg() {
		if( _initStream ) {
			cout.rdbuf(_initStream);
			if( _outFile.is_open() )	_outFile.close();
		}
	}
};
