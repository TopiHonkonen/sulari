*** Settings ***
Library	SerialLibrary

*** Variables ***
${com}	/dev/ttyACM0
${board}	nRF5340

*** Test Cases ***
Connect Serial
	Log To Console  Connecting to ${com} ${board}
	Add Port  ${com}  baudrate=115200   encoding=ascii
	Port Should Be Open  ${com}
	Reset Input Buffer
	Reset Output Buffer

Serial Led Control
	#Boundary checks
	Write Data	000001X	encoding=ascii
	${read} =	Read Until	terminator=58	encoding=ascii
	Should Be Equal As Strings	${read}	1X
	Write Data	0000-1X	encoding=ascii
	${read} =	Read Until	terminator=58	encoding=ascii
	Should Be Equal As Strings	${read}	-2X

	Write Data	000100X	encoding=ascii
	${read} =	Read Until	terminator=58	encoding=ascii
	Should Be Equal As Strings	${read}	60X
	Write Data	00-100X	encoding=ascii
	${read} =	Read Until	terminator=58	encoding=ascii
	Should Be Equal As Strings	${read}	-2X

	Write Data	010000X	encoding=ascii
	${read} =	Read Until	terminator=58	encoding=ascii
	Should Be Equal As Strings	${read}	3600X
	Write Data	-10000X	encoding=ascii
	${read} =	Read Until	terminator=58	encoding=ascii
	Should Be Equal As Strings	${read}	-2X

	Write Data	000059X	encoding=ascii
	${read} =	Read Until	terminator=58	encoding=ascii
	Should Be Equal As Strings	${read}	59X
	Write Data	000060X	encoding=ascii
	${read} =	Read Until	terminator=58	encoding=ascii
	Should Be Equal As Strings	${read}	-2X

	Write Data	005900X	encoding=ascii
	${read} =	Read Until	terminator=58	encoding=ascii
	Should Be Equal As Strings	${read}	3540X
	Write Data	006000X	encoding=ascii
	${read} =	Read Until	terminator=58	encoding=ascii
	Should Be Equal As Strings	${read}	-2X

	Write Data	230000X	encoding=ascii
	${read} =	Read Until	terminator=58	encoding=ascii
	Should Be Equal As Strings	${read}	82800X
	Write Data	240000X	encoding=ascii
	${read} =	Read Until	terminator=58	encoding=ascii
	Should Be Equal As Strings	${read}	-2X

Disconnect Serial
	Log To Console  Disconnecting ${board}
	[TearDown]  Delete Port  ${com}
