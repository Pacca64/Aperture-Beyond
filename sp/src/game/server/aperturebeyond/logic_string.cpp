//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ========
//
// Purpose: Simple logical entity that counts up to a threshold value, then
//			fires an output when reached.
//
//=============================================================================

#include "cbase.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include<cmath>

class CLogicStringEntity : public CLogicalEntity
{
public:
	DECLARE_CLASS(CLogicStringEntity, CLogicalEntity);
	DECLARE_DATADESC();

	// Constructor
	CLogicStringEntity()
	{

	}

	void Activate(void);

	// Input function
	void InputPrintString(inputdata_t &inputData);
	void InputGetString(inputdata_t &inputData);
	void InputSetString(inputdata_t &inputData);
	void InputAddToString(inputdata_t &inputData);
	void InputAddToFrontOfString(inputdata_t &inputData);
	void InputAddSpaceToString(inputdata_t &inputData);
	void InputAddSpaceToFrontOfString(inputdata_t &inputData);
	void InputIncrementNumber(inputdata_t &inputData);
	void InputAddToNumber(inputdata_t &inputData);
	void InputGiveStringNumber(inputdata_t &inputData);
	void InputRemoveStringNumber(inputdata_t &inputData);
	void InputSetNumber(inputdata_t &inputData);
	void InputFindReplace(inputdata_t &inputData);
	void InputReplaceControlCharacters(inputdata_t &inputData);
	void InputSetTrailingZeroes(inputdata_t &inputData);

	bool FindAndReplace(std::string toFind, std::string toReplace);

	void FindControlCharacters();

private:

	bool m_HasNumber;	//If false, appended number never appears. If True, counter value is appended to end of string.
	int	m_nCounter;	// Internal counter. Used to append numbers to the end of strings.
	int	m_nTrailingZeroes;	// How many trailing zeroes the number should have in front of it. 
	const char *m_string;	//Old string variable. Avoid using this!!!
	std::string utlString;			//A buffer value that can be used to concatenate and modify strings. MUST BE SAVED OR STRING POINTER WILL BREAK!!!
	std::string outputString;	//String object for outgoing strings, used for PrintString and GetString. Not saving this causes the string data to be lost!

	COutputString	m_GetString;	//Output event for when something wants our string output.
};

LINK_ENTITY_TO_CLASS(logic_string, CLogicStringEntity);

// Start of our data description for the class
BEGIN_DATADESC(CLogicStringEntity)

// For save/load
DEFINE_KEYFIELD(m_nCounter, FIELD_INTEGER, "initcounter"),

// For save/load
DEFINE_KEYFIELD(m_nTrailingZeroes, FIELD_INTEGER, "trailingzeroes"),

// Links our member variable to our keyvalue from Hammer
DEFINE_KEYFIELD(m_string, FIELD_STRING, "initstring"),

// Links our member variable to our keyvalue from Hammer
DEFINE_KEYFIELD(m_HasNumber, FIELD_BOOLEAN, "hasnumber"),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_VOID, "PrintString", InputPrintString),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_VOID, "GetString", InputGetString),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_STRING, "SetString", InputSetString),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_STRING, "AddToString", InputAddToString),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_STRING, "AddToFrontOfString", InputAddToFrontOfString),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_STRING, "AddSpaceToString", InputAddSpaceToString),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_STRING, "AddSpaceToFrontOfString", InputAddSpaceToFrontOfString),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_VOID, "IncrementNumber", InputIncrementNumber),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_INTEGER, "AddToNumber", InputAddToNumber),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_VOID, "GiveStringNumber", InputGiveStringNumber),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_VOID, "RemoveStringNumber", InputRemoveStringNumber),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_INTEGER, "SetNumber", InputSetNumber),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_STRING, "FindReplace", InputFindReplace),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_VOID, "ReplaceControlCharacters", InputReplaceControlCharacters),

// Links our input name from Hammer to our input member function
DEFINE_INPUTFUNC(FIELD_INTEGER, "SetTrailingZeroes", InputSetTrailingZeroes),

// Links our output member to the output name used by Hammer
DEFINE_OUTPUT(m_GetString, "OutString"),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Parse control characters on spawn, should parse any
//			setup in hammer.
//-----------------------------------------------------------------------------
void CLogicStringEntity::Activate(){
	BaseClass::Activate();

	//For some reason, the game crashes when this object is handed an empty string
	//this tries to circumvent that by ensuring we have a string with at least 1 character.
	if (std::string(m_string).empty()){
		m_string = "  ";	
	}

	utlString = std::string(m_string);
	FindControlCharacters();
}

