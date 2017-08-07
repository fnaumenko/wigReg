/*
 * wigRegulation
 * Copyright (C) 2015 Fedor Naumenko
*/
#include "def.h"
#include "common.h"
#include "TxtFile.h"
#include <fstream>
#include "wigReg.h"

using namespace std;

const string Product::Title = "wigReg";
const string Product::Version = "1.0";
const string Product::Descr = "Regulates wiggle format from MACS and PeakFanger";

const string progTip = "program-source";
const string progDescr = progTip + " generated input wiggle:\nPeakRanger, MACS or AUTOdetection";

const string FileIn = "input.wig";
const string FileOut = "output.wig";
const string StdOut = "stdout";

/***** define Options enums and structures *****/

enum eOptGroup	{ oOPTION };	// oOTHER should be the last 
const char* Options::_OptGroups [] = { "Options" };
const BYTE	Options::_GroupCount = oOPTION + 1;

enum eOptProg	{ oPR, oMACS, oAUTO };
const char* ProgVals [] = { "PR", "MACS", "AUTO" };

//{ char, str, optOblig, valRequired, type, group, numVal, minVal, maxVal, strVal, descr }
Options::Option Options::_Options [] = {
	{ 'p', NULL,	 0,	true, tENUM,  oOPTION, float(oAUTO), 0, 3, (char*)ProgVals,
		progDescr.c_str() },
	{ 'f',"frag-len",0,	true, tINT,	oOPTION, 200, 50, 400, NULL, "length of fragment. Ignored for wiggle from MACS" },
	{ 's',"space",	 0, true, tINT,	oOPTION, 10, 1, 100, NULL, "resolution: minimal span in bps from which raw tags will be saved.\nIgnored for wiggle from MACS" },
	{ 't', "time",	 0,	false,tENUM,  oOPTION, FALSE, 0, 2, NULL, "output run time" },
	{ 'h', "help",	 0,	false,tHELP, oOPTION, vUNDEF, 0, 0, NULL, "print usage information and quit" }
};

// -z,--gzip	compress the output

const BYTE	Options::_OptCount = oHELP + 1;
const BYTE	Options::_UsageCount = 2;

const Options::Usage Options::_Usages[] = {
	{ vUNDEF, sBLANK + FileIn + sBLANK + FileOut},
	{ vUNDEF, sBLANK + FileIn + sBLANK + StdOut	}
};

ofstream outfile;				// file ostream duplicated cout; inizialised by file in code
//dostream dout(cout, outfile);	// stream's duplicator

/*****************************************/

int main(int argc, char* argv[])
{
	if( argc == 1 )	
	{ Options::PrintUsage(false);	return 0; }		// output tip
	short fileInd = Options::Tokenize(argc, argv, "files");
	if( fileInd < 0 )				return 1;	// wrong otpion
	// check if output file is setting
	if( fileInd == argc-1 )
		Err(Err::P_MISSED, StrEmpty, FileOut).Throw(false);
	int ret = 0;	// main() return code

	Timer timer(Options::GetBVal(oTIME));
	timer.Start();
	try { 
		WigReg wig(*(argv + fileInd), *(argv + fileInd + 1));
	}
	catch(Err &e)				{ ret = 1;	cout << e.what() << EOL; }
	catch(const exception &e)	{ ret = 1;	cout << e.what() << EOL; }
	catch(...)					{ ret = 1;	cout << "Unregistered error\n"; }
	timer.Stop(true);
//#ifdef OS_Windows
	//system("pause");
//#endif
	return ret;
}

/************************ class Wig ************************/

#define DQUOT	'"'
const char* kyeTrack	= "track type=wiggle_0";
const char* keyStep		= "variableStep";
const char* keyChrom	= "chrom=";
const char* keySpan		= "span=";
const char* keyName		= "name=";
const char* keyDescr	= "description=";
const char* keySpace	= "space=";
const char* progSpec	= "regulated";
const string sRecords	= "records";

const char* sPR		= "PeakRanger";
const char* sMACS	= "MACS";

// Returns a pointer to the substring defined by key.
//	@str: null-terminated string to search
//	@key: null-terminated string to search for
//	return: a pointer to the substring after key, or NULL if key does not appear in str
const char* KeyStr(const char* str, const char* key) {
	const char* strKey = strstr(str, key);
	return strKey ? (strKey + strlen(key)) : NULL;
}

// Checks definition or declaration line for key
//	return: point to substring followed after the key
const char* CheckSpec(const char* line, const char* key, const TabFile & file) 
{
	const char* strKey = KeyStr(line, key);
	if( !strKey )
		Err(string("wrong wig format: absent '") + key + "' definition" ,
		file.RecordNumbToStr()).Throw();
	return strKey;
}

// Sets program-source, reading from line, to paraneter
//	@line: checked line
//	@prog: point to variable to set a program-source value
//	return: true if program-source value is set
bool SetProg(const char* line, BYTE* prog)
{
	if( *prog == oAUTO ) {
		if( strstr(line, sPR) )		{ *prog = oPR;	return true; }
		if( strstr(line, sMACS) )	{ *prog = oMACS;return true; }
		return false;
	}
	return true;
}

// Replaces file name and correct description
void WigReg::CorrectDef(const char* line, const char* fName, BYTE space)
{
	_outFile << kyeTrack << BLANK << keyName
			 << DQUOT << FS::ShortFileName(fName) << DQUOT;
	const char* sstr = KeyStr(line, keyDescr);
	if( sstr ) {	// description exists
		const char* endDescr = strchr(sstr+1, DQUOT);	// str after end of descr
		_outFile<< BLANK << keyDescr
				<< string(sstr, endDescr-sstr)		// descr without last quote	
				<< MSGSEP_BLANK << progSpec			// ': regulated'
				<< BLANK << keySpace << BSTR(space)	// space
				<< endDescr;						// str after last quote
	}
	else {			// description doesn't exist
		_outFile<< BLANK << keyDescr
				<< DQUOT << progSpec << BLANK << keySpace << BSTR(space) << DQUOT;	// descr
		sstr = strrchr(line, DQUOT);
		if( sstr && strlen(line) > size_t(sstr - line) )
			_outFile << ++sstr;						// str after last quote
	}
	_outFile << EOL;
}

WigReg::WigReg(const char* inFileName, const char* outFileName) : _empty(true)
{
	FS::CheckFileExist(inFileName);
	TabFile file(inFileName, 2, true, TxtFile::READ, NULL, '\0', false);
	if( !file.Length() )
		Err(Err::TF_EMPTY, inFileName, sRecords).Throw();

	// set outstream
	if( _stricmp(outFileName, StdOut.c_str()) ) {
		_outFile.open (outFileName, ios_base::out | ios_base::trunc );
		_initStream = cout.rdbuf(_outFile.rdbuf());
	}
	else 
		_initStream = NULL;

	chrid cID = Chrom::UnID;	// current readed chromosome
	const char* line;			// current readed line
	const char* defLine = NULL;	// definition line
	const char* sSpan;			// pointer to the substring - span value
	chrlen	pos, newPos = 0,	// current line's, new line's positions
			startPos = 0,		// current writing region's position
			posDiff,
			spanCnt = 1,		// current span counter (for the same values); for MACS only
			prevSpanCnt = 0,	// previous span count: needs to unit lines
								// with different values but with the same span;  for MACS only
			span,				// current declarative span
			prevSpan,			// previous declarative span; for PR only
			fragSize = Options::GetIVal(oFRAG_LEN);
	//chrlen*	pnewPos = &newPos;	// for case space > 1 only
	wigval	val, newVal = 0;	// current, new readed values
	bool firstLine = true;
	BYTE	prog = Options::GetIVal(oPROGR),
			space = Options::GetIVal(oSPACE);

	if(space > 1)		fragSize = AlignPos(fragSize, space, 0);

	while( line = file.GetLine() )	{
		if( firstLine )
			if( line[0] == HASH )		// comment line: typical at PeakRenger wiggle
				SetProg(line, &prog);
			else {						// definition line
				CheckSpec(line, kyeTrack, file);
				if( KeyStr(line, progSpec) )
					Err("is " + string(progSpec) + " already", inFileName).Throw();
				if( !SetProg(line, &prog) )
					Err("can not to recognize a "+progTip, inFileName).Throw();
				if( _initStream	)	// if write to file
					if(prog == oPR)
						CorrectDef(line, outFileName, space);	// write definition line now
					else
						defLine = line;		// postpone writing definition line to read a space
				firstLine = false;
			}
		else if( isdigit(line[0]) ) {	// data line
			pos = newPos;
			val = newVal;
			newPos = file.IntField(0);
			newVal = abs(file.IntField(1));		// abs() in case of PeakRanger negative strand
			if( pos )		// second data line for current chromosome
				if( prog == oMACS ) {
					if( newPos-pos == span	&& val == newVal ) {
						spanCnt++;		// accumulate span for given val
						continue;
					}
					if( prevSpanCnt != spanCnt )	// change accumulative span
						PrintDeclLine((prevSpanCnt = spanCnt) * span);
					PrintLine(startPos, val);
					startPos = newPos;
					spanCnt = 1;
				}
				else {		// PEAKRANGER
					if(space > 1)		newPos = AlignPos(newPos, space, 1);
					posDiff = newPos-pos;
					if(posDiff > span)
						PrintDeclLine(prevSpan = min(posDiff, fragSize)); // fill "gap"
					else {
						if( !posDiff ) {	// possible if space > 1:
							// just skip this line,
							// but save a maximum value between this and next lines
							if(val > newVal)	newVal = val;
							continue;
						}
						PrintDeclLine(prevSpan = span);	// write single record
					}
					PrintLine(pos, val);
				}
			else {		// first data line for current chromosome
				if(space > 1)		newPos = AlignPos(newPos, space, 1);
				startPos = newPos;
			}
		}
		else {							// declaration line
			CheckSpec(line, keyStep, file);
			chrid newcID = Chrom::IDbyAbbrName(CheckSpec(line, keyChrom, file));
			if( cID != Chrom::UnID && newcID != cID ) {	// new chromosome
				// last data line for current chromosome
				if( prog == oMACS )	PrintRecord(startPos, spanCnt*span, val);
				else				PrintRecord(newPos, fragSize, val);
				pos = newPos = 0;
			}
			cID = newcID;
			startPos = pos;
			sSpan = KeyStr(line, keySpan);
			if( sSpan ) {
				prevSpan = span = atoi(sSpan);
				_declLine = string(line, sSpan-line);
			}
			else {	// absent keySpan: set by default
				prevSpan = span = 1;
				_declLine = string(line) + sBLANK + keySpan;
			}
			if( defLine ) {	// delayed writing definition line; for MACS only
				CorrectDef(defLine, outFileName, span);
				defLine = NULL;
			}
		}
	}
	if( file.IsBad() )	// may be wrong Line
		Err(file.ErrCode(), file.RecordNumbToStr()).Throw();
	// last data line
	if( !_empty) 
		if( prog == oMACS )	PrintRecord(startPos, spanCnt*span, val);
		else				PrintRecord(newPos, fragSize, val);
}

/************************ end of class Wig ************************/