//-----------------------------------------------------------------------------
// Purpose: Print the current string for debugging purposes
//-----------------------------------------------------------------------------
void CLogicStringEntity::InputPrintString(inputdata_t &inputData)
{
	if (m_HasNumber){
		std::string number = std::to_string(m_nCounter);

		int loopCounter = m_nTrailingZeroes;		//initialize loop counter to check for number of zeroes.
		int powerOfTen = std::pow(10, m_nTrailingZeroes);	//get power of ten for our trailing zeroes. 1 is 10, 2 is 100, etc.

		//This loop adds 0s to the front of our number string.
		while (loopCounter > 0){
			if (m_nCounter < powerOfTen){	//if number counter is greater then our power of ten (for 1 zero, if counter < 10)...
				number = std::string("0") + number;	//add a zero to the front of it
			}

			std::sqrt(powerOfTen);	//sqrt the power of ten (if starting from 2 zeroes, convert from 100 to 10)
			loopCounter -= 1;
		}

		outputString = utlString + number;
		ConMsg(outputString.c_str());
		ConMsg("\n");
	} else {
		ConMsg(utlString.c_str());
		ConMsg("\n");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Output the current string through the "GetString" output.
//-----------------------------------------------------------------------------
void CLogicStringEntity::InputGetString(inputdata_t &inputData)
{
	if (m_HasNumber){
		std::string numberString = std::to_string(m_nCounter);

		int loopCounter = m_nTrailingZeroes;		//initialize loop counter to check for number of zeroes.
		int powerOfTen = std::pow(10, m_nTrailingZeroes);	//get power of ten for our trailing zeroes. 1 is 10, 2 is 100, etc.

		//This loop adds 0s to the front of our number string.
		while (loopCounter > 0){
			if (m_nCounter < powerOfTen){	//if number counter is greater then our power of ten (for 1 zero, if counter < 10)...
				numberString = std::string("0") + numberString;	//add a zero to the front of it
			}

			std::sqrt(powerOfTen);	//sqrt the power of ten (if starting from 2 zeroes, convert from 100 to 10)
			loopCounter -= 1;
		}

		outputString = utlString + numberString;
		m_GetString.Set(castable_string_t(outputString.c_str()), inputData.pActivator, inputData.pCaller);
		//DevMsg("GetString called, HasNumber is on, value is ");
		//DevMsg(outputString.c_str());
		//DevMsg("\n");
	} else {
		m_GetString.Set(castable_string_t(utlString.c_str()), inputData.pActivator, inputData.pCaller);
		//DevMsg("GetString called, HasNumber is off, value is ");
		//DevMsg(utlString.c_str());
		//DevMsg("\n");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set our string to the given input string. Overwrites old string!
//			We also search for the control character '\' and use special syntax
//			to add quotes, since inputs don't support control characters.
//-----------------------------------------------------------------------------
void CLogicStringEntity::InputSetString(inputdata_t &inputData)
{
	const char *inputString = inputData.value.String();
	utlString = std::string(inputString);

	//DevMsg("SetString called, string is ");
	//DevMsg(utlString.c_str());
	//DevMsg("\n");

	FindControlCharacters();	//parse out control characters in our string.
}

//-----------------------------------------------------------------------------
// Purpose: Find all the control characters in our current string,
//			And replace them. Only supports "\quote" for now.
//-----------------------------------------------------------------------------
void CLogicStringEntity::FindControlCharacters(){
	//find our special escape sequence "\quote", and replace it with a quotation mark.
	//the console and I/O system need special handling around quotation marks.
	//The loop will continue to replace until nothing needs replacing.
	bool loopControl = true;

	while (loopControl){
		loopControl = FindAndReplace(std::string("/quote"), std::string("\""));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Find the first instance of 'toFind' in the string, then 
//			replace it with 'replacement'. 
//-----------------------------------------------------------------------------
bool CLogicStringEntity::FindAndReplace(std::string toFind, std::string Replacement){

	//if string is empty, get out of here! This could cause undefined behaviour!
	if (utlString.empty()){
		return false;
	}

	size_t pos = utlString.find(toFind);

	if (pos == std::string::npos) {
		//DevMsg("string ");
		//DevMsg(toFind.c_str());
		//DevMsg(" was not found\n");

		return false;
	} else {
		utlString.replace(pos, toFind.length(), Replacement);

		//DevMsg("string ");
		//DevMsg(Replacement.c_str());
		//DevMsg(" added at ");
		//DevMsg(std::to_string(pos).c_str());
		//DevMsg("\n");

		//DevMsg("String is now ");
		//DevMsg(utlString.c_str());
		//DevMsg("\n");

		return true;
	}
}



//-----------------------------------------------------------------------------
// Purpose: Concatenates our string with the input string.
//-----------------------------------------------------------------------------
void CLogicStringEntity::InputAddToString(inputdata_t &inputData)
{
	utlString = utlString + inputData.value.String();

	FindControlCharacters();	//parse out control characters in our string.
}

//-----------------------------------------------------------------------------
// Purpose: Concatenates our string with the input string, from the front instead of the back.
//-----------------------------------------------------------------------------
void CLogicStringEntity::InputAddToFrontOfString(inputdata_t &inputData)
{
	utlString = inputData.value.String() + utlString;

	FindControlCharacters();	//parse out control characters in our string.
}

//-----------------------------------------------------------------------------
// Purpose: Concatenates our string with the input string.
//-----------------------------------------------------------------------------
void CLogicStringEntity::InputAddSpaceToString(inputdata_t &inputData)
{
	utlString = utlString + " ";

	FindControlCharacters();	//parse out control characters in our string.
}

//-----------------------------------------------------------------------------
// Purpose: Concatenates our string with the input string, from the front instead of the back.
//-----------------------------------------------------------------------------
void CLogicStringEntity::InputAddSpaceToFrontOfString(inputdata_t &inputData)
{
	utlString = " " + utlString;

	FindControlCharacters();	//parse out control characters in our string.
}

//-----------------------------------------------------------------------------
// Purpose: Adds 1 to counter number.
//-----------------------------------------------------------------------------
void CLogicStringEntity::InputIncrementNumber(inputdata_t &inputData)
{
	m_nCounter += 1;
}

//-----------------------------------------------------------------------------
// Purpose: Adds given value to the counter number.
//-----------------------------------------------------------------------------
void CLogicStringEntity::InputAddToNumber(inputdata_t &inputData)
{
	m_nCounter += inputData.value.Int();
}

//-----------------------------------------------------------------------------
// Purpose: Adds a number on the end of the string.
//-----------------------------------------------------------------------------
void CLogicStringEntity::InputGiveStringNumber(inputdata_t &inputData)
{
	m_HasNumber = true;
}

//-----------------------------------------------------------------------------
// Purpose: Removes number from the end of our string.
//-----------------------------------------------------------------------------
void CLogicStringEntity::InputRemoveStringNumber(inputdata_t &inputData)
{
	m_HasNumber = false;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the counter value.
//-----------------------------------------------------------------------------
void CLogicStringEntity::InputSetNumber(inputdata_t &inputData)
{
	m_nCounter = inputData.value.Int();
}

//-----------------------------------------------------------------------------
// Purpose: Find all instances of first string and replace with 
//			second string. Strings are split up via ~.
//-----------------------------------------------------------------------------
void CLogicStringEntity::InputFindReplace(inputdata_t &inputData)
{
	std::string input = std::string(inputData.value.String());
	
	if (input.empty()){
		DevMsg("Empty String!");
		return;
	}

	std::istringstream inputStream(input);

	std::string toFind;

	std::getline(inputStream, toFind, '%');//popuate toFind with first substring

	DevMsg("toFind is ");
	DevMsg(toFind.c_str());

	std::string toReplace;

	std::getline(inputStream, toReplace, '%');//popuate toReplace with second substring

	DevMsg("toReplace is ");
	DevMsg(toReplace.c_str());

	size_t pos = toReplace.find(toFind);

	if (pos == std::string::npos) {
		//We want to not find toReplace in toFind
	}
	else {
		DevMsg("FindReplace failed! ");
		DevMsg(toFind.c_str());
		DevMsg(" is inside of ");
		DevMsg(toReplace.c_str());
		DevMsg("\n");
		return;
	}

	//The loop will continue to replace until nothing needs replacing.
	bool loopControl = true;

	while (loopControl){
		loopControl = FindAndReplace(toFind, toReplace);
	}

	/*
	std::vector<std::string> strings;
	std::istringstream f("denmark;sweden;india;us");
	std::string s;
	while (std::getline(f, s, ';')) {
	DevMsg(s.c_str());
	DevMsg("\n");
	strings.push_back(s);
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: Sets the amount of trailing zeroes our appended number should have.
//-----------------------------------------------------------------------------
void CLogicStringEntity::InputSetTrailingZeroes(inputdata_t &inputData)
{
	m_nTrailingZeroes = inputData.value.Int();
}

//-----------------------------------------------------------------------------
// Purpose: Replaces control characters in the string.
//-----------------------------------------------------------------------------
void CLogicStringEntity::InputReplaceControlCharacters(inputdata_t &inputData)
{
	FindControlCharacters();
}
